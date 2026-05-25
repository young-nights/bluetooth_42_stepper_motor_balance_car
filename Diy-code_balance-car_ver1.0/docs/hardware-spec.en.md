# 42 Stepper Motor Balance Car — Hardware Specification

> **Project**: Bluetooth 42 Stepper Motor Balance Car
>
> **Version**: Ver 1.0
>
> **Document Date**: 2025-05-25

---

## 1. MCU

| Parameter | Value |
|-----------|-------|
| Model | STM32F103RCT6 |
| Core | ARM Cortex-M3 |
| Clock | 72 MHz |
| Flash | 256 KB |
| SRAM | 48 KB |
| Package | LQFP64 |

---

## 2. IMU (Inertial Measurement Unit)

| Parameter | Value |
|-----------|-------|
| Model | MPU6050 (InvenSense) |
| Interface | I2C (bit-banged / hardware) |
| Attitude Estimation | Complementary filter |
| Filter Coefficient α | 0.96 (gyro weight 96%, accelerometer 4%) |
| Accelerometer Range | ±2g |
| Gyroscope Range | ±2000°/s |
| Attitude Sample Rate | 100 Hz (10 ms period) |
| Pitch Resolution | 0.01° |
| Mechanical Midpoint | 0.36° |

---

## 3. Motors

| Parameter | Value |
|-----------|-------|
| Model | 42 Stepper Motor A0022 |
| Phase | 2-phase |
| Step Angle | 1.8° (200 steps/rev) |
| Rated Voltage | 3.0V |
| Rated Current | 1.5A / phase |
| Phase Resistance | 2.0Ω (@20°C) |
| Phase Inductance | 5.6mH (@1kHz) |
| Holding Torque | 0.40 N·m |
| Max Speed | 800 RPM |
| Rotor Inertia | 38 g·cm² |
| Weight | 286.3g |
| Shaft Diameter | 5mm |
| Frame Size | NEMA17 (42×42mm) |

---

## 4. Motor Drivers

| Parameter | Value |
|-----------|-------|
| Model | DRV8825 (Texas Instruments) |
| Mode | 16× microstepping + mixed decay |
| Max Drive Current | 2.5A / phase (peak) |
| PWM Output Channel | Left: TIM3_CH3 (PB0); Right: TIM3_CH4 (PB1) |
| PWM Frequency | 20 kHz |
| Enable Pin | General-purpose GPIO |
| Direction Control | STEP/DIR mode |

---

## 5. Wheels

| Parameter | Value |
|-----------|-------|
| Diameter | 65mm |
| Circumference | 204mm (π × 65mm) |
| Material | Rubber tire |

---

## 6. Control System

| Parameter | Value |
|-----------|-------|
| Control Architecture | Vertical PD + Speed PI cascade control |
| Control Period | 5ms (200 Hz) |
| Attitude Update Period | 10ms (100 Hz), MPU6050 read rate |
| Thread Priority | 20 |
| Thread Stack Size | 4096 bytes |
| OS | RT-Thread v4.x |

---

## 7. PID Parameters

| Loop | Parameter | Current Value | Notes |
|------|-----------|---------------|-------|
| **Vertical (Balance)** | Kp | 15.0 | Proportional gain |
| | Ki | 0 | Integral term disabled |
| | Kd | 4.0 | Derivative damping (suppresses HF oscillation) |
| | Output Limit | ±1300 | Duty cycle limit |
| **Speed Loop** | Kp | 0 (disabled) | Accelerometer integration too noisy |
| | Ki | 0 (disabled) | Not yet enabled; pending encoder solution |

> **Note**: The speed loop is currently disabled (Kp=0, Ki=0). Speed estimation is based on MPU6050
> accelerometer integration, which accumulates significant drift noise in a stepper motor system
> without encoders. This noise interferes with vertical loop tuning.
> Re-enable the speed loop after installing wheel encoders.

---

## 8. Bluetooth Communication

| Parameter | Value |
|-----------|-------|
| Module Interface | UART2 (115200bps, 8N1) |
| Bluetooth Mode | SPP (Serial Port Profile) |
| UUID | 00001101-0000-1000-8000-00805F9B34FB |
| Device Name | BalanceCar |
| Protocol | Custom frame format (Header 0x55AA / CRC16 Modbus) |

---

## 9. Power

| Parameter | Value |
|-----------|-------|
| Battery Type | Li-Po / Li-Ion pack |
| Motor Supply | 12V (direct via DRV8825) |
| MCU Supply | 3.3V (LDO regulated) |
| MPU6050 Supply | 3.3V |

---

## 10. Mechanical

| Parameter | Value |
|-----------|-------|
| Wheelbase | ~150mm |
| Center of Gravity Height | ~50mm (near battery bottom) |
| Total Weight (incl. battery) | ~800g |

---

*Document maintained by: Balance Car Project Team | Last updated: 2025-05-25*
