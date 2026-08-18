# LAB5-2 - Create a breathing LED with TIM6 and software PWM

[Back to the main README](../README.md)

## Goal

Make the onboard user LED gradually brighten and dim forever by combining the TIM6 counter with GPIO-based software pulse-width modulation.

This lab uses a normal GPIO output rather than a hardware PWM output channel. Its central idea is simple: switch the LED on and off rapidly, then gradually change the percentage of each cycle for which the LED remains on.

## Schematic evidence

### MCU GPIO connection

![STM32F769I-DISCO GPIO pin mapping for the user LEDs](../assets/schematics/gpio-pin-mapping.png)

The project defines `LED1` as `PJ5`. On the MB1225 board schematic, that GPIO is connected to the `LD_USER2` net, which drives the physical green user LED.

### User LED circuit and electrical polarity

![STM32F769I-DISCO user LED circuit and current-limiting resistors](../assets/schematics/user-led-circuit.png)

The `LD_USER2` path goes through `R63`, then through the green `LD2` LED, and finally to ground. Therefore, driving `PJ5` high sources current through the resistor and LED:

```c
HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);   // LED on
HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); // LED off
```

This revision-specific schematic and the working project code establish the active-high behavior. The generic wording in Section 5.16 of [ST UM2033](https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf) describes active-low LEDs, but that sentence conflicts with the MB1225 B-02 user-LED circuit and with its own nearby LED-color descriptions. For this particular board revision, follow the actual schematic and verified firmware behavior.

## Why this lab uses software PWM

`TIM6` is a basic timer: it provides a counter, prescaler, auto-reload register, and update events, but it does not provide a `TIM6_CH1` output that can be routed to the user LED.

