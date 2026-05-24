package com.myitem.car_ctrl_app;

import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Color;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.view.GravityCompat;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.drawerlayout.widget.DrawerLayout;

import com.google.android.material.navigation.NavigationView;

import java.util.Locale;
import java.util.Objects;

/**
 * Main activity with navigation drawer.
 * Displays real-time car data: speed, distance, and attitude angles.
 * Receives data through BluetoothService callback.
 */
public class MainActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener {

    private BluetoothService bluetoothService;

    // Drawer
    private DrawerLayout drawerLayout;
    private ActionBarDrawerToggle drawerToggle;

    // Data display TextViews
    private TextView tvRealSpeed;
    private TextView tvNormalSpeed;
    private TextView tvDistance;
    private TextView tvPitch;
    private TextView tvYaw;
    private TextView tvRoll;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);

        bluetoothService = BluetoothService.getInstance(this);

        // Initialize Toolbar
        Toolbar toolbarHome = findViewById(R.id.toolbar_home);
        setSupportActionBar(toolbarHome);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setHomeAsUpIndicator(R.drawable.ic_menu);

        // Initialize navigation drawer
        drawerLayout = findViewById(R.id.drawer_layout);
        NavigationView navigationView = findViewById(R.id.nav_view);

        drawerToggle = new ActionBarDrawerToggle(
                this, drawerLayout, toolbarHome,
                R.string.navigation_drawer_open,
                R.string.navigation_drawer_close
        );
        drawerLayout.addDrawerListener(drawerToggle);

        // Animated scrim: semi-transparent overlay with custom color
        drawerLayout.setScrimColor(Color.argb(160, 0, 0, 0));

        drawerToggle.syncState();
        navigationView.setNavigationItemSelectedListener(this);
        // Highlight default selected item (Home)
        navigationView.setCheckedItem(R.id.nav_home);

        // Set up window insets
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            androidx.core.graphics.Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        // Bind data display TextViews
        tvRealSpeed = findViewById(R.id.textView15);
        tvNormalSpeed = findViewById(R.id.textView25);
        tvDistance = findViewById(R.id.textView23);
        tvPitch = findViewById(R.id.textView24);
        tvYaw = findViewById(R.id.textView27);
        tvRoll = findViewById(R.id.textView26);

        registerServiceCallback();
        updateToolbarTitle();
    }

    // ===================== Navigation =====================

    @Override
    public boolean onNavigationItemSelected(@NonNull MenuItem item) {
        final int id = item.getItemId();
        item.setChecked(true);
        drawerLayout.closeDrawer(GravityCompat.START);

        Intent intent = null;
        if (id == R.id.nav_calibration) {
            intent = new Intent(MainActivity.this, CalibrationActivity.class);
        } else if (id == R.id.nav_connection) {
            intent = new Intent(MainActivity.this, SettingsActivity.class);
        } else if (id == R.id.nav_control) {
            intent = new Intent(MainActivity.this, ControlActivity.class);
        }

        if (intent != null) {
            startActivity(intent);
        }

        return true;
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        drawerToggle.syncState();
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        drawerToggle.onConfigurationChanged(newConfig);
    }

    // ===================== Callback Registration =====================

    /**
     * Register the BluetoothService callback for connection state and data updates.
     */
    private void registerServiceCallback() {
        bluetoothService.setCallback(new BluetoothService.ConnectionCallback() {
            @Override
            public void onConnected(String deviceName) {
                runOnUiThread(() -> {
                    getSupportActionBar().setTitle("● Connected: " + deviceName);
                    Toast.makeText(MainActivity.this, "Connected to " + deviceName, Toast.LENGTH_SHORT).show();
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
    private Runnable speedPollRunnable;
    private boolean isPollingData = false;

    private void startDataPolling() {
        if (isPollingData) return;
        isPollingData = true;

        speedPollRunnable = new Runnable() {
            @Override
            public void run() {
                if (bluetoothService.isConnected() && isPollingData) {
                    bluetoothService.sendQueryCommand(BluetoothService.QUERY_SPEED);
                    bluetoothService.sendQueryCommand(BluetoothService.QUERY_EULER);
                    pollingHandler.postDelayed(this, 1000);
                }
            }
        };
        pollingHandler.post(speedPollRunnable);
    }

    private void stopDataPolling() {
        isPollingData = false;
        if (speedPollRunnable != null) {
            pollingHandler.removeCallbacks(speedPollRunnable);
        }
    }

    // ===================== Data Handling =====================

    /**
     * Parse received frame data and update the display.
     */
    private void handleReceivedData(byte type, byte cmd, byte[] data) {
        if (data == null) return;

        // Response to speed query
        if (type == BluetoothService.TYPE_POST && cmd == BluetoothService.QUERY_SPEED) {
            if (data.length >= 2) {
                int speed = ((data[0] & 0xFF) << 8) | (data[1] & 0xFF);
                if ((speed & 0x8000) != 0) speed -= 0x10000;
                final int finalSpeed = speed;
                runOnUiThread(() -> animateTextView(tvRealSpeed,
                        getString(R.string.RealSpeed_name) + finalSpeed + " cm/s"));
            }
        }

        // Response to Euler angle query
        if (type == BluetoothService.TYPE_POST && cmd == BluetoothService.QUERY_EULER) {
            if (data.length >= 12) {
                try {
                    float roll = bytesToFloat(data, 0);
                    float pitch = bytesToFloat(data, 4);
                    float yaw = bytesToFloat(data, 8);
                    runOnUiThread(() -> {
                        animateTextView(tvRoll, getString(R.string.Roll_name) + String.format(Locale.US, "%.2f°", roll));
                        animateTextView(tvPitch, getString(R.string.Pitch_name) + String.format(Locale.US, "%.2f°", pitch));
                        animateTextView(tvYaw, getString(R.string.Yaw_name) + String.format(Locale.US, "%.2f°", yaw));
                    });
                } catch (Exception ignored) {}
            }
        }

        // Car ready notification
        if (type == BluetoothService.TYPE_ACT && cmd == BluetoothService.REPORT_CAR_READY) {
            runOnUiThread(() -> Toast.makeText(MainActivity.this, "Car is ready!", Toast.LENGTH_SHORT).show());
        }
    }

    private static float bytesToFloat(byte[] data, int offset) {
        int bits = ((data[offset] & 0xFF) << 24)
                 | ((data[offset + 1] & 0xFF) << 16)
                 | ((data[offset + 2] & 0xFF) << 8)
                 | (data[offset + 3] & 0xFF);
        return Float.intBitsToFloat(bits);
    }

    /**
     * Animate a TextView text change with a quick fade-out/in transition.
     * Skips animation if the text hasn't actually changed.
     */
    private void animateTextView(TextView textView, String newText) {
        CharSequence current = textView.getText();
        if (current != null && current.toString().equals(newText)) return;
        textView.animate()
                .alpha(0.3f)
                .setDuration(100)
                .withEndAction(() -> {
                    textView.setText(newText);
                    textView.animate().alpha(1.0f).setDuration(150).start();
                })
                .start();
    }

    private void resetDisplay() {
        tvRealSpeed.setText(getString(R.string.RealSpeed_name) + "-- cm/s");
        tvNormalSpeed.setText(getString(R.string.NormalSpeed_name) + "-- cm/s");
        tvDistance.setText(getString(R.string.Distant_name) + "-- cm");
        tvRoll.setText(getString(R.string.Roll_name) + "--°");
        tvPitch.setText(getString(R.string.Pitch_name) + "--°");
        tvYaw.setText(getString(R.string.Yaw_name) + "--°");
    }

    private void updateToolbarTitle() {
        if (bluetoothService.isConnected()) {
            getSupportActionBar().setTitle("● Connected");
        } else {
            getSupportActionBar().setTitle("○ Disconnected");
        }
    }

    // ===================== Lifecycle =====================

    @Override
    protected void onResume() {
        super.onResume();
        registerServiceCallback();
        updateToolbarTitle();
        if (bluetoothService.isConnected() && !isPollingData) {
            startDataPolling();
        }
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
}
