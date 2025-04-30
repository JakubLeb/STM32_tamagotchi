<h1 align="center">
  <br>
  <img src="https://github.com/user-attachments/assets/6de2ebac-a827-41ec-adac-e05586c6a51b" alt="logo" width="200">
  <br>
  STM32_Tamagotchi
  <br>
</h1>

<h4 align="center">A virtual pet game implemented on the STM32F411RE using peripherals like OLED (SSD1309), BH1750 light sensor, and momentary push buttons.</h4>

<p align="center">
  <a href="#-communication-protocol">Communication Protocol</a> •
  <a href="#-specification">Specification</a> •
  <a href="#-how-to-set-up">How To Set Up</a> •
  <a href="#-how-to-use">How To Use</a>
</p>

<p align="center">
  <img src="https://github.com/user-attachments/assets/90ea1446-0616-4c1c-a507-089f280435ec" alt="preview">
</p>

---

## 📡 Communication Protocol

### Frame Format

| Field        | Start | Sender | Receiver | Cmd Len | Data    | CRC    | End  |
|--------------|-------|--------|----------|---------|---------|--------|------|
| Character    | `{`   | XXX    | XXX      | 000     | X…X     | 0000   | `}`  |
| Hex Value    | 0x7B  | Any ASCII except `{`, `}` | Same | 0x30–0x39 | Escaped chars | Hex (0x30–0x39, 0x41–0x46) | 0x7D |
| Length (char)| 1     | 3      | 3        | 3       | 0–256   | 4      | 1    |

> Use `\{` and `\}` as escape sequences inside the Data field.

### Error Handling Includes

- Multiple `{` or `}` characters
- Escape character misuse
- CRC mismatch
- Data length mismatch
- Invalid receiver or unknown command
- Forbidden characters in the wrong fields

---

## 🕹 Commands

| Command             | Format                | Description                          |
|---------------------|-----------------------|--------------------------------------|
| `FOOD_ADD[n]`       | Add food (1–5)        | Adds food units                      |
| `FOOD_SUBTRACT[n]`  | Subtract food (1–5)   | Removes food                         |
| `FOOD_SET[n]`       | Set food (0–5)        | Direct set                           |
| `FOOD_CLEAR[X]`     | Reset food            | Clears food to 0                     |
| `ENERGY_ADD[n]`     | Add energy (1–5)      | Adds energy                          |
| `ENERGY_SUBTRACT[n]`| Subtract energy       | Removes energy                       |
| `FUN_ADD[n]`        | Add fun (1–5)         | Adds fun                             |
| `FUN_CLEAR[X]`      | Reset fun             | Clears fun to 0                      |
| `MENU_CHANGE[n]`    | Change menu (1–4)     | Switch between menus                 |
| `WAKEUP_MIN[n]`     | Minigame control      | Modify wakeup parameter              |

### Example Command Frame

```text
{PC0STM011FOOD_ADD[3]5DCF}
```

---

## 🔁 Response Messages

### ❗ Error Responses

| Code         | Size (chars) | Description                                                  |
|--------------|---------------|--------------------------------------------------------------|
| `WRCMD`      | 5             | Invalid command format or unrecognized keyword              |
| `WRFRAME`    | 7             | Incorrectly structured frame                                |
| `WRVALUE`    | 7             | Value out of allowed range or format                        |
| `WRMENU`     | 6             | Menu change not allowed during a mini-game                 |
| `WRCRC`      | 6             | CRC mismatch – frame integrity error                        |
| `WRLENDATA`  | 9             | Command length field doesn't match actual data length       |
| `WRWAITGAME` | 10            | Attempt to change attributes during active mini-game        |

### ✅ Success Responses

| Code          | Size (chars) | Description                                        |
|---------------|---------------|----------------------------------------------------|
| `ACCEPTMENU`  | 10            | Menu successfully changed                         |
| `ACCEPTADDAT` | 11            | Value successfully added to an attribute          |
| `ACCEPTSET`   | 9             | Attribute value successfully set                  |
| `ACCEPTSUBAT` | 11            | Value successfully subtracted from an attribute   |
| `ACCEPTRESAT` | 11            | Attribute successfully reset                      |

### Example Response

```text
{STMPC0011ACCEPTADDAT8FBB}
```

---

## ⚙️ Specification

- `1x` NUCLEO-F411RE (STM32)
- `1x` SSD1309 OLED 128×64 2.42"
- `1x` BH1750 Light Sensor (I²C)
- `4x` Buttons (SparkFun PRT-14460)
- `4x` 1.2kΩ Resistors
- `4x` Male-Male Wires
- `15x` Male-Female Wires

---

## 🛠 How To Set Up

<p align="center">
  <img src="https://github.com/user-attachments/assets/5985cfd4-7a68-48b9-af54-37629f853e36" alt="project setup">
</p>

### 🔆 BH1750 Light Sensor

| BH1750 Pin | Description               | STM32 Pin          |
|------------|---------------------------|---------------------|
| VIN        | Power supply (3.3V or 5V) | 3.3V                |
| 3Vo        | 3.3V Output               | Not connected       |
| GND        | Ground                    | GND                 |
| SCL        | I²C Clock Line            | PB8                 |
| SDA        | I²C Data Line             | PB9                 |
| ADDR       | I²C Address               | GND                 |

### 🖥 SSD1309 OLED Display (SPI)

| SSD1309 Pin | Description             | STM32 Pin |
|-------------|-------------------------|-----------|
| GND         | Ground                  | GND       |
| VCC         | Power supply (3.3V)     | 3.3V      |
| SCK         | SPI Clock               | PA5       |
| SDA         | SPI Data (MOSI)         | PA7       |
| RES         | Reset                   | PA6       |
| DC          | Data/Command            | PB6       |
| CS          | Chip Select             | PB7       |

### 🔘 Buttons

| Button | STM32 Pin |
|--------|-----------|
| P1     | PC0       |
| P2     | PC1       |
| P3     | PC2       |
| P4     | PC3       |

---

## 🧩 How To Use

1. Flash the firmware to the STM32F411RE board.
2. Connect the components according to the pinout diagrams above.
3. Open a UART terminal (e.g., PuTTY or serial monitor).
4. Send command frames in the described protocol format.
5. Watch the OLED display for Tamagotchi actions and responses.
6. Play the mini-games using the buttons:
   - **Menu 2**: "Sleep" minigame reacts to ambient light using BH1750.
   - **Menu 3**: "Chase the food" game uses button input to collect food and gain fun.
