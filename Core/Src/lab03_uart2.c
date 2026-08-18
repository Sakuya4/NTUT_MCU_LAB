#include "lab03_uart2.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern UART_HandleTypeDef huart1;
static void UART_ReadLine(char *buffer, uint16_t size); // tool
static void QuickSort(int32_t numbers[], int32_t left, int32_t right); //3-3-B will use it, or you can use bubble sort or other


/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB3_1(void)
*
*   DESCRIPTION:
*       user type number a & b, will add it.
*
*********************************************************************/
void LAB3_1(void)
{
    char input[32];
    char message[64];

    int32_t a;
    int32_t b;
    int32_t result;

    while (1)
    {
        char text1[] = "Input a: ";
        HAL_UART_Transmit(&huart1, (uint8_t *)text1, strlen(text1), HAL_MAX_DELAY);

        UART_ReadLine(input, sizeof(input));
        a = (int32_t)strtol(input, NULL, 10);

        char text2[] = "Input b: ";
        HAL_UART_Transmit(&huart1, (uint8_t *)text2, strlen(text2), HAL_MAX_DELAY);

        UART_ReadLine(input, sizeof(input));
        b = (int32_t)strtol(input, NULL, 10);

        result = a + b;

        snprintf(message, sizeof(message), "a + b = %ld\r\n\r\n", (long)result);
        HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
    }
}

/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB3_2_A(void)
*
*   DESCRIPTION:
*       Fibonacci, user type number, it can trans number to Fibonacci number
*       e.g 10 -> 0 1 1 2 3 5 8 13 21 34
*
*********************************************************************/

void LAB3_2_A(void)
{
    char input[32];
    char message[64];

    int32_t n;
    uint32_t a;
    uint32_t b;
    uint32_t next;

    while(1)
    {
    char text[] = "Input Fibonacci count: ";
    HAL_UART_Transmit(&huart1, (uint8_t *)text, strlen(text), HAL_MAX_DELAY);

    UART_ReadLine(input, sizeof(input));
    n = (int32_t)strtol(input, NULL, 10);

    if (n <= 0 || n > 47)
    {
      char error[] = "Please input 1 ~ 47.\r\n\r\n";
      HAL_UART_Transmit(&huart1, (uint8_t *)error, strlen(error), HAL_MAX_DELAY);
      continue;
    }

    // init
    a=0;
    b=1;

    char title[] = "Fibonacci: ";
    HAL_UART_Transmit(&huart1, (uint8_t *)title, strlen(title), HAL_MAX_DELAY);

    for (int32_t i = 0; i < n; i++)
    {
      snprintf(message, sizeof(message), "%lu ", (unsigned long)a);
      HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);

      next = a + b;
      a = b;
      b = next;
    }

    char newline[] = "\r\n\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)newline, strlen(newline), HAL_MAX_DELAY);

    }

}

/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB3_2_B(void)
*
*   DESCRIPTION:
*       Calculate GCD and LCM from two numbers entered by the user.
*       e.g. 12, 18 -> GCD = 6, LCM = 36
*
*********************************************************************/

void LAB3_2_B(void)
{
char input[32];
char message[80];
int32_t a;
int32_t b;
int32_t x;
int32_t y;
int32_t temp;
int32_t gcd;
int32_t lcm;

while(1)
	{
    char text1[] = "Input a: ";
    HAL_UART_Transmit(&huart1, (uint8_t *)text1, strlen(text1), HAL_MAX_DELAY);
	UART_ReadLine(input, sizeof(input));
    a = (int32_t)strtol(input, NULL, 10);

    char text2[] = "Input b: ";
    HAL_UART_Transmit(&huart1, (uint8_t *)text2, strlen(text2), HAL_MAX_DELAY);
    UART_ReadLine(input, sizeof(input));
    b = (int32_t)strtol(input, NULL, 10);

    x = a;
    y = b;
    if (x < 0)
        {
        x = -x;
        }

    if (y < 0)
        {
        y = -y;
        }

    if(x==0&&y==0)
    	{
    	char error[]="GCD and LCM are undefined.\r\n\r\n";
    	HAL_UART_Transmit(&huart1, (uint8_t *)error, strlen(error), HAL_MAX_DELAY);
    	continue;
    	}
    while(y!=0)  // Euclidean algo
    	{
    	temp=x%y;
    	x=y;
    	y=temp;
    	}
    gcd=x;

    if(a==0||b==0)
    	{
    	lcm=0;
    	}
    else
    	{
    	lcm = (a / gcd) * b;

    	if (lcm < 0)
    		{
    		lcm = -lcm;
    		}
    	}
    snprintf(message, sizeof(message), "GCD = %ld\r\nLCM = %ld\r\n\r\n", (long)gcd, (long)lcm);
    HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);

	}

}

