/* ADS1256 24?ADC???? - STM32 HAL??? */


#include "string.h"
#include "stdio.h"
#include "tim.h"
#include "usart.h"   // 包含 USART 句柄定义（如 huart1）
#include "ads1256/ads125x.h"
#include "../usr/KEY/key.h"
#include "../usr/app_main.h"

#include <sys/stat.h>

#include "sdcard/sd_functions.h"
#include "sdcard/sd_benchmark.h"
#include "fatfs.h"


void key_event_handler(uint8_t key_id, key_event_t event);
#define ADC_MAX_NUM 2*36 //3组ADC,每组最多存储5个值
#define  ADS1256_CS_LOW() HAL_GPIO_WritePin(ADS1256_CS_GPIO_Port,ADS1256_CS_Pin,0); //3组ADC,每组最多存储5个值
#define  ADS1256_CS_HIGH() HAL_GPIO_WritePin(ADS1256_CS_GPIO_Port,ADS1256_CS_Pin,1); //3组ADC,每组最多存储5个值
uint16_t ADC_Values[ADC_MAX_NUM]={0};
uint16_t adc_value_flg=0;
uint16_t ADC_Switch=0;
static uint8_t key_0;
static uint8_t key_1;
static uint8_t key1_click;
static uint8_t key2_click;
uint16_t pulse_width_arr[30];
static uint8_t pul_index=0;

uint32_t adc_index[6];

enum {IDE,ADC_GET,MEASURE,PRINT,AD_PRINT,STOP,ADC_TEST};
uint8_t ADS1256_RxBuf[3];      // 存储原始 24bit 数据
int32_t ADS1256_RawData = 0;   // 转换后的有符号整数
float ADS1256_Voltage = 0;     // 转换后的电压
uint8_t adc_get_flag;
uint8_t adc_get_ch=0;
//fliter
typedef struct {
    int32_t last_raw;   // 上次原始值
    int32_t filtered;     // 滤波输出（浮点或整形均可）
    uint8_t init_flag;  // 是否初始化过
} ADS_Filter_t;

#define SPIKE_THRESHOLD   100000     // 尖峰检测阈值，具体看你的信号幅度
#define ALPHA             0.01f      // EMA 平滑系数 (0~1)，越大越灵敏


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
        // printf("PB0 EXTI triggered!\n");
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        ADS1256_CS_LOW();
        // 启动 DMA 从 SPI 读取 3 字节数据
        // HAL_SPI_Receive_DMA(&hspi1, ADS1256_RxBuf, 3);
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        // ADS1256_CS_HIGH();

        ADS1256_RawData = ((int32_t)ADS1256_RxBuf[0] << 16) |
                          ((int32_t)ADS1256_RxBuf[1] << 8)  |
                          ((int32_t)ADS1256_RxBuf[2]);

        // 24bit 转换为有符号
        if (ADS1256_RawData & 0x800000)
            ADS1256_RawData |= 0xFF000000;

        ADS1256_Voltage = (float)ADS1256_RawData / 8388607.0f * 2.5f; // Vref=2.5V

        printf("ADC=%ld, V=%.6fV\r\n", ADS1256_RawData, ADS1256_Voltage);
        HAL_SPI_Receive_IT(&hspi1, ADS1256_RxBuf, 3);

    }
}
ADS125X_t adc1;
char SD_FileName[] = "hello.txt";
uint8_t WriteBuffer[] = "01 write buff to sd \r\n";
uint8_t write_cnt =0;	//写SD卡次数

uint8_t bufr[80];
UINT br;

