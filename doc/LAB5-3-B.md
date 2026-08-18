# LAB5-3-B - Control a stopwatch with short and long button presses

[Back to the main README](../README.md)

## Goal

Implement a UART stopwatch that counts seconds using TIM1. A short press of the onboard user button alternates between start and stop; a long press of at least one second resets the stopwatch to `00:00`.

This is the exercise currently selected in `main.c`.

## Schematic evidence

### MCU GPIO mapping

![STM32F769I-DISCO GPIO pin mapping for the user button](../assets/schematics/gpio-pin-mapping.png)

The MCU connection sheet assigns the `B_USER` signal to `PA0 / WKUP`. Therefore, the project defines the user button as `USER_BUTTON_GPIO_Port = GPIOA` and `USER_BUTTON_Pin = GPIO_PIN_0`.

### User button circuit

![STM32F769I-DISCO user button, pull-down resistor, and filter capacitor](../assets/schematics/user-button-circuit.png)

The blue `B1` button connects the input path to `3V3` when pressed. `R70` pulls the button node toward ground when the button is released; `R68` and `C54` are also visible in the signal path.

Consequently:

```c
HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET
```

means the button is physically pressed. This is why the code checks for `GPIO_PIN_SET` rather than `GPIO_PIN_RESET`.

### USART1 and ST-LINK serial connection

![STM32F769I-DISCO USART1 Virtual COM Port pin mapping](../assets/schematics/usart1-vcp-pin-mapping.png)

![STM32F769I-DISCO ST-LINK Virtual COM Port circuit](../assets/schematics/stlink-vcp-circuit.png)

The MCU sends stopwatch text from `PA9 / USART1_TX` through the onboard ST-LINK Virtual COM Port. `PA10 / USART1_RX` is also part of the configured serial interface, although the stopwatch commands themselves come from button `B1`.

## User-manual cross-check

[UM2033 Sections 5.15 and 5.16](https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf) identify the stopwatch's two user interfaces: blue button `B1` is logic high when pressed, and USART1 reaches the PC through the ST-LINK Virtual COM Port at `CN16`.

Use the blue `B1` for short and long presses; the black `B2` resets the MCU instead. Open the terminal at `115200 8N1` to see stopwatch output, and enable the TIM1 update interrupt so that elapsed time advances once per second.

## CubeMX settings

| Setting | Current value | Reason |
|---|---|---|
| System clock | `HSI = 16 MHz` | Provides the TIM1 clock source. |
| TIM1 prescaler | `15999` | Produces a `1 kHz` timer counter. |
| TIM1 period | `999` | Generates a timer update every `1 second`. |
| TIM1 update interrupt | Enabled | Lets the callback increment elapsed seconds. |
| User button | `PA0`, GPIO input | Matches the `B_USER` connection on the schematic. |
| Button active state | `GPIO_PIN_SET` | The button connects the input toward `3V3` when pressed. |
| USART1 | `PA9` TX, `PA10` RX | Uses the onboard ST-LINK Virtual COM Port. |
| UART format | `115200 8-N-1` | Matches the project's generated serial configuration. |

## Initialization order

The current `main.c` initializes the peripherals before selecting this exercise:

```c
MX_GPIO_Init();
MX_USART1_UART_Init();
MX_ADC1_Init();
MX_TIM1_Init();
MX_TIM6_Init();
LAB5_3_B();
```

## Why the stopwatch advances once per second

```text
TIM1 counter frequency = 16,000,000 / (15999 + 1)
                       = 1,000 Hz

TIM1 update frequency = 1,000 / (999 + 1)
                      = 1 Hz
```

The timer continues generating one update interrupt each second. However, the stopwatch increments only when its software running flag is set.

## Stopwatch state variables

```c
static volatile uint32_t stopwatchSeconds = 0;
static volatile uint8_t stopwatchRunning = 0;
static volatile uint8_t stopwatchUpdate = 0;
```

| Variable | Meaning |
|---|---|
| `stopwatchSeconds` | Total elapsed stopwatch time in seconds. |
| `stopwatchRunning` | `1` while counting; `0` while stopped. |
| `stopwatchUpdate` | Requests a refreshed UART display. |

These values are marked `volatile` because the timer interrupt and the main loop access shared stopwatch state.

## Step 1: initialize and start TIM1

```c
stopwatchSeconds = 0;
stopwatchRunning = 0;
stopwatchUpdate = 1;

__HAL_TIM_SET_COUNTER(&htim1, 0);
__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
HAL_TIM_Base_Start_IT(&htim1);
```

