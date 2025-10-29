#include "ads1256.h"
#include "stdio.h"
#include "spi.h"   // hspi1

void ADS1256_write_reg(uint8_t ADS1256_command,uint8_t *WriteBuf,uint8_t len);
void ADS1256_read_reg(uint8_t ADS1256_command,uint8_t *ReadBuf,uint8_t len);
void ADS1256_write_bit(uint8_t temp);//功能:写一字节数据
uint8_t ADS1256_read_bit(void);      //功能:读一字节数据



/* === GPIO 宏定义 === */
#define ADS1256_Write_CS_L     HAL_GPIO_WritePin(ADS1256_CS_GPIO_Port, ADS1256_CS_Pin, GPIO_PIN_RESET)
#define ADS1256_Write_CS_H    HAL_GPIO_WritePin(ADS1256_CS_GPIO_Port, ADS1256_CS_Pin, GPIO_PIN_SET)

#define ADS1256_SYNC_LOW()   HAL_GPIO_WritePin(ADS1256_SYNC_GPIO_Port, ADS1256_SYNC_Pin, GPIO_PIN_RESET)
#define ADS1256_SYNC_HIGH()  HAL_GPIO_WritePin(ADS1256_SYNC_GPIO_Port, ADS1256_SYNC_Pin, GPIO_PIN_SET)

#define ADS1256_Write_RST_L    HAL_GPIO_WritePin(ADS1256_RST_GPIO_Port, ADS1256_RST_Pin, GPIO_PIN_RESET)
#define ADS1256_Write_RST_H   HAL_GPIO_WritePin(ADS1256_RST_GPIO_Port, ADS1256_RST_Pin, GPIO_PIN_SET)

#define ADS1256_Read_DRDY  HAL_GPIO_ReadPin(ADS1256_DRDY_GPIO_Port, ADS1256_DRDY_Pin)

/****************************************
功能：写一字节数据
*****************************************/
void ADS1256_write_bit(uint8_t temp)
{
    // HAL_SPI_Transmit 函数会阻塞, 直到数据发送完成
    // 1. &hspi1: 您的 SPI 句柄指针
    // 2. &temp:  要发送数据的指针
    // 3. 1:      发送的数据量 (1 个字节)
    // 4. 100:    超时时间 (单位: 毫秒)
    HAL_SPI_Transmit(&hspi1, &temp, 1, 100);
}

/****************************************
功能：读一字节数据
*****************************************/
uint8_t ADS1256_read_bit(void)
{
    uint8_t received_data = 0;

    // HAL_SPI_Receive 函数会阻塞, 直到 1 个字节接收完成
    // 1. &hspi1: 您的 SPI 句柄指针
    // 2. &received_data: 存储接收数据的变量指针
    // 3. 1:              接收的数据量 (1 个字节)
    // 4. 100:            超时时间 (单位: 毫秒)
    HAL_SPI_Receive(&hspi1, &received_data, 1, 100);

    return received_data;
}

/****************************************
功能：读DRDY引脚状态
*****************************************/
uint8_t ADS1256_DRDY(void)
{
    return ADS1256_Read_DRDY;
}

/****************************************
功能：读单次数据命令
*****************************************/
void ADS1256_RDATA(void)
{
    while(ADS1256_Read_DRDY);
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0x01);
    ADS1256_Write_CS_H;
}


/****************************************
功能:连续读数据命令
*****************************************/
void ADS1256_RDATAC(void)
{
    while(ADS1256_Read_DRDY);
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0x03);
    ADS1256_Write_CS_H;
}


/****************************************
功能:停止连续读数据命令
*****************************************/
void ADS1256_SDATAC(void)
{
    while(ADS1256_Read_DRDY);
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0x0F);
    ADS1256_Write_CS_H;
}


/****************************************
功能:补偿和增益自我校准命令
*****************************************/
void ADS1256_SELFCAL(void)
{
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0xF0);
    ADS1256_Write_CS_H;
}


/****************************************
功能:补偿自我校准
*****************************************/
void ADS1256_SELFOCAL(void)
{
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0xF1);
    ADS1256_Write_CS_H;
}


/****************************************
功能:增益自我校准
*****************************************/
void ADS1256_SELFGCAL(void)
{
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0xF2);
    ADS1256_Write_CS_H;
}


/****************************************
功能:系统补偿校准
*****************************************/
void ADS1256_SYSOCAL(void)
{
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0xF3);
    ADS1256_Write_CS_H;
}


