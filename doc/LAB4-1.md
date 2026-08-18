# LAB4-1 - Read the internal temperature sensor with ADC1

[Back to the main README](../README.md)

Commit: [`dead5fb`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/dead5fb8cd5da1f0c3300d6e28650eaf522442a1)

## Goal

Use the STM32F769's internal temperature sensor as the ADC input, convert the 12-bit reading into millivolts and an approximate chip temperature, and send the result to a USB serial terminal once per second.

No potentiometer, external temperature sensor, jumper wire, or additional analog GPIO pin is required.

## Schematic evidence

### ADC analog supply and voltage reference

![STM32F769I-DISCO analog supply and ADC voltage-reference circuit](../assets/schematics/adc-analog-reference-circuit.png)

The MCU connection sheet shows the `3V3` supply reaching the `VDDA` analog supply through the board's supply-filtering network. The `VREF+` connection also reaches the MCU and has local decoupling capacitors. This is why the current firmware approximates the ADC reference as `3300 mV` when converting a 12-bit ADC code into voltage.

The temperature sensor itself is inside the STM32F769 silicon, so it does not appear as a separate external component or board-level GPIO wire in this schematic. ST identifies it as an internal connection to `ADC1_IN18` in [the STM32F769NI datasheet, Section 3.43](https://www.st.com/resource/en/datasheet/stm32f769ni.pdf).

### External analog inputs available on the board

![STM32F769I-DISCO Arduino analog-input connector](../assets/schematics/arduino-analog-inputs.png)

The Arduino-compatible connector provides external alternatives such as `A0 -> PA6 -> ADC1_IN6`, `A1 -> PA4 -> ADC1_IN4`, and `A2 -> PC2 -> ADC1_IN12`. A potentiometer-based exercise would need one of these physical analog inputs and external wiring.

This lab intentionally does not use those connector pins. CubeMX instead enables the internal virtual signal `VP_ADC1_TempSens_Input`, which selects `ADC_CHANNEL_TEMPSENSOR` without consuming a physical ADC GPIO.

### USART1 connection to the computer

![STM32F769I-DISCO USART1 VCP pin mapping](../assets/schematics/usart1-vcp-pin-mapping.png)

![STM32F769I-DISCO ST-LINK Virtual COM Port circuit](../assets/schematics/stlink-vcp-circuit.png)

The board schematic maps `PA9` to `VCP_TX` and `PA10` to `VCP_RX`. The transmit path reaches the ST-LINK controller through `VCP_TX -> SB18 -> STLINK_RX`, and the ST-LINK USB connection exposes the serial output to the computer. This lab only transmits measurements, although USART1 remains configured for both TX and RX.

## User-manual cross-check

[UM2033 Section 6.2, Table 6](https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf) lists external Arduino analog connector `CN14`, including `A0 = PA6 / ADC1_IN6` and `A1 = PA4 / ADC1_IN4`. Those pins are useful for a different external-analog-input exercise, but they are **not used here**: this lab selects the MCU's internal `ADC_CHANNEL_TEMPSENSOR` and needs no sensor wiring.

Section 5.15 documents the USART1 Virtual COM Port at `CN16`. Open that port at `115200 8N1` to view the internal-temperature readings.

## CubeMX settings

### ADC1

| Setting | Current value | Reason |
|---|---|---|
| ADC instance | `ADC1` | The internal temperature sensor is connected to ADC1. |
| Input channel | `ADC_CHANNEL_TEMPSENSOR` | Selects the MCU's internal temperature sensor. |
| CubeMX signal | `VP_ADC1_TempSens_Input` | Uses an internal signal rather than a physical GPIO. |
| Resolution | 12 bits | Produces a digital value from `0` to `4095`. |
| Clock prescaler | `ADC_CLOCK_SYNC_PCLK_DIV2` | Divides the current APB2 clock by two. |
| Scan conversion | Disabled | Only one regular conversion channel is used. |
| Continuous conversion | Disabled | Each measurement is started explicitly in software. |
| External trigger | None / software start | The code starts conversion with `HAL_ADC_Start()`. |
| Data alignment | Right | Returns a conventional right-aligned 12-bit value. |
| Number of conversions | `1` | Only the temperature-sensor channel is sampled. |
| Sampling time | `ADC_SAMPLETIME_3CYCLES` | Matches the current project configuration; see the accuracy note below. |

### USART1

| Setting | Value |
|---|---|
| Mode | Asynchronous, transmit and receive |
| TX pin | `PA9` |
| RX pin | `PA10` |
| Baud rate | `115200` bit/s |
| Serial format | 8 data bits, no parity, 1 stop bit |
| Hardware flow control | None |

The serial terminal must use `115200 8-N-1`.

## Initialization order

The ADC and UART must both be initialized before the lab enters its infinite loop:

```c
MX_GPIO_Init();
MX_USART1_UART_Init();
MX_ADC1_Init();
LAB4_1();
```

## ADC conversion concept

The current implementation performs one blocking ADC conversion per iteration:

```c
HAL_ADC_Start(&hadc1);
HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
adcValue = HAL_ADC_GetValue(&hadc1);
HAL_ADC_Stop(&hadc1);
```

`HAL_ADC_Start()` begins the software-triggered conversion, `HAL_ADC_PollForConversion()` waits for completion, `HAL_ADC_GetValue()` returns the 12-bit sample, and `HAL_ADC_Stop()` ends the current conversion sequence.

The raw sample is converted into an estimated sensor voltage:

```c
voltageMv = (adcValue * 3300) / 4095;
```

Here, `4095` is the maximum value of a 12-bit ADC, and `3300` represents the assumed `3.3 V` analog reference.

## Temperature-conversion concept

ST specifies a typical sensor voltage of `0.76 V` at `25 C` and an average slope of `2.5 mV/C` in [the STM32F769NI datasheet, Table 78](https://www.st.com/resource/en/datasheet/stm32f769ni.pdf). The code expresses these typical values using integer arithmetic:

```c
temperature = ((voltageMv - 760) * 10 / 25) + 25;
```

The equation estimates how far the measured sensor voltage is above or below `760 mV`, converts that difference using the `2.5 mV/C` slope, and offsets the result by `25 C`.

This value represents an approximate MCU die temperature, not a calibrated ambient-air temperature. Integer division, the assumed `3.3 V` reference, self-heating, device-to-device variation, and the absence of factory-calibration values all affect the result. ST provides `TS_CAL1` and `TS_CAL2` calibration values in Table 79 for applications that need a better calibrated estimate.

## Sampling-time accuracy note

The current project uses the `16 MHz` HSI clock, an APB2 divider of `1`, and an ADC prescaler of `2`, so the ADC clock is approximately `8 MHz`:

```text
Current ADC sample time = 3 cycles / 8 MHz = 0.375 us
```

ST specifies a minimum temperature-sensor sampling time of `10 us` for `1 C` accuracy in [the STM32F769NI datasheet, Table 78](https://www.st.com/resource/en/datasheet/stm32f769ni.pdf). Therefore, the existing `ADC_SAMPLETIME_3CYCLES` setting does not meet that timing requirement. At the current ADC clock, `ADC_SAMPLETIME_84CYCLES` would provide `10.5 us`; longer available sampling times would also satisfy the minimum.

This document records the current firmware accurately and does not modify its ADC configuration.

## Expected result

The serial terminal receives one measurement approximately every second:

```text
ADC = 963, Voltage = 776 mV, Temperature = 31 C
ADC = 965, Voltage = 777 mV, Temperature = 31 C
ADC = 975, Voltage = 785 mV, Temperature = 35 C
```

Actual readings depend on the chip temperature, reference voltage, and current ADC sampling configuration.
