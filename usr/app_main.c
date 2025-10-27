/* ADS1256 24?ADC???? - STM32 HAL??? */


#include "string.h"
#include "stdio.h"
#include "tim.h"

#include "../usr/KEY/key.h"
#include "../usr/app_main.h"


void key_event_handler(uint8_t key_id, key_event_t event);

static uint8_t key_0;
static uint8_t key_1;
/**
 * @brief ???
 */
void app_main(void)
{
    
    /* ???ADS1256 */
    // ADS1256_Init();
    key_init();
    key_0 = key_register(KEY_0_GPIO_Port, KEY_0_Pin, true); // PA0??????
    key_1 = key_register(KEY_1_GPIO_Port, KEY_1_Pin, true); // PA0??????
    key_set_callback(key_event_handler);
    HAL_TIM_Base_Start_IT(&htim11);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    /* ??? */
    while(1)
    {
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
        
        HAL_Delay(500);  // ??500ms
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
            // DOWN??????LED2??
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
        // ms++;/*????*/
        if (ms % 1000 == 0) /*??1s???????*/
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