ST identifies `TIM6` and `TIM7` as basic-configuration timers in [AN4776, Section 1](https://www.st.com/resource/en/application_note/an4776-how-to-use-generalpurpose-timer-peripheral-on-stm32-mcus-stmicroelectronics.pdf). Consequently, this project uses TIM6 as an accurate clock and toggles the ordinary `PJ5` GPIO in software.

Hardware PWM is still a good alternative on another board if its LED is physically connected to a real timer output channel:

```c
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
```

Those two lines cannot magically turn TIM6 into a channel-based PWM timer or reroute the existing onboard LED.

## CubeMX settings

| Setting | Current value | Reason |
|---|---|---|
| System clock | `HSI = 16 MHz` | Supplies the timer input used in this project. |
| APB1 prescaler | `/1` | Gives TIM6 a `16 MHz` input clock in the current configuration. |
| TIM6 prescaler | `159` | Produces a `100 kHz` timer counter. |
| TIM6 auto-reload | `65535` | Allows the counter to pass `100` without wrapping prematurely. |
| LED GPIO | `PJ5` / `LED1` | Matches the schematic's `LD_USER2` connection. |
| GPIO mode | Push-pull output | Allows the MCU to actively drive the LED on and off. |
| TIM6 PWM channel | None | TIM6 is used only as a basic counter. |

## The three frequencies students must not confuse

The relevant configuration is:

```c
htim6.Init.Prescaler = 159;
htim6.Init.Period = 65535;
```

ST gives the counter-clock equation in [Getting started with TIM](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_TIM):

```text
TIM6 counter frequency = 16,000,000 / (159 + 1)
                       = 100,000 Hz

One TIM6 counter tick = 10 us

Software PWM period = 100 counter ticks
                    = 1,000 us
                    = 1 ms

Software PWM frequency = 1 / 1 ms
                       = 1,000 Hz
```

However, allowing TIM6 to count all the way to its auto-reload value produces a completely different event frequency:

```text
TIM6 overflow frequency = 100,000 / (65535 + 1)
                        = approximately 1.53 Hz
```

| Signal or event | Approximate frequency | Used for |
|---|---|---|
| TIM6 counter increment | `100 kHz` | Fine-grained timing of the LED on-time and off-time. |
| Software-generated PWM | `1 kHz` | LED brightness control without visible slow flashing. |
| Natural TIM6 overflow | `1.53 Hz` | Not used for the breathing LED. |

An earlier interrupt-based implementation used the `1.53 Hz` overflow as its software-PWM clock. That was the actual reason the LED blinked or jumped in brightness instead of fading smoothly.

## Existing official PWM waveform examples

### Read the high level, low level, pulse width, and period

![Microchip PWM waveform showing the high pulse, low interval, pulse width, and complete period](https://developerhelp.microchip.com/xwiki/bin/download/products/mcu-mpu/8bit-pic/peripherals/ccp/pwm/WebHome/pwm.png?rev=1.1)

*Image source: [Microchip Developer Help - Pulse Width Modulation on 8-bit PIC Devices](https://developerhelp.microchip.com/xwiki/bin/view/products/mcu-mpu/8bit-pic/peripherals/ccp/pwm/).*

Although Microchip uses a different MCU family in this illustration, the digital PWM waveform principle is identical to this STM32 lab:

| Waveform feature | Electrical meaning | Corresponding action in `LAB5_2()` |
|---|---|---|
| High level, or top of the square wave | GPIO output is high; this board's active-high LED is on. | `HAL_GPIO_WritePin(..., GPIO_PIN_SET)` |
| Low level, or bottom of the square wave | GPIO output is low; the LED is off. | `HAL_GPIO_WritePin(..., GPIO_PIN_RESET)` |
| Rising edge | The signal changes from low to high. | Start the LED on-time after resetting the timer counter. |
| Falling edge | The signal changes from high to low. | Turn the LED off when the counter reaches `duty`. |
| Pulse width, or `T_ON` | Length of the high portion of one PWM cycle. | Wait until `__HAL_TIM_GET_COUNTER(&htim6) >= duty`. |
| Off-time, or `T_OFF` | Length of the low portion of one PWM cycle. | Keep the LED off until the counter reaches `100`. |
| Period, or `T` | One complete high-plus-low cycle. | `100` TIM6 counter ticks, or approximately `1 ms`. |

The relationship is:

```text
T = T_ON + T_OFF

duty cycle = T_ON / T x 100%
```

The LED is not supplied with a gradually changing analog voltage. Its GPIO output switches between high and low; changing the width of the high portion changes how much time the LED spends on during each period.

### Compare different duty cycles at the same frequency

![NXP PWM waveforms comparing 25 percent, 50 percent, and 75 percent duty cycles at a fixed period](https://community.nxp.com/t5/image/serverpage/image-id/236673i80065EBA8B0A9EEC?v=v2)

*Image source: [NXP TechSupport - PWM, PFM, and auto-skip mode comparison](https://community.nxp.com/t5/Power-Management/What-are-the-differences-between-the-PWM-amp-PFM-amp-auto-skip/td-p/1705652).*

NXP explains that PWM keeps the waveform frequency fixed while changing its duty cycle. That is exactly what this project does: the complete period remains `100` TIM6 counts, while the high portion changes as `duty` moves from `0` to `100`.

```c
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
```

For `duty = 25`, the high section lasts for `25` counts and the low section lasts for `75` counts. For `duty = 75`, those durations are reversed. The full period is still `100` counts in both cases.

### Connect the rising counter ramp to the PWM output

![Microchip digital PWM diagram showing an increasing timer counter, duty-cycle threshold, and resulting output waveform](https://developerhelp.microchip.com/xwiki/bin/download/applications/power/digital-power-converter-basics/digital-peripherals-mimicking-analog-behavior/transition-to-digital-pwm/WebHome/power-digital-duty-cycle.png?rev=1.1)

*Image source: [Microchip Developer Help - Transition to Digital PWM](https://developerhelp.microchip.com/xwiki/bin/view/applications/power/digital-power-converter-basics/digital-peripherals-mimicking-analog-behavior/transition-to-digital-pwm/).*

In the Microchip figure, a counter rises until it reaches a programmed threshold, and that comparison determines when the output waveform falls. Hardware PWM performs this comparison inside a timer peripheral. `LAB5_2()` reproduces the same concept explicitly in C:

| Official digital PWM concept | Equivalent in this project |
|---|---|
| Increasing timer counter | `__HAL_TIM_GET_COUNTER(&htim6)` |
| Duty-cycle compare value | `duty` |
| Beginning of a new period | `__HAL_TIM_SET_COUNTER(&htim6, 0)` |
| Output changes high at the beginning | `HAL_GPIO_WritePin(..., GPIO_PIN_SET)` |
| Output changes low at the duty threshold | `HAL_GPIO_WritePin(..., GPIO_PIN_RESET)` |
| End of the PWM period | The counter reaches `100`. |

ST also publishes oscilloscope captures for `10%`, `50%`, and `80%` duty cycles in [Getting started with HRTIM, Section 3: Result](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_HRTIM#3._Result). Those existing official images show the same visual concept: the period remains fixed while the high portion changes width.

The ST article uses a hardware high-resolution timer; this lab recreates the same duty-cycle principle using the TIM6 counter plus a GPIO pin.

## Duty cycle explained with the actual code values

The firmware chooses `100` timer counts as one complete PWM period:

```text
duty cycle = LED on-time / complete PWM period
```

| `duty` | LED on-time | LED off-time | Approximate brightness |
|---|---|---|---|
| `0` | `0 us` | `1000 us` | Off. |
| `10` | `100 us` | `900 us` | Very dim. |
| `25` | `250 us` | `750 us` | Approximately one quarter of full brightness. |
| `50` | `500 us` | `500 us` | Approximately half of full brightness. |
| `75` | `750 us` | `250 us` | Bright. |
| `100` | Approximately `1000 us` | Approximately `0 us` | Nearly fully on. |

At `duty = 100`, the implementation still briefly writes the pin low between consecutive cycles, so the output is practically full brightness rather than a mathematically uninterrupted high level.

## Full teaching version of the implemented algorithm

The following formatting is simplified for readability, but the operations and constants match the current `LAB5_2()` implementation:

```c
void LAB5_2(void)
{
    int32_t duty = 0;
    int32_t direction = 1;

    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_AUTORELOAD(&htim6, 0xFFFF);
    HAL_TIM_Base_Start(&htim6);

    while (1)
    {
        for (uint32_t cycle = 0; cycle < 20; cycle++)
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

        duty += direction;

        if (duty >= 100 || duty <= 0)
        {
            direction = -direction;
        }
    }
}
```

## Why every important block is written this way

### 1. Start dark and remember the fading direction

```c
int32_t duty = 0;
int32_t direction = 1;
```

`duty` is the current on-time expressed as a value between `0` and `100`. `direction = 1` means the LED is getting brighter; `direction = -1` means it is getting dimmer.

A signed type is necessary for `direction` because the value must become `-1`.

### 2. Start the LED and timer in a known state

```c
HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
__HAL_TIM_SET_AUTORELOAD(&htim6, 0xFFFF);
HAL_TIM_Base_Start(&htim6);
```

The first line explicitly turns the active-high LED off. This matters because the generated `MX_GPIO_Init()` initially sets `LED1` high.

`__HAL_TIM_SET_AUTORELOAD(&htim6, 0xFFFF)` makes the 16-bit timer count up to `65535`. The PWM loop must be able to observe a counter value of at least `100`; if ARR were left at `24`, the counter would repeatedly wrap from `24` to `0`, and the condition `counter < 100` would never finish.

`HAL_TIM_Base_Start()` starts the timer counter without requesting update interrupts. The algorithm reads the counter directly and therefore does not depend on the slow `1.53 Hz` TIM6 overflow.

### 3. Hold each brightness level long enough to see a gradual fade

```c
for (uint32_t cycle = 0; cycle < 20; cycle++)
```

One PWM cycle is approximately `1 ms`, so repeating the same duty value `20` times holds that brightness for about `20 ms`:

```text
One brightness step = 20 x 1 ms
                    = approximately 20 ms

0% -> 100% -> 0% = approximately 200 brightness steps
                 = approximately 4 seconds
```

Increasing `20` makes the breathing slower; decreasing it makes the breathing faster. This changes the fade speed, not the approximately `1 kHz` PWM frequency.

### 4. Restart the counter at the beginning of each PWM cycle

```c
__HAL_TIM_SET_COUNTER(&htim6, 0);
```

This writes `0` to the timer's `CNT` register. Starting every software PWM period from the same reference makes the next comparisons easy to understand.

### 5. Keep the LED on for the selected duty value

```c
if (duty > 0)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

    while (__HAL_TIM_GET_COUNTER(&htim6) < (uint32_t)duty)
    {
    }
}
```

For `duty = 30`, the LED turns on while the counter moves from `0` to `30`, corresponding to approximately `300 us`.

The `if (duty > 0)` condition prevents the code from producing a short unwanted flash when the intended brightness is exactly `0%`.

### 6. Turn the LED off for the rest of the fixed period

```c
HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

while (__HAL_TIM_GET_COUNTER(&htim6) < 100)
{
}
```

For `duty = 30`, the LED remains off while the counter advances from `30` to `100`, corresponding to approximately `700 us`.

The number `100` remains fixed. This keeps the PWM period approximately constant while `duty` changes only the ratio between on-time and off-time.

### 7. Reverse direction at the brightness limits

```c
duty += direction;

if (duty >= 100 || duty <= 0)
{
    direction = -direction;
}
```

The resulting sequence is:

```text
0, 1, 2, ... 99, 100, 99, 98, ... 1, 0, 1, ...
```

Without the direction reversal, the LED would become bright once and would not repeatedly fade back down.

## What the HAL functions and macros actually do

| Function or macro | Actual purpose | Why the lab needs it |
|---|---|---|
| `HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)` | Drives the selected GPIO output high. | Turns this active-high LED on. |
| `HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)` | Drives the selected GPIO output low. | Turns the LED off. |
| `HAL_TIM_Base_Start(&htim6)` | Starts TIM6 as a basic running timer. | Provides the `100 kHz` counter used for timing. |
| `__HAL_TIM_SET_AUTORELOAD(&htim6, value)` | Writes the timer auto-reload value. | Prevents a too-small ARR from wrapping before count `100`. |
| `__HAL_TIM_SET_COUNTER(&htim6, 0)` | Writes `0` to the timer `CNT` register. | Starts a new software PWM period. |
| `__HAL_TIM_GET_COUNTER(&htim6)` | Reads the timer `CNT` register. | Measures the current position within the PWM period. |

These are not mysterious delay functions: the actual STM32F7 HAL definitions show that the counter macros directly read or write `Instance->CNT`, and the auto-reload macro updates `Instance->ARR`. See the project's [STM32F7 HAL TIM header](../Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_tim.h) and [STM32F7 HAL GPIO implementation](../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c).

## Porting the idea to another student board

A student using a different STM32 board or firmware package should keep the algorithm and replace the hardware-specific pieces:

```c
HAL_TIM_Base_Start(&studentTimer);

while (1)
{
    reset_timer_counter();
    turn_led_on();
    wait_until_counter_reaches(duty);
    turn_led_off();
    wait_until_counter_reaches(period);
    gradually_change_duty();
}
```

The names of the timer, GPIO port, and GPIO pin can differ. The counter frequency must be recalculated for the new board, and an active-low LED requires the on/off GPIO states to be swapped.

If the LED really is connected to a timer channel, hardware PWM can replace the busy-waiting software loop. The software method is intentionally easy to follow, but it keeps the CPU busy while waiting for each counter threshold.

## Expected result

The physical green LED repeatedly transitions from dark to bright and back to dark. With the current `20`-cycle hold value, one complete breathing cycle takes approximately `4 seconds`.

## References

- [ST: Getting started with TIM - prescaler, auto-reload, update interrupts, and PWM](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_TIM)
- [ST: Getting started with HRTIM - official 10%, 50%, and 80% PWM waveform captures](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_HRTIM#3._Result)
- [Microchip Developer Help: Pulse Width Modulation on 8-bit PIC Devices](https://developerhelp.microchip.com/xwiki/bin/view/products/mcu-mpu/8bit-pic/peripherals/ccp/pwm/)
- [Microchip Developer Help: Transition to Digital PWM](https://developerhelp.microchip.com/xwiki/bin/view/applications/power/digital-power-converter-basics/digital-peripherals-mimicking-analog-behavior/transition-to-digital-pwm/)
- [NXP TechSupport: PWM, PFM, and auto-skip mode comparison](https://community.nxp.com/t5/Power-Management/What-are-the-differences-between-the-PWM-amp-PFM-amp-auto-skip/td-p/1705652)
- [ST AN4776: How to use general-purpose timer peripheral on STM32 MCUs](https://www.st.com/resource/en/application_note/an4776-how-to-use-generalpurpose-timer-peripheral-on-stm32-mcus-stmicroelectronics.pdf)
- [ST MB1225 STM32F769I-DISCO schematic](https://www.st.com/resource/en/schematic_pack/mb1225-f769i-b02_schematic.pdf)
- [Project STM32F7 HAL TIM header](../Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_tim.h)
- [Project STM32F7 HAL TIM implementation](../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c)
