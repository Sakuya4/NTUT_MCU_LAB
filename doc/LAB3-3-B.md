# LAB3-3-B - Sort UART input with Quick Sort

[Back to the main README](../README.md)

Commit: [`1b427c7`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/1b427c70fe040f453a28d1fedbd3381a10746597)

## Goal

Receive up to ten signed integers from the serial terminal, sort them in ascending order with Quick Sort, and transmit the sorted result.

## Schematic evidence

### USART1 pin mapping

![STM32F769I-DISCO USART1 VCP pin mapping](../assets/schematics/usart1-vcp-pin-mapping.png)

The MCU schematic labels `PA9` as `VCP_TX` and `PA10` as `VCP_RX`. These physical connections determine the firmware assignments `PA9 = USART1_TX` and `PA10 = USART1_RX`.

### ST-LINK Virtual COM Port circuit

![STM32F769I-DISCO ST-LINK Virtual COM Port circuit](../assets/schematics/stlink-vcp-circuit.png)

The transmit path runs from `PA9 / VCP_TX` through `SB18` to `STLINK_RX`. The receive path runs from `STLINK_TX` through `SB17` to `PA10 / VCP_RX`. The ST-LINK controller then provides the USB Virtual COM Port used by the serial terminal.

The sorting task requires both connections: the PC supplies the element count and each number, and the MCU returns prompts, echoed input, and the sorted sequence.

## CubeMX settings

| Setting | Value | Reason |
|---|---|---|
| USART1 mode | Asynchronous, transmit and receive | The lab accepts numbers and returns sorted output. |
| TX pin | `PA9` | Matches the schematic's `VCP_TX` connection. |
| RX pin | `PA10` | Matches the schematic's `VCP_RX` connection. |
| Baud rate | `115200` bit/s | The PC terminal must use the same baud rate. |
| Serial format | 8 data bits, no parity, 1 stop bit | Uses the standard `115200 8-N-1` format. |
| Hardware flow control | None | Only the documented TX/RX VCP signals are used. |
| Oversampling | 16 | Matches the generated USART1 initialization. |

The current `main.c` starts this lab after peripheral initialization:

```c
MX_GPIO_Init();
MX_USART1_UART_Init();
LAB3_3_B();
```

## Input validation concept

`UART_ReadLine()` receives and echoes each terminal line, and `strtol()` converts the text into a signed `int32_t`. The number of elements must be between `1` and `10`:

```c
int32_t numbers[10];

if (count <= 0 || count > 10)
{
    /* Report the accepted range and restart. */
    continue;
}
```

This limit matches the ten-element array and prevents the input loop from writing past its allocated storage. Signed values and duplicate values can be included.

## Quick Sort concept

The implementation selects the middle array element as the pivot. Two indices move inward until they find values on the wrong side of that pivot, then those values are exchanged:

```c
pivot = numbers[(left + right) / 2];

while (i <= j)
{
    while (numbers[i] < pivot) i++;
    while (numbers[j] > pivot) j--;

    if (i <= j)
    {
        temp = numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = temp;
        i++;
        j--;
    }
}
```

After partitioning, `QuickSort()` recursively sorts the remaining left and right subranges. Once the recursion finishes, the lab transmits every element in ascending order.

## Expected result

```text
Type numbers to sort (1 ~ 10): 7
Number 1: 54
Number 2: 54
Number 3: 66
Number 4: 125
Number 5: 14
Number 6: 8
Number 7: 3
Sorted: 3 8 14 54 54 66 125
```

The current `main.c` calls `LAB3_3_B()`, so this is the active LAB3 exercise in the latest project revision.
