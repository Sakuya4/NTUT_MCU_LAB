# NTUT MCU Lab

GPIO, UART, and ADC practice for the **STM32F769I-DISCO** board using STM32CubeIDE, STM32CubeMX, and the STM32 HAL.

## Hardware and source

- Development board: STM32F769I-DISCO (MB1225, schematic revision B-02)
- MCU: STM32F769NIH6
- Framework: STM32Cube HAL
- Official schematic: [MB1225 STM32F769I-DISCO schematic](https://www.st.com/resource/en/schematic_pack/mb1225-f769i-b02_schematic.pdf)

## Lab documentation

### LAB1 - GPIO

- [LAB1-1 - Toggle both LEDs every 500 ms](doc/LAB1-1.md)
- [LAB1-2 - Control each LED explicitly](doc/LAB1-2.md)
- [LAB1-3 - Turn on the LED while the user button is pressed](doc/LAB1-3.md)
- [LAB1-4 - Toggle the LED on each button press](doc/LAB1-4.md)

### LAB2 - GPIO counter and UART

- [LAB2-1 - Display a two-bit button counter on the LEDs](doc/LAB2-1.md)
- [LAB2-2 - Transmit the button counter over USART1](doc/LAB2-2.md)

### LAB3 - Bidirectional UART and algorithms

- [LAB3-1 - Add two multi-digit integers over UART](doc/LAB3-1.md)
- [LAB3-2-A - Generate a Fibonacci sequence over UART](doc/LAB3-2-A.md)
- [LAB3-2-B - Calculate GCD and LCM over UART](doc/LAB3-2-B.md)
- [LAB3-3-A - Play the ultimate password guessing game over UART](doc/LAB3-3-A.md)
- [LAB3-3-B - Sort UART input with Quick Sort](doc/LAB3-3-B.md)

### LAB4 - ADC and internal temperature sensing

- [LAB4-1 - Read the internal temperature sensor with ADC1](doc/LAB4-1.md)
- [LAB4-2 - Control LEDs according to the internal temperature](doc/LAB4-2.md)

## Selecting a lab

Each lab function contains its own infinite loop, so only one lab should be called at a time in `main.c` after `MX_GPIO_Init()`:

```c
MX_GPIO_Init();
MX_USART1_UART_Init();
MX_ADC1_Init();
LAB4_2();
```

The current `main.c` calls `LAB4_2()`. LAB1 functions are stored in `Core/Src/lab01_gpio.c`, LAB2 functions are stored in `Core/Src/lab02_uart.c`, LAB3 functions are stored in `Core/Src/lab03_uart2.c`, and LAB4 functions are stored in `Core/Src/lab04_adc.c`.
