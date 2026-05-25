/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-05-25     whites       Coordinate-descent PID auto-tuning
 */
#include "rtt_pid_autotune.h"
#include "macFlash.h"
#include "bsp_math.h"
#include <math.h>

// externs required by the tuning module
extern mac_pid_t carPID;
extern uint8_t If_Car_Was_Picked_Up(void);
extern EulerAngles carEulerAngles;

/* Global tuning instance */
tune_data_t tune;

/* Forward declarations */
static void param_clip(void);
static void apply_to_car(void);

/*----------------------------------------------------------------------------*/
/*  Public API                                                               */
/*----------------------------------------------------------------------------*/

/**
 * @brief  One-time initialisation. Clears the tuning structure and
 *         attempts to load saved parameters from Flash.
 */
void autotune_init(void)
{
    rt_memset(&tune, 0, sizeof(tune));
    tune.state = TUNE_IDLE;
    /* Try loading Flash-stored params on cold boot */
    autotune_load_from_flash();
}

/**
 * @brief  Arm the auto-tuner. The actual parameter search begins on the
 *         next call to autotune_update().
 */
void autotune_start(void)
{
    /* Set safe starting point */
    tune.best_kp    = AUTOTUNE_SAFE_KP;
    tune.best_kd    = AUTOTUNE_SAFE_KD;
    tune.best_cost  = 999999.0f;
    tune.round      = 1;
    tune.phase      = 0;          /* Start by optimising Kp */
    tune.direction  = 0;
    tune.sample_count    = 0;
    tune.angle_err_sum_sq = 0.0f;
    tune.gyro_sum_sq      = 0.0f;
    tune.save_to_flash = 1;
    tune.pause_until = 0;

    /* Apply safe defaults immediately */
    carPID.verticalParam.kp = tune.best_kp;
    carPID.verticalParam.kd = tune.best_kd;

    tune.current_kp = tune.best_kp;
    tune.current_kd = tune.best_kd;

    tune.state = TUNE_INIT;

    rt_kprintf("AUTOTUNE: Started. Init Kp=%.2f Kd=%.2f\n",
               tune.best_kp, tune.best_kd);
}

/**
 * @brief  Abort tuning immediately and restore the best-known parameters.
 */
void autotune_stop(void)
{
    carPID.verticalParam.kp = tune.best_kp;
    carPID.verticalParam.kd = tune.best_kd;
    tune.state = TUNE_IDLE;
    rt_kprintf("AUTOTUNE: Stopped. Restored Kp=%.2f Kd=%.2f\n",
               tune.best_kp, tune.best_kd);
}

/*----------------------------------------------------------------------------*/
/*  Main update routine – called every 5 ms from the balance thread          */
/*----------------------------------------------------------------------------*/

/**
 * @brief  Per-cycle tuning step.
 * @return 1 if the tuner is still active, 0 otherwise.
 */
