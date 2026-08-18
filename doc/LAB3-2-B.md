# LAB3-2-B - Calculate GCD and LCM over UART

[Back to the main README](../README.md)

Commit: [`ab79e22`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/ab79e22da140db8e11bc0e9ce550645a2ad4222b)

## Goal

Receive two signed integers through the Virtual COM Port, calculate their greatest common divisor and least common multiple, and print both results.

## Schematic evidence

### USART1 pin mapping

![STM32F769I-DISCO USART1 VCP pin mapping](../assets/schematics/usart1-vcp-pin-mapping.png)

The board connection sheet labels `PA9` as `VCP_TX` and `PA10` as `VCP_RX`. Consequently, USART1 must use `PA9` for terminal output and `PA10` for terminal input.

### ST-LINK Virtual COM Port circuit

![STM32F769I-DISCO ST-LINK Virtual COM Port circuit](../assets/schematics/stlink-vcp-circuit.png)

The board routes MCU transmission through `VCP_TX -> SB18 -> STLINK_RX` and MCU reception through `STLINK_TX -> SB17 -> VCP_RX`. The onboard ST-LINK translates this UART traffic to USB, providing the computer's serial connection.

Both schematic paths are necessary: the PC sends the two input values, and the MCU returns prompts, echoed characters, the GCD, and the LCM.

## CubeMX settings

| Setting | Value | Reason |
|---|---|---|
| USART1 mode | Asynchronous, transmit and receive | Input and results travel in opposite directions. |
| TX pin | `PA9` | Matches the `VCP_TX` schematic net. |
| RX pin | `PA10` | Matches the `VCP_RX` schematic net. |
| Baud rate | `115200` bit/s | Must match the serial terminal. |
| Serial format | 8 data bits, no parity, 1 stop bit | Uses the standard `115200 8-N-1` format. |
| Hardware flow control | None | The implemented VCP connection uses TX/RX only. |
| Oversampling | 16 | Matches the generated peripheral settings. |

Call `MX_USART1_UART_Init()` before `LAB3_2_B()`.

## GCD and LCM concept

`UART_ReadLine()` receives each complete integer string, and `strtol()` converts it into a signed `int32_t`. The algorithm converts negative inputs into positive working values before applying the Euclidean algorithm:

```c
while (y != 0)
{
    temp = x % y;
    x = y;
    y = temp;
}

gcd = x;
```

Each remainder produces a smaller equivalent GCD problem. Once the remainder reaches zero, the remaining nonzero value is the GCD.

If either input is zero, the LCM is defined as zero. Otherwise, the implementation uses:

```c
lcm = (a / gcd) * b;
```

The result is converted to a positive value when necessary. The special case `a = 0` and `b = 0` is rejected because this implementation does not define a GCD for two zero inputs.

## Expected result

```text
Input a: 12
Input b: 18
GCD = 6
LCM = 36
```

Negative input values are also normalized for the GCD calculation, while the reported LCM remains nonnegative.