/****************************************
功能:系统增益校准
*****************************************/
void ADS1256_SYSGCAL(void)
{
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0xF4);
    ADS1256_Write_CS_H;
}


/****************************************
功能:AD转换同步
*****************************************/
void ADS1256_SYNC(void)
{
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0xFC);
    ADS1256_Write_CS_H;
}


/****************************************
功能:启动待机模式
*****************************************/
void ADS1256_ATANDBY(void)
{
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0xFD);
    ADS1256_Write_CS_H;
}


/****************************************
功能:系统复位
*****************************************/
void ADS1256_RESET(void)
{
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0xFE);
    ADS1256_Write_CS_H;
}


/****************************************
功能:退出待机模式
*****************************************/
void ADS1256_WAKEUP(void)
{
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0xFF);
    ADS1256_Write_CS_H;
}

/****************************************
//功能:单次读取时，读取一次ADC数据
*****************************************/
uint32_t ADS1256_OneRead_AdcData(void)
{
    uint32_t Data,Data1,Data2,Data3;
    ADS1256_Write_CS_L;
    ADS1256_write_bit(0x01);
    HAL_Delay(1);
    Data1 = (uint32_t)ADS1256_read_bit();
    Data2 = (uint32_t)ADS1256_read_bit();
    Data3 = (uint32_t)ADS1256_read_bit();
    ADS1256_Write_CS_H;
    Data = (Data1<<16) | (Data2<<8) | Data3;
    return (Data);
}
/****************************************
//功能:连续读取时，读取一次ADC数据
*****************************************/
uint32_t ADS1256_ContinuousRead_AdcData(void)
{
    uint32_t Data,Data1,Data2,Data3;
    ADS1256_Write_CS_L;
    Data1 = (uint32_t)ADS1256_read_bit();
    Data2 = (uint32_t)ADS1256_read_bit();
    Data3 = (uint32_t)ADS1256_read_bit();
    ADS1256_Write_CS_H;
    Data = (Data1<<16) | (Data2<<8) | Data3;
    return (Data);
}
/****************************************
功能：ADS1256写寄存器
说明：根据要求写入寄存器和命令字
*****************************************/
//void ADS1256_write_reg(uint8_t ADS1256_command,uint8_t *WriteBuf,uint8_t len)
//{
//    ADS1256_Write_CS_L;
//    ADS1256_write_bit(ADS1256_command | 0x50);
//    ADS1256_write_bit(len-1);
//    for(int i=0;i<len;i++)
//    {
//        ADS1256_write_bit(WriteBuf[i]);
//    }
//    ADS1256_Write_CS_H;
//}
void ADS1256_write_reg(uint8_t reg_addr, uint8_t *WriteBuf, uint8_t len)
{
    uint8_t cmd[2];
    cmd[0] = 0x50 | (reg_addr & 0x0F);
    cmd[1] = len - 1;

    ADS1256_Write_CS_L;

    HAL_SPI_Transmit(&hspi1, cmd, 2, HAL_MAX_DELAY);      // 发送命令和长度
    HAL_SPI_Transmit(&hspi1, WriteBuf, len, HAL_MAX_DELAY); // 发送寄存器数据

    ADS1256_Write_CS_H;
}


/****************************************
功能：ADS1256读寄存器
说明：根据要求写入寄存器地址
*****************************************/
//void ADS1256_read_reg(uint8_t ADS1256_command,uint8_t *ReadBuf,uint8_t len)
//{
//    ADS1256_Write_CS_L;
//    ADS1256_write_bit(ADS1256_command | 0x10);
//    ADS1256_write_bit(len-1);
////    HAL_Delay(10);//最少等待50个ADC时钟周期
//    for(volatile int i=0; i<50; i++);
//    for(int i=0;i<len;i++)
//    {
//        ReadBuf[i]=ADS1256_read_bit();
//    }
//    ADS1256_Write_CS_H;
//}
void ADS1256_read_reg(uint8_t reg_addr, uint8_t *ReadBuf, uint8_t len)
{
    uint8_t cmd[2];
    cmd[0] = 0x10 | (reg_addr & 0x0F);
    cmd[1] = len - 1;

    ADS1256_Write_CS_L;

    // 发送读寄存器命令 + 读取长度
    HAL_SPI_Transmit(&hspi1, cmd, 2, HAL_MAX_DELAY);

    // t6 等待至少 50 SPI 时钟周期
    // 对于 1MHz SPI，50个周期=50us；这里可以用简单延时
    for(volatile int i=0; i<50; i++); // 或使用 delay_us(1)50次

    // 读取寄存器数据
    HAL_SPI_Receive(&hspi1, ReadBuf, len, HAL_MAX_DELAY);

    ADS1256_Write_CS_H;
}



