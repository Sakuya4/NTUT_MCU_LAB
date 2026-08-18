# LAB3-3-A - Play the ultimate password guessing game over UART

[Back to the main README](../README.md)

Commit: [`c47263e`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/c47263e549f7998751afd7aa284e058e09385ab4)

## Goal

Run an interactive number-guessing game through the serial terminal, update the valid range after each guess, and start a new round when the correct answer is entered.

## Schematic evidence

### USART1 pin mapping

![STM32F769I-DISCO USART1 VCP pin mapping](../assets/schematics/usart1-vcp-pin-mapping.png)

The schematic maps the Virtual COM Port transmit net to `PA9` and the receive net to `PA10`. Therefore, CubeMX configures `PA9` as `USART1_TX` and `PA10` as `USART1_RX`.

### ST-LINK Virtual COM Port circuit

![STM32F769I-DISCO ST-LINK Virtual COM Port circuit](../assets/schematics/stlink-vcp-circuit.png)

The ST-LINK interface receives the MCU's `VCP_TX` signal through `SB18` and drives the MCU's `VCP_RX` signal through `SB17`. Its USB connection provides the PC-side Virtual COM Port used for the game.

Every guess travels from the terminal to `PA10`, while the current range, input echo, validation messages, and success message travel from `PA9` back to the terminal.

## User-manual cross-check

[UM2033 Section 5.15](https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf) states that USART1 is connected to the ST-LINK Virtual COM Port through connector `CN16`. The guessing game uses this bidirectional connection to receive each guess and send the updated range and result.

Select the ST-LINK COM port at `115200 8N1`, with no flow control. Use a data-capable USB cable on `CN16`; no user-button input or external serial adapter is required.

## CubeMX settings

| Setting | Value | Reason |
|---|---|---|
| USART1 mode | Asynchronous, transmit and receive | The game continuously exchanges prompts and guesses. |
| TX pin | `PA9` | Connected to `VCP_TX` on the board schematic. |
| RX pin | `PA10` | Connected to `VCP_RX` on the board schematic. |
| Baud rate | `115200` bit/s | Must match the PC serial terminal. |
| Serial format | 8 data bits, no parity, 1 stop bit | Standard `115200 8-N-1` communication. |
| Hardware flow control | None | No RTS/CTS connection is required. |
| Oversampling | 16 | Matches the existing USART1 initialization. |

Call `MX_USART1_UART_Init()` before `LAB3_3_A()`.

## Guessing-game concept

Each round starts with an inclusive range from `1` to `100`. The target number is derived from the current HAL system tick:

```c
answer = (HAL_GetTick() % 100) + 1;
```

This produces a value between `1` and `100`, but it is a simple tick-derived value rather than a dedicated hardware random-number generator.

The existing `UART_ReadLine()` helper collects and echoes a complete guess, and `strtol()` converts the entered text to an integer. Guesses outside the current inclusive range are rejected. Otherwise:

- A guess below the answer changes the lower bound to `guess + 1`.
- A guess above the answer changes the upper bound to `guess - 1`.
- A correct guess prints the success message and begins a new round.

Excluding the previous incorrect guess keeps the displayed range progressively smaller.

## Expected result

For a round whose target happens to be `42`:

```text
******* password *******
Range: 1 ~ 100
Guess: 50
Range: 1 ~ 49
Guess: 30
Range: 31 ~ 49
Guess: 42
Correct, Answer = 42
```

The actual target changes according to the system tick when the round starts.
