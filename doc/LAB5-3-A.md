# LAB5-3-A - Control breathing LED speed over UART

[Back to the main README](../README.md)

## Goal

Receive a speed level from `1` to `5` over USART1, then use TIM6-based software PWM to make the onboard LED breathe at the selected speed.

The LED brightness algorithm is the same as [LAB5-2](LAB5-2.md). This exercise adds UART input and changes how long each brightness level is held.

## Schematic evidence

### GPIO mapping and onboard LED circuit

![STM32F769I-DISCO GPIO pin mapping for the user LED](../assets/schematics/gpio-pin-mapping.png)

![STM32F769I-DISCO user LED and current-limiting resistor circuit](../assets/schematics/user-led-circuit.png)

Project `LED1` uses `PJ5`, which drives the `LD_USER2` net, resistor `R63`, and the physical green `LD2` LED. The schematic connects the LED's other side to ground, so this board revision uses `GPIO_PIN_SET` for on and `GPIO_PIN_RESET` for off.

### USART1 and ST-LINK serial connection

![STM32F769I-DISCO USART1 Virtual COM Port pin mapping](../assets/schematics/usart1-vcp-pin-mapping.png)

![STM32F769I-DISCO ST-LINK Virtual COM Port circuit](../assets/schematics/stlink-vcp-circuit.png)

`PA9 / VCP_TX` sends the speed prompt and confirmation to the PC. `PA10 / VCP_RX` receives the selected speed through the onboard ST-LINK Virtual COM Port.

## User-manual cross-check

[UM2033 Sections 5.15 and 5.16](https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf) identify both interfaces used here. USART1 is available through the onboard ST-LINK Virtual COM Port at `CN16`, and physical green `LD2` is controlled by `PJ5`, which this project labels `LED1`.

Open the terminal at `115200 8N1` to enter speed levels `1` through `5`. Follow the B-02 schematic for active-high LED behavior; the surrounding manual prose incorrectly describes the user LED polarity for this board revision. TIM6 supplies the software time base and does not need an output channel or an interrupt.

## CubeMX settings

| Setting | Current value | Reason |
|---|---|---|
| System and timer input clock | `16 MHz` | Provides the source clock for TIM6. |
| TIM6 prescaler | `159` | Produces a `100 kHz` counter; one tick is `10 us`. |
| TIM6 auto-reload | `65535` | Keeps the counter range above the software PWM period of `100` ticks. |
| LED output | `PJ5` / `LED1` | Matches the `LD_USER2` schematic connection. |
| USART1 TX | `PA9` | Matches the Virtual COM Port transmit path. |
| USART1 RX | `PA10` | Matches the Virtual COM Port receive path. |
| UART format | `115200 8-N-1` | Matches the generated USART1 initialization. |

## Initialization order

```c
MX_GPIO_Init();
MX_USART1_UART_Init();
MX_ADC1_Init();
MX_TIM1_Init();
MX_TIM6_Init();
LAB5_3_A();
```

## Step 1: ask the user for a speed

```c
char text[] = "Breathing Speed (1 ~ 5): ";

HAL_UART_Transmit(&huart1, (uint8_t *)text,
                  strlen(text), HAL_MAX_DELAY);

UART_ReadLine(input, sizeof(input));
speed = (int32_t)strtol(input, NULL, 10);
```

`HAL_UART_Transmit()` prints the prompt, `UART_ReadLine()` waits for and echoes a full input line, and `strtol()` converts the decimal text to a number.

The valid speed range is checked before starting the LED:

```c
if (speed < 1 || speed > 5)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)error,
                      strlen(error), HAL_MAX_DELAY);
    return;
}
```

The current implementation asks for the speed once before entering its infinite breathing loop. It does not dynamically receive a new speed while the LED is already breathing.

## Step 2: convert speed level to hold time

```c
holdCycle = 60 - (speed * 10);
```

One software PWM cycle is approximately `1 ms`, and a full fade contains approximately `200` brightness steps:

