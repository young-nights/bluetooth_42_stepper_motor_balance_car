package com.myitem.car_ctrl_app;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import java.util.Locale;
import java.util.Objects;

/**
 * Attitude calibration activity.
 * Displays real-time Roll / Pitch / Yaw Euler angles polled from the MCU.
 * Provides calibration command buttons: gyroscope, six-side, and mechanical midpoint.
 */
public class CalibrationActivity extends AppCompatActivity {

    private BluetoothService bluetoothService;

    // Attitude value TextViews
    private TextView tvRollValue;
    private TextView tvPitchValue;
    private TextView tvYawValue;

    // Calibration buttons
    private Button btnGyroCalib;
    private Button btnSixSideCalib;
    private Button btnMechCalib;

    // Polling
    private final Handler mainHandler = new Handler(Looper.getMainLooper());
    private Runnable pollingRunnable;
    private static final long POLL_INTERVAL_MS = 1000; // 1 second
    private boolean isPolling = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_calibration);

        // Setup Toolbar
        Toolbar toolbar = findViewById(R.id.toolbar_calibration);
        setSupportActionBar(toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle("姿态校准");

        bluetoothService = BluetoothService.getInstance(this);

        initViews();
        setupCalibrationButtons();

        // Register callback for Euler angle data
        bluetoothService.setCallback(new BluetoothService.ConnectionCallback() {
            @Override
            public void onConnected(String deviceName) {
                // Start polling when connected
                startPolling();
            }

            @Override
            public void onDisconnected() {
                runOnUiThread(() -> {
                    stopPolling();
                    Toast.makeText(CalibrationActivity.this, "Bluetooth disconnected", Toast.LENGTH_SHORT).show();
                });
            }

            @Override
            public void onDataReceived(byte type, byte cmd, byte[] data) {
                handleReceivedData(type, cmd, data);
            }

            @Override
            public void onConnectionFailed(String error) {
                // Ignore in this activity
            }
        });

        // Start polling immediately if already connected
        if (bluetoothService.isConnected()) {
            startPolling();
        }
    }

    private void initViews() {
        tvRollValue = findViewById(R.id.rollValue);
        tvPitchValue = findViewById(R.id.pitchValue);
        tvYawValue = findViewById(R.id.yawValue);

        btnGyroCalib = findViewById(R.id.btn_gyro_calib);
        btnSixSideCalib = findViewById(R.id.btn_six_side_calib);
        btnMechCalib = findViewById(R.id.btn_mech_calib);
    }

    // ===================== Calibration Buttons =====================

    private void setupCalibrationButtons() {
        btnGyroCalib.setOnClickListener(v -> {
            if (!checkConnected()) return;
            bluetoothService.sendControlCommand(BluetoothService.CMD_GYRO_CALIB);
            Toast.makeText(this, "Gyroscope calibration sent", Toast.LENGTH_SHORT).show();
        });

        btnSixSideCalib.setOnClickListener(v -> {
            if (!checkConnected()) return;
            bluetoothService.sendControlCommand(BluetoothService.CMD_SIX_SIDE_CALIB);
            Toast.makeText(this, "Six-side calibration sent", Toast.LENGTH_SHORT).show();
        });

        btnMechCalib.setOnClickListener(v -> {
            if (!checkConnected()) return;
            bluetoothService.sendControlCommand(BluetoothService.CMD_MECH_CALIB);
            Toast.makeText(this, "Mechanical midpoint calibration sent", Toast.LENGTH_SHORT).show();
        });
    }

    // ===================== Polling =====================

    private void startPolling() {
        if (isPolling) return;
        isPolling = true;

        pollingRunnable = new Runnable() {
            @Override
            public void run() {
                if (bluetoothService.isConnected() && isPolling) {
                    bluetoothService.sendQueryCommand(BluetoothService.QUERY_EULER);
                    mainHandler.postDelayed(this, POLL_INTERVAL_MS);
                }
            }
        };
        mainHandler.post(pollingRunnable);
    }

    private void stopPolling() {
        isPolling = false;
        if (pollingRunnable != null) {
            mainHandler.removeCallbacks(pollingRunnable);
        }
    }

    // ===================== Data Handling =====================

    /**
     * Parse received frame data.
     * Expected response for QUERY_EULER: TYPE_POST, cmd=0xA1, data=[roll(4B), pitch(4B), yaw(4B)] as floats.
     */
    private void handleReceivedData(byte type, byte cmd, byte[] data) {
        // Response to Euler angle query
        if (type == BluetoothService.TYPE_POST && cmd == BluetoothService.QUERY_EULER) {
            if (data != null && data.length >= 12) {
                try {
                    float roll = bytesToFloat(data, 0);
                    float pitch = bytesToFloat(data, 4);
                    float yaw = bytesToFloat(data, 8);
                    runOnUiThread(() -> updateAttitudeDisplay(roll, pitch, yaw));
                } catch (Exception e) {
                    // Data format mismatch, ignore
                }
            }
        }

        // Car ready notification
        if (type == BluetoothService.TYPE_ACT && cmd == BluetoothService.REPORT_CAR_READY) {
            runOnUiThread(() -> Toast.makeText(CalibrationActivity.this,
                    "Car is ready!", Toast.LENGTH_SHORT).show());
        }
    }

    private void updateAttitudeDisplay(float roll, float pitch, float yaw) {
        String rollStr = String.format(Locale.US, "%.2f°", roll);
        String pitchStr = String.format(Locale.US, "%.2f°", pitch);
        String yawStr = String.format(Locale.US, "%.2f°", yaw);

        // Fade-out → update → fade-in for smooth value transitions
        animateValueChange(tvRollValue, rollStr);
        animateValueChange(tvPitchValue, pitchStr);
        animateValueChange(tvYawValue, yawStr);
    }

    /**
     * Animate a value TextView change with fade transition.
     * Skips animation if the value hasn't changed.
     */
    private void animateValueChange(TextView textView, String newText) {
        CharSequence current = textView.getText();
        if (current != null && current.toString().equals(newText)) return;
        textView.animate()
                .alpha(0.2f)
                .setDuration(120)
                .withEndAction(() -> {
                    textView.setText(newText);
                    textView.animate().alpha(1.0f).setDuration(180).start();
                })
                .start();
    }

    /**
     * Convert 4 bytes (big-endian) to a float value.
     */
    private static float bytesToFloat(byte[] data, int offset) {
        int bits = ((data[offset] & 0xFF) << 24)
                 | ((data[offset + 1] & 0xFF) << 16)
                 | ((data[offset + 2] & 0xFF) << 8)
                 | (data[offset + 3] & 0xFF);
        return Float.intBitsToFloat(bits);
    }

    // ===================== Helpers =====================

    private boolean checkConnected() {
        if (!bluetoothService.isConnected()) {
            Toast.makeText(this, "Bluetooth not connected. Please connect in Settings first.", Toast.LENGTH_SHORT).show();
            return false;
        }
        return true;
    }

    // ===================== Lifecycle =====================

    @Override
    protected void onPause() {
        super.onPause();
        stopPolling();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (bluetoothService.isConnected() && !isPolling) {
            startPolling();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopPolling();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
