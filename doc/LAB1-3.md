# LAB1-3 - Turn on the LED while the user button is pressed

[Back to the main README](../README.md)

Commit: [`0d75b3c`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/0d75b3c962a782af7e7813bd2449297295166547)

## Goal

Read the blue user button and control a user LED from the current button level.

## Schematic evidence

![STM32F769I-DISCO user button circuit](../assets/schematics/user-button-circuit.png)

![STM32F769I-DISCO user LED circuit](../assets/schematics/user-led-circuit.png)

The schematic connects the `B_USER` signal to `PA0/WKUP`. `R70` provides an external path to ground when the button is released, while pressing `B1` connects the signal toward `3V3`. The button is active high: released reads `GPIO_PIN_RESET`, and pressed reads `GPIO_PIN_SET`.

`R68` and `C54` also form an RC input filter. Because the board already provides an external pull-down path, I configure `PA0` as `GPIO_Input` with `GPIO_NOPULL` instead of adding an internal pull resistor.

The LED cathode is connected to ground, so its GPIO output is also active high.

## User-manual cross-check

[UM2033 Section 5.16](https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf) identifies blue `B1` as the user button and explicitly states that its input is `1` when pressed and `0` when released. The nearby black `B2` is the reset button and must not be used for this exercise.

Table 4 maps green `LD2` to `PJ5`, which is named `LED1` in this project. Follow the B-02 schematic rather than the contradictory active-low sentence in the manual: the LED is active high.

## CubeMX settings

| Pin | Project label | Mode | Pull | Reason |
|---|---|---|---|---|
| `PA0/WKUP` | `USER_BUTTON` | GPIO Input | No Pull | The external circuit defines the released low state |
| `PJ5` | `LED1` | GPIO Output, Push-Pull, Low Speed | No Pull | Drives the active-high LED circuit |

CubeMX must also enable the GPIOA and GPIOJ peripheral clocks before these pins are used.

## Implementation concept

```c
if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}
else
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
}
```

The program continuously copies the active-high button state to the active-high LED output.

## Expected result

The user LED turns on while the blue user button is held and turns off when it is released.
