package com.myitem.car_ctrl_app;

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

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import java.util.Objects;

/**
 * Car control activity.
 * Direction buttons use onTouchListener for jog control (press=ON, release=OFF).
 * ToggleButtons enable continuous modes (straight / backward / upright).
 * Plus/minus buttons adjust and send target speed via CMD_SET_SPEED.
 */
public class ControlActivity extends AppCompatActivity {

    private BluetoothService bluetoothService;
    private int currentSpeed = 50; // Default speed value

    // Direction buttons
    private Button btnForward;
    private Button btnBackward;
    private Button btnLeft;
    private Button btnRight;

    // ToggleButtons
    private ToggleButton toggleStraight;
    private ToggleButton toggleBackward;
    private ToggleButton toggleUpright;

    // Speed control
    private TextView tvAccelValue;
    private TextView tvDecelValue;
    private ImageButton btnAccelPlus;
    private ImageButton btnAccelMinus;
    private ImageButton btnDecelPlus;
    private ImageButton btnDecelMinus;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_control);

        // Setup Toolbar
        Toolbar toolbar = findViewById(R.id.toolbar_control);
        setSupportActionBar(toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle("小车控制");

        bluetoothService = BluetoothService.getInstance(this);

        initViews();
        setupDirectionButtons();
        setupToggleButtons();
        setupSpeedControls();
    }

    private void initViews() {
        // Direction buttons
        btnForward = findViewById(R.id.forward_button);
        btnBackward = findViewById(R.id.down_button);
        btnLeft = findViewById(R.id.left_button);
        btnRight = findViewById(R.id.right_button);

        // ToggleButtons
        toggleStraight = findViewById(R.id.toggleButton_goStraight);
        toggleBackward = findViewById(R.id.toggleButton2_back);
        toggleUpright = findViewById(R.id.toggleButton3_upRight);

        // Speed controls
        tvAccelValue = findViewById(R.id.tv_default_plus_value);
        tvDecelValue = findViewById(R.id.tv_default_minus_value);
        btnAccelPlus = findViewById(R.id.btnPlus_accelerate);
        btnAccelMinus = findViewById(R.id.btnMinus_accelerate);
        btnDecelPlus = findViewById(R.id.btnPlus_decelerate);
        btnDecelMinus = findViewById(R.id.btnMinus_decelerate);
    }

    // ===================== Direction Buttons (Jog Control) =====================

    private void setupDirectionButtons() {
        // Forward: press sends forward ON, release sends forward OFF
        setupDirectionButton(btnForward, BluetoothService.CMD_FORWARD);

        // Backward
        setupDirectionButton(btnBackward, BluetoothService.CMD_BACKWARD);

        // Left
        setupDirectionButton(btnLeft, BluetoothService.CMD_TURN_LEFT);

        // Right
        setupDirectionButton(btnRight, BluetoothService.CMD_TURN_RIGHT);
    }

    private void setupDirectionButton(Button button, byte subCmd) {
        button.setOnTouchListener((v, event) -> {
            if (!checkConnected()) return false;
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    // Scale down + fade on press
                    v.animate().scaleX(0.85f).scaleY(0.85f).alpha(0.7f)
                            .setDuration(100).start();
                    bluetoothService.sendControlCommand(subCmd, (byte) 1);
                    return true;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    // Spring back with overshoot on release
                    v.animate().scaleX(1.0f).scaleY(1.0f).alpha(1.0f)
                            .setDuration(250)
                            .setInterpolator(new OvershootInterpolator())
                            .start();
                    bluetoothService.sendControlCommand(subCmd, (byte) 0);
                    return true;
            }
            return false;
        });
    }

    // ===================== ToggleButtons (Continuous Mode) =====================

    private void setupToggleButtons() {
        // Straight toggle
        toggleStraight.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (!checkConnected()) {
                buttonView.setChecked(!isChecked);
                return;
            }
            animateToggleBounce(buttonView);
            bluetoothService.sendControlCommand(BluetoothService.CMD_FORWARD, (byte) (isChecked ? 1 : 0));
        });

        // Backward toggle
        toggleBackward.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (!checkConnected()) {
                buttonView.setChecked(!isChecked);
                return;
            }
            animateToggleBounce(buttonView);
            bluetoothService.sendControlCommand(BluetoothService.CMD_BACKWARD, (byte) (isChecked ? 1 : 0));
        });

        // Upright toggle (balance enable)
        toggleUpright.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (!checkConnected()) {
                buttonView.setChecked(!isChecked);
                return;
            }
            animateToggleBounce(buttonView);
            bluetoothService.sendControlCommand(BluetoothService.CMD_UPRIGHT, (byte) (isChecked ? 1 : 0));
        });
    }

    /**
     * Elastic bounce animation for toggle button state changes.
     */
    private void animateToggleBounce(View view) {
        view.animate()
                .scaleX(0.9f).scaleY(0.9f)
                .setDuration(80)
                .withEndAction(() -> view.animate()
                        .scaleX(1.0f).scaleY(1.0f)
                        .setDuration(300)
                        .setInterpolator(new OvershootInterpolator(1.2f))
                        .start())
                .start();
    }

    // ===================== Speed Controls =====================

    private void setupSpeedControls() {
        // Apply ripple effect to all speed buttons
        btnAccelPlus.setForeground(getDrawable(R.drawable.btn_control_ripple));
        btnAccelMinus.setForeground(getDrawable(R.drawable.btn_control_ripple));
        btnDecelPlus.setForeground(getDrawable(R.drawable.btn_control_ripple));
        btnDecelMinus.setForeground(getDrawable(R.drawable.btn_control_ripple));

        // Accelerate + (increase target speed)
        btnAccelPlus.setOnClickListener(v -> {
            currentSpeed = Math.min(currentSpeed + 5, 100);
            animateSpeedValue(tvAccelValue, String.valueOf(currentSpeed));
            sendSpeedCommand();
        });

        // Accelerate - (decrease target speed)
        btnAccelMinus.setOnClickListener(v -> {
            currentSpeed = Math.max(currentSpeed - 5, 0);
            animateSpeedValue(tvAccelValue, String.valueOf(currentSpeed));
            sendSpeedCommand();
        });

        // Decelerate + (increase deceleration target speed)
        btnDecelPlus.setOnClickListener(v -> {
            int decelSpeed = parseSpeed(tvDecelValue);
            decelSpeed = Math.min(decelSpeed + 5, 100);
            animateSpeedValue(tvDecelValue, String.valueOf(decelSpeed));
            sendSpeedCommand();
        });

        // Decelerate -
        btnDecelMinus.setOnClickListener(v -> {
            int decelSpeed = parseSpeed(tvDecelValue);
            decelSpeed = Math.max(decelSpeed - 5, 0);
            animateSpeedValue(tvDecelValue, String.valueOf(decelSpeed));
            sendSpeedCommand();
        });
    }

    /**
     * Update speed display with a quick scale-pulse animation.
     */
    private void animateSpeedValue(TextView tv, String value) {
        tv.setText(value);
        tv.animate()
                .scaleX(1.3f).scaleY(1.3f)
                .setDuration(80)
                .withEndAction(() -> tv.animate()
                        .scaleX(1.0f).scaleY(1.0f)
                        .setDuration(150)
                        .setInterpolator(new OvershootInterpolator())
                        .start())
                .start();
    }

    /**
     * Send the current speed value via CMD_SET_SPEED.
     * Speed is sent as a 2-byte signed short in big-endian format.
     */
    private void sendSpeedCommand() {
        if (!checkConnected()) return;
        short speedShort = (short) currentSpeed;
        byte[] speedBytes = new byte[] {
                (byte) ((speedShort >> 8) & 0xFF),  // High byte
                (byte) (speedShort & 0xFF)           // Low byte
        };
        bluetoothService.sendControlCommandWithData(BluetoothService.CMD_SET_SPEED, speedBytes);
    }

    private int parseSpeed(TextView tv) {
        try {
            return Integer.parseInt(tv.getText().toString().trim());
        } catch (NumberFormatException e) {
            return 50;
        }
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
        // Release all active controls when leaving the page to prevent stuck commands
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
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
