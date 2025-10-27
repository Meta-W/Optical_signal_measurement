//
// Created by wt on 2025/10/26.
//
#include "../app_main.h"
#include "main.h"
#include <stdint.h>
#ifndef ADS1256_ADS1256_H
#define ADS1256_ADS1256_H
/* GPIO定义 */
#define ADS1256_CS_PIN       GPIO_PIN_11
#define ADS1256_CS_PORT      GPIOB
#define ADS1256_DRDY_PIN     GPIO_PIN_12
#define ADS1256_DRDY_PORT    GPIOB
#define ADS1256_RESET_PIN    GPIO_PIN_0
#define ADS1256_RESET_PORT   GPIOB

/* CS片选控制 */
#define ADS1256_CS_LOW()     HAL_GPIO_WritePin(ADS1256_CS_PORT, ADS1256_CS_PIN, GPIO_PIN_RESET)
#define ADS1256_CS_HIGH()    HAL_GPIO_WritePin(ADS1256_CS_PORT, ADS1256_CS_PIN, GPIO_PIN_SET)

/* DRDY数据就绪信号读取 */
#define ADS1256_DRDY_READ()  HAL_GPIO_ReadPin(ADS1256_DRDY_PORT, ADS1256_DRDY_PIN)

/* RESET复位信号控制 */
#define ADS1256_RESET_LOW()  HAL_GPIO_WritePin(ADS1256_RESET_PORT, ADS1256_RESET_PIN, GPIO_PIN_RESET)
#define ADS1256_RESET_HIGH() HAL_GPIO_WritePin(ADS1256_RESET_PORT, ADS1256_RESET_PIN, GPIO_PIN_SET)

/* 按键定义 */
#define K1 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)

/* ========== ADS1256寄存器地址 ========== */
#define REG_STATUS  0   // 状态寄存器
#define REG_MUX     1   // 输入多路复用器控制寄存器
#define REG_ADCON   2   // A/D控制寄存器
#define REG_DRATE   3   // A/D数据速率寄存器
#define REG_IO      4   // GPIO控制寄存器
#define REG_OFC0    5   // 偏移校准寄存器0 (LSB)
#define REG_OFC1    6   // 偏移校准寄存器1
#define REG_OPC2    7   // 偏移校准寄存器2 (MSB)
#define REG_FSC0    8   // 满量程校准寄存器0 (LSB)
#define REG_FSC1    9   // 满量程校准寄存器1
#define REG_FSC2    10  // 满量程校准寄存器2 (MSB)

/* ========== ADS1256操作命令 ========== */
#define CMD_WAKEUP     0xFF  // 唤醒命令
#define CMD_RDATA      0x01  // 读取数据命令
#define CMD_RDATAC     0x03  // 连续读取数据命令
#define CMD_SDATAC     0x0F  // 停止连续读取数据命令
#define CMD_RREG       0x10  // 读寄存器命令
#define CMD_WREG       0x50  // 写寄存器命令
#define CMD_SELFCAL    0xF0  // 自校准命令
#define CMD_SELFOCAL   0xF1  // 自偏移校准命令
#define CMD_SELFGCAL   0xF2  // 自增益校准命令
#define CMD_SYSOCAL    0xF3  // 系统偏移校准命令
#define CMD_SYSGCAL    0xF4  // 系统增益校准命令
#define CMD_SYNC       0xFC  // 同步命令
#define CMD_STANDBY    0xFD  // 待机命令
#define CMD_RESET      0xFE  // 复位命令

/* ========== PGA增益设置 ========== */
#define PGA_1          0x00  // 增益1
#define PGA_2          0x01  // 增益2
#define PGA_4          0x02  // 增益4
#define PGA_8          0x03  // 增益8
#define PGA_16         0x04  // 增益16
#define PGA_32         0x05  // 增益32
#define PGA_64         0x06  // 增益64

/* ========== 正输入通道选择 ========== */
#define POSITIVE_AIN0      (0x00<<4)
#define POSITIVE_AIN1      (0x01<<4)
#define POSITIVE_AIN2      (0x02<<4)
#define POSITIVE_AIN3      (0x03<<4)
#define POSITIVE_AIN4      (0x04<<4)
#define POSITIVE_AIN5      (0x05<<4)
#define POSITIVE_AIN6      (0x06<<4)
#define POSITIVE_AIN7      (0x07<<4)
#define POSITIVE_AINCOM    (0x08<<4)  // 公共端

/* ========== 负输入通道选择 ========== */
#define NEGTIVE_AIN0       0x00
#define NEGTIVE_AIN1       0x01
#define NEGTIVE_AIN2       0x02
#define NEGTIVE_AIN3       0x03
#define NEGTIVE_AIN4       0x04
#define NEGTIVE_AIN5       0x05
#define NEGTIVE_AIN6       0x06
#define NEGTIVE_AIN7       0x07
#define NEGTIVE_AINCOM     0x08       // 公共端

/* ========== 数据速率设置 (fCLKIN=7.68MHz) ========== */
#define DATARATE_30K       0xF0  // 30000 SPS
#define DATARATE_15K       0xE0  // 15000 SPS
#define DATARATE_7_5K      0xD0  // 7500 SPS
#define DATARATE_3_7_5K    0xC0  // 3750 SPS
#define DATARATE_2K        0xB0  // 2000 SPS

/* ========== STATUS寄存器配置 ========== */
#define MSB_FIRST          (0x00<<3)  // 高位在前
#define LSB_FIRST          (0x01<<3)  // 低位在前
#define ACAL_OFF           (0x00<<2)  // 关闭自动校准
#define ACAL_ON            (0x01<<2)  // 开启自动校准
#define BUFEN_OFF          (0x00<<1)  // 关闭输入缓冲
#define BUFEN_ON           (0x01<<1)  // 开启输入缓冲

/* ========== ADCON寄存器配置 ========== */
#define CLKOUT_OFF         (0x00<<5)  // 关闭时钟输出
#define CLKOUT_CLKIN       (0x01<<5)  // 时钟输出
#define DETECT_OFF         (0x00<<3)  // 关闭传感器检测
#define DETECT_ON_2UA      (0x02<<3)  // 传感器检测电流2uA
#endif //ADS1256_ADS1256_H