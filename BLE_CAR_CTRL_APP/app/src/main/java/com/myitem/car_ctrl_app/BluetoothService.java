package com.myitem.car_ctrl_app;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * Singleton Bluetooth service manager for the balance car.
 * Handles device scanning, connection, data transmission, and frame parsing
 * using the proprietary STM32 communication protocol.
 */
public class BluetoothService {

    private static final String TAG = "BluetoothService";
    private static final UUID SPP_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    // --- Frame protocol constants ---
    public static final byte FRAME_HEADER_1 = (byte) 0x55;
    public static final byte FRAME_HEADER_2 = (byte) 0xAA;
    public static final byte DEVICE_ID_H = 0x00;
    public static final byte DEVICE_ID_L = 0x73;

    /** Frame type: control command (APP → MCU) */
    public static final byte TYPE_SET = 0x31;
    /** Frame type: active report (MCU → APP) */
    public static final byte TYPE_ACT = 0x32;
    /** Frame type: query command (APP → MCU) */
    public static final byte TYPE_GET = 0x33;
    /** Frame type: response / post status (MCU → APP) */
    public static final byte TYPE_POST = 0x66;

    /** Status: acknowledge (ACK) */
    public static final byte STATUS_ACK = 0x01;
    /** Status: request (ASK) */
    public static final byte STATUS_ASK = 0x02;
    /** Status: error (ERR) */
    public static final byte STATUS_ERR = 0x00;

    // --- Sub-command codes ---
    /** Control: forward */
    public static final byte CMD_FORWARD = 0x10;
    /** Control: backward */
    public static final byte CMD_BACKWARD = 0x11;
    /** Control: turn left */
    public static final byte CMD_TURN_LEFT = 0x12;
    /** Control: turn right */
    public static final byte CMD_TURN_RIGHT = 0x13;
    /** Control: upright / start balance */
    public static final byte CMD_UPRIGHT = 0x14;
    /** Control: stop / lie down */
    public static final byte CMD_STOP = 0x15;
    /** Control: set target speed */
    public static final byte CMD_SET_SPEED = 0x16;
    /** Calibration: six-side calibration */
    public static final byte CMD_SIX_SIDE_CALIB = 0x17;
    /** Calibration: gyroscope static calibration */
    public static final byte CMD_GYRO_CALIB = 0x1E;
    /** Calibration: mechanical midpoint calibration */
    public static final byte CMD_MECH_CALIB = 0x21;

    /** Query: get speed */
    public static final byte QUERY_SPEED = (byte) 0xA0;
    /** Query: get Euler angles (three axes) */
    public static final byte QUERY_EULER = (byte) 0xA1;

    /** Active report: car ready */
    public static final byte REPORT_CAR_READY = (byte) 0xA2;

    // --- Ring buffer for frame parsing ---
    private static final int RING_BUFFER_SIZE = 4096;
    private final byte[] ringBuffer = new byte[RING_BUFFER_SIZE];
    private int ringWritePos = 0;
    private int ringReadPos = 0;
    private int ringAvailable = 0;
    private final Object ringLock = new Object();

    // --- Singleton ---
    private static volatile BluetoothService instance;
    private final Context appContext;
    private final Handler mainHandler;

    // --- Bluetooth components ---
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket bluetoothSocket;
    private OutputStream outputStream;
    private InputStream inputStream;

    // --- Threads ---
    private Thread connectThread;
    private Thread receiveThread;
    private volatile boolean isReceiving = false;
    private volatile boolean isConnected = false;

    // --- Callback ---
    private ConnectionCallback callback;

    // --- Scan state ---
    private boolean isScanning = false;
    private final List<BluetoothDevice> discoveredDevices = new ArrayList<>();

    /**
     * Callback interface for connection state changes and data reception.
     */
    public interface ConnectionCallback {
        /** Called when a Bluetooth connection is successfully established. */
        void onConnected(String deviceName);
        /** Called when the Bluetooth connection is lost or closed. */
        void onDisconnected();
        /**
         * Called when a complete, valid frame is received from the MCU.
         * @param type   Frame type field (TYPE_ACT / TYPE_POST)
         * @param cmd    Command / status field
         * @param data   Payload data bytes (may be empty)
         */
        void onDataReceived(byte type, byte cmd, byte[] data);
        /** Called when a connection attempt fails. */
        void onConnectionFailed(String error);
    }

    public interface ScanCallback {
        /** Called when a new device is discovered during scanning. */
        void onDeviceDiscovered(BluetoothDevice device);
        /** Called when the scan finishes. */
        void onScanFinished();
    }

