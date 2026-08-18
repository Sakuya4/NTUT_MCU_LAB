# LAB5-1 - Build a UART clock with a one-second TIM1 interrupt

[Back to the main README](../README.md)

## Goal

Receive an initial hour, minute, and second from a serial terminal, then use the TIM1 update interrupt to advance and print a 24-hour clock once every second.

This exercise deliberately separates two jobs: TIM1 keeps time in the background, while the main loop formats and transmits the updated time over USART1.

## Schematic evidence

### USART1 pin connections

![STM32F769I-DISCO USART1 Virtual COM Port pin mapping](../assets/schematics/usart1-vcp-pin-mapping.png)

The MCU schematic connects `PA9` to `VCP_TX` and `PA10` to `VCP_RX`. Therefore, the project assigns `PA9 = USART1_TX` and `PA10 = USART1_RX` instead of choosing arbitrary UART pins.

### ST-LINK Virtual COM Port circuit

![STM32F769I-DISCO ST-LINK Virtual COM Port circuit](../assets/schematics/stlink-vcp-circuit.png)

The transmit path is `PA9 / VCP_TX -> SB18 -> STLINK_RX`; the receive path is `STLINK_TX -> SB17 -> PA10 / VCP_RX`. The onboard ST-LINK exposes these signals as the USB Virtual COM Port used by the PC terminal.

TIM1 itself is an internal MCU peripheral, so it does not require an external timer connection on the board schematic.

## User-manual cross-check

[UM2033 Section 5.15](https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf) identifies USART1 as the ST-LINK Virtual COM Port on connector `CN16`. Open the PC terminal at `115200 8N1` to see each one-second clock update.

The board manual describes available board-level clock sources, but this particular project's `SystemClock_Config()` selects the MCU's internal `16 MHz` HSI oscillator without the PLL. TIM1 therefore receives the project-configured internal clock; no external timer pin or external-crystal setup is required for this lab.

## CubeMX settings

| Peripheral or setting | Current value | Why it is required |
|---|---|---|
| System clock | `HSI = 16 MHz` | Supplies the project clock used for timer calculations. |
| APB2 prescaler | `/1` | Gives TIM1 a `16 MHz` timer input in this configuration. |
| TIM1 clock source | Internal clock | Allows TIM1 to count without an external signal. |
| TIM1 prescaler | `15999` | Divides `16 MHz` by `15999 + 1`, producing a `1 kHz` counter clock. |
| TIM1 period | `999` | Generates one update event after `999 + 1` counter ticks. |
| TIM1 update interrupt | Enabled in NVIC | Allows the timer overflow to reach the application callback. |
| USART1 pins | `PA9` TX, `PA10` RX | Match the board's ST-LINK Virtual COM Port circuit. |
| USART1 format | `115200 8-N-1` | Matches the current generated UART initialization. |

## Why TIM1 generates exactly one interrupt per second

ST explains that the timer counter frequency is the timer input clock divided by `PSC + 1`, and that an update event occurs after `ARR + 1` counter ticks in up-counting mode. See [ST: Getting started with TIM](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_TIM).

For the actual settings in `main.c`:

```c
htim1.Init.Prescaler = 15999;
htim1.Init.Period = 999;
```

the calculation is:

```text
TIM1 counter clock = 16,000,000 / (15999 + 1)
                   = 1,000 Hz

TIM1 update frequency = 1,000 / (999 + 1)
                      = 1 Hz

TIM1 update period = 1 second
```

The important distinction is that `1,000 Hz` is the internal counter speed, while `1 Hz` is the overflow or update-interrupt frequency.

## Initialization order

To select this exercise, initialize the existing peripherals and call `LAB5_1()`:

```c
MX_GPIO_Init();
MX_USART1_UART_Init();
MX_ADC1_Init();
MX_TIM1_Init();
MX_TIM6_Init();
LAB5_1();
```

Only one lab function should be called because each exercise owns its main loop.

## How the implementation works

### 1. Receive the starting time

```c
HAL_UART_Transmit(&huart1, (uint8_t *)text1, strlen(text1), HAL_MAX_DELAY);
UART_ReadLine(input, sizeof(input));
hour = (int32_t)strtol(input, NULL, 10);
```

`HAL_UART_Transmit()` displays a prompt. `UART_ReadLine()` waits for the user to finish a line and echoes the typed characters. `strtol()` converts the decimal text into an integer.

The same sequence is repeated for minutes and seconds. The application accepts hours `0-23` and minutes or seconds `0-59`:

```c
if (hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
    second < 0 || second > 59)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)error, strlen(error), HAL_MAX_DELAY);
    return;
}
```

This condition validates the resulting numeric ranges; it does not independently reject every malformed text representation accepted by `strtol()`.

### 2. Start the timer from a clean state

```c
__HAL_TIM_SET_COUNTER(&htim1, 0);
__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
timerUpdate = 0;
HAL_TIM_Base_Start_IT(&htim1);
```

| Function or macro | Meaning in this lab |
|---|---|
| `__HAL_TIM_SET_COUNTER(&htim1, 0)` | Writes `0` to the TIM1 counter so the first second begins from a known position. |
| `__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE)` | Clears an old update flag that might otherwise look like a fresh timer event. |
| `HAL_TIM_Base_Start_IT(&htim1)` | Starts TIM1 and enables its update interrupt. |

The exact counter and flag macros are visible in the project's [STM32F7 HAL TIM header](../Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_tim.h).

### 3. Follow the interrupt into the HAL callback

The interrupt handler generated in `stm32f7xx_it.c` forwards the TIM1 interrupt to the HAL:

```c
void TIM1_UP_TIM10_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}
```

The HAL then calls the project callback:

```c
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
    }
}
```

The actual shared callback also contains the LAB5-3-B stopwatch condition; that additional condition does not affect the clock when `stopwatchRunning == 0`.

The `htim->Instance == TIM1` check matters because the same HAL callback can be shared by multiple timers. ST demonstrates the same interrupt-start and callback pattern in [its official timer tutorial](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_TIM).

### 4. Print from the main loop instead of the interrupt

```c
if (timerUpdate == 1)
{
    timerUpdate = 0;

    snprintf(message, sizeof(message), "%02ld:%02ld:%02ld\r\n",
             (long)hour, (long)minute, (long)second);

    HAL_UART_Transmit(&huart1, (uint8_t *)message,
                      strlen(message), HAL_MAX_DELAY);
}
```

The interrupt only updates the clock and sets a flag. The slower blocking UART transmission stays in the main loop, which keeps the interrupt handler short and easy to understand.

The clock variables and `timerUpdate` are declared `volatile` because they are written in the interrupt context and read in the main application context.

## Expected result

```text
Hour(0~23): 23
Minute(0~59): 59
Second(0~59): 58
Timer Start: 23:59:58
23:59:59
00:00:00
00:00:01
```

## References

- [ST: Getting started with TIM](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_TIM)
- [ST AN4776: How to use general-purpose timer peripheral on STM32 MCUs](https://www.st.com/resource/en/application_note/an4776-how-to-use-generalpurpose-timer-peripheral-on-stm32-mcus-stmicroelectronics.pdf)
- [ST MB1225 STM32F769I-DISCO schematic](https://www.st.com/resource/en/schematic_pack/mb1225-f769i-b02_schematic.pdf)
- [Project STM32F7 HAL TIM header](../Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_tim.h)