int autotune_update(float angle_err, float gyro)
{
    float cost, angle_rms, gyro_rms;
    int32_t converged;

    switch (tune.state) {

    case TUNE_IDLE:
    case TUNE_DONE:
    case TUNE_ABORT:
        return 0;

    /* ---- Init: launch the first parameter trial ---- */
    case TUNE_INIT:
        tune.direction  = 0;
        tune.sample_count    = 0;
        tune.angle_err_sum_sq = 0.0f;
        tune.gyro_sum_sq      = 0.0f;

        /* First trial: start from safe Kp, Kd */
        tune.current_kp = tune.best_kp;
        tune.current_kd = tune.best_kd;
        param_clip();
        apply_to_car();

        tune.state = TUNE_RUNNING;
        rt_kprintf("AUTOTUNE: Round %ld Phase %ld – trial Kp=%.2f Kd=%.2f\n",
                   tune.round, tune.phase, tune.current_kp, tune.current_kd);
        return 1;

    /* ---- Pause: wait 500 ms for the car to stabilise ---- */
    case TUNE_PAUSE:
        if (rt_tick_get() >= tune.pause_until) {
            tune.state = TUNE_RUNNING;
            tune.direction = 0;
            tune.sample_count = 0;
            tune.angle_err_sum_sq = 0.0f;
            tune.gyro_sum_sq = 0.0f;
            rt_kprintf("AUTOTUNE: Round %ld Phase %ld – trial Kp=%.2f Kd=%.2f\n",
                       tune.round, tune.phase, tune.current_kp, tune.current_kd);
        }
        return 1;

    /* ---- Running: collect data, evaluate cost, decide next step ---- */
    case TUNE_RUNNING:
        /* ---- Safety check ---- */
        if (fabsf(angle_err) > 35.0f) {
            carPID.verticalParam.kp = tune.best_kp;
            carPID.verticalParam.kd = tune.best_kd;
            tune.state = TUNE_ABORT;
            rt_kprintf("AUTOTUNE: ABORT – angle %.1f > 35°\n", angle_err);
            return 0;
        }
        if (If_Car_Was_Picked_Up()) {
            carPID.verticalParam.kp = tune.best_kp;
            carPID.verticalParam.kd = tune.best_kd;
            tune.state = TUNE_ABORT;
            rt_kprintf("AUTOTUNE: ABORT – car picked up\n");
            return 0;
        }

        /* Accumulate squared errors */
        tune.angle_err_sum_sq += angle_err * angle_err;
        tune.gyro_sum_sq      += gyro * gyro;
        tune.sample_count++;

        /* Not enough data yet */
        if (tune.sample_count < AUTOTUNE_SAMPLE_COUNT)
            return 1;

        /* ---- 2 s of data collected: evaluate cost ---- */
        angle_rms = sqrtf(tune.angle_err_sum_sq / (float)AUTOTUNE_SAMPLE_COUNT);
        gyro_rms  = sqrtf(tune.gyro_sum_sq      / (float)AUTOTUNE_SAMPLE_COUNT);
        cost = angle_rms + AUTOTUNE_COST_LAMBDA * gyro_rms;

        rt_kprintf("AUTOTUNE: Kp=%.2f Kd=%.2f J=%.4f (ar=%.3f gr=%.3f)\n",
                   tune.current_kp, tune.current_kd, cost, angle_rms, gyro_rms);

        /* ---- Decision logic ---- */
        if (cost < tune.best_cost) {
            /* Improvement: accept and continue */
            tune.best_cost = cost;
            if (tune.phase == 0) {
                tune.best_kp = tune.current_kp;
                if (tune.direction == 0) tune.direction = 1;
                tune.current_kp += (float)tune.direction * AUTOTUNE_KP_STEP;
            } else {
                tune.best_kd = tune.current_kd;
                if (tune.direction == 0) tune.direction = 1;
                tune.current_kd += (float)tune.direction * AUTOTUNE_KD_STEP;
            }
            rt_kprintf("  -> Accept. New best Kp=%.2f Kd=%.2f J=%.4f\n",
                       tune.best_kp, tune.best_kd, tune.best_cost);
        } else {
            /* No improvement */
            if (tune.direction == 0) {
                /* First direction failed – try reverse */
                tune.direction = -1;
                if (tune.phase == 0) {
                    tune.current_kp = tune.best_kp - AUTOTUNE_KP_STEP;
                } else {
                    tune.current_kd = tune.best_kd - AUTOTUNE_KD_STEP;
                }
                rt_kprintf("  -> Reject. Trying opposite direction.\n");
            } else if (tune.direction == 1) {
                /* Forward failed – try backward from best */
                tune.direction = -1;
                if (tune.phase == 0) {
                    tune.current_kp = tune.best_kp - AUTOTUNE_KP_STEP;
                } else {
                    tune.current_kd = tune.best_kd - AUTOTUNE_KD_STEP;
                }
                rt_kprintf("  -> Reject. Trying opposite direction from best.\n");
            } else {
                /* Both directions exhausted – this phase converged */
                converged = 1;
                goto phase_converge;
            }
        }

        param_clip();
        apply_to_car();

        /* Reset sample accumulators for next trial */
        tune.sample_count    = 0;
        tune.angle_err_sum_sq = 0.0f;
        tune.gyro_sum_sq      = 0.0f;

        /* Insert 500 ms pause between trials */
        tune.pause_until = rt_tick_get() + rt_tick_from_millisecond(500);
        tune.state = TUNE_PAUSE;
        return 1;
    }

    return 0;

phase_converge:
    rt_kprintf("AUTOTUNE: Phase %ld converged. Best Kp=%.2f Kd=%.2f J=%.4f\n",
               tune.phase, tune.best_kp, tune.best_kd, tune.best_cost);

    /* Switch to next phase or next round */
    if (tune.phase == 0) {
        /* Done with Kp → optimise Kd */
        tune.phase = 1;
        tune.direction = 0;
        tune.current_kp = tune.best_kp;
        tune.current_kd = tune.best_kd + AUTOTUNE_KD_STEP;
    } else {
        /* Both phases done → next round */
        tune.round++;
        if (tune.round <= 3) {
            tune.phase = 0;
            tune.direction = 0;
            tune.current_kp = tune.best_kp + AUTOTUNE_KP_STEP;
            tune.current_kd = tune.best_kd;
        } else {
            /* All 3 rounds complete */
            carPID.verticalParam.kp = tune.best_kp;
            carPID.verticalParam.kd = tune.best_kd;
            tune.state = TUNE_DONE;
            rt_kprintf("AUTOTUNE: DONE after %ld rounds. "
                       "Best Kp=%.2f Kd=%.2f J=%.4f\n",
                       tune.round, tune.best_kp, tune.best_kd, tune.best_cost);
            if (tune.save_to_flash) {
                autotune_save_to_flash();
            }
            return 0;
        }
    }

    param_clip();
    apply_to_car();
    tune.sample_count    = 0;
    tune.angle_err_sum_sq = 0.0f;
    tune.gyro_sum_sq      = 0.0f;
    tune.pause_until = rt_tick_get() + rt_tick_from_millisecond(500);
    tune.state = TUNE_PAUSE;
    return 1;
}

