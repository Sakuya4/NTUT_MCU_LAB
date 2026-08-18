#include "lab04_adc.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;

/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB4_1(void)
*
*   DESCRIPTION:
*       Read STM32 internal temperature sensor by ADC1 and display
*       ADC value and internal temperature through UART.
*
*********************************************************************/

void LAB4_1(void)
{
int32_t adcValue;
int32_t voltageMv;
int32_t temperature;
char message[80];

	while(1)
		{
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
        adcValue = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        voltageMv = (adcValue * 3300) / 4095;

        temperature = ((voltageMv - 760) * 10 / 25) + 25;

        snprintf(message, sizeof(message),
                 "ADC = %ld, Voltage = %ld mV, Temperature = %ld C\r\n",
                 (long)adcValue,
                 (long)voltageMv,
                 (long)temperature);

        HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);

        HAL_Delay(1000);
		}

}

/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB4_2(void)
*
*   DESCRIPTION:
*       Read the internal temperature sensor using ADC1 and divide
*       the temperature into different ranges to control two LEDs.
*
*********************************************************************/

void LAB4_2(void)
{
int32_t adcValue;
int32_t voltageMv;
int32_t temperature;
char message[80];
	while (1)
	   {
	   HAL_ADC_Start(&hadc1);
	   HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	   adcValue = HAL_ADC_GetValue(&hadc1);
	   HAL_ADC_Stop(&hadc1);
	   voltageMv = (adcValue * 3300) / 4095;
	   temperature = ((voltageMv - 760) * 10 / 25) + 25;
		   if (temperature < 30)
		   	   {
			   HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin|LED2_Pin);
			   HAL_Delay(500);
		   	   }
			   else if (temperature < 35)
			   {
			   HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			   HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
			   }
			   else if (temperature < 40)
			   {
			   HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
			   HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
			   }
			   else
			   {
			   HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			   HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
			   }

		snprintf(message, sizeof(message), "ADC = %ld, Temperature = %ld C\r\n",
		(long)adcValue, (long)temperature);
		HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);

		HAL_Delay(500);
	   }

}
