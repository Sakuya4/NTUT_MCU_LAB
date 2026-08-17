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

void LAB1_3(void)
{
    while (1)
    {
        if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
        {
          HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
        }

        else
        {
          HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        }
    }
}

void LAB1_4(void)
{
  GPIO_PinState currentButtonState;
  GPIO_PinState previousButtonState = GPIO_PIN_RESET;

  while(1)
  {
	currentButtonState = HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);
	if((currentButtonState == GPIO_PIN_SET)&&(previousButtonState == GPIO_PIN_RESET))
	{
	  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
	}

	previousButtonState = currentButtonState;
  }


}