/*----------------------------------------------------------------------------*/
/*  Default / Flash persistence                                              */
/*----------------------------------------------------------------------------*/

void autotune_load_default(void)
{
    tune.best_kp = AUTOTUNE_SAFE_KP;
    tune.best_kd = AUTOTUNE_SAFE_KD;
    carPID.verticalParam.kp = tune.best_kp;
    carPID.verticalParam.kd = tune.best_kd;
    tune.state = TUNE_IDLE;
    rt_kprintf("AUTOTUNE: Loaded defaults Kp=%.2f Kd=%.2f\n",
               tune.best_kp, tune.best_kd);
}

/**
 * @brief  Store best Kp/Kd to Flash.
 * Format: magic(2B) | kp(4B float) | kd(4B float) | crc16(2B) = 12 B
 */
void autotune_save_to_flash(void)
{
    uint16_t crc;
    uint8_t  buf[12];
    uint32_t tmp;

    /* Pack data */
    *(uint16_t *)(buf + 0)  = TUNE_FLASH_MAGIC;
    tmp = *(uint32_t *)&tune.best_kp;
    *(uint16_t *)(buf + 2)  = (uint16_t)(tmp & 0xFFFF);
    *(uint16_t *)(buf + 4)  = (uint16_t)(tmp >> 16);
    tmp = *(uint32_t *)&tune.best_kd;
    *(uint16_t *)(buf + 6)  = (uint16_t)(tmp & 0xFFFF);
    *(uint16_t *)(buf + 8)  = (uint16_t)(tmp >> 16);

    /* CRC over magic + kp + kd (10 bytes) */
    crc = CrcCalc_Crc16Modbus(buf, 10);
    *(uint16_t *)(buf + 10) = crc;

    /* Erase page, then write half-words */
    macNorFlash_Erase_Page(TUNE_FLASH_ADDR, 1);
    for (int i = 0; i < 6; i++) {
        macNorFlash_Write_HalfWord(TUNE_FLASH_ADDR + (uint32_t)(i * 2),
                                   *(uint16_t *)(buf + i * 2));
    }
    rt_kprintf("AUTOTUNE: Saved to Flash. Kp=%.2f Kd=%.2f\n",
               tune.best_kp, tune.best_kd);
}

