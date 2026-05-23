/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-05-23     whites       the first version
 */
#ifndef APPLICATIONS_MACSYS_INC_MACCONFIG_H_
#define APPLICATIONS_MACSYS_INC_MACCONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Motor & Mechanical Constants */
#define MOTOR_PULSE_PER_REV     3200        /* Pulses per revolution (1.8 deg step, 16x microstep) */
#define TIRE_DIAMETER_MM        65          /* Tire diameter in mm */
#define TIRE_CIRCUMFERENCE_MM   204         /* Tire circumference in mm (PI * 65 ≈ 204) */
#define MECHANICAL_MEDIAN_DEG   0.36f       /* Mechanical median angle (balance equilibrium point) */
#define PID_DT_SEC              0.005f      /* PID control period (5ms = 200Hz) */
#define SPEED_TARGET_VALUE      50.0f       /* Speed target value for forward/backward movement */
#define TURN_BIAS               200         /* Differential steering bias for left/right turns */

/* Filter & Dead Zone */
#define VELOCITY_DEAD_ZONE      5.0f        /* Velocity dead zone threshold */
#define ACCEL_DEAD_ZONE         0.05f       /* Acceleration dead zone threshold */
#define MVF_WINDOW_SIZE         4           /* Moving average filter window size */

/* Sensor & Control Timing */
#define MPU_SAMPLE_RATE_HZ      100         /* MPU6050 sample rate in Hz */
#define BALANCE_FREQ_HZ         200         /* Balance control loop frequency in Hz */

/* Protocol */
#define CMD_MAX_LENGTH          60          /* Maximum command length for buffer overflow protection */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* APPLICATIONS_MACSYS_INC_MACCONFIG_H_ */
