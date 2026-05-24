package com.myitem.car_ctrl_app;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Set;

/**
 * Settings / Bluetooth connection page.
 * Handles runtime permission requests, device scanning, connection, and disconnection.
 */
public class SettingsActivity extends AppCompatActivity {

    private BluetoothService bluetoothService;
    private DeviceAdapter deviceAdapter;
    private final List<BluetoothDevice> deviceList = new ArrayList<>();

    private Button btnScan;
    private Button btnDisconnect;
    private ProgressBar progressScan;
    private TextView tvConnectionStatus;
    private TextView tvNoDevices;

    private final ActivityResultLauncher<String[]> permissionLauncher =
            registerForActivityResult(new ActivityResultContracts.RequestMultiplePermissions(), result -> {
                boolean allGranted = true;
                for (Boolean granted : result.values()) {
                    if (granted == null || !granted) {
                        allGranted = false;
                        break;
                    }
                }
                if (allGranted) {
                    performScan();
                } else {
                    Toast.makeText(this, "Bluetooth permissions are required to scan for devices", Toast.LENGTH_LONG).show();
                }
            });

    private final ActivityResultLauncher<Intent> enableBtLauncher =
            registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), result -> {
                if (result.getResultCode() == RESULT_OK) {
                    checkPermissionsAndScan();
                } else {
                    Toast.makeText(this, "Bluetooth must be enabled", Toast.LENGTH_SHORT).show();
                }
            });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        // Setup Toolbar
        Toolbar toolbar = findViewById(R.id.toolbar_settings);
        setSupportActionBar(toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle("高级设置");

        bluetoothService = BluetoothService.getInstance(this);

        // Bind UI
        RecyclerView recyclerDevices = findViewById(R.id.recycler_devices);
        btnScan = findViewById(R.id.btn_scan);
        btnDisconnect = findViewById(R.id.btn_disconnect);
        progressScan = findViewById(R.id.progress_scan);
        tvConnectionStatus = findViewById(R.id.tv_connection_status);
        tvNoDevices = findViewById(R.id.tv_no_devices);

        // Setup RecyclerView
        recyclerDevices.setLayoutManager(new LinearLayoutManager(this));
        deviceAdapter = new DeviceAdapter(deviceList, this::onDeviceClicked);
        recyclerDevices.setAdapter(deviceAdapter);

        // Scan button
        btnScan.setOnClickListener(v -> {
            if (!bluetoothService.isBluetoothEnabled()) {
                requestEnableBluetooth();
            } else {
                checkPermissionsAndScan();
            }
        });

        // Disconnect button
        btnDisconnect.setOnClickListener(v -> {
            bluetoothService.disconnect();
            updateConnectionUI(false);
        });

        // Initial state check
        updateConnectionUI(bluetoothService.isConnected());

        // Register BluetoothService callback
        bluetoothService.setCallback(new BluetoothService.ConnectionCallback() {
            @Override
            public void onConnected(String deviceName) {
                runOnUiThread(() -> {
                    updateConnectionUI(true);
                    Toast.makeText(SettingsActivity.this, "Connected to " + deviceName, Toast.LENGTH_SHORT).show();
                });
            }

            @Override
            public void onDisconnected() {
                runOnUiThread(() -> {
                    updateConnectionUI(false);
                    Toast.makeText(SettingsActivity.this, "Disconnected", Toast.LENGTH_SHORT).show();
                });
            }

            @Override
            public void onDataReceived(byte type, byte cmd, byte[] data) {
                // Handled by other activities; ignore here
            }

            @Override
            public void onConnectionFailed(String error) {
                runOnUiThread(() -> {
                    updateConnectionUI(false);
                    Toast.makeText(SettingsActivity.this, error, Toast.LENGTH_LONG).show();
                });
            }
        });

        // Load paired devices
        loadPairedDevices();
    }

    // ===================== Permissions =====================

    private void checkPermissionsAndScan() {
        List<String> neededPermissions = getRequiredPermissions();

        if (neededPermissions.isEmpty()) {
            performScan();
        } else {
            permissionLauncher.launch(neededPermissions.toArray(new String[0]));
        }
    }

    private List<String> getRequiredPermissions() {
        List<String> perms = new ArrayList<>();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN)
                    != PackageManager.PERMISSION_GRANTED) {
                perms.add(Manifest.permission.BLUETOOTH_SCAN);
            }
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT)
                    != PackageManager.PERMISSION_GRANTED) {
                perms.add(Manifest.permission.BLUETOOTH_CONNECT);
            }
        }
        // Older Android versions need location permission for scanning
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
                && Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION)
                    != PackageManager.PERMISSION_GRANTED) {
                perms.add(Manifest.permission.ACCESS_FINE_LOCATION);
            }
        }
        return perms;
    }

    // ===================== Bluetooth Enable =====================

    private void requestEnableBluetooth() {
        Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        enableBtLauncher.launch(enableBtIntent);
    }

    // ===================== Scanning =====================

    private void performScan() {
        deviceList.clear();
        loadPairedDevices();
        setScanningUI(true);

        bluetoothService.startScan(new BluetoothService.ScanCallback() {
            @Override
            public void onDeviceDiscovered(BluetoothDevice device) {
                runOnUiThread(() -> {
                    int idx = findDeviceIndex(device.getAddress());
                    if (idx < 0) {
                        deviceList.add(device);
                    } else {
                        deviceList.set(idx, device); // update
                    }
                    deviceAdapter.notifyDataSetChanged();
                    tvNoDevices.setVisibility(View.GONE);
                });
            }

            @Override
            public void onScanFinished() {
                runOnUiThread(() -> {
                    setScanningUI(false);
                    if (deviceList.isEmpty()) {
                        tvNoDevices.setVisibility(View.VISIBLE);
                    }
                });
            }
        });
    }

    private int findDeviceIndex(String address) {
        for (int i = 0; i < deviceList.size(); i++) {
            if (deviceList.get(i).getAddress().equals(address)) {
                return i;
            }
        }
        return -1;
    }

    private void loadPairedDevices() {
        BluetoothAdapter adapter = bluetoothService.getBluetoothAdapter();
        if (adapter == null) return;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT)
                    != PackageManager.PERMISSION_GRANTED) {
                return;
            }
        }
        Set<BluetoothDevice> paired = adapter.getBondedDevices();
        if (paired != null) {
            for (BluetoothDevice d : paired) {
                if (findDeviceIndex(d.getAddress()) < 0) {
                    deviceList.add(d);
                }
            }
            deviceAdapter.notifyDataSetChanged();
            if (!deviceList.isEmpty()) {
                tvNoDevices.setVisibility(View.GONE);
            }
        }
    }

    // ===================== Connection =====================

    private void onDeviceClicked(BluetoothDevice device) {
        if (bluetoothService.isScanning()) {
            bluetoothService.stopScan();
            setScanningUI(false);
        }
        Toast.makeText(this, "Connecting to " + device.getName() + "...", Toast.LENGTH_SHORT).show();
        tvConnectionStatus.setText("Connecting...");
        tvConnectionStatus.setVisibility(View.VISIBLE);
        bluetoothService.connect(device);
    }

    // ===================== UI Helpers =====================

    private void setScanningUI(boolean scanning) {
        if (scanning) {
            progressScan.setAlpha(0f);
            progressScan.setVisibility(View.VISIBLE);
            progressScan.animate().alpha(1f).setDuration(200).start();
        } else {
            progressScan.animate().alpha(0f).setDuration(150)
                    .withEndAction(() -> progressScan.setVisibility(View.GONE)).start();
        }
        btnScan.setEnabled(!scanning);
        btnScan.setText(scanning ? "Scanning..." : "Scan Devices");
    }

    private void updateConnectionUI(boolean connected) {
        // Fade transition on connection status text
        tvConnectionStatus.animate().alpha(0f).setDuration(120)
                .withEndAction(() -> {
                    tvConnectionStatus.setText(connected ? "● Connected" : "○ Disconnected");
                    tvConnectionStatus.animate().alpha(1f).setDuration(180).start();
                }).start();

        // Cross-fade between scan and disconnect buttons
        if (connected) {
            btnScan.animate().alpha(0f).setDuration(150)
                    .withEndAction(() -> btnScan.setVisibility(View.GONE)).start();
            btnDisconnect.setAlpha(0f);
            btnDisconnect.setVisibility(View.VISIBLE);
            btnDisconnect.animate().alpha(1f).setDuration(200).start();
            btnDisconnect.setEnabled(true);
        } else {
            btnDisconnect.animate().alpha(0f).setDuration(150)
                    .withEndAction(() -> btnDisconnect.setVisibility(View.GONE)).start();
            btnScan.setAlpha(0f);
            btnScan.setVisibility(View.VISIBLE);
            btnScan.animate().alpha(1f).setDuration(200).start();
        }
    }

    // ===================== Lifecycle =====================

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bluetoothService.stopScan();
        // Do NOT disconnect on destroy; connection should persist across activities
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    // ===================== RecyclerView Adapter =====================

    private static class DeviceAdapter extends RecyclerView.Adapter<DeviceAdapter.ViewHolder> {
        private final List<BluetoothDevice> devices;
        private final OnDeviceClickListener listener;

        interface OnDeviceClickListener {
            void onClick(BluetoothDevice device);
        }

        DeviceAdapter(List<BluetoothDevice> devices, OnDeviceClickListener listener) {
            this.devices = devices;
            this.listener = listener;
        }

        @NonNull
        @Override
        public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.item_device, parent, false);
            return new ViewHolder(view);
        }

        @Override
        public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
            BluetoothDevice device = devices.get(position);
            String name = device.getName();
            holder.tvDeviceName.setText(name != null && !name.isEmpty() ? name : "Unknown Device");
            holder.tvDeviceAddress.setText(device.getAddress());
            holder.itemView.setOnClickListener(v -> listener.onClick(device));
        }

        @Override
        public int getItemCount() {
            return devices.size();
        }

        static class ViewHolder extends RecyclerView.ViewHolder {
            TextView tvDeviceName;
            TextView tvDeviceAddress;

            ViewHolder(View itemView) {
                super(itemView);
                tvDeviceName = itemView.findViewById(R.id.tv_device_name);
                tvDeviceAddress = itemView.findViewById(R.id.tv_device_address);
            }
        }
    }
}
