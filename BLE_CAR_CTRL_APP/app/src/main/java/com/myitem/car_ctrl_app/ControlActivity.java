package com.myitem.car_ctrl_app;

import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Color;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.animation.OvershootInterpolator;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.view.GravityCompat;
import androidx.drawerlayout.widget.DrawerLayout;

import com.google.android.material.navigation.NavigationView;

import java.util.Objects;

/**
 * Home page: car control with navigation drawer.
 * Direction buttons use onTouchListener for jog control (press=ON, release=OFF).
 * ToggleButtons enable continuous modes (straight / backward / upright).
 */
public class ControlActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener {

    private BluetoothService bluetoothService;
    private int currentSpeed = 50;

    // Drawer
    private DrawerLayout drawerLayout;
    private ActionBarDrawerToggle drawerToggle;

    // Direction buttons
    private Button btnForward, btnBackward, btnLeft, btnRight;
    // ToggleButtons
    private ToggleButton toggleStraight, toggleBackward, toggleUpright;
    // Speed control
    private TextView tvAccelValue, tvDecelValue;
    private ImageButton btnAccelPlus, btnAccelMinus, btnDecelPlus, btnDecelMinus;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_control);

        bluetoothService = BluetoothService.getInstance(this);

        // Setup Toolbar
        Toolbar toolbar = findViewById(R.id.toolbar_control);
        setSupportActionBar(toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setHomeAsUpIndicator(R.drawable.ic_menu);
        getSupportActionBar().setTitle("小车控制");

        // Initialize navigation drawer
        drawerLayout = findViewById(R.id.drawer_layout_control);
        NavigationView navigationView = findViewById(R.id.nav_view_control);

        drawerToggle = new ActionBarDrawerToggle(
                this, drawerLayout, toolbar,
                R.string.navigation_drawer_open,
                R.string.navigation_drawer_close
        );
        drawerLayout.addDrawerListener(drawerToggle);
        drawerLayout.setScrimColor(Color.argb(160, 0, 0, 0));
        drawerToggle.syncState();
        navigationView.setNavigationItemSelectedListener(this);
        navigationView.setCheckedItem(R.id.nav_home);

        initViews();

        // Defer listener setup until after first frame
        getWindow().getDecorView().post(() -> {
            setupDirectionButtons();
            setupToggleButtons();
            setupSpeedControls();
        });
    }

    // ===================== Navigation Drawer =====================

    @Override
    public boolean onNavigationItemSelected(@NonNull MenuItem item) {
        final int id = item.getItemId();
        item.setChecked(true);
        drawerLayout.closeDrawer(GravityCompat.START);

        Intent intent = null;
        if (id == R.id.nav_data) {
            intent = new Intent(this, MainActivity.class);
        } else if (id == R.id.nav_calibration) {
            intent = new Intent(this, CalibrationActivity.class);
        } else if (id == R.id.nav_connection) {
            intent = new Intent(this, SettingsActivity.class);
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

    // ===================== Views =====================

    private void initViews() {
        btnForward = findViewById(R.id.forward_button);
        btnBackward = findViewById(R.id.down_button);
        btnLeft = findViewById(R.id.left_button);
        btnRight = findViewById(R.id.right_button);

        toggleStraight = findViewById(R.id.toggleButton_goStraight);
        toggleBackward = findViewById(R.id.toggleButton2_back);
        toggleUpright = findViewById(R.id.toggleButton3_upRight);

        tvAccelValue = findViewById(R.id.tv_default_plus_value);
        tvDecelValue = findViewById(R.id.tv_default_minus_value);
        btnAccelPlus = findViewById(R.id.btnPlus_accelerate);
        btnAccelMinus = findViewById(R.id.btnMinus_accelerate);
        btnDecelPlus = findViewById(R.id.btnPlus_decelerate);
        btnDecelMinus = findViewById(R.id.btnMinus_decelerate);
    }

    // ===================== Direction Buttons =====================

    private void setupDirectionButtons() {
        setupDirectionButton(btnForward, BluetoothService.CMD_FORWARD);
        setupDirectionButton(btnBackward, BluetoothService.CMD_BACKWARD);
        setupDirectionButton(btnLeft, BluetoothService.CMD_TURN_LEFT);
        setupDirectionButton(btnRight, BluetoothService.CMD_TURN_RIGHT);
    }

    private void setupDirectionButton(Button button, byte subCmd) {
        button.setOnTouchListener((v, event) -> {
            if (!checkConnected()) return false;
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    v.animate().scaleX(0.85f).scaleY(0.85f).alpha(0.7f).setDuration(100).start();
                    bluetoothService.sendControlCommand(subCmd, (byte) 1);
                    return true;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    v.animate().scaleX(1.0f).scaleY(1.0f).alpha(1.0f)
                            .setDuration(250).setInterpolator(new OvershootInterpolator()).start();
                    bluetoothService.sendControlCommand(subCmd, (byte) 0);
                    return true;
            }
            return false;
        });
    }

    // ===================== ToggleButtons =====================

    private void setupToggleButtons() {
        toggleStraight.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (!checkConnected()) { buttonView.setChecked(!isChecked); return; }
            animateToggleBounce(buttonView);
            bluetoothService.sendControlCommand(BluetoothService.CMD_FORWARD, (byte) (isChecked ? 1 : 0));
        });
        toggleBackward.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (!checkConnected()) { buttonView.setChecked(!isChecked); return; }
            animateToggleBounce(buttonView);
            bluetoothService.sendControlCommand(BluetoothService.CMD_BACKWARD, (byte) (isChecked ? 1 : 0));
        });
        toggleUpright.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (!checkConnected()) { buttonView.setChecked(!isChecked); return; }
            animateToggleBounce(buttonView);
            bluetoothService.sendControlCommand(BluetoothService.CMD_UPRIGHT, (byte) (isChecked ? 1 : 0));
        });
    }

    private void animateToggleBounce(View view) {
        view.animate().scaleX(0.9f).scaleY(0.9f).setDuration(80)
                .withEndAction(() -> view.animate().scaleX(1.0f).scaleY(1.0f)
                        .setDuration(300).setInterpolator(new OvershootInterpolator(1.2f)).start())
                .start();
    }

    // ===================== Speed Controls =====================

    private void setupSpeedControls() {
        btnAccelPlus.setForeground(getDrawable(R.drawable.btn_control_ripple));
        btnAccelMinus.setForeground(getDrawable(R.drawable.btn_control_ripple));
        btnDecelPlus.setForeground(getDrawable(R.drawable.btn_control_ripple));
        btnDecelMinus.setForeground(getDrawable(R.drawable.btn_control_ripple));

        btnAccelPlus.setOnClickListener(v -> { currentSpeed = Math.min(currentSpeed + 5, 100); animateSpeedValue(tvAccelValue, String.valueOf(currentSpeed)); sendSpeedCommand(); });
        btnAccelMinus.setOnClickListener(v -> { currentSpeed = Math.max(currentSpeed - 5, 0); animateSpeedValue(tvAccelValue, String.valueOf(currentSpeed)); sendSpeedCommand(); });
        btnDecelPlus.setOnClickListener(v -> { int s = parseSpeed(tvDecelValue); s = Math.min(s + 5, 100); animateSpeedValue(tvDecelValue, String.valueOf(s)); sendSpeedCommand(); });
        btnDecelMinus.setOnClickListener(v -> { int s = parseSpeed(tvDecelValue); s = Math.max(s - 5, 0); animateSpeedValue(tvDecelValue, String.valueOf(s)); sendSpeedCommand(); });
    }

    private void animateSpeedValue(TextView tv, String value) {
        tv.setText(value);
        tv.animate().scaleX(1.3f).scaleY(1.3f).setDuration(80)
                .withEndAction(() -> tv.animate().scaleX(1.0f).scaleY(1.0f)
                        .setDuration(150).setInterpolator(new OvershootInterpolator()).start())
                .start();
    }

    private void sendSpeedCommand() {
        if (!checkConnected()) return;
        short s = (short) currentSpeed;
        bluetoothService.sendControlCommandWithData(BluetoothService.CMD_SET_SPEED,
                new byte[]{(byte) (s >> 8), (byte) s});
    }

    private int parseSpeed(TextView tv) {
        try { return Integer.parseInt(tv.getText().toString().trim()); }
        catch (NumberFormatException e) { return 50; }
    }

    // ===================== Helpers =====================

    private boolean checkConnected() {
        if (!bluetoothService.isConnected()) {
            Toast.makeText(this, R.string.bluetooth_not_connected, Toast.LENGTH_SHORT).show();
            return false;
        }
        return true;
    }

    // ===================== Lifecycle =====================

    @Override
    protected void onPause() {
        super.onPause();
        if (bluetoothService.isConnected()) {
            bluetoothService.sendControlCommand(BluetoothService.CMD_FORWARD, (byte) 0);
            bluetoothService.sendControlCommand(BluetoothService.CMD_BACKWARD, (byte) 0);
            bluetoothService.sendControlCommand(BluetoothService.CMD_TURN_LEFT, (byte) 0);
            bluetoothService.sendControlCommand(BluetoothService.CMD_TURN_RIGHT, (byte) 0);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            if (drawerLayout.isDrawerOpen(GravityCompat.START)) {
                drawerLayout.closeDrawer(GravityCompat.START);
            }
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
