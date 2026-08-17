# LAB1-1 - Toggle both LEDs every 500 ms

[Back to the main README](../README.md)

Commit: [`02a790a`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/02a790af192e8fa14fcc95a6b7e1670a745e42d6)

## Goal

Use one HAL function to invert both user LED outputs every 500 ms.

## Schematic evidence

![STM32F769I-DISCO user LED circuit](../assets/schematics/user-led-circuit.png)

Both LED cathodes are connected to ground. The MCU drives each LED anode through a series current-limiting resistor: `R62` for the red LED and `R63` for the green LED. A GPIO high level therefore sources current and turns the corresponding LED on, while a GPIO low level turns it off.

Because the LEDs are simple digital loads, I configure `PJ5` and `PJ13` as low-speed push-pull outputs with no internal pull resistors.

## CubeMX settings

| Pin | Project label | Mode | Initial level | Reason |
|---|---|---|---|---|
| `PJ5` | `LED1` | GPIO Output, Push-Pull, No Pull, Low Speed | High | Starts one LED in the on state |
| `PJ13` | `LED2` | GPIO Output, Push-Pull, No Pull, Low Speed | Low | Starts the other LED in the off state |

## Implementation concept

```c
HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin | LED2_Pin);
HAL_Delay(500);
```

`HAL_GPIO_TogglePin()` inverts both output bits. Since the two outputs start at opposite logic levels, the LEDs exchange states every 500 ms.

## Expected result

The red and green user LEDs alternate every 500 ms.
