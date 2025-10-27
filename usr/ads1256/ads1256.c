//
// Created by wt on 2025/10/26.
//

#include "ads1256.h"
#include "../app_main.h"
/* 类型定义 */
typedef unsigned char uchar;
typedef unsigned int uint;
uchar dat, a, ds[10], i;
long AD_DATA;
unsigned char result[3];

/**
 * @brief SPI写入一个字节
 * @param data 要写入的数据
 */
void ADS1256_WriteByte(uint8_t data)
{
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
}

/**
 * @brief SPI读取一个字节
 * @return 读取到的数据
 */
uint8_t ADS1256_ReadByte(void)
{
    uint8_t data = 0xFF;
    HAL_SPI_Receive(&hspi1, &data, 1, HAL_MAX_DELAY);
    return data;
}

/**
 * @brief 等待DRDY信号变低(数据就绪)
 * @param timeout 超时时间(毫秒)
 * @return 0-成功, 1-超时
 */
uint8_t ADS1256_WaitDRDY(uint32_t timeout)
{
    uint32_t start = HAL_GetTick();
    while(ADS1256_DRDY_READ() == GPIO_PIN_SET)
    {
        if((HAL_GetTick() - start) > timeout)
            return 1; // 超时
    }
    return 0; // 成功
}

/**
 * @brief 写ADS1256寄存器
 * @param reg 寄存器地址
 * @param data 要写入的数据
 */
void ADS1256_WriteReg(uint8_t reg, uint8_t data)
{
    ADS1256_CS_LOW();
    ADS1256_WaitDRDY(100);

    ADS1256_WriteByte(CMD_WREG | reg);  // 写寄存器命令 + 地址
    ADS1256_WriteByte(0x00);            // 写入1个字节
    ADS1256_WriteByte(data);            // 写入数据

    ADS1256_CS_HIGH();
    // delay_us(10);
}

/**
 * @brief 读ADS1256寄存器
 * @param reg 寄存器地址
 * @return 寄存器的值
 */
uint8_t ADS1256_ReadReg(uint8_t reg)
{
    uint8_t data;

    ADS1256_CS_LOW();
    ADS1256_WaitDRDY(100);

    ADS1256_WriteByte(CMD_RREG | reg);  // 读寄存器命令 + 地址
    ADS1256_WriteByte(0x00);            // 读取1个字节
    // delay_us(10);
    data = ADS1256_ReadByte();          // 读取数据

    ADS1256_CS_HIGH();
    return data;
}

/**
 * @brief 读取ADS1256的24位ADC数据
 */
void ADS1256_ReadData(void)
{
    ADS1256_CS_LOW();

    // 等待数据就绪
    if(ADS1256_WaitDRDY(100) != 0)
    {
        ADS1256_CS_HIGH();
        return;
    }

    // 发送读数据命令
    ADS1256_WriteByte(CMD_RDATA);
    // delay_us(10);  // t6延时, min=50*(1/7.68MHz)=6.5us

    // 读取3个字节的数据
    result[0] = ADS1256_ReadByte();  // MSB
    result[1] = ADS1256_ReadByte();  // MID
    result[2] = ADS1256_ReadByte();  // LSB

    ADS1256_CS_HIGH();
}

/**
 * @brief 初始化ADS1256
 */
void ADS1256_Init(void)
{
    // 硬件复位(可选)
    ADS1256_RESET_LOW();
    HAL_Delay(10);
    ADS1256_RESET_HIGH();
    HAL_Delay(10);

    ADS1256_CS_LOW();

    // 等待芯片就绪
    ADS1256_WaitDRDY(100);

    // 发送复位命令
    ADS1256_WriteByte(CMD_RESET);
    HAL_Delay(10);

    ADS1256_CS_HIGH();
    HAL_Delay(10);

    // 配置状态寄存器: 自动校准使能, 模拟输入缓冲禁用
    ADS1256_WriteReg(REG_STATUS, 0xF4);

    // 配置输入多路复用器: AIN0为正输入, AINCOM为负输入
    ADS1256_WriteReg(REG_MUX, POSITIVE_AIN0 + NEGTIVE_AINCOM);

    // 配置A/D控制寄存器: 时钟输出关闭, 传感器检测关闭, 增益1
    ADS1256_WriteReg(REG_ADCON, CLKOUT_OFF + DETECT_OFF + PGA_1);

    // 配置数据速率: 15K SPS
    ADS1256_WriteReg(REG_DRATE, DATARATE_15K);

    // 等待稳定
    HAL_Delay(10);
    ADS1256_WaitDRDY(100);

    // 发送唤醒命令
    ADS1256_CS_LOW();
    ADS1256_WriteByte(CMD_WAKEUP);
    ADS1256_CS_HIGH();

    HAL_Delay(10);
}

/**
 * @brief 通过UART发送ASCII格式的电压数据
 * @param channel 通道号(0-3)
 */
void Send_Data_ASCII(uint8_t channel)
{
    char buffer[20];

    // 计算各位数字
    ds[1] = AD_DATA / 100000;
    ds[2] = AD_DATA % 100000 / 10000;
    ds[3] = AD_DATA % 10000 / 1000;
    ds[4] = AD_DATA % 1000 / 100;
    ds[5] = AD_DATA % 100 / 10;
    ds[6] = AD_DATA % 10;

    // 格式化输出: V[通道][符号][整数位].[小数位]\r\n
    sprintf(buffer, "V%d%c%d.%d%d%d%d%d\r\n",
            channel,
            ds[0],
            ds[1],
            ds[2], ds[3], ds[4], ds[5], ds[6]);

    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

/**
 * @brief 写入MUX寄存器并读取电压值
 * @param mux_config MUX配置值(正输入+负输入)
 */
void Write_Reg_Mux(uint8_t mux_config)
{
    dat = 0;

    // 配置输入通道
    ADS1256_WriteReg(REG_MUX, mux_config);
    HAL_Delay(5);  // 等待通道切换稳定

    // 读取AD数据
    ADS1256_ReadData();

    // 判断是否为负电压(最高位为1表示负数)
    if((result[0] & 0x80) == 0x80)
    {
        // 二进制补码转换
        result[0] = ~result[0];
        result[1] = ~result[1];
        result[2] = ~result[2];
        ds[0] = '-';  // 负号
    }
    else
    {
        ds[0] = '+';  // 正号
    }

    // 计算电压值
    // AD_DATA = (24位ADC值 * Vref) / (2^23 * PGA)
    // 这里假设Vref=5V, PGA=1
    // 83.88607 = 2^23 / 100000 用于将结果转换为5位小数的整数表示
    AD_DATA = ((result[0] * 65536 + result[1] * 256 + result[2]) * 5) / 83.88607;
}