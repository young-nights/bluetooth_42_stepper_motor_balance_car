/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-18     zphu       the first version
 */
#include "macFlash.h"
#include <stm32f1xx.h>



// HAL Read Nor-Falsh---------------------------------------------------------------------------------------------------------------------

/**
 * @brief  片内flash读取指定地址的数据
 * @param  addr     : 读取指定地址的数据
 * @return read
 */
uint32_t macNorFlash_Read_Word(uint32_t addr)
{
    return *((__IO uint32_t*)(addr));
}


/**
 * @brief  片内flash读取指定地址的数据
 * @param  addr     : 读取指定地址的数据
 * @return read
 */
uint16_t macNorFlash_Read_HalfWord(uint32_t addr)
{
    return *((__IO uint16_t*)(addr));
}



/**
 * @brief  片内flash读取指定地址的数据
 * @param  addr     : 读取指定地址的数据
 * @return read
 */
uint8_t macNorFlash_Read_Byte(uint32_t addr)
{
    return *((__IO uint8_t*)(addr));
}




// HAL Write Nor-Falsh---------------------------------------------------------------------------------------------------------------------

/**
 * @brief  片内flash写入指定地址的数据
 * @param  addr     : 写入指定地址的数据
 *         dat      : 写入的数据
 * @return NULL
 */
void macNorFlash_Write_Word(uint32_t addr, uint32_t dat)
{
    HAL_FLASH_Unlock();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, dat);
    HAL_FLASH_Lock();
}




/**
 * @brief  片内flash写入指定地址的数据
 * @param  addr     : 写入指定地址的数据
 *         dat      : 写入的数据
 * @return NULL
 */
void macNorFlash_Write_HalfWord(uint32_t addr, uint16_t dat)
{
    HAL_FLASH_Unlock();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, dat);
    HAL_FLASH_Lock();
}




// HAL Erase Nor-Falsh---------------------------------------------------------------------------------------------------------------------

/**
 * @brief  片内flash擦除页数据
 * @param  addr      : 擦除页的起始地址
 *         PageNumber: 擦除页的数量
 * @return 实际擦除区域的大小
 */

void macNorFlash_Erase_Page(uint32_t addr, uint8_t PageNumber)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef macEraseFlash;
    macEraseFlash.NbPages = PageNumber;
    macEraseFlash.PageAddress = addr;
    macEraseFlash.TypeErase = FLASH_TYPEERASE_PAGES;

    uint32_t EraseData = 0xFF;
    HAL_FLASHEx_Erase(&macEraseFlash, &EraseData);
    HAL_FLASH_Lock();

}
