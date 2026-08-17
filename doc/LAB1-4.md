# LAB1-4 - Toggle the LED on each button press

[Back to the main README](../README.md)

Commit: [`2ed0059`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/2ed00590ff6809dcea33dca9094377727a833ce6)

## Goal

Detect a new button press and toggle the LED once for each accepted press.

## Schematic evidence

![STM32F769I-DISCO user button circuit](../assets/schematics/user-button-circuit.png)

![STM32F769I-DISCO user LED circuit](../assets/schematics/user-led-circuit.png)

The button circuit is active high. The external pull-down path holds `PA0` low when the button is released, and pressing the button drives the input toward `3V3`. I therefore configure `PA0` as a GPIO input with no internal pull resistor.

The LED circuit is also active high, so `PJ5` remains a low-speed push-pull output. The new software concept is rising-edge detection: compare the current button sample with the previous sample and react only when the state changes from low to high.

## CubeMX settings

| Pin | Project label | Mode | Pull | Reason |
|---|---|---|---|---|
| `PA0/WKUP` | `USER_BUTTON` | GPIO Input | No Pull | The external pull-down holds the released state low |
| `PJ5` | `LED1` | GPIO Output, Push-Pull, Low Speed | No Pull | Drives the active-high LED circuit |

## Implementation concept

```c
GPIO_PinState currentButtonState;
GPIO_PinState previousButtonState = GPIO_PIN_RESET;

currentButtonState = HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);

if ((currentButtonState == GPIO_PIN_SET) &&
    (previousButtonState == GPIO_PIN_RESET))
{
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}

previousButtonState = currentButtonState;
```

The condition is true only on a detected rising edge, so holding the button does not intentionally toggle the LED repeatedly. The board's `R68/C54` network filters very short changes, but the current firmware does not include a timed software-debounce check.

## Expected result

Each accepted button press changes the user LED between on and off. Holding the button keeps the current LED state.
