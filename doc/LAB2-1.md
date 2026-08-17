# LAB2-1 - Display a two-bit button counter on the LEDs

[Back to the main README](../README.md)

Commit: [`585ba5f`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/585ba5f2672aef17dcd92cf3b80fb6eaab072cb8)

## Goal

Increment a counter once for each accepted button press, limit the value to `0` through `3`, and represent its two bits with the two user LEDs.

## Schematic evidence

![STM32F769I-DISCO user button circuit](../assets/schematics/user-button-circuit.png)

![STM32F769I-DISCO user LED circuit](../assets/schematics/user-led-circuit.png)

The `B_USER` signal is connected to `PA0/WKUP`. `R70` provides an external pull-down path, so the released button reads low. Pressing `B1` connects the signal toward `3V3`, so a valid press reads `GPIO_PIN_SET`. Because the board already defines the released state, I configure `PA0` as a GPIO input with no internal pull resistor.

`R68` and `C54` provide hardware filtering. The firmware adds software debounce by delaying for 20 ms, checking the input again, waiting for release, and delaying for another 20 ms.

The two LED cathodes are connected to ground. Their MCU pins must therefore be configured as active-high, low-speed push-pull outputs with no pull resistors.

## CubeMX settings

| Pin | Project label | Mode | Pull | Purpose |
|---|---|---|---|---|
| `PA0/WKUP` | `USER_BUTTON` | GPIO Input | No Pull | Active-high button input |
| `PJ5` | `LED1` | GPIO Output, Push-Pull, Low Speed | No Pull | Displays counter bit 0 |
| `PJ13` | `LED2` | GPIO Output, Push-Pull, Low Speed | No Pull | Displays counter bit 1 |

## Counter concept

```c
count++;

if (count > 3)
{
    count = 0;
}
```

Two LEDs can represent two binary bits, giving four possible values:

```text
00 = 0
01 = 1
10 = 2
11 = 3
```

The masks `0x01` and `0x02` select bit 0 and bit 1:

```c
if (count & 0x01)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}
else
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
}

if (count & 0x02)
{
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}
else
{
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
}
```

## Current LED behavior

The board LED circuit is active high, but the current code writes `GPIO_PIN_RESET` when a counter bit is `1`. Therefore, the visible LED pattern is the inverse of the usual convention in which an illuminated LED represents binary `1`.

| Count | Binary `bit1 bit0` | `LED2` for bit 1 | `LED1` for bit 0 |
|---:|:---:|:---:|:---:|
| 0 | `00` | On | On |
| 1 | `01` | On | Off |
| 2 | `10` | Off | On |
| 3 | `11` | Off | Off |

Before the first button press, the visible LED state comes from the CubeMX initial output levels. The table applies after the counter has been written to the LEDs.

## Expected result

Each valid press advances the counter through `1`, `2`, `3`, `0`, and then repeats. The two LEDs show the inverted two-bit pattern listed above.
