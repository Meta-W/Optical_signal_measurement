/* ADS1256 24?ADC???? - STM32 HAL??? */


#include "string.h"
#include "stdio.h"
#include "tim.h"
#include "usart.h"   // 包含 USART 句柄定义（如 huart1）
#include "ads1256/ads125x.h"
#include "../usr/KEY/key.h"
#include "../usr/app_main.h"


void key_event_handler(uint8_t key_id, key_event_t event);
#define ADC_MAX_NUM 2*36 //3组ADC,每组最多存储5个值
uint16_t ADC_Values[ADC_MAX_NUM]={0};
uint16_t adc_value_flg=0;
uint16_t ADC_Switch=0;
static uint8_t key_0;
static uint8_t key_1;
static uint8_t key1_click;
uint16_t pulse_width[30];
static uint8_t pul_index=0;

uint32_t adc_index[6];

enum {IDE,MEASURE,PRINT,STOP,ADC_TEST};


ADS125X_t adc1;

int ads_test(void)
{
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
    ADS125X_Init(&adc1, &hspi1, ADS125X_DRATE_10SPS, ADS125X_PGA1, 0);

    printf("...done\n");
    /* USER CODE END 2 */


    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
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

        HAL_Delay(100);
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}


/**
 * @brief ???
 */
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
//    HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);
//    HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
//    HAL_ADC_Start_IT(&hadc1);
    HAL_Delay(1000);
    // ADS1256_Init();
    ads_test();
    uint8_t state=IDE;
    /* ??? */
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

                break;
            case MEASURE:
                HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);
                HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
                if(pul_index%30==0)
                    state=PRINT;
                break;
//                HAL_Delay(1000);
            case PRINT:

                for(uint8_t i=0;i<30;i++)
                {

                    data_high = pulse_width[i]>>8;
                    data_low = pulse_width[i]&0x00ff;
                    sprintf(data_print,"%d:%d\r\n",i,pulse_width[i]);
                    HAL_UART_Transmit(&huart1,data_print,sizeof(data_print),100);
//                    HAL_UART_Transmit(&huart1,data_low,1,100);

                }
                state = STOP;
                break;
            case STOP:
                HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_1);
                HAL_TIM_IC_Stop_IT(&htim2,TIM_CHANNEL_2);
                state = ADC_TEST;
                break;
            case ADC_TEST:
                for (uint8_t i=0 ;i<6;i++){
                    printf("%d,%d\r\n",i,adc_index[i]);
                }
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
        }

    }
}
/**
 * @brief GPIO????????
 * @param GPIO_Pin ???????
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_0)
    {
        // ????DRDY????
        GPIO_PinState pin_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);

        if(pin_state == GPIO_PIN_RESET)
        {
            // ???????????????
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
            // ????????????????
            // ???????????????????
        }
        else
        {
            // ???????????????
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);

            // ??????????????
        }
    }
}
uint32_t adc_index_value=0;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

    static uint16_t t = 0;
    static uint16_t d = 0;
    static uint8_t adc_index_index=0;
    if(htim->Instance == TIM2)
    {

        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
//            LEDDT[0]=1;
            t = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
//            HAL_UART_Transmit(&huart1,(uint8_t *)&d,2,100);
            pulse_width[pul_index%30]=t-d;
            pul_index++;
//            freq = 1000000 / t;
//            duty = (float)d/t*100;
//            HAL_ADC_Stop_IT(&hadc1);
            ADC_Switch=0;

            adc_index[adc_index_index++]=adc_index_value;
            adc_index_value=0;
        }

        else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
        {
//            LEDDT[1]=2;
            d =  HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2)+1;
//            HAL_ADC_Start_IT(&hadc1);
//            HAL_TIM_Base_Start_IT(&htim11);
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