/****************************************
功能：寄存器设置初始化,如果初始化成功返回0，失败返回1
*****************************************/
uint8_t ADS1256_config(ADC_ConfigTypeDef AdcConfig)
{
    uint8_t ReadBuf[5]={0};
    uint8_t WriteBuf[5]={0};

    //状态寄存器初始化---------------------------------------------------
    if(AdcConfig.AinBuf!=0)    WriteBuf[0]|=0x02;
    //模拟多路选择器初始化---------------------------------------------------
    WriteBuf[1]|=(AdcConfig.AIN_P<<4);
    WriteBuf[1]|=(AdcConfig.AIN_N);
    //AD控制寄存器初始化---------------------------------------------------
    WriteBuf[2]|=(AdcConfig.ClockOut<<5);
    WriteBuf[2]|=(AdcConfig.SenserTestCurrent<<3);
    WriteBuf[2]|=(AdcConfig.PGA);
    //数据速度寄存器初始化---------------------------------------------------
    WriteBuf[3]=AdcConfig.SPS;
    //I/O控制寄存器初始化---------------------------------------------------
    WriteBuf[4]=0x00;

    ADS1256_write_reg(0x00,WriteBuf,5);
    ADS1256_read_reg(0x00,ReadBuf,5);
    for (int i = 0; i < 5; ++i) {

        printf("%X-",WriteBuf[i]);
        printf("%X,",ReadBuf[i]);
    }
    printf("\n");
    //寄存器配置验证---------------------------------------------------
// STATUS 寄存器：只验证 BUF 和 ACAL
    if ((ReadBuf[0] & 0x06) != (WriteBuf[0] & 0x06)) return 1;

// MUX、ADCON、DRATE 全验证
    if (ReadBuf[1] != WriteBuf[1]) return 2;
    if (ReadBuf[2] != WriteBuf[2]) return 3;
    if (ReadBuf[3] != WriteBuf[3]) return 4;

// IO 寄存器：掩码可写位
    if ((ReadBuf[4] & 0xFE) != (WriteBuf[4] & 0xFE)) return 5;


    return 0;
}

/****************************************
功能：执行一次ADC内部增益和补偿校准
*****************************************/
void ADS1256_AdjSELF(void)
{
    ADS1256_WAKEUP();HAL_Delay(15); while(ADS1256_DRDY()!=0);//唤醒设备
    ADS1256_SDATAC();HAL_Delay(15); while(ADS1256_DRDY()!=0);	//取消连续读取数据
    ADS1256_SELFCAL();HAL_Delay(15); while(ADS1256_DRDY()!=0);//功能命令:补偿和增益自我校准命令
}

/****************************************
功能：执行一次系统增益和补偿校准，调用该函数时需要保证系统输入为零
*****************************************/
void ADS1256_AdjSYS(void)
{
    ADS1256_WAKEUP(); HAL_Delay(15); while(ADS1256_DRDY()!=0);//唤醒设备
    ADS1256_SDATAC(); HAL_Delay(15); while(ADS1256_DRDY()!=0);	//取消连续读取数据
    ADS1256_SELFCAL();HAL_Delay(15); while(ADS1256_DRDY()!=0);//功能命令:补偿和增益自我校准命令
    ADS1256_SYSOCAL();HAL_Delay(15); while(ADS1256_DRDY()!=0);//功能命令:系统补偿校准
    ADS1256_SYSGCAL();HAL_Delay(15); while(ADS1256_DRDY()!=0);//功能命令:系统增益校准
}

/****************************************
//功能:把读数转化成电压值,输入分别为 ： 读回的二进制值   参考电压   内置增益
*****************************************/
double ADS1256_DataFormatting(uint32_t Data , double Vref ,uint8_t PGA)
{
    /*
    电压计算公式；
            设：AD采样的电压为Vin ,AD采样二进制值为X，参考电压为 Vr ,内部集成运放增益为G
            Vin = ( (2*Vr) / G ) * ( x / (2^23 -1))
    */
    double ReadVoltage;
    if(Data & 0x00800000)
    {
        Data = (~Data) & 0x00FFFFFF;
        ReadVoltage = -(((double)Data) / 8388607) * ((2*Vref) / ((double)PGA));
    }
    else
        ReadVoltage =  (((double)Data) / 8388607) * ((2*Vref) / ((double)PGA));

    return(ReadVoltage);
}








