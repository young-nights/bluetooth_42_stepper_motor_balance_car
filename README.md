# Bluetooth 42 Stepper Motor Balance Car

> 基于 STM32F103 + RT-Thread 的双轮自平衡小车 | Two-wheel Self-balancing Robot

[![Platform](https://img.shields.io/badge/MCU-STM32F103RC-blue)](https://www.st.com)
[![RTOS](https://img.shields.io/badge/RTOS-RT--Thread_v4.0.3-green)](https://www.rt-thread.org)
[![Language](https://img.shields.io/badge/Language-C-orange)]()
[![License](https://img.shields.io/badge/License-MIT-lightgrey)](./LICENSE)

---

## 📋 项目简介

本项目设计并实现了一台基于 **STM32F103RC（Cortex-M3）** + **RT-Thread 实时操作系统**的双轮自平衡小车。使用两个 42 步进电机作为执行器，MPU6050 六轴传感器作为姿态感知单元，通过蓝牙与手机 APP 进行无线通信控制。

## ✨ 功能特性

| 功能 | 描述 | 状态 |
|------|------|------|
| 🎯 直立平衡 | PD 直立环姿态闭环控制 | ✅ |
| 🚀 速度控制 | PI 速度环闭环控制，积分分离 | ✅ |
| ⬆️ 前进/后退 | 蓝牙 APP 遥控 | ✅ |
| ↩️ 左转/右转 | 蓝牙 APP 遥控 + 差速转向 | ✅ (已优化) |
| ▶️ 启动/停止 | 蓝牙 APP 启停小车站立 | ✅ |
| 📐 姿态解算 | MPU6050 互补滤波，100Hz | ✅ |
| 🔧 传感器校准 | 陀螺仪静态校准 + 六面加速度计校准 | ✅ |
| 💡 LED 指示 | 双色 LED 状态指示（闪烁/呼吸） | ✅ |
| 🔊 蜂鸣器 | 指令接收成功提示音 | ✅ |
| 💾 参数存储 | Flash 首次启动检测 | ✅ (已修复) |
| 📡 蓝牙通信 | 自定义二进制协议 + CRC16 校验 | ✅ |
| 🛡️ 电机保护 | 故障检测 + 拿起检测 | ✅ (已实现) |
| 📊 调试输出 | Vofa+ 串口波形可视化（可关闭） | ✅ |

## 🧱 硬件平台

| 组件 | 型号/参数 |
|------|-----------|
| **主控 MCU** | STM32F103RC（Cortex-M3, 72MHz, 256KB Flash, 48KB SRAM） |
| **姿态传感器** | MPU6050（6 轴陀螺仪+加速度计），I²C 接口 |
| **电机** | 42 步进电机（步距角 1.8°, 16 细分, 3200 脉冲/转） |
| **电机驱动** | DRV8825 兼容驱动 |
| **蓝牙模块** | 双模蓝牙（SPP + BLE），UART2 |
| **轮胎** | 直径 65mm，周长 ≈ 204mm |
| **电源** | 电池供电，ADC1 电压检测 |

## 🏗 系统架构

```
┌─────────────────────────────────────────────────┐
│                   应用层                         │
│  main.c │ PID控制 │ 电机控制 │ 蓝牙协议 │ 姿态解算 │
├─────────────────────────────────────────────────┤
│               RT-Thread Kernel                   │
│  线程调度 │ IPC │ 设备驱动框架 │ 软件定时器       │
├─────────────────────────────────────────────────┤
│              STM32 HAL / CMSIS                   │
├─────────────────────────────────────────────────┤
│          STM32F103RC 硬件 (Cortex-M3)            │
└─────────────────────────────────────────────────┘
```

**控制算法**: 双环串级 PID（直立环 PD + 速度环 PI），200Hz 控制频率

## 📁 仓库结构

```
bluetooth_42_stepper_motor_balance_car/
├── Diy-code_balance-car_ver1.0/   # 完整工程源码
│   ├── applications/              # 应用层代码
│   │   ├── macAPP/                # 核心应用（PID/电机/蓝牙/打印）
│   │   ├── macBSP/                # 板级支持（LED/蜂鸣器/PWM）
│   │   ├── macMPU/                # MPU6050 驱动+校准+姿态解算
│   │   ├── macMath/               # 数学工具（滤波/线性化/卡尔曼）
│   │   ├── macFLASH/              # Flash 参数存储
│   │   └── macSYS/                # 系统/类型/常量定义
│   ├── drivers/                   # RT-Thread 驱动层
│   ├── packages/                  # RT-Thread 软件包
│   ├── libraries/                 # STM32 HAL + CMSIS
│   ├── rt-thread/                 # RT-Thread 内核源码
│   └── docs/                      # 📚 项目文档
│       ├── balance-car-design.md       # 中文设计文档
│       ├── balance-car-design.en.md    # 英文设计文档
│       ├── balance-car-design.html     # HTML 版（含 Mermaid 图表）
│       └── fix-report.html             # 代码优化修复报告
├── 1.Schematic/                   # 原理图
├── 2.Protolcal/                   # 通信协议文档
├── LICENSE
└── README.md
```

## 🚀 快速开始

### 环境要求

- **IDE**: RT-Thread Studio
- **工具链**: `arm-none-eabi-gcc` (GCC ARM Embedded)
- **构建系统**: SCons
- **调试器**: ST-Link / J-Link / DAPLink

### 编译

```bash
cd Diy-code_balance-car_ver1.0
scons -j4
```

### 烧录

- **RT-Thread Studio**: 配置调试器 → 一键下载
- **命令行**: `openocd -f board/stm32f103c8.cfg -c "program rt-thread.bin 0x08000000 verify reset exit"`
- **串口 ISP**: 通过 USART1 使用 STM32Flash 工具

### 11 个运行时线程

| 线程 | 优先级 | 频率 | 功能 |
|------|--------|------|------|
| 姿态解算 | 5 | 100Hz | MPU6050 互补滤波 |
| 蓝牙解码 | 6 | — | 指令接收与解析 |
| PID 控制 | 20 | 200Hz | 双环平衡控制 |
| 校准 | 6 | — | 传感器校准 |
| 调试打印 | 29 | — | Vofa+ 输出 |
| 软定时器×3 | 4 | 1ms/100ms | LED/蜂鸣器/电机检测 |

## 📡 蓝牙通信协议

自定义二进制帧格式，CRC16-Modbus 校验：

```
┌──────┬──────┬──────┬──────────┬───────┬──────────┬───────┐
│ 0x55 │ 0xAA │ Len  │ DeviceID │ Cmd   │   Data   │ CRC16 │
│ 帧头 │ 帧头 │ 1Byte│  H:L     │ Type  │  N Byte  │ 2Byte │
└──────┴──────┴──────┴──────────┴───────┴──────────┴───────┘
```

支持帧类型：`0x31` 设置 / `0x32` 上报 / `0x33` 获取 / `0x66` 推送，共 24 条指令。

## 🔧 代码优化记录 (2026-05-23)

本仓库代码已经过全面审查与优化，共修复 **23 个问题**：

| 严重程度 | 数量 | 关键修复 |
|----------|------|----------|
| 🔴 严重 | 8 | 线程安全/Flash写入/PWM映射/缓冲区溢出/引脚冲突/栈溢出 |
| 🟡 中等 | 10 | 控制周期/互补滤波/速度闭环/超时/数据校验/拿起检测 |
| 🟢 低优 | 5 | 魔术数字/ISR打印/重复include/命名修正/SConscript |

详细修复报告见 [`docs/fix-report.html`](./Diy-code_balance-car_ver1.0/docs/fix-report.html)

## 📚 文档

完整设计文档（含 Mermaid 架构图/时序图/状态机）：

- [中文版](./Diy-code_balance-car_ver1.0/docs/balance-car-design.md)
- [英文版](./Diy-code_balance-car_ver1.0/docs/balance-car-design.en.md)
- [HTML 版（推荐浏览器打开）](./Diy-code_balance-car_ver1.0/docs/balance-car-design.html)

## 📝 License

MIT License. See [LICENSE](./LICENSE) for details.
