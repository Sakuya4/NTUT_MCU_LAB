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
