#include "lab05_timer.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim6;

static void UART_ReadLine(char *buffer, uint16_t size);


/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB5_1(void)
*
*   DESCRIPTION:
*       Enter Hour / Min / Sec through UART and use TIM1 interrupt
*       to update the time every second.
*       e.g. 10:30:55 -> 10:30:56 -> 10:30:57
*
*********************************************************************/
static volatile int32_t hour = 0;
static volatile int32_t minute = 0;
static volatile int32_t second = 0;
static volatile uint8_t timerUpdate = 0;


void LAB5_1(void)
{
char input[32];
char message[64];

char text1[]="Hour(0~23): ";
HAL_UART_Transmit(&huart1, (uint8_t *)text1, strlen(text1), HAL_MAX_DELAY);
UART_ReadLine(input, sizeof(input));
hour = (int32_t)strtol(input, NULL, 10);

char text2[]="Minute(0~59): ";
HAL_UART_Transmit(&huart1, (uint8_t *)text2, strlen(text2), HAL_MAX_DELAY);
UART_ReadLine(input, sizeof(input));
minute = (int32_t)strtol(input, NULL, 10);

char text3[]="Second(0~59): ";
HAL_UART_Transmit(&huart1, (uint8_t *)text3, strlen(text3), HAL_MAX_DELAY);
UART_ReadLine(input, sizeof(input));
second = (int32_t)strtol(input, NULL, 10);

	if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59)
		{
		char error[] = "Invalid time.\r\n";
		HAL_UART_Transmit(&huart1, (uint8_t *)error, strlen(error), HAL_MAX_DELAY);
		return;
		}
snprintf(message, sizeof(message), "Timer Start: %02ld:%02ld:%02ld\r\n", (long)hour, (long)minute, (long)second);
HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
__HAL_TIM_SET_COUNTER(&htim1, 0);
__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
timerUpdate = 0;
HAL_TIM_Base_Start_IT(&htim1);

	while(1)
		{
		if(timerUpdate==1)
			{
			timerUpdate = 0;
			snprintf(message, sizeof(message), "%02ld:%02ld:%02ld\r\n", (long)hour, (long)minute, (long)second);
			HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
			}

		}

}


/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB5_2(void)
*
*   DESCRIPTION:
*       Use Timer interrupt to generate software PWM for the onboard LED.
*       Duty cycle changes from 1% to 100%, then from 100% to 1%
*       to create a breathing light effect.
*
*********************************************************************/
static volatile uint8_t pwmCounter = 0;
static volatile uint8_t pwmDuty = 1;


void LAB5_2(void)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

    pwmCounter = 0;
    pwmDuty = 1;

    HAL_TIM_Base_Start_IT(&htim6);

    while (1)
    {
        /* Dark -> Bright */
        for (int32_t duty = 1; duty <= 19; duty++)
        {
            pwmDuty = duty;
            HAL_Delay(150);
        }

        /* Bright -> Dark */
        for (int32_t duty = 19; duty >= 1; duty--)
        {
            pwmDuty = duty;
            HAL_Delay(150);
        }
    }
}

/*********************************************************************
 *
 * TOOL
 *
 *
*********************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        second++;

        if (second >= 60)
        {
            second = 0;
            minute++;
        }

        if (minute >= 60)
        {
            minute = 0;
            hour++;
        }

        if (hour >= 24)
        {
            hour = 0;
        }

        timerUpdate = 1;
    }

    if (htim->Instance == TIM6)
    {
        pwmCounter++;

        if (pwmCounter >= 20)
        {
            pwmCounter = 0;
        }

        if (pwmCounter < pwmDuty)
        {
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
        }
    }
}

static void UART_ReadLine(char *buffer, uint16_t size)
{
    uint16_t index = 0;
    uint8_t ch;

    while (1)
    {
        HAL_UART_Receive(&huart1, &ch, 1, HAL_MAX_DELAY);

        if (ch == '\r' || ch == '\n')
        {
            if (index == 0)
            {
                continue;
            }

            buffer[index] = '\0';

            char newline[] = "\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t *)newline, strlen(newline), HAL_MAX_DELAY);

            break;
        }

        if (index < size - 1)
        {
            buffer[index] = ch;
            index++;

            HAL_UART_Transmit(&huart1, &ch, 1, HAL_MAX_DELAY);
        }
    }
}