/**
 * @brief  Load Kp/Kd from Flash. Falls back to defaults on any error.
 */
void autotune_load_from_flash(void)
{
    uint16_t magic;

    magic = macNorFlash_Read_HalfWord(TUNE_FLASH_ADDR);
    if (magic != TUNE_FLASH_MAGIC) {
        rt_kprintf("AUTOTUNE: Flash magic mismatch (0x%04X), using defaults.\n", magic);
        autotune_load_default();
        return;
    }

    /* Read raw half-words: kp_lo, kp_hi, kd_lo, kd_hi, crc */
    uint16_t raw[5];
    for (int i = 0; i < 5; i++) {
        raw[i] = macNorFlash_Read_HalfWord(TUNE_FLASH_ADDR + 2 + (uint32_t)(i * 2));
    }

    /* Reconstruct floats (little-endian, stored as two half-words each) */
    uint32_t kp_bits = ((uint32_t)raw[1] << 16) | raw[0];
    uint32_t kd_bits = ((uint32_t)raw[3] << 16) | raw[2];
    float kp = *(float *)&kp_bits;
    float kd = *(float *)&kd_bits;

    /* CRC check */
    uint8_t crc_buf[10];
    *(uint16_t *)(crc_buf + 0) = magic;
    *(uint16_t *)(crc_buf + 2) = raw[0];
    *(uint16_t *)(crc_buf + 4) = raw[1];
    *(uint16_t *)(crc_buf + 6) = raw[2];
    *(uint16_t *)(crc_buf + 8) = raw[3];
    uint16_t crc_calc = CrcCalc_Crc16Modbus(crc_buf, 10);

    if (raw[4] != crc_calc) {
        rt_kprintf("AUTOTUNE: Flash CRC mismatch (calc=0x%04X stored=0x%04X), using defaults.\n",
                   crc_calc, raw[4]);
        autotune_load_default();
        return;
    }

    tune.best_kp = kp;
    tune.best_kd = kd;
    carPID.verticalParam.kp = kp;
    carPID.verticalParam.kd = kd;
    tune.state = TUNE_IDLE;

    rt_kprintf("AUTOTUNE: Loaded from Flash. Kp=%.2f Kd=%.2f\n", kp, kd);
}

/*----------------------------------------------------------------------------*/
/*  Helpers                                                                  */
/*----------------------------------------------------------------------------*/

/** Clamp trial parameters to the allowed range. */
static void param_clip(void)
{
    if (tune.current_kp < AUTOTUNE_KP_MIN) tune.current_kp = AUTOTUNE_KP_MIN;
    if (tune.current_kp > AUTOTUNE_KP_MAX) tune.current_kp = AUTOTUNE_KP_MAX;
    if (tune.current_kd < AUTOTUNE_KD_MIN) tune.current_kd = AUTOTUNE_KD_MIN;
    if (tune.current_kd > AUTOTUNE_KD_MAX) tune.current_kd = AUTOTUNE_KD_MAX;
}

/** Write trial parameters to the live PID struct. */
static void apply_to_car(void)
{
    carPID.verticalParam.kp = tune.current_kp;
    carPID.verticalParam.kd = tune.current_kd;
}