void adc_init(void)
{
    uint8_t ret;
    HAL_TIM_Base_Start(&htim1);
    adc1.csPort = ADS1256_CS_GPIO_Port;
    adc1.csPin = ADS1256_CS_Pin;
    adc1.drdyPort = ADS1256_DRDY_GPIO_Port;
    adc1.drdyPin = ADS1256_DRDY_Pin;
    adc1.rstPort = GPIOC;
    adc1.rstPin = GPIO_PIN_13;
    adc1.vref = 2.5f;
    adc1.oscFreq = ADS125X_OSC_FREQ;

    printf("\n");
    printf("ADC config...\n");
    //When we start the ADS1256, the preconfiguration already sets the MUX to [AIN0+AINCOM].
    // 10 SPS / PGA=1 / buffer off
    do
    {
    ret = ADS125X_Init(&adc1, &hspi1, ADS125X_DRATE_7500SPS, ADS125X_PGA1, 0);

    }
    while (ret != 0);

}
int ads_test(void)
{
    HAL_TIM_Base_Start(&htim1);
    adc1.csPort = ADS1256_CS_GPIO_Port;
    adc1.csPin = ADS1256_CS_Pin;
    adc1.drdyPort = ADS1256_DRDY_GPIO_Port;
    adc1.drdyPin = ADS1256_DRDY_Pin;
    adc1.rstPort = GPIOC;
    adc1.rstPin = GPIO_PIN_13;
    adc1.vref = 2.499505f;
    adc1.oscFreq = ADS125X_OSC_FREQ;

    printf("\n");
    printf("ADC config...\n");
    //When we start the ADS1256, the preconfiguration already sets the MUX to [AIN0+AINCOM].
    // 10 SPS / PGA=1 / buffer off
    ADS125X_Init(&adc1, &hspi1, ADS125X_DRATE_7500SPS, ADS125X_PGA1, 0);

    printf("...done\n");
    ADS125X_Channel_Set(&adc1, ADS125X_MUXP_AIN0);
    ADS125X_Channel_Set(&adc1, ADS125X_MUXP_AIN0);
    ADS125X_Channel_Set(&adc1, ADS125X_MUXP_AIN0);
	// ADS125X_CMD_Send(&adc1, ADS125X_CMD_RDATAC);
    // HAL_SPI_Receive_IT(&hspi1, ADS1256_RxBuf, 3);

    // HAL_NVIC_SetPriority(EXTI0_IRQn,0,0);
    // HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    while (1) {

        //you can use this way
        float volt[2] = { 0.0f, 0.0f };
        ADS125X_Channel_Set(&adc1, ADS125X_MUXP_AIN0);
        volt[0] = ADS125X_ADC_ReadVolt(&adc1);
        printf("%.15f\n", volt[0]);

        // ADS125X_Channel_Set(&adc1, ADS125X_MUXP_AIN1);
        // ADS125X_ChannelDiff_Set(&adc1, ADS125X_MUXP_AIN0, ADS125X_MUXN_AIN1);
        // volt[1] = ADS125X_ADC_ReadVolt(&adc1);
        // printf("%.15f\n", volt[1]);


        //or this way
    //  ADS125X_DRDY_Wait(&adc1);
    //  ADS125X_cycle_Through_Channels(&adc1, volt)

        // HAL_Delay(100);
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
    extern Disk_drvTypeDef disk;



FIL file;
FRESULT res;
UINT bw;

void write_csv_example(void)
{

    // 打开 CSV 文件，若存在则覆盖
    res = f_open(&file, "0:/data.csv", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) { printf("f_open failed: %d\r\n", res); return; }

    // 写入 CSV 表头
    char header[] = "Time(ms),Temperature(C),Voltage(V)\r\n";
    f_write(&file, header, strlen(header), &bw);

    // 写入多行数据
    for (int i = 0; i < 10; i++)
    {
        char line[64];
        int time_ms = i*100;
        float temp = 25.0 + i*0.5;
        float volt = 3.3 - i*0.05;

        // 格式化成 CSV 行
        sprintf(line, "%d,%.2f,%.2f\r\n", time_ms, temp, volt);

        // 写入文件
        f_write(&file, line, strlen(line), &bw);
    }

    // 关闭文件
    f_close(&file);
    printf("CSV write finished!\r\n");
}
void csv_init(void)
{

    // 打开 CSV 文件，若存在则覆盖
    res = f_open(&file, "0:/data.csv", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) { printf("f_open failed: %d\r\n", res); return; }

    // 写入 CSV 表头
    char header[] = "Time(ms),Temperature(C),Voltage(V)\r\n";
    f_write(&file, header, strlen(header), &bw);

    // 写入多行数据
    for (int i = 0; i < 10; i++)
    {
        char line[64];
        int time_ms = i*100;
        float temp = 25.0 + i*0.5;
        float volt = 3.3 - i*0.05;

        // 格式化成 CSV 行
        sprintf(line, "%d,%.2f,%.2f\r\n", time_ms, temp, volt);

        // 写入文件
        f_write(&file, line, strlen(line), &bw);
    }

    // 关闭文件
    f_close(&file);
    printf("CSV write finished!\r\n");
}
int sd_test(void)
{
    // memset(&disk, 0, sizeof(disk));
    // sd_benchmark();

#define max_records 20
    CsvRecord myrecords[max_records];
    CsvRecord1 myrecords1[max_records];
    int record_count = 0;
    sd_mount();
    // sd_list_files();
    // sd_read_csv_ver1("0:/test.csv", myrecords1, max_records, &record_count);
    write_csv_example();
    sd_unmount();

    // sd_mount();
    // sd_list_files();
    // sd_append_file("File6.txt", "This is Appended Text\n");
    // sd_read_file("File6.txt", bufr, 80, &br);
    // printf("DATA from File:::: %s\n\n",bufr);
    // sd_unmount();
    while (1) ;
}

typedef struct {
    uint16_t value;
    int index;
} MinResult;

MinResult find_min(uint16_t *arr, int len) {
    MinResult result = {arr[0], 0};

    for (int i = 1; i < len; i++) {
        if (arr[i] < result.value) {
            result.value = arr[i];
            result.index = i;
        }
    }

    return result;
}

int32_t ADS1256_Filter_Update_Int(ADS_Filter_t *f, int32_t raw)
{
    if (!f->init_flag) {
        f->filtered = raw;
        f->last_raw = raw;
        f->init_flag = 1;
        return (int32_t)f->filtered;
    }

    int32_t diff = abs(raw - f->last_raw);
    // printf("d%d\n",diff);
    if (diff > SPIKE_THRESHOLD)
        raw = f->last_raw;

    // 定点低通滤波：ALPHA = 1/8
    // f->filtered = f->filtered + ((raw - f->filtered) >> 4);

    f->filtered = raw;
    f->last_raw = raw;
    return (int32_t)f->filtered;
}

/**
 * @brief   从 ADS1256 读取 N 次 ADC，并求平均
 * @param   sample_count  采样次数
 * @return  平均 ADC 值（浮点）
 */
double ADS1256_AverageFromArray(int32_t *adc_array, uint8_t length)
{
    if (adc_array == NULL || length == 0)
        return 0.0f;

    int64_t sum = 0;
    // for (uint8_t i = 0; i < length/4; i++)
    // {
    //     sum += adc_array[i+length/2];
    // }
    // for (uint8_t i = 0; i < length/4; i--)
    // {
    //     sum += adc_array[i+length/2];
    // }
    for (uint8_t i = 0; i < length; i++)
    {
        sum += adc_array[i];
    }
    double avg_adc = (double)sum / (length);

    // 转换为电压
    double voltage = avg_adc * (2.0f * adc1.vref) / (8388607.0f * adc1.pga);
	// return ((float) adsCode * (2.0f * ads->vref)) / (ads->pga * 8388607.0f); // 0x7fffff = 8388607.0f   //cancel float funsion cannt be faster
    return voltage;
}
double ADS1256_AverageFloat(double *adc_array, uint8_t length)
{
    if (adc_array == NULL || length == 0)
        return 0.0f;

    float sum = 0;
    for (uint8_t i = 0; i < length; i++)
    {
        sum += adc_array[i];
    }

    float avg_volt = (float)sum / length;

    return avg_volt;
}
float volt_buf[100];
uint8_t volt_buf_index = 0;
typedef struct{
    uint8_t index;
    uint8_t channel;
    int32_t value[100];
}AD_rawdata_t;
typedef struct{
    uint8_t index;
    uint8_t channel;
    double value[100];
}AD_rawdata_f_t;
typedef struct{
    uint8_t index;
    uint8_t channel;
    double value[255];
}AD_voltdata_t;
typedef struct{
    uint8_t index;
    double value[255];
}AD_voltdata_op_t;

AD_rawdata_t ad_rawdata;
AD_rawdata_f_t ad_rawdata_f;
AD_voltdata_t ad_voltdata;
AD_voltdata_op_t ad_voltdata_out[6];
uint8_t time_100ms_flag=0;
uint8_t print_flag=0;
void app_main(void)
{
    
    /* ???ADS1256 */
    // ADS1256_Init();
    key_init();
    HAL_GPIO_WritePin(ADS1256_CS_GPIO_Port,ADS1256_CS_Pin,0);
    HAL_UART_Transmit(&huart1,"hello\r\n",7,100);
    key_0 = key_register(KEY_0_GPIO_Port, KEY_0_Pin, true); // PA0??????
    key_1 = key_register(KEY_1_GPIO_Port, KEY_1_Pin, true); // PA0??????
    key_set_callback(key_event_handler);
    HAL_TIM_Base_Start_IT(&htim11);
    HAL_TIM_Base_Start(&htim3);
    HAL_GPIO_WritePin(ADS1256_CS_GPIO_Port,ADS1256_CS_Pin,0);
//    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
//    HAL_ADC_Start_IT(&hadc1);
    // sd_test();

    HAL_Delay(1000);
    // ads_test();
    // ads_test();0x3fc-2.5
    adc_init();
    uint8_t state=IDE;

    uint8_t adc_get_ch_last;
    ad_rawdata.index = 0;
    ad_rawdata.channel = 0;
    uint8_t vofa_sel_ch=0;
    ADS125X_Channel_Set(&adc1, ADS125X_MUXP_AIN0);
    ADS125X_Channel_Set(&adc1, ADS125X_MUXP_AIN0);
    ADS125X_Channel_Set(&adc1, ADS125X_MUXP_AIN0);

    printf("enter while\n");
    __volatile__ double volt_avg;
    __volatile__ float volt;
    uint8_t vofa_wave_index = 0;
    ADS_Filter_t f;

    while(1)
    {

        uint8_t data_high;
        uint8_t data_low;
        uint8_t data_print[20];

        switch (state) {
            case IDE:

                if (key1_click==1)
                {
                    key1_click=0;
                    state=MEASURE;
                }
                if (key2_click==1)
                {
                    key2_click=0;
                    vofa_sel_ch++;
                    if (vofa_sel_ch==6)
                        vofa_sel_ch=0;
                }
                if (ADC_Switch)
                {
                    state=ADC_GET;
                }
                if (time_100ms_flag&0)
                {
                    time_100ms_flag=0;
                    if (ad_voltdata.index>0)
                    {
                        state=AD_PRINT;
                        // printf("i%d,av%lf\n",ad_voltdata.index);

                    }

                }
                break;
            case ADC_GET:
                {
                    f.init_flag=0;
                    if (adc_get_ch!=adc_get_ch_last)
                    {
                        ADS125X_Channel_Set(&adc1, ADS125X_MUXP_AIN0);
                        adc_get_ch_last=adc_get_ch;
                    }
                    // volt = ADS125X_ADC_ReadVolt(&adc1);
                    // ad_rawdata_f.value[ad_rawdata.index++] =  volt;

                    ad_rawdata.value[ad_rawdata.index++] =  ADS1256_Filter_Update_Int(&f,ADS125X_ADC_ReadRaw(&adc1));
                    // printf("%d\n",ad_rawdata.value[ad_rawdata.index]);
                    // ad_rawdata.value[ad_rawdata.index++] = ADS125X_ADC_ReadRaw(&adc1);
                    // printf("Voltage: %lf\r\n", volt);
                    // printf("%d\n",ad_rawdata.value[ad_rawdata.index]);
                    // if (ad_rawdata.channel==5)
                    //     printf("\n");
                    // else
                    //     printf(",");
                    if (ADC_Switch==0)
                    {
                        volt_avg=0;
                        // volt_avg = ADS1256_AverageFloat(ad_rawdata_f.value, ad_rawdata.index);
                        volt_avg = ADS1256_AverageFromArray(ad_rawdata.value, ad_rawdata.index);
                        // ad_voltdata.value[ad_voltdata.index++] = volt_avg;
                        // ad_voltdata.channel = ad_rawdata.channel;

                        // ad_voltdata_out[ad_rawdata.channel].value[ad_voltdata_out[ad_rawdata.channel].index++] = volt_avg;
                    // printf("i%d,v%lf\n",ad_rawdata.channel,volt_avg);
                        printf("%lf",volt_avg);
                        if (ad_rawdata.channel==6)
                            printf("\n");
                        else
                            printf(",");

// #define VOFA
#ifdef VOFA
                        if (vofa_wave_index==ad_rawdata.channel)
                        {
                            printf("%lf",ad_rawdata.channel,volt_avg);
                            vofa_wave_index++;
                            if (vofa_wave_index==7)
                            {
                                vofa_wave_index=0;
                                printf("\n");
                            }
                            else
                                printf(",");
                        }
#else
                        // printf("i%d,v%lf\n",ad_rawdata.channel,volt_avg);
                        // printf("i%d,v%lf\n",ad_rawdata.index,volt_avg);
#endif

                        // HAL_Delay(1);
                        ad_rawdata.index = 0;
                        state=IDE;

                    }
                    break;
                }
            case MEASURE:
                HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);
                HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
                if(print_flag)
                {
                    state=PRINT;
                    // HAL_Delay(100);
                }
                break;
            case PRINT:
                print_flag=0;
                for(uint8_t i=0;i<7;i++)
                {
                    printf("%d:%d\n",i,pulse_width_arr[i]);

//                    HAL_UART_Transmit(&huart1,data_low,1,100);

                }
                state = STOP;
                break;
            case AD_PRINT:
                __volatile double volt[7];
                volt[0]=ADS1256_AverageFloat(ad_voltdata_out[0].value, ad_voltdata_out[0].index);
                volt[1]=ADS1256_AverageFloat(ad_voltdata_out[1].value, ad_voltdata_out[1].index);
                volt[2]=ADS1256_AverageFloat(ad_voltdata_out[2].value, ad_voltdata_out[2].index);
                volt[3]=ADS1256_AverageFloat(ad_voltdata_out[3].value, ad_voltdata_out[3].index);
                volt[4]=ADS1256_AverageFloat(ad_voltdata_out[4].value, ad_voltdata_out[4].index);
                volt[5]=ADS1256_AverageFloat(ad_voltdata_out[5].value, ad_voltdata_out[5].index);
                volt[6]=ADS1256_AverageFloat(ad_voltdata_out[6].value, ad_voltdata_out[6].index);
                // printf("i%d,av%lf\n",ad_voltdata.index,volt);
                // printf("av:%lf\n",volt);
                for(uint8_t i=0;i<7;i++)
                {
                    printf("%lf",volt[i]);
                    ad_voltdata_out[i].index=0;
                    if (i<6)
                        printf(",");
                    else
                        printf("\n");
                }
                ad_voltdata.index = 0;

                state = IDE;
                break;
            case STOP:
            printf("ic stop\n");
                HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_1);
                HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_2);
                state = ADC_TEST;
                break;
            case ADC_TEST:

                state = IDE;

                break;
            default:
                break;

        }
        // // ????0: AIN0 - AIN1
        // a = POSITIVE_AIN0 + NEGTIVE_AIN1;
        // Write_Reg_Mux(a);
        // Send_Data_ASCII(0);
        //
        // // ????1: AIN2 - AIN3
        // a = POSITIVE_AIN2 + NEGTIVE_AIN3;
        // Write_Reg_Mux(a);
        // Send_Data_ASCII(1);
        //
        // // ????2: AIN4 - AIN5
        // a = POSITIVE_AIN4 + NEGTIVE_AIN5;
        // Write_Reg_Mux(a);
        // Send_Data_ASCII(2);
        //
        // // ????3: AIN6 - AIN7
        // a = POSITIVE_AIN6 + NEGTIVE_AIN7;
        // Write_Reg_Mux(a);
        // Send_Data_ASCII(3);
        
