#include "lab02_uart.h"
#include "main.h"

void LAB2_1(void)
{
    uint8_t count = 0;

    while (1)
    {
        if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET)
        {
            HAL_Delay(20);

            if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET)
            {
                count++;

                if (count > 3)
                {
                    count = 0;
                }

                /* LED0 show bit 0 */
                if (count & 0x01)
                {
                    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
                }
                else
                {
                    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
                }
                /* LED0 show bit 0 */

                /* LED1 show bit 1 */
                if (count & 0x02)
                {
                    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
                }
                else
                {
                    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
                }
                /* LED1 show bit 1 */

                while (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET)
                {
                }

                HAL_Delay(20);
            }
        }
    }
}