`stopwatchUpdate = 1` requests an initial `Stopwatch: 00:00` display. Resetting the counter and clearing the update flag makes the timer start from a known state. `HAL_TIM_Base_Start_IT()` starts the timer with update interrupts enabled.

Starting the timer does not mean the stopwatch is running: `stopwatchRunning` remains `0` until the first short button press.

## Step 2: detect a stable button press

```c
if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET)
{
    HAL_Delay(20);

    if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET)
    {
        /* Confirmed button press. */
    }
}
```

The first read notices the press. The `20 ms` delay allows mechanical contact bounce to settle, and the second read confirms that the button is still pressed.

The board already includes a button-side resistor and capacitor, but this small software debounce makes the teaching example easier to reason about.

## Step 3: measure how long the button remains pressed

```c
pressStart = HAL_GetTick();

while (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET)
{
}

pressTime = HAL_GetTick() - pressStart;
HAL_Delay(20);
```

`HAL_GetTick()` returns the HAL millisecond time base. The program records the time when the confirmed press begins, waits for the user to release the button, and subtracts the starting tick from the release tick.

The resulting `pressTime` is measured in milliseconds. The decision is made after the button is released, so a long press resets the display on release rather than immediately at the one-second threshold.

The blocking wait does not stop TIM1 interrupts: while the main loop is waiting for the release, the timer interrupt can still advance a running stopwatch in the background.

## Step 4: select reset or start/stop

### Long press: reset

```c
if (pressTime >= 1000)
{
    stopwatchRunning = 0;
    stopwatchSeconds = 0;
    stopwatchUpdate = 1;

    HAL_UART_Transmit(&huart1, (uint8_t *)resetText,
                      strlen(resetText), HAL_MAX_DELAY);
}
```

`1000` means `1000 ms`, or `1 second`. A long press stops the stopwatch, clears the elapsed time, and requests an updated display.

### Short press: start or stop

```c
if (stopwatchRunning == 0)
{
    stopwatchRunning = 1;
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
}
else
{
    stopwatchRunning = 0;
}
```

Resetting the TIM1 counter when starting makes the first visible second occur approximately one full second after the button press instead of at an arbitrary position within an already-running timer period.

A short press while stopped resumes counting from the existing `stopwatchSeconds` value. A short press while running pauses the count without clearing that value.

## Step 5: increment only while running

The shared timer callback contains this stopwatch-specific condition:

```c
if (htim->Instance == TIM1)
{
    /* Existing LAB5-1 clock maintenance also runs here. */

    if (stopwatchRunning == 1)
    {
        stopwatchSeconds++;
        stopwatchUpdate = 1;
    }
}
```

TIM1 still interrupts every second while the stopwatch is paused, but the elapsed value remains unchanged because `stopwatchRunning == 0`.

The callback also updates the LAB5-1 clock variables because both exercises share the same TIM1 callback. Those separate clock variables are not printed by LAB5-3-B.

## Step 6: convert total seconds to minutes and seconds

```c
if (stopwatchUpdate == 1)
{
    stopwatchUpdate = 0;

    minute = stopwatchSeconds / 60;
    second = stopwatchSeconds % 60;

    snprintf(message, sizeof(message), "Stopwatch: %02lu:%02lu\r\n",
             (unsigned long)minute, (unsigned long)second);

    HAL_UART_Transmit(&huart1, (uint8_t *)message,
                      strlen(message), HAL_MAX_DELAY);
}
```

Integer division produces the number of complete minutes, and the remainder operator produces the seconds left over:

```text
stopwatchSeconds = 125

minute = 125 / 60 = 2
second = 125 % 60 = 5

Printed result: Stopwatch: 02:05
```

As in LAB5-1, the interrupt sets a flag and the main loop handles the slower UART printing.

## Expected result

```text
Short Press: Start / Stop, Long Press: Reset
Stopwatch: 00:00
Start
Stopwatch: 00:01
Stopwatch: 00:02
Stopwatch: 00:03
Stop
Start
Stopwatch: 00:04
Reset
Stopwatch: 00:00
```

## References

- [ST: Getting started with TIM - update interrupts and callbacks](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_TIM)
- [ST AN4776: How to use general-purpose timer peripheral on STM32 MCUs](https://www.st.com/resource/en/application_note/an4776-how-to-use-generalpurpose-timer-peripheral-on-stm32-mcus-stmicroelectronics.pdf)
- [ST UM2033: Discovery kit with STM32F769NI MCU](https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf)
- [ST MB1225 STM32F769I-DISCO schematic](https://www.st.com/resource/en/schematic_pack/mb1225-f769i-b02_schematic.pdf)
- [Project STM32F7 HAL TIM header](../Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_tim.h)
