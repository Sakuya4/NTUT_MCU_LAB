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
*       Use the TIM6 counter to generate software PWM for the onboard LED.
*       Duty cycle changes from 0% to 100%, then from 100% to 0%
*       to create a breathing light effect.
*
*********************************************************************/
void LAB5_2(void)
{
int32_t duty = 0;
int32_t direction = 1;

HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
__HAL_TIM_SET_AUTORELOAD(&htim6, 0xFFFF);
HAL_TIM_Base_Start(&htim6);

    while (1)
		{

			for (uint32_t cycle = 0; cycle < 20; cycle++)
				{
				__HAL_TIM_SET_COUNTER(&htim6, 0);

					if (duty > 0)
						{
						HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
							while (__HAL_TIM_GET_COUNTER(&htim6) < (uint32_t)duty)
								{
								}
						}

				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

					while (__HAL_TIM_GET_COUNTER(&htim6) < 100)
						{
						}
				}

			duty += direction;
				if (duty >= 100 || duty <= 0)
				{
				direction = -direction;
				}
		}
}

/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB5_3_A(void)
*
*   DESCRIPTION:
*       Control the breathing LED speed through UART.
*       Speed level 1 is slow and speed level 5 is fast.
*
*********************************************************************/
void LAB5_3_A(void)
{
char input[32];
char message[64];

int32_t speed;
int32_t duty = 0;
int32_t direction = 1;
uint32_t holdCycle;

char text[] = "Breathing Speed (1 ~ 5): ";
HAL_UART_Transmit(&huart1, (uint8_t *)text, strlen(text), HAL_MAX_DELAY);

UART_ReadLine(input, sizeof(input));
speed = (int32_t)strtol(input, NULL, 10);

	if (speed < 1 || speed > 5)
		{
		char error[] = "Invalid speed.\r\n";
		HAL_UART_Transmit(&huart1, (uint8_t *)error, strlen(error), HAL_MAX_DELAY);
		return;
		}

holdCycle = 60 - (speed * 10);

snprintf(message, sizeof(message), "Speed Level = %ld\r\n", (long)speed);
HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
__HAL_TIM_SET_AUTORELOAD(&htim6, 0xFFFF);
HAL_TIM_Base_Start(&htim6);

    while (1)
    	{
			for (uint32_t cycle = 0; cycle < holdCycle; cycle++)
				{
				__HAL_TIM_SET_COUNTER(&htim6, 0);
				if (duty > 0)
					{
					HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

						while (__HAL_TIM_GET_COUNTER(&htim6) < (uint32_t)duty)
						{
						}

					}

				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

					while (__HAL_TIM_GET_COUNTER(&htim6) < 100)
					{
					}
				}

        duty += direction;
			if (duty >= 100 || duty <= 0)
			{
			direction = -direction;
			}
    	}
}

/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB5_3_B(void)
*
*   DESCRIPTION:
*       Use TIM1 as a stopwatch timer.
*       Short press the user button to Start / Stop.
*       Long press the user button to Reset the stopwatch.
*
*********************************************************************/
static volatile uint32_t stopwatchSeconds = 0;
static volatile uint8_t stopwatchRunning = 0;
static volatile uint8_t stopwatchUpdate = 0;

void LAB5_3_B(void)
{
uint32_t pressStart;
uint32_t pressTime;
uint32_t minute;
uint32_t second;
char message[64];

stopwatchSeconds = 0;
stopwatchRunning = 0;
stopwatchUpdate = 1;

__HAL_TIM_SET_COUNTER(&htim1, 0);
__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
HAL_TIM_Base_Start_IT(&htim1);

char text[] = "Short Press: Start / Stop, Long Press: Reset\r\n";
HAL_UART_Transmit(&huart1, (uint8_t *)text, strlen(text), HAL_MAX_DELAY);

	while(1)
		{
			if(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET)
				{
				HAL_Delay(20);
					if(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET)
						{
						pressStart = HAL_GetTick();
							while(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET)
								{
								}
						pressTime = HAL_GetTick() - pressStart;
						HAL_Delay(20);
							if(pressTime >= 1000)
								{
					            stopwatchRunning = 0;
								stopwatchSeconds = 0;
								stopwatchUpdate = 1;
								char resetText[] = "Reset\r\n";
								HAL_UART_Transmit(&huart1, (uint8_t *)resetText, strlen(resetText), HAL_MAX_DELAY);
								}
							else
								{
									if(stopwatchRunning == 0)
										{
										stopwatchRunning = 1;
										__HAL_TIM_SET_COUNTER(&htim1, 0);
										__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
										char startText[] = "Start\r\n";
										HAL_UART_Transmit(&huart1, (uint8_t *)startText, strlen(startText), HAL_MAX_DELAY);
										}
									else
										{
							            stopwatchRunning = 0;
										char stopText[] = "Stop\r\n";
										HAL_UART_Transmit(&huart1, (uint8_t *)stopText, strlen(stopText), HAL_MAX_DELAY);
										}
								}
						}
				}
			if(stopwatchUpdate == 1)
				{
				stopwatchUpdate = 0;
	            minute = stopwatchSeconds / 60;
	            second = stopwatchSeconds % 60;
	            snprintf(message, sizeof(message), "Stopwatch: %02lu:%02lu\r\n", (unsigned long)minute, (unsigned long)second);
	            HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
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

        if(stopwatchRunning==1)
        {
            stopwatchSeconds++;
            stopwatchUpdate = 1;
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
