#include "lab01_gpio.h"
#include "main.h"

void LAB1_1(void)
{
  while(1)
  {
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin|LED2_Pin);
	HAL_Delay(500);
  }
}

void LAB1_2(void)
{
  while(1)
  {
	  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
	  HAL_Delay(500);

	  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	  HAL_Delay(500);
  }
}
