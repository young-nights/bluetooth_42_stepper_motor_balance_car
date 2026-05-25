/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-05-25     whites       PID auto-tuning with coordinate descent
 */
#ifndef APPLICATIONS_MACAPP_INC_RTT_PID_AUTOTUNE_H_
#define APPLICATIONS_MACAPP_INC_RTT_PID_AUTOTUNE_H_

#include "macSYS.h"

/* Auto-tuning state enumeration */
typedef enum {
    TUNE_IDLE      = 0,   /* Not active */
    TUNE_INIT      = 1,   /* Initialize and set starting PID values */
    TUNE_RUNNING   = 2,   /* Actively collecting data and iterating */
    TUNE_PAUSE     = 3,   /* Recovery pause between phases */
    TUNE_CONVERGE  = 4,   /* A single optimization phase converged */
    TUNE_DONE      = 5,   /* All rounds complete */
    TUNE_ABORT     = 6    /* Halted due to safety trigger */
} tune_state_t;

/* Auto-tuning runtime data structure */
typedef struct {
    tune_state_t state;           /* Current tuning state */
    float        best_kp;         /* Best proportional gain found */
    float        best_kd;         /* Best derivative gain found */
    float        best_cost;       /* Lowest cost value (J) */
    float        current_kp;      /* Current trial Kp */
    float        current_kd;      /* Current trial Kd */
    int32_t      round;           /* Round counter (1..3) */
    int32_t      phase;           /* 0 = optimize Kp, 1 = optimize Kd */
    int32_t      direction;       /* 0 = not set, 1 = forward, -1 = backward */
    int32_t      sample_count;    /* Accumulated samples in current phase */
    float        angle_err_sum_sq;/* Sum of squared angle errors */
    float        gyro_sum_sq;     /* Sum of squared gyro readings */
    rt_tick_t    pause_until;     /* Tick when pause ends (0 = not paused) */
    uint8_t      save_to_flash;   /* 1 = save result to Flash when done */
} tune_data_t;

extern tune_data_t tune;

/* ---- Public API ---- */

/** Initialise the auto-tuning module (called once at boot). */
void autotune_init(void);

/** Begin a tuning session. Sets state to TUNE_INIT. */
void autotune_start(void);

/** Immediately abort tuning and restore best-known parameters. */
void autotune_stop(void);

/**
 * Per-cycle update called from the 5 ms control loop.
 * @param angle_err  Current pitch angle error (degrees)
 * @param gyro       Current gyro reading (deg/s after unit conversion)
 * @return 1 if tuning is still active, 0 otherwise
 */
int  autotune_update(float angle_err, float gyro);

/** Reset PID parameters to safe defaults (Kp=8.0, Kd=3.0). */
void autotune_load_default(void);

/** Persist best Kp/Kd to internal Flash. */
void autotune_save_to_flash(void);

/** Load Kp/Kd from Flash (validates magic + CRC). */
void autotune_load_from_flash(void);

/* Flash storage address (STM32F103RCT6, sector 126) */
#define TUNE_FLASH_ADDR    0x0801F800
#define TUNE_FLASH_MAGIC   0x5A5A

/* Auto-tuning algorithm constants */
#define AUTOTUNE_SAMPLE_COUNT   400         /* Samples per phase (2 s @ 5 ms) */
#define AUTOTUNE_COST_LAMBDA    0.15f       /* Gyro weight in cost function */
#define AUTOTUNE_KP_MIN         4.0f        /* Kp lower bound */
#define AUTOTUNE_KP_MAX         25.0f       /* Kp upper bound */
#define AUTOTUNE_KD_MIN         1.0f        /* Kd lower bound */
#define AUTOTUNE_KD_MAX         8.0f        /* Kd upper bound */
#define AUTOTUNE_KP_STEP        2.0f        /* Kp search step size */
#define AUTOTUNE_KD_STEP        1.0f        /* Kd search step size */
#define AUTOTUNE_SAFE_KP        8.0f        /* Safe default Kp */
#define AUTOTUNE_SAFE_KD        3.0f        /* Safe default Kd */

#endif /* APPLICATIONS_MACAPP_INC_RTT_PID_AUTOTUNE_H_ */
