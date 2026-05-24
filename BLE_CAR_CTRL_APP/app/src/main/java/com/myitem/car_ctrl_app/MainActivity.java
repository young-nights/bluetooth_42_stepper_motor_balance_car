package com.myitem.car_ctrl_app;

import android.os.Bundle;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import java.util.Locale;
import java.util.Objects;

/**
 * Data panel activity: displays real-time car data (speed, attitude angles).
 * Receives data through BluetoothService callback.
 */
public class MainActivity extends AppCompatActivity {

    private BluetoothService bluetoothService;

    // Data display TextViews
    private TextView tvRealSpeed, tvNormalSpeed, tvDistance;
    private TextView tvPitch, tvRoll, tvYaw;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        bluetoothService = BluetoothService.getInstance(this);

        // Setup Toolbar
        Toolbar toolbarHome = findViewById(R.id.toolbar_home);
        setSupportActionBar(toolbarHome);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle("数据面板");

        // Bind data display TextViews
        tvRealSpeed = findViewById(R.id.tv_real_speed_value);
        tvNormalSpeed = findViewById(R.id.tv_normal_speed_value);
        tvDistance = findViewById(R.id.tv_distance_value);
        tvPitch = findViewById(R.id.tv_pitch_value);
        tvRoll = findViewById(R.id.tv_roll_value);
        tvYaw = findViewById(R.id.tv_yaw_value);

        registerServiceCallback();
        updateToolbarTitle();
    }

    // ===================== Callback =====================

    private void registerServiceCallback() {
        bluetoothService.setCallback(new BluetoothService.ConnectionCallback() {
            @Override
            public void onConnected(String deviceName) {
                runOnUiThread(() -> {
                    getSupportActionBar().setTitle("● " + deviceName);
                    startDataPolling();
                });
            }

            @Override
            public void onDisconnected() {
                runOnUiThread(() -> {
                    getSupportActionBar().setTitle("○ Disconnected");
                    stopDataPolling();
                    resetDisplay();
                });
            }

            @Override
            public void onDataReceived(byte type, byte cmd, byte[] data) {
                handleReceivedData(type, cmd, data);
            }

            @Override
            public void onConnectionFailed(String error) {
                runOnUiThread(() -> getSupportActionBar().setTitle("○ Disconnected"));
            }
        });
    }

    // ===================== Data Polling =====================

    private final android.os.Handler pollingHandler = new android.os.Handler(android.os.Looper.getMainLooper());
    private Runnable pollRunnable;
    private boolean isPollingData = false;

    private void startDataPolling() {
        if (isPollingData) return;
        isPollingData = true;
        pollRunnable = new Runnable() {
            @Override
            public void run() {
                if (bluetoothService.isConnected() && isPollingData) {
                    bluetoothService.sendQueryCommand(BluetoothService.QUERY_SPEED);
                    bluetoothService.sendQueryCommand(BluetoothService.QUERY_EULER);
                    pollingHandler.postDelayed(this, 1000);
                }
            }
        };
        pollingHandler.post(pollRunnable);
    }

    private void stopDataPolling() {
        isPollingData = false;
        if (pollRunnable != null) pollingHandler.removeCallbacks(pollRunnable);
    }

    // ===================== Data Handling =====================

    private void handleReceivedData(byte type, byte cmd, byte[] data) {
        if (data == null) return;

        if (type == BluetoothService.TYPE_POST && cmd == BluetoothService.QUERY_SPEED && data.length >= 2) {
            int speed = ((data[0] & 0xFF) << 8) | (data[1] & 0xFF);
            if ((speed & 0x8000) != 0) speed -= 0x10000;
            final int fs = speed;
            runOnUiThread(() -> animateTextView(tvRealSpeed, fs + " cm/s"));
        }

        if (type == BluetoothService.TYPE_POST && cmd == BluetoothService.QUERY_EULER && data.length >= 12) {
            try {
                float roll = bytesToFloat(data, 0);
                float pitch = bytesToFloat(data, 4);
                float yaw = bytesToFloat(data, 8);
                runOnUiThread(() -> {
                    animateTextView(tvPitch, String.format(Locale.US, "%.2f°", pitch));
                    animateTextView(tvRoll, String.format(Locale.US, "%.2f°", roll));
                    animateTextView(tvYaw, String.format(Locale.US, "%.2f°", yaw));
                });
            } catch (Exception ignored) {}
        }

        if (type == BluetoothService.TYPE_ACT && cmd == BluetoothService.REPORT_CAR_READY) {
            runOnUiThread(() -> Toast.makeText(MainActivity.this, R.string.car_ready, Toast.LENGTH_SHORT).show());
        }
    }

    private static float bytesToFloat(byte[] data, int offset) {
        int bits = ((data[offset] & 0xFF) << 24) | ((data[offset + 1] & 0xFF) << 16)
                 | ((data[offset + 2] & 0xFF) << 8) | (data[offset + 3] & 0xFF);
        return Float.intBitsToFloat(bits);
    }

    private void animateTextView(TextView textView, String newText) {
        CharSequence current = textView.getText();
        if (current != null && current.toString().equals(newText)) return;
        textView.animate().alpha(0.3f).setDuration(100)
                .withEndAction(() -> {
                    textView.setText(newText);
                    textView.animate().alpha(1.0f).setDuration(150).start();
                }).start();
    }

    private void resetDisplay() {
        tvRealSpeed.setText("-- cm/s");
        tvNormalSpeed.setText("-- cm/s");
        tvDistance.setText("-- cm");
        tvPitch.setText("--°");
        tvRoll.setText("--°");
        tvYaw.setText("--°");
    }

    private void updateToolbarTitle() {
        getSupportActionBar().setTitle(bluetoothService.isConnected() ? "● Connected" : "○ Disconnected");
    }

    // ===================== Lifecycle =====================

    @Override
    protected void onResume() {
        super.onResume();
        registerServiceCallback();
        updateToolbarTitle();
        if (bluetoothService.isConnected() && !isPollingData) startDataPolling();
    }

    @Override
    protected void onPause() {
        super.onPause();
        stopDataPolling();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopDataPolling();
        bluetoothService.disconnect();
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