/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB3_3_A(void)
*
*   DESCRIPTION:
*       Ultimate password guessing game.
*       User enters a number and the program narrows the valid range
*       until the correct answer is found.
*       e.g. 1 ~ 100, answer = 65
*
*********************************************************************/

void LAB3_3_A(void)
{
char input[32];
char message[80];

int32_t answer;
int32_t guess;
int32_t min;
int32_t max;

	while(1)
		{
		min = 1;
		max = 100;
		answer=(HAL_GetTick()%100)+1;

		char start[]="\r\n******* password *******\r\n";
		HAL_UART_Transmit(&huart1, (uint8_t *)start, strlen(start), HAL_MAX_DELAY);

		while(1)
			{
			snprintf(message, sizeof(message), "Range: %ld ~ %ld\r\nGuess: ", (long)min, (long)max);
			HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);

            UART_ReadLine(input, sizeof(input));
            guess = (int32_t)strtol(input, NULL, 10);

            	if (guess < min || guess > max)
					{
					char error[] = "Out of range\r\n\r\n";
					HAL_UART_Transmit(&huart1, (uint8_t *)error, strlen(error), HAL_MAX_DELAY);
					continue;
					}
                if (guess == answer)
					{
					snprintf(message, sizeof(message), "Correct, Answer = %ld\r\n", (long)answer);
					HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
					break;
					}
                else if (guess < answer)
					{
					min = guess + 1;
					}
                else
					{
					max = guess - 1;
					}
			}

		}

}

/*********************************************************************
*
*   PROCEDURE NAME:
*       LAB3_3_B(void)
*
*   DESCRIPTION:
*       Sort a list of numbers entered by the user in ascending order.
*       e.g. 5 numbers: 8 3 10 1 6 -> 1 3 6 8 10
*
*********************************************************************/

void LAB3_3_B(void)
{
char input[32];
char message[64];

int32_t numbers[10];
int32_t count;

	while(1)
		{
		char text[] = "\r\nType numbers to sort (1 ~ 10): ";
		HAL_UART_Transmit(&huart1, (uint8_t *)text, strlen(text), HAL_MAX_DELAY);
        UART_ReadLine(input, sizeof(input));
        count = (int32_t)strtol(input, NULL, 10);

        	if (count<1||count>10)
        		{
        		char error[]="Please input number 1~10:\r\n";
        		HAL_UART_Transmit(&huart1, (uint8_t *)error, strlen(error), HAL_MAX_DELAY);
        		continue;
        		}
        	for (int32_t i = 0; i < count; i++)
        		{
                snprintf(message, sizeof(message), "Number %ld: ", (long)(i + 1));
                HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);

                UART_ReadLine(input, sizeof(input));
                numbers[i] = (int32_t)strtol(input, NULL, 10);
        		}

        QuickSort(numbers, 0, count - 1);
        char result[] = "Sorted: ";
        HAL_UART_Transmit(&huart1, (uint8_t *)result, strlen(result), HAL_MAX_DELAY);

			for (int32_t i = 0; i < count; i++)
				{
				snprintf(message, sizeof(message), "%ld ", (long)numbers[i]);
				HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
				}
		char newline[] = "\r\n";
		HAL_UART_Transmit(&huart1, (uint8_t *)newline, strlen(newline), HAL_MAX_DELAY);
		}
}


/*********************************************************************
 *
 * TOOL
 * DESCRIPTION: Can type long number, and use "enter" can transmit it
 *
 *
*********************************************************************/
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

/*********************************************************************
 *
 * TOOL
 * DESCRIPTION: Quick sort, use in LAB3-3-B
 *
 *
*********************************************************************/

static void QuickSort(int32_t numbers[], int32_t left, int32_t right)
{
    int32_t i = left;
    int32_t j = right;
    int32_t pivot = numbers[(left + right) / 2];
    int32_t temp;

    while (i <= j)
    {
        while (numbers[i] < pivot)
        {
            i++;
        }

        while (numbers[j] > pivot)
        {
            j--;
        }

        if (i <= j)
        {
            temp = numbers[i];
            numbers[i] = numbers[j];
            numbers[j] = temp;

            i++;
            j--;
        }
    }

    if (left < j)
    {
        QuickSort(numbers, left, j);
    }

    if (i < right)
    {
        QuickSort(numbers, i, right);
    }
}


