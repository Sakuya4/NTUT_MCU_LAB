# NTUT MCU Lab

GPIO practice for the **STM32F769I-DISCO** board using STM32CubeIDE, STM32CubeMX, and the STM32 HAL.

## Hardware and source

- Development board: STM32F769I-DISCO (MB1225, schematic revision B-02)
- MCU: STM32F769NIH6
- Framework: STM32Cube HAL
- Official schematic: [MB1225 STM32F769I-DISCO schematic](https://www.st.com/resource/en/schematic_pack/mb1225-f769i-b02_schematic.pdf)

## Lab documentation

- [LAB1-1 - Toggle both LEDs every 500 ms](doc/LAB1-1.md)
- [LAB1-2 - Control each LED explicitly](doc/LAB1-2.md)
- [LAB1-3 - Turn on the LED while the user button is pressed](doc/LAB1-3.md)
- [LAB1-4 - Toggle the LED on each button press](doc/LAB1-4.md)

## Selecting a lab

Each lab function contains its own infinite loop, so only one lab should be called at a time in `main.c` after `MX_GPIO_Init()`:

```c
MX_GPIO_Init();
LAB1_4(); /* Change this to LAB1_1(), LAB1_2(), LAB1_3(), or LAB1_4(). */
```

The current `main.c` calls `LAB1_4()`. All four lab functions are implemented in `Core/Src/lab01_gpio.c`, with declarations in `Core/Inc/lab01_gpio.h`.
