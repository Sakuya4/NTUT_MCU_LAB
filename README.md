# NTUT MCU Lab

GPIO practice for the **STM32F769I-DISCO** board using STM32CubeIDE, STM32CubeMX, and the STM32 HAL.

This README explains each lab from the hardware schematic first: identify the board net, trace it to the STM32 pin, decide the electrical logic level, and then choose the matching CubeMX and HAL settings.

## Hardware and source

- Development board: STM32F769I-DISCO (MB1225, schematic revision B-02)
- MCU: STM32F769NIH6
- Framework: STM32Cube HAL
- Official schematic: [MB1225 STM32F769I-DISCO schematic](https://www.st.com/resource/en/schematic_pack/mb1225-f769i-b02_schematic.pdf)
- Relevant sheets: Sheet 4 (MCU pin mapping) and Sheet 13 (user button and LEDs)

The images below are crops from the official STMicroelectronics schematic listed above.

## Schematic-to-firmware pin map

![STM32F769NIH6 GPIO pin mapping](assets/schematics/gpio-pin-mapping.png)

The schematic maps the three board signals as follows:

| Board function | Schematic net | MCU pin | Physical part | Electrical behavior |
|---|---|---:|---|---|
| User LED 1 | `LD_USER1` | `PJ13` | `LD1`, red | Active high |
| User LED 2 | `LD_USER2` | `PJ5` | `LD2`, green | Active high |
| User button | `B_USER` | `PA0/WKUP` | `B1`, blue | Released = low, pressed = high |

> **Naming note:** the current project keeps `LED1 = PJ5` and `LED2 = PJ13`. These firmware aliases are opposite to the physical board designators: project `LED1` controls physical green `LD2`, while project `LED2` controls physical red `LD1`.

## Selecting a lab

Each lab function contains its own infinite loop, so only one lab should be called at a time in `main.c` after `MX_GPIO_Init()`:

```c
MX_GPIO_Init();
LAB1_4(); /* Change this to LAB1_1(), LAB1_2(), LAB1_3(), or LAB1_4(). */
```

---

## LAB1-1 - Toggle both LEDs every 500 ms

Commit: [`02a790a`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/02a790af192e8fa14fcc95a6b7e1670a745e42d6)

### Schematic evidence

![STM32F769I-DISCO user LED circuit](assets/schematics/user-led-circuit.png)

Both LED cathodes are connected to ground. The MCU drives each LED anode through a series current-limiting resistor: `R62` for the red LED and `R63` for the green LED. Therefore, a GPIO high level sources current and turns the LED on; a GPIO low level turns it off. Because these are simple digital loads, I configure `PJ5` and `PJ13` as low-speed push-pull outputs with no internal pull resistor.

### CubeMX settings

| Pin | Project label | Mode | Initial level | Reason |
|---|---|---|---|---|
| `PJ5` | `LED1` | GPIO Output, Push-Pull, No Pull, Low Speed | High | Physical green `LD2` starts on |
| `PJ13` | `LED2` | GPIO Output, Push-Pull, No Pull, Low Speed | Low | Physical red `LD1` starts off |

### Implementation concept

```c
HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin | LED2_Pin);
HAL_Delay(500);
```

`HAL_GPIO_TogglePin()` inverts both output bits. Since the two LEDs start at opposite levels, they exchange states every 500 ms: green, red, green, red.

### Expected result

The red and green user LEDs alternate every 500 ms.

---

## LAB1-2 - Control each LED explicitly

Commit: [`4ac0ab2`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/4ac0ab246083fc835700e01deb8299e6ced264b9)

### Schematic evidence

![STM32F769I-DISCO user LED circuit](assets/schematics/user-led-circuit.png)

The same active-high LED circuit is used in this lab. A `GPIO_PIN_SET` output turns the selected LED on, and `GPIO_PIN_RESET` turns it off. The pin modes therefore remain push-pull outputs with no pull resistors.

### CubeMX settings

| Pin | Project label | Mode | Hardware controlled |
|---|---|---|---|
| `PJ5` | `LED1` | GPIO Output, Push-Pull, No Pull, Low Speed | Physical green `LD2` |
| `PJ13` | `LED2` | GPIO Output, Push-Pull, No Pull, Low Speed | Physical red `LD1` |

### Implementation concept

```c
HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
HAL_Delay(500);

HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
HAL_Delay(500);
```

Unlike LAB1-1, this lab writes the required output level to each LED explicitly. This is useful when the next state must not depend on the current GPIO state.

### Expected result

The red and green user LEDs alternate every 500 ms, but their states are assigned directly rather than toggled.

---

## LAB1-3 - Turn on the LED while the user button is pressed

Commit: [`0d75b3c`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/0d75b3c962a782af7e7813bd2449297295166547)

### Schematic evidence

![STM32F769I-DISCO user button circuit](assets/schematics/user-button-circuit.png)

![STM32F769I-DISCO user LED circuit](assets/schematics/user-led-circuit.png)

Sheet 4 connects `B_USER` to `PA0/WKUP`. On Sheet 13, `R70` provides an external path to ground when the button is released, while pressing `B1` connects the signal toward `3V3`. The button is therefore active high: released reads `GPIO_PIN_RESET`, and pressed reads `GPIO_PIN_SET`.

`R68` and `C54` also form an RC input filter. Since the board already provides an external pull-down path, I configure `PA0` as `GPIO_Input` with `GPIO_NOPULL`; enabling another internal pull resistor is unnecessary and would change the intended input network.

### CubeMX settings

| Pin | Project label | Mode | Pull | Reason |
|---|---|---|---|---|
| `PA0/WKUP` | `USER_BUTTON` | GPIO Input | No Pull | External `R70` defines the released low state |
| `PJ5` | `LED1` | GPIO Output, Push-Pull | No Pull | Drives physical green `LD2` |

CubeMX must also enable the GPIOA and GPIOJ peripheral clocks before these pins are used.

### Implementation concept

```c
if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}
else
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
}
```

The program continuously copies the active-high button state to the active-high LED output.

### Expected result

The physical green `LD2` turns on while the blue user button is held and turns off when it is released.

---

## LAB1-4 - Toggle the LED on each button press

Commit: [`2ed0059`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/2ed00590ff6809dcea33dca9094377727a833ce6)

### Schematic evidence

![STM32F769I-DISCO user button circuit](assets/schematics/user-button-circuit.png)

![STM32F769I-DISCO user LED circuit](assets/schematics/user-led-circuit.png)

The GPIO configuration is the same as LAB1-3: `PA0` is an active-high input with an external pull-down network, and `PJ5` is an active-high output for the physical green `LD2`. The new concept is edge detection. The program compares the current and previous button samples and reacts only to the low-to-high transition that represents a new press.

### CubeMX settings

| Pin | Project label | Mode | Pull | Reason |
|---|---|---|---|---|
| `PA0/WKUP` | `USER_BUTTON` | GPIO Input | No Pull | External pull-down holds the released state low |
| `PJ5` | `LED1` | GPIO Output, Push-Pull, Low Speed | No Pull | Active-high LED drive |

### Implementation concept

```c
GPIO_PinState currentButtonState;
GPIO_PinState previousButtonState = GPIO_PIN_RESET;

currentButtonState = HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);

if ((currentButtonState == GPIO_PIN_SET) &&
    (previousButtonState == GPIO_PIN_RESET))
{
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}

previousButtonState = currentButtonState;
```

The condition is true only on a detected rising edge, so holding the button does not intentionally toggle the LED repeatedly. The board's `R68/C54` network filters very short changes, but the current firmware does not include a timed software-debounce check. If contact bounce is observed during testing, a later lab can require the input to remain stable for a defined interval before accepting the edge.

### Expected result

Each accepted button press changes the physical green `LD2` between on and off; holding the button keeps the current LED state.

## Current project state

The current `main.c` calls `LAB1_4()`. All four lab functions remain available in `Core/Src/lab01_gpio.c`, with declarations in `Core/Inc/lab01_gpio.h`.
