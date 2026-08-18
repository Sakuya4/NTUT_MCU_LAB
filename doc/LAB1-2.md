# LAB1-2 - Control each LED explicitly

[Back to the main README](../README.md)

Commit: [`4ac0ab2`](https://github.com/Sakuya4/NTUT_MCU_LAB/commit/4ac0ab246083fc835700e01deb8299e6ced264b9)

## Goal

Set the two user LED outputs to known states and exchange those states every 500 ms.

## Schematic evidence

![STM32F769I-DISCO user LED circuit](../assets/schematics/user-led-circuit.png)

The LED cathodes are connected to ground, so the circuits are active high. `GPIO_PIN_SET` turns an LED on, while `GPIO_PIN_RESET` turns it off. The pins remain low-speed push-pull outputs with no pull resistors.

## User-manual cross-check

[UM2033 Section 5.16, Table 4](https://www.st.com/resource/en/user_manual/um2033-discovery-kit-with-stm32f769ni-mcu-stmicroelectronics.pdf) maps physical red `LD1` to `PJ13` and physical green `LD2` to `PJ5`. Because this project names `PJ5` as `LED1` and `PJ13` as `LED2`, the firmware labels do not match the printed `LD1` and `LD2` designators.

Although the surrounding manual text describes active-low LEDs, the B-02 schematic shows grounded cathodes. Consequently, `GPIO_PIN_SET` means on in this project.

## CubeMX settings

| Pin | Project label | Mode | Reason |
|---|---|---|---|
| `PJ5` | `LED1` | GPIO Output, Push-Pull, No Pull, Low Speed | Drives one user LED directly |
| `PJ13` | `LED2` | GPIO Output, Push-Pull, No Pull, Low Speed | Drives the other user LED directly |

## Implementation concept

```c
HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
HAL_Delay(500);

HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
HAL_Delay(500);
```

Unlike LAB1-1, this lab writes the required output level to each LED explicitly. This method is useful when the next output state must not depend on the current state.

## Expected result

The red and green user LEDs alternate every 500 ms, but their states are assigned directly rather than toggled.