| UART speed | `holdCycle` | Time per brightness level | Approximate full fade |
|---|---|---|---|
| `1` | `50` PWM cycles | `50 ms` | `10 seconds`. |
| `2` | `40` PWM cycles | `40 ms` | `8 seconds`. |
| `3` | `30` PWM cycles | `30 ms` | `6 seconds`. |
| `4` | `20` PWM cycles | `20 ms` | `4 seconds`. |
| `5` | `10` PWM cycles | `10 ms` | `2 seconds`. |

The relationship is intentionally reversed: a higher speed number produces a smaller `holdCycle`, so each brightness value remains visible for less time.

For example:

```text
speed = 1 -> 60 - (1 x 10) = 50 -> slow breathing
speed = 5 -> 60 - (5 x 10) = 10 -> fast breathing
```

These times are approximate because GPIO writes and the loop itself also consume a small amount of execution time.

## Step 3: start the counter and active-high LED

```c
HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
__HAL_TIM_SET_AUTORELOAD(&htim6, 0xFFFF);
HAL_TIM_Base_Start(&htim6);
```

`GPIO_PIN_RESET` starts with the LED off. The auto-reload value ensures the counter can reach `100`. `HAL_TIM_Base_Start()` starts TIM6 as a running time base without depending on its overflow interrupt.

The timer timing is unchanged from LAB5-2:

```text
TIM6 counter clock = 16,000,000 / (159 + 1)
                   = 100,000 Hz

PWM period = 100 counter ticks
           = 1 ms

PWM frequency = approximately 1 kHz
```

## Step 4: generate the same PWM brightness repeatedly

```c
for (uint32_t cycle = 0; cycle < holdCycle; cycle++)
{
    __HAL_TIM_SET_COUNTER(&htim6, 0);

    if (duty > 0)
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

        while (__HAL_TIM_GET_COUNTER(&htim6) < (uint32_t)duty)
        {
        }
    }

    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

    while (__HAL_TIM_GET_COUNTER(&htim6) < 100)
    {
    }
}
```

If `duty = 40`, the LED is on for approximately `40` counter ticks and off for the remaining `60` ticks. The loop repeats that same `40%` brightness `holdCycle` times before moving to the next brightness step.

Crucially, the UART speed changes the number of repeated PWM periods, not the timer prescaler, PWM period, or LED pin.

## Step 5: reverse the fade at both limits

```c
duty += direction;

if (duty >= 100 || duty <= 0)
{
    direction = -direction;
}
```

The brightness moves repeatedly through `0% -> 100% -> 0%`. `direction` is signed because it alternates between `1` and `-1`.

For a detailed explanation of every timer macro, the `duty > 0` condition, and the reason TIM6 overflow interrupts cannot provide smooth PWM here, see [LAB5-2 - Create a breathing LED with TIM6 and software PWM](LAB5-2.md).

## Expected result

```text
Breathing Speed (1 ~ 5): 4
Speed Level = 4
```

The onboard green LED then repeats a complete dark-bright-dark cycle approximately every `4 seconds`.

An out-of-range input produces:

```text
Breathing Speed (1 ~ 5): 9
Invalid speed.
```

## References and official waveforms

- [ST: Getting started with TIM - timer clocks, interrupts, and PWM](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_TIM)
- [ST: Getting started with HRTIM - official PWM waveform captures at multiple duty cycles](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_HRTIM#3._Result)
- [ST AN4776: How to use general-purpose timer peripheral on STM32 MCUs](https://www.st.com/resource/en/application_note/an4776-how-to-use-generalpurpose-timer-peripheral-on-stm32-mcus-stmicroelectronics.pdf)
- [ST MB1225 STM32F769I-DISCO schematic](https://www.st.com/resource/en/schematic_pack/mb1225-f769i-b02_schematic.pdf)
- [Project STM32F7 HAL TIM header](../Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_tim.h)
