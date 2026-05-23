/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-18     zphu       the first version
 */
#ifndef APPLICATIONS_MACSYS_INC_MACFLASH_H_
#define APPLICATIONS_MACSYS_INC_MACFLASH_H_
#include "macSYS.h"



#if defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
 #define PAGE_SIZE          (0x400)    //页的大小1K
 #define FLASH_SIZE         (0x20000)  //Flash空间128K
#elif defined STM32F10X_CL
 #define PAGE_SIZE          (0x400)    //页的大小1K
 #define FLASH_SIZE         (0x20000)  //Flash空间128K
#elif defined STM32F10X_HD || defined (STM32F10X_HD_VL)
 #define PAGE_SIZE          (0x800)    //页的大小2K
 #define FLASH_SIZE         (0x80000)  //Flash空间512K
#elif defined STM32F10X_XL
 #define PAGE_SIZE          (0x800)    //页的大小2K
 #define FLASH_SIZE         (0x100000) //Flash空间1M
#else 
 #error "Please select first the STM32 device to be used (in stm32f10x.h)"    
#endif


/* 定义各个扇区的起始地址 */
/* Base address of the Flash sectors Bank 1 */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 1 Kbyte */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08000400) /* Base @ of Sector 1, 1 Kbyte */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08000800) /* Base @ of Sector 2, 1 Kbyte */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x08000C00) /* Base @ of Sector 3, 1 Kbyte */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08001000) /* Base @ of Sector 4, 1 Kbyte */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08001400) /* Base @ of Sector 5, 1 Kbyte */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08001800) /* Base @ of Sector 6, 1 Kbyte */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08001C00) /* Base @ of Sector 7, 1 Kbyte */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08002000) /* Base @ of Sector 8, 1 Kbyte */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x08002400) /* Base @ of Sector 9, 1 Kbyte */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x08002800) /* Base @ of Sector 10, 1 Kbyte */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x08002C00) /* Base @ of Sector 11, 1 Kbyte */
#define ADDR_FLASH_SECTOR_12    ((uint32_t)0x08003000) /* Base @ of Sector 12, 1 Kbyte */
#define ADDR_FLASH_SECTOR_13    ((uint32_t)0x08003400) /* Base @ of Sector 13, 1 Kbyte */
#define ADDR_FLASH_SECTOR_14    ((uint32_t)0x08003800) /* Base @ of Sector 14, 1 Kbyte */
#define ADDR_FLASH_SECTOR_15    ((uint32_t)0x08003C00) /* Base @ of Sector 15, 1 Kbyte */
#define ADDR_FLASH_SECTOR_16    ((uint32_t)0x08004000) /* Base @ of Sector 16, 1 Kbyte */
#define ADDR_FLASH_SECTOR_17    ((uint32_t)0x08004400) /* Base @ of Sector 17, 1 Kbyte */
#define ADDR_FLASH_SECTOR_18    ((uint32_t)0x08004800) /* Base @ of Sector 18, 1 Kbyte */
#define ADDR_FLASH_SECTOR_19    ((uint32_t)0x08004C00) /* Base @ of Sector 19, 1 Kbyte */
#define ADDR_FLASH_SECTOR_20    ((uint32_t)0x08005000) /* Base @ of Sector 20, 1 Kbyte */
#define ADDR_FLASH_SECTOR_21    ((uint32_t)0x08005400) /* Base @ of Sector 21, 1 Kbyte */
#define ADDR_FLASH_SECTOR_22    ((uint32_t)0x08005800) /* Base @ of Sector 22, 1 Kbyte */
#define ADDR_FLASH_SECTOR_23    ((uint32_t)0x08005C00) /* Base @ of Sector 23, 1 Kbyte */
#define ADDR_FLASH_SECTOR_24    ((uint32_t)0x08006000) /* Base @ of Sector 24, 1 Kbyte */
#define ADDR_FLASH_SECTOR_25    ((uint32_t)0x08006400) /* Base @ of Sector 25, 1 Kbyte */
#define ADDR_FLASH_SECTOR_26    ((uint32_t)0x08006800) /* Base @ of Sector 26, 1 Kbyte */
#define ADDR_FLASH_SECTOR_27    ((uint32_t)0x08006C00) /* Base @ of Sector 27, 1 Kbyte */
#define ADDR_FLASH_SECTOR_28    ((uint32_t)0x08007000) /* Base @ of Sector 28, 1 Kbyte */
#define ADDR_FLASH_SECTOR_29    ((uint32_t)0x08007400) /* Base @ of Sector 29, 1 Kbyte */
#define ADDR_FLASH_SECTOR_30    ((uint32_t)0x08007800) /* Base @ of Sector 30, 1 Kbyte */
#define ADDR_FLASH_SECTOR_31    ((uint32_t)0x08007C00) /* Base @ of Sector 31, 1 Kbyte */
#define ADDR_FLASH_SECTOR_32    ((uint32_t)0x08008000) /* Base @ of Sector 32, 1 Kbyte */
#define ADDR_FLASH_SECTOR_33    ((uint32_t)0x08008400) /* Base @ of Sector 33, 1 Kbyte */
#define ADDR_FLASH_SECTOR_34    ((uint32_t)0x08008800) /* Base @ of Sector 34, 1 Kbyte */
#define ADDR_FLASH_SECTOR_35    ((uint32_t)0x08008C00) /* Base @ of Sector 35, 1 Kbyte */
#define ADDR_FLASH_SECTOR_36    ((uint32_t)0x08009000) /* Base @ of Sector 36, 1 Kbyte */
#define ADDR_FLASH_SECTOR_37    ((uint32_t)0x08009400) /* Base @ of Sector 37, 1 Kbyte */
#define ADDR_FLASH_SECTOR_38    ((uint32_t)0x08009800) /* Base @ of Sector 38, 1 Kbyte */
#define ADDR_FLASH_SECTOR_39    ((uint32_t)0x08009C00) /* Base @ of Sector 39, 1 Kbyte */
#define ADDR_FLASH_SECTOR_40    ((uint32_t)0x0800A000) /* Base @ of Sector 40, 1 Kbyte */
#define ADDR_FLASH_SECTOR_41    ((uint32_t)0x0800A400) /* Base @ of Sector 41, 1 Kbyte */
#define ADDR_FLASH_SECTOR_42    ((uint32_t)0x0800A800) /* Base @ of Sector 42, 1 Kbyte */
#define ADDR_FLASH_SECTOR_43    ((uint32_t)0x0800AC00) /* Base @ of Sector 43, 1 Kbyte */
#define ADDR_FLASH_SECTOR_44    ((uint32_t)0x0800B000) /* Base @ of Sector 44, 1 Kbyte */
#define ADDR_FLASH_SECTOR_45    ((uint32_t)0x0800B400) /* Base @ of Sector 45, 1 Kbyte */
#define ADDR_FLASH_SECTOR_46    ((uint32_t)0x0800B800) /* Base @ of Sector 46, 1 Kbyte */
#define ADDR_FLASH_SECTOR_47    ((uint32_t)0x0800BC00) /* Base @ of Sector 47, 1 Kbyte */
#define ADDR_FLASH_SECTOR_48    ((uint32_t)0x0800C000) /* Base @ of Sector 48, 1 Kbyte */
#define ADDR_FLASH_SECTOR_49    ((uint32_t)0x0800C400) /* Base @ of Sector 49, 1 Kbyte */
#define ADDR_FLASH_SECTOR_50    ((uint32_t)0x0800C800) /* Base @ of Sector 50, 1 Kbyte */
#define ADDR_FLASH_SECTOR_51    ((uint32_t)0x0800CC00) /* Base @ of Sector 51, 1 Kbyte */
#define ADDR_FLASH_SECTOR_52    ((uint32_t)0x0800D000) /* Base @ of Sector 52, 1 Kbyte */
#define ADDR_FLASH_SECTOR_53    ((uint32_t)0x0800D400) /* Base @ of Sector 53, 1 Kbyte */
#define ADDR_FLASH_SECTOR_54    ((uint32_t)0x0800D800) /* Base @ of Sector 54, 1 Kbyte */
#define ADDR_FLASH_SECTOR_55    ((uint32_t)0x0800DC00) /* Base @ of Sector 55, 1 Kbyte */
#define ADDR_FLASH_SECTOR_56    ((uint32_t)0x0800E000) /* Base @ of Sector 56, 1 Kbyte */
#define ADDR_FLASH_SECTOR_57    ((uint32_t)0x0800E400) /* Base @ of Sector 57, 1 Kbyte */
#define ADDR_FLASH_SECTOR_58    ((uint32_t)0x0800E800) /* Base @ of Sector 58, 1 Kbyte */
#define ADDR_FLASH_SECTOR_59    ((uint32_t)0x0800EC00) /* Base @ of Sector 59, 1 Kbyte */
#define ADDR_FLASH_SECTOR_60    ((uint32_t)0x0800F000) /* Base @ of Sector 60, 1 Kbyte */
#define ADDR_FLASH_SECTOR_61    ((uint32_t)0x0800F400) /* Base @ of Sector 61, 1 Kbyte */
#define ADDR_FLASH_SECTOR_62    ((uint32_t)0x0800F800) /* Base @ of Sector 62, 1 Kbyte */
#define ADDR_FLASH_SECTOR_63    ((uint32_t)0x0800FC00) /* Base @ of Sector 63, 1 Kbyte */
#define ADDR_FLASH_SECTOR_64    ((uint32_t)0x08010000) /* Base @ of Sector 64, 1 Kbyte */
#define ADDR_FLASH_SECTOR_65    ((uint32_t)0x08010400) /* Base @ of Sector 65, 1 Kbyte */
#define ADDR_FLASH_SECTOR_66    ((uint32_t)0x08010800) /* Base @ of Sector 66, 1 Kbyte */
#define ADDR_FLASH_SECTOR_67    ((uint32_t)0x08010C00) /* Base @ of Sector 67, 1 Kbyte */
#define ADDR_FLASH_SECTOR_68    ((uint32_t)0x08011000) /* Base @ of Sector 68, 1 Kbyte */
#define ADDR_FLASH_SECTOR_69    ((uint32_t)0x08011400) /* Base @ of Sector 69, 1 Kbyte */
#define ADDR_FLASH_SECTOR_70    ((uint32_t)0x08011800) /* Base @ of Sector 70, 1 Kbyte */
#define ADDR_FLASH_SECTOR_71    ((uint32_t)0x08011C00) /* Base @ of Sector 71, 1 Kbyte */
#define ADDR_FLASH_SECTOR_72    ((uint32_t)0x08012000) /* Base @ of Sector 72, 1 Kbyte */
#define ADDR_FLASH_SECTOR_73    ((uint32_t)0x08012400) /* Base @ of Sector 73, 1 Kbyte */
#define ADDR_FLASH_SECTOR_74    ((uint32_t)0x08012800) /* Base @ of Sector 74, 1 Kbyte */
#define ADDR_FLASH_SECTOR_75    ((uint32_t)0x08012C00) /* Base @ of Sector 75, 1 Kbyte */
#define ADDR_FLASH_SECTOR_76    ((uint32_t)0x08013000) /* Base @ of Sector 76, 1 Kbyte */
#define ADDR_FLASH_SECTOR_77    ((uint32_t)0x08013400) /* Base @ of Sector 77, 1 Kbyte */
#define ADDR_FLASH_SECTOR_78    ((uint32_t)0x08013800) /* Base @ of Sector 78, 1 Kbyte */
#define ADDR_FLASH_SECTOR_79    ((uint32_t)0x08013C00) /* Base @ of Sector 79, 1 Kbyte */
#define ADDR_FLASH_SECTOR_80    ((uint32_t)0x08014000) /* Base @ of Sector 80, 1 Kbyte */
#define ADDR_FLASH_SECTOR_81    ((uint32_t)0x08014400) /* Base @ of Sector 81, 1 Kbyte */
#define ADDR_FLASH_SECTOR_82    ((uint32_t)0x08014800) /* Base @ of Sector 82, 1 Kbyte */
#define ADDR_FLASH_SECTOR_83    ((uint32_t)0x08014C00) /* Base @ of Sector 83, 1 Kbyte */
#define ADDR_FLASH_SECTOR_84    ((uint32_t)0x08015000) /* Base @ of Sector 84, 1 Kbyte */
#define ADDR_FLASH_SECTOR_85    ((uint32_t)0x08015400) /* Base @ of Sector 85, 1 Kbyte */
#define ADDR_FLASH_SECTOR_86    ((uint32_t)0x08015800) /* Base @ of Sector 86, 1 Kbyte */
#define ADDR_FLASH_SECTOR_87    ((uint32_t)0x08015C00) /* Base @ of Sector 87, 1 Kbyte */
#define ADDR_FLASH_SECTOR_88    ((uint32_t)0x08016000) /* Base @ of Sector 88, 1 Kbyte */
#define ADDR_FLASH_SECTOR_89    ((uint32_t)0x08016400) /* Base @ of Sector 89, 1 Kbyte */
#define ADDR_FLASH_SECTOR_90    ((uint32_t)0x08016800) /* Base @ of Sector 90, 1 Kbyte */
#define ADDR_FLASH_SECTOR_91    ((uint32_t)0x08016C00) /* Base @ of Sector 91, 1 Kbyte */
#define ADDR_FLASH_SECTOR_92    ((uint32_t)0x08017000) /* Base @ of Sector 92, 1 Kbyte */
#define ADDR_FLASH_SECTOR_93    ((uint32_t)0x08017400) /* Base @ of Sector 93, 1 Kbyte */
#define ADDR_FLASH_SECTOR_94    ((uint32_t)0x08017800) /* Base @ of Sector 94, 1 Kbyte */
#define ADDR_FLASH_SECTOR_95    ((uint32_t)0x08017C00) /* Base @ of Sector 95, 1 Kbyte */
#define ADDR_FLASH_SECTOR_96    ((uint32_t)0x08018000) /* Base @ of Sector 96, 1 Kbyte */
#define ADDR_FLASH_SECTOR_97    ((uint32_t)0x08018400) /* Base @ of Sector 97, 1 Kbyte */
#define ADDR_FLASH_SECTOR_98    ((uint32_t)0x08018800) /* Base @ of Sector 98, 1 Kbyte */
#define ADDR_FLASH_SECTOR_99    ((uint32_t)0x08018C00) /* Base @ of Sector 99, 1 Kbyte */
#define ADDR_FLASH_SECTOR_100    ((uint32_t)0x08019000) /* Base @ of Sector 100, 1 Kbyte */
#define ADDR_FLASH_SECTOR_101    ((uint32_t)0x08019400) /* Base @ of Sector 101, 1 Kbyte */
#define ADDR_FLASH_SECTOR_102    ((uint32_t)0x08019800) /* Base @ of Sector 102, 1 Kbyte */
#define ADDR_FLASH_SECTOR_103    ((uint32_t)0x08019C00) /* Base @ of Sector 103, 1 Kbyte */
#define ADDR_FLASH_SECTOR_104    ((uint32_t)0x0801A000) /* Base @ of Sector 104, 1 Kbyte */
#define ADDR_FLASH_SECTOR_105    ((uint32_t)0x0801A400) /* Base @ of Sector 105, 1 Kbyte */
#define ADDR_FLASH_SECTOR_106    ((uint32_t)0x0801A800) /* Base @ of Sector 106, 1 Kbyte */
#define ADDR_FLASH_SECTOR_107    ((uint32_t)0x0801AC00) /* Base @ of Sector 107, 1 Kbyte */
#define ADDR_FLASH_SECTOR_108    ((uint32_t)0x0801B000) /* Base @ of Sector 108, 1 Kbyte */
#define ADDR_FLASH_SECTOR_109    ((uint32_t)0x0801B400) /* Base @ of Sector 109, 1 Kbyte */
#define ADDR_FLASH_SECTOR_110    ((uint32_t)0x0801B800) /* Base @ of Sector 110, 1 Kbyte */
#define ADDR_FLASH_SECTOR_111    ((uint32_t)0x0801BC00) /* Base @ of Sector 111, 1 Kbyte */
#define ADDR_FLASH_SECTOR_112    ((uint32_t)0x0801C000) /* Base @ of Sector 112, 1 Kbyte */
#define ADDR_FLASH_SECTOR_113    ((uint32_t)0x0801C400) /* Base @ of Sector 113, 1 Kbyte */
#define ADDR_FLASH_SECTOR_114    ((uint32_t)0x0801C800) /* Base @ of Sector 114, 1 Kbyte */
#define ADDR_FLASH_SECTOR_115    ((uint32_t)0x0801CC00) /* Base @ of Sector 115, 1 Kbyte */
#define ADDR_FLASH_SECTOR_116    ((uint32_t)0x0801D000) /* Base @ of Sector 116, 1 Kbyte */
#define ADDR_FLASH_SECTOR_117    ((uint32_t)0x0801D400) /* Base @ of Sector 117, 1 Kbyte */
#define ADDR_FLASH_SECTOR_118    ((uint32_t)0x0801D800) /* Base @ of Sector 118, 1 Kbyte */
#define ADDR_FLASH_SECTOR_119    ((uint32_t)0x0801DC00) /* Base @ of Sector 119, 1 Kbyte */
#define ADDR_FLASH_SECTOR_120    ((uint32_t)0x0801E000) /* Base @ of Sector 120, 1 Kbyte */
#define ADDR_FLASH_SECTOR_121    ((uint32_t)0x0801E400) /* Base @ of Sector 121, 1 Kbyte */
#define ADDR_FLASH_SECTOR_122    ((uint32_t)0x0801E800) /* Base @ of Sector 122, 1 Kbyte */
#define ADDR_FLASH_SECTOR_123    ((uint32_t)0x0801EC00) /* Base @ of Sector 123, 1 Kbyte */
#define ADDR_FLASH_SECTOR_124    ((uint32_t)0x0801F000) /* Base @ of Sector 124, 1 Kbyte */
#define ADDR_FLASH_SECTOR_125    ((uint32_t)0x0801F400) /* Base @ of Sector 125, 1 Kbyte */
#define ADDR_FLASH_SECTOR_126    ((uint32_t)0x0801F800) /* Base @ of Sector 126, 1 Kbyte */
#define ADDR_FLASH_SECTOR_127    ((uint32_t)0x0801FC00) /* Base @ of Sector 127, 1 Kbyte */




/* Flash基地址 */
#define ADDR_FLASH_START_ADDR_BASE   ADDR_FLASH_SECTOR_0
/* 用户存储地址 - 1KB用于存储标志位 */
#define ADDR_FLASH_START_ADDR_DATA   ADDR_FLASH_SECTOR_127






uint32_t macNorFlash_Read_Word(uint32_t addr);
uint16_t macNorFlash_Read_HalfWord(uint32_t addr);
uint8_t macNorFlash_Read_Byte(uint32_t addr);
void macNorFlash_Write_Word(uint32_t addr, uint32_t dat);
void macNorFlash_Write_HalfWord(uint32_t addr, uint16_t dat);
void macNorFlash_Erase_Page(uint32_t addr, uint8_t PageNumber);




#endif /* APPLICATIONS_MACSYS_INC_MACFLASH_H_ */