//        HAL_Delay(1000);  // ??500ms
    }
}

/**
 * @brief UART????????
 * @param huart UART??
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        // ????????
        // dat????????????
    }
}

/**
 * @brief ????????
 * @param key_id ??ID
 * @param event ????
 */
void key_event_handler(uint8_t key_id, key_event_t event)
{
    // ????ID????????????
    uint8_t data_uart[] = {0x01, 0x02, 0x03};
    // printf("key_en");

    // protocol_send_packet(0xAA, 0x10, data_uart, 3, 0x55);
    if (key_id == key_0)
    {
        switch (event)
        {
        case KEY_EVENT_CLICK:
            key1_click=1;
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            break;
        case KEY_EVENT_LONG_PRESS:
            break;

        default:
            break;
        }
    }
    else if (key_id == key_1)
    {
        switch (event)
        {
        case KEY_EVENT_CLICK:
            key2_click=1;

            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            break;
        case KEY_EVENT_LONG_PRESS:
        default:
            break;
        }
    }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint32_t ms=0;
    if (htim == &htim11) /*?????*/
    {

        key_systick_handler();
         ms++;/*????*/
        if (ms % 500 == 0) /*??1s???????*/
        {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); /*??LED*/
            /*???????????1s*/
        }else if (ms % 5 ==0)
        {
            adc_get_flag^=1;
        }
        else if (ms % 201==0)
        {

            time_100ms_flag=1;
        }

    }
}

