# LAB3-2-A - Generate a Fibonacci sequence over UART

[Back to the main README](../README.md)

Commit: [`77e4938`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/77e4938276f571f0f426d10180727a17b932e691)

## Goal

Read the requested number of Fibonacci terms from the serial terminal, validate the count, and print the sequence through USART1.

## Schematic evidence

### USART1 pin mapping

![STM32F769I-DISCO USART1 VCP pin mapping](../assets/schematics/usart1-vcp-pin-mapping.png)

The MCU schematic identifies `PA9` as `VCP_TX` and `PA10` as `VCP_RX`. These labeled connections explain the CubeMX assignments `PA9 = USART1_TX` and `PA10 = USART1_RX`.

### ST-LINK Virtual COM Port circuit

![STM32F769I-DISCO ST-LINK Virtual COM Port circuit](../assets/schematics/stlink-vcp-circuit.png)

The TX and RX directions cross at the ST-LINK interface: `PA9 / VCP_TX` connects through `SB18` to `STLINK_RX`, and `PA10 / VCP_RX` connects through `SB17` to `STLINK_TX`. The ST-LINK USB connector exposes these signals to the computer as a Virtual COM Port.

The terminal-to-board direction supplies the requested term count, while the board-to-terminal direction carries the prompt, echoed input, generated numbers, and validation messages.

## CubeMX settings

| Setting | Value | Reason |
|---|---|---|
| USART1 mode | Asynchronous, transmit and receive | The lab reads a count and returns a sequence. |
| TX pin | `PA9` | Connected to the schematic's `VCP_TX` net. |
| RX pin | `PA10` | Connected to the schematic's `VCP_RX` net. |
| Baud rate | `115200` bit/s | Must match the computer's serial terminal. |
| Serial format | 8 data bits, no parity, 1 stop bit | Standard `115200 8-N-1` terminal configuration. |
| Hardware flow control | None | No separate RTS/CTS path is used. |
| Oversampling | 16 | Matches the current USART1 configuration. |

Call `MX_USART1_UART_Init()` before `LAB3_2_A()`.

## Input and sequence concept

`UART_ReadLine()` collects and echoes a complete terminal line before `strtol()` converts it to the requested count. The implementation accepts only `1` through `47`:

```c
if (n <= 0 || n > 47)
{
    /* Report the accepted range and restart the prompt. */
    continue;
}
```

The sequence begins with `a = 0` and `b = 1`. Each iteration prints the current `a`, computes the next sum, and advances the two stored values:

```c
next = a + b;
a = b;
b = next;
```

The limit keeps the printed sequence within the lab's intended `uint32_t` range: when `n = 47`, the final displayed term is `F(46) = 1836311903`. The loop does not separately check unsigned arithmetic overflow when preparing its next value.

## Expected result

```text
Input Fibonacci count: 8
Fibonacci: 0 1 1 2 3 5 8 13
```

An out-of-range request produces the existing validation message:

```text
Input Fibonacci count: 0
Please input 1 ~ 47.
```