    private BluetoothService(Context context) {
        this.appContext = context.getApplicationContext();
        this.mainHandler = new Handler(Looper.getMainLooper());
        this.bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    /**
     * Get or create the singleton BluetoothService instance.
     */
    public static synchronized BluetoothService getInstance(Context context) {
        if (instance == null) {
            instance = new BluetoothService(context);
        }
        return instance;
    }

    // ===================== CRC16 MODBUS =====================

    /**
     * Calculate Modbus CRC-16.
     * Polynomial: 0xA001, initial value: 0xFFFF.
     *
     * @param data   Byte array
     * @param offset Start offset
     * @param length Number of bytes to process
     * @return 16-bit CRC value
     */
    public static int crc16Modbus(byte[] data, int offset, int length) {
        int crc = 0xFFFF;
        for (int i = offset; i < offset + length; i++) {
            crc ^= (data[i] & 0xFF);
            for (int j = 0; j < 8; j++) {
                if ((crc & 0x0001) != 0) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc & 0xFFFF;
    }

    // ===================== FRAME BUILDING =====================

    /**
     * Build a complete protocol frame.
     * Frame format: | 0x55 | 0xAA | Length | DevID_H | DevID_L | Type | Cmd | Data... | CRC16_H | CRC16_L |
     *
     * @param type Frame type (TYPE_SET / TYPE_GET)
     * @param cmd  Status / sub-command byte
     * @param data Payload data (may be null or empty)
     * @return Complete frame bytes ready to send
     */
    public byte[] buildFrame(byte type, byte cmd, byte[] data) {
        int dataLen = (data != null) ? data.length : 0;
        int length = 5 + dataLen; // DevID(2) + Type(1) + Cmd(1) + Data(N)
        int totalLen = 2 + 1 + length + 2; // Header(2) + Length(1) + content + CRC(2)
        byte[] frame = new byte[totalLen];

        frame[0] = FRAME_HEADER_1;
        frame[1] = FRAME_HEADER_2;
        frame[2] = (byte) length;
        frame[3] = DEVICE_ID_H;
        frame[4] = DEVICE_ID_L;
        frame[5] = type;
        frame[6] = cmd;
        if (data != null && dataLen > 0) {
            System.arraycopy(data, 0, frame, 7, dataLen);
        }

        // CRC16 is calculated from the Length byte (inclusive) through the end of the Data field.
        // Total bytes covered = 1 (Length) + length (content) = length + 1.
        int crc = crc16Modbus(frame, 2, length + 1);
        frame[totalLen - 2] = (byte) ((crc >> 8) & 0xFF);   // CRC16_H (big-endian)
        frame[totalLen - 1] = (byte) (crc & 0xFF);           // CRC16_L

        return frame;
    }

    /**
     * Convenience: build and send a control command frame (TYPE_SET, STATUS_ASK).
     * Data = [subCmd] or [subCmd, value].
     */
    public void sendControlCommand(byte subCmd, byte value) {
        byte[] data = new byte[] {subCmd, value};
        sendFrame(buildFrame(TYPE_SET, STATUS_ASK, data));
    }

    /**
     * Convenience: build and send a control command with only a sub-command byte.
     * Used for calibration commands that need no value parameter.
     */
    public void sendControlCommand(byte subCmd) {
        byte[] data = new byte[] {subCmd};
        sendFrame(buildFrame(TYPE_SET, STATUS_ASK, data));
    }

    /**
     * Convenience: build and send a multi-byte data control command.
     * Used for set-speed (CMD_SET_SPEED) which sends a multi-byte speed value.
     */
    public void sendControlCommandWithData(byte subCmd, byte[] payload) {
        byte[] data = new byte[1 + payload.length];
        data[0] = subCmd;
        System.arraycopy(payload, 0, data, 1, payload.length);
        sendFrame(buildFrame(TYPE_SET, STATUS_ASK, data));
    }

    /**
     * Convenience: build and send a query command frame (TYPE_GET, STATUS_ASK).
     */
    public void sendQueryCommand(byte queryCmd) {
        byte[] data = new byte[] {queryCmd};
        sendFrame(buildFrame(TYPE_GET, STATUS_ASK, data));
    }

    /**
     * Write a pre-built frame to the output stream on a background thread.
     */
    private void sendFrame(final byte[] frame) {
        if (!isConnected || outputStream == null) {
            mainHandler.post(() -> {
                if (callback != null) {
                    callback.onConnectionFailed("Not connected to device");
                }
            });
            return;
        }
        new Thread(() -> {
            try {
                synchronized (BluetoothService.this) {
                    outputStream.write(frame);
                    outputStream.flush();
                }
            } catch (IOException e) {
                Log.e(TAG, "sendFrame failed: " + e.getMessage());
                handleDisconnection();
            }
        }).start();
    }

    // ===================== SCANNING =====================

    /**
     * Start Bluetooth device discovery.
     */
    public void startScan(final ScanCallback scanCallback) {
        if (bluetoothAdapter == null) {
            mainHandler.post(() -> scanCallback.onScanFinished());
            return;
        }
        if (isScanning) {
            bluetoothAdapter.cancelDiscovery();
        }

        discoveredDevices.clear();
        isScanning = true;

        // Register broadcast receiver for discovery results
        BroadcastReceiver receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                    BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                    if (device != null && !discoveredDevices.contains(device)) {
                        discoveredDevices.add(device);
                        scanCallback.onDeviceDiscovered(device);
                    }
                } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                    isScanning = false;
                    appContext.unregisterReceiver(this);
                    scanCallback.onScanFinished();
                }
            }
        };

        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothDevice.ACTION_FOUND);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        appContext.registerReceiver(receiver, filter);

        bluetoothAdapter.startDiscovery();
    }

    /**
     * Stop an ongoing scan.
     */
    public void stopScan() {
        if (bluetoothAdapter != null && isScanning) {
            bluetoothAdapter.cancelDiscovery();
            isScanning = false;
        }
    }

    /**
     * Check if a scan is currently in progress.
     */
    public boolean isScanning() {
        return isScanning;
    }

    // ===================== CONNECTION =====================

    /**
     * Connect to a specific Bluetooth device.
     */
    public void connect(final BluetoothDevice device) {
        if (isConnected) {
            disconnect();
        }
        stopScan();

        connectThread = new Thread(() -> {
            try {
                // Cancel discovery as it slows down connection
                if (bluetoothAdapter != null) {
                    bluetoothAdapter.cancelDiscovery();
                }

                // Create RFCOMM socket
                bluetoothSocket = device.createRfcommSocketToServiceRecord(SPP_UUID);
                bluetoothSocket.connect();

                outputStream = bluetoothSocket.getOutputStream();
                inputStream = bluetoothSocket.getInputStream();
                isConnected = true;

                // Start receive thread
                startReceiveThread();

                mainHandler.post(() -> {
                    if (callback != null) {
                        callback.onConnected(device.getName() != null ? device.getName() : device.getAddress());
                    }
                });
            } catch (IOException e) {
                Log.e(TAG, "Connection failed: " + e.getMessage());
                isConnected = false;
                try {
                    if (bluetoothSocket != null) {
                        bluetoothSocket.close();
                    }
                } catch (IOException ignored) {}
                mainHandler.post(() -> {
                    if (callback != null) {
                        callback.onConnectionFailed("Connection failed: " + e.getMessage());
                    }
                });
            }
        });
        connectThread.start();
    }

    /**
     * Disconnect from the current device.
     */
    public void disconnect() {
        isConnected = false;
        isReceiving = false;

        stopScan();

        if (receiveThread != null) {
            receiveThread.interrupt();
            receiveThread = null;
        }

        try {
            if (inputStream != null) {
                inputStream.close();
                inputStream = null;
            }
        } catch (IOException ignored) {}

        try {
            if (outputStream != null) {
                outputStream.close();
                outputStream = null;
            }
        } catch (IOException ignored) {}

        try {
            if (bluetoothSocket != null) {
                bluetoothSocket.close();
                bluetoothSocket = null;
            }
        } catch (IOException ignored) {}

        connectThread = null;
    }

    // ===================== DATA RECEPTION =====================

    /**
     * Start the background thread that reads from the InputStream.
     */
    private void startReceiveThread() {
        isReceiving = true;
        receiveThread = new Thread(() -> {
            byte[] temp = new byte[256];
            while (isReceiving && isConnected) {
                try {
                    int bytesRead = inputStream.read(temp);
                    if (bytesRead > 0) {
                        writeToRingBuffer(temp, bytesRead);
                        parseFramesFromRing();
                    } else if (bytesRead < 0) {
                        // Stream closed
                        break;
                    }
                } catch (IOException e) {
                    if (isReceiving) {
                        Log.e(TAG, "Receive error: " + e.getMessage());
                    }
                    break;
                }
            }
            // If we exit the loop unexpectedly, handle disconnection
            if (isReceiving) {
                handleDisconnection();
            }
        });
        receiveThread.start();
    }

    private void writeToRingBuffer(byte[] data, int len) {
        synchronized (ringLock) {
            for (int i = 0; i < len; i++) {
                ringBuffer[ringWritePos] = data[i];
                ringWritePos = (ringWritePos + 1) % RING_BUFFER_SIZE;
                if (ringAvailable < RING_BUFFER_SIZE) {
                    ringAvailable++;
                } else {
                    // Buffer full, advance read pointer
                    ringReadPos = (ringReadPos + 1) % RING_BUFFER_SIZE;
                }
            }
        }
    }

    /**
     * Attempt to extract complete frames from the ring buffer.
     * Scans for 0x55 0xAA header, reads length, validates CRC.
     */
    private void parseFramesFromRing() {
        synchronized (ringLock) {
            while (ringAvailable >= 5) { // Minimum: header(2) + length(1) + CRC(2) = 5 bytes before data
                // Scan for header
                int headerIdx = -1;
                int scanPos = ringReadPos;
                int scanned = 0;
                while (scanned < ringAvailable - 1) {
                    byte b1 = ringBuffer[scanPos];
                    byte b2 = ringBuffer[(scanPos + 1) % RING_BUFFER_SIZE];
                    if (b1 == FRAME_HEADER_1 && b2 == FRAME_HEADER_2) {
                        headerIdx = scanPos;
                        break;
                    }
                    scanPos = (scanPos + 1) % RING_BUFFER_SIZE;
                    scanned++;
                }
                if (headerIdx < 0) {
                    // No header found, discard all but the last byte
                    if (ringAvailable > 1) {
                        ringReadPos = (ringReadPos + ringAvailable - 1) % RING_BUFFER_SIZE;
                        ringAvailable = 1;
                    }
                    return;
                }

                // Discard bytes before header
                int discarded = (headerIdx - ringReadPos + RING_BUFFER_SIZE) % RING_BUFFER_SIZE;
                ringReadPos = headerIdx;
                ringAvailable -= discarded;

                // Need at least: header(2) + length(1) + 2(CRC) = 5 bytes to find frame size
                if (ringAvailable < 5) return;

                int lengthPos = (ringReadPos + 2) % RING_BUFFER_SIZE;
                int length = ringBuffer[lengthPos] & 0xFF;
                int frameTotalLen = 2 + 1 + length + 2; // header + length_byte + content + crc

                if (length < 5) {
                    // Invalid length, skip header bytes
                    ringReadPos = (ringReadPos + 2) % RING_BUFFER_SIZE;
                    ringAvailable -= 2;
                    continue;
                }

                if (ringAvailable < frameTotalLen) {
                    // Not enough data for complete frame, wait for more
                    return;
                }

                // Extract full frame
                byte[] frame = new byte[frameTotalLen];
                for (int i = 0; i < frameTotalLen; i++) {
                    frame[i] = ringBuffer[(ringReadPos + i) % RING_BUFFER_SIZE];
                }

                // Advance ring buffer past this frame
                ringReadPos = (ringReadPos + frameTotalLen) % RING_BUFFER_SIZE;
                ringAvailable -= frameTotalLen;

                // Validate CRC
                int computedCrc = crc16Modbus(frame, 2, length + 1);
                int receivedCrc = ((frame[frameTotalLen - 2] & 0xFF) << 8) | (frame[frameTotalLen - 1] & 0xFF);

                if (computedCrc != receivedCrc) {
                    Log.w(TAG, "CRC mismatch: computed=0x" + Integer.toHexString(computedCrc)
                            + " received=0x" + Integer.toHexString(receivedCrc));
                    continue; // Skip invalid frame
                }

                // Extract fields
                byte type = frame[5];
                byte cmd = frame[6];
                int dataLen = length - 5; // length = DevID(2) + Type(1) + Cmd(1) + Data(N)
                byte[] data = new byte[Math.max(0, dataLen)];
                if (dataLen > 0) {
                    System.arraycopy(frame, 7, data, 0, dataLen);
                }

                // Notify callback on main thread
                final byte fType = type;
                final byte fCmd = cmd;
                final byte[] fData = data;
                mainHandler.post(() -> {
                    if (callback != null) {
                        callback.onDataReceived(fType, fCmd, fData);
                    }
                });
            }
        }
    }

    private void handleDisconnection() {
        isConnected = false;
        isReceiving = false;
        mainHandler.post(() -> {
            if (callback != null) {
                callback.onDisconnected();
            }
        });
        disconnect();
    }

    // ===================== CALLBACK REGISTRATION =====================

    /**
     * Register the connection callback. Only one callback is active at a time.
     */
    public void setCallback(ConnectionCallback callback) {
        this.callback = callback;
    }

    /**
     * Remove the current callback.
     */
    public void removeCallback() {
        this.callback = null;
    }

    // ===================== STATE ACCESSORS =====================

    public boolean isConnected() {
        return isConnected;
    }

    public BluetoothAdapter getBluetoothAdapter() {
        return bluetoothAdapter;
    }

    public boolean isBluetoothEnabled() {
        return bluetoothAdapter != null && bluetoothAdapter.isEnabled();
    }
}