uint32_t adc_index_value=0;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

    static uint16_t t = 0;
    static uint16_t last_pulse_width = 0;
    static uint16_t pulse_width = 0;
    static uint16_t d = 0;
    static uint16_t cnt = 0;
    int16_t tmp = 0;
    uint16_t tmp1 = 0;
    static MinResult min ;
    if(htim->Instance == TIM2)
    {

        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            if (cnt<100)
                cnt++;
            t = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
            pulse_width=t-d;
            // pul_index++;
            pulse_width_arr[pul_index++]=pulse_width;
            // if (pul_index==7)pul_index=0;
            // printf("%d\n",pulse_width);

            tmp =((last_pulse_width-pulse_width))*100/last_pulse_width;
            if (tmp>40)
            {
                // printf("%d\n",tmp);
                // printf("%d\n",cnt);
                // printf("%d,%d\n",last_pulse_width,pulse_width);
                pul_index=0;
            }

            if (pul_index==6)
                print_flag=1;

            last_pulse_width = pulse_width;
            //open adc sample
            if (cnt==100)
                ADC_Switch=0;
            ad_rawdata.channel=pul_index;

        }

        else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
        {
//            LEDDT[1]=2;
            d =  HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2)+1;
//            HAL_ADC_Start_IT(&hadc1);
//            HAL_TIM_Base_Start_IT(&htim11);
            if (cnt==100)
                ADC_Switch=1;

//            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
//            HAL_UART_Transmit(&huart1,(uint8_t *)&d,2,100);
        }


    }
}

//ADC转换完成自动调用函数



// 重定向 printf 到串口
int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)&ptr[i], 1, HAL_MAX_DELAY);
    }
    return len;
}
