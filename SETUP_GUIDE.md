# MiniOsci — Setup & Build Guide

> Step-by-step instructions to build, flash, and work with this project.

---

## Prerequisites

### Hardware Required

| Component | Quantity | Notes |
|-----------|----------|-------|
| STM32F411CEU6 BlackPill | 1 | WeAct V3 recommended |
| 2.8" ILI9341 TFT (SPI) | 1 | 240×320, 4-wire SPI interface |
| Tactile push buttons | 4 | Momentary, normally open |
| 10 kΩ resistor | 2 | For ADC voltage dividers |
| 100 kΩ resistor | 2 | For ADC voltage dividers |
| 1 kΩ resistor | 1 | Function generator RC filter |
| 10 nF ceramic capacitor | 1 | Function generator RC filter |
| 10 kΩ resistor (pull-down) | 4 | For push buttons (optional if using internal pull) |
| Breadboard + jumper wires | — | For prototyping |
| ST-Link V2 | 1 | SWD debugger/programmer |

### Software Required

| Software | Version | Download |
|----------|---------|----------|
| STM32CubeIDE | 1.13+ | [st.com/stm32cubeide](https://www.st.com/en/development-tools/stm32cubeide.html) |
| ST-Link drivers | Latest | Included with STM32CubeIDE |
| Git | Any | [git-scm.com](https://git-scm.com/) |

---

## Step 1: Clone the Repository

```bash
git clone https://github.com/Jashdhokiya/miniOsci.git
cd miniOsci
```

---

## Step 2: Hardware Assembly

### 2.1 — TFT Display Wiring

Connect the ILI9341 TFT display to the BlackPill:

| TFT Pin | → | BlackPill Pin | Wire Color |
|---------|---|---------------|------------|
| VCC | → | 3.3V | Red |
| GND | → | GND | Black |
| CS | → | PA4 | Blue |
| RST | → | PB6 | Blue |
| DC | → | PA3 | Blue |
| MOSI (SDA) | → | PA7 | Blue |
| SCK (CLK) | → | PA5 | Blue |
| LED | → | PB7 | Blue |

> **Note**: MISO is not used (TX-only SPI). Leave it unconnected.

### 2.2 — Push Buttons

Each button connects between **3.3V** and the button pin, with a **10kΩ pull-down resistor** to GND:

```
3.3V ──── [Button] ──── PB0 ──── [10kΩ] ──── GND    (Run/Hold)
3.3V ──── [Button] ──── PB1 ──── [10kΩ] ──── GND    (Time/Div)
3.3V ──── [Button] ──── PB2 ──── [10kΩ] ──── GND    (Volt/Div)
3.3V ──── [Button] ──── PB3 ──── [10kΩ] ──── GND    (Channel)
```

> Pressing the button pulls the pin HIGH → triggers a rising-edge EXTI interrupt.

### 2.3 — ADC Input Voltage Dividers

For each analog input channel, build a resistive voltage divider:

```
Signal In ──── [100kΩ] ──── PA0 ──── [10kΩ] ──── GND    (Channel 1)
Signal In ──── [100kΩ] ──── PA1 ──── [10kΩ] ──── GND    (Channel 2)
```

- Divider ratio: 10k / (100k + 10k) = **1:11**
- This allows measuring up to **36.3V** safely (3.3V × 11)
- For signals **under 3.3V**, you can connect directly to PA0/PA1 (bypass the divider)

### 2.4 — Function Generator (RC Filter)

Build a simple low-pass filter on PB8:

```
PB8 ──── [1 kΩ] ────┬──── Analog Output
                     │
                  [10 nF]
                     │
                    GND
```

- Output impedance: ~1 kΩ
- Cutoff frequency: ~16 kHz
- **To test**: connect "Analog Output" to PA0 with a jumper wire

### 2.5 — ST-Link Connection

| ST-Link Pin | → | BlackPill Pin |
|-------------|---|---------------|
| SWDIO | → | SWDIO (labeled on board) |
| SWCLK | → | SWCLK (labeled on board) |
| GND | → | GND |
| 3.3V | → | 3.3V (optional, can power via USB instead) |

---

## Step 3: Import the Project

1. Open **STM32CubeIDE**
2. Go to **File → Import**
3. Select **General → Import an Existing STM32CubeMX Configuration File (.ioc)**
4. Browse to the cloned `miniOsci` folder and select `MiniOsci.ioc`
5. Click **Finish** — the IDE will create the project structure

> **Alternative**: If you see "Import Existing Projects into Workspace", that works too if the `.cproject` file is present.

---

## Step 4: Build the Project

1. In the **Project Explorer**, right-click on `MiniOsci`
2. Select **Build Project** (or press `Ctrl+B`)
3. Check the **Console** tab at the bottom for build output
4. Expected result: `Build Finished. 0 errors, 0 warnings`

### If you get errors:

| Error | Fix |
|-------|-----|
| `func_gen.h: No such file` | Right-click project → Properties → C/C++ Build → Settings → MCU GCC Compiler → Include paths → Add `../Core/Inc` and `../Drivers/ILI9341` |
| `undefined reference to MX_TIM4_Init` | Make sure `Core/Src/tim.c` is included in the build (check source locations in project properties) |
| `multiple definition of TIM5_IRQHandler` | Make sure it's only defined once in `stm32f4xx_it.c` |

---

## Step 5: Flash to Hardware

1. Connect the **ST-Link** to the BlackPill via the SWD pins
2. Power the BlackPill via USB
3. In STM32CubeIDE: **Run → Debug Configurations**
4. Create a new **STM32 C/C++ Application** debug configuration
5. Click **Debug** (or press `F11`)
6. The firmware will be flashed and the debugger will pause at `main()`
7. Press **F8** (Resume) to run

### Quick Flash (no debugger):

```
Run → Run As → STM32 C/C++ Application
```

---

## Step 6: Verify Everything Works

Follow this testing sequence:

### Test 1: Display

**Expected**: Splash screen appears on the TFT showing:
```
OSCILLOSCOPE
Dual Channel
+ Function Generator
STM32F411 BlackPill
```
Then clears after 1.5 seconds and shows the oscilloscope grid.

**If no display**: Check SPI wiring (PA5, PA7, PA3, PA4), power (3.3V), and LED pin (PB7).

### Test 2: Buttons

| Button | Expected Behavior |
|--------|-------------------|
| PB0 (Run/Hold) | Bottom bar toggles between "RUN" (green) and "HOLD" (red) |
| PB1 (Time/Div) | Bottom bar T: value cycles: 10µs → 20µs → ... → 1ms |
| PB2 (Volt/Div) | Bottom bar V: value cycles: 0.5V → 1V → 2V → 5V |
| PB3 (Channel) | Top bar toggles between CH1 (yellow) and CH2 (cyan) |

**If buttons don't respond**: Check pull-down resistors and EXTI wiring.

### Test 3: Function Generator

1. Measure PB8 with a multimeter → should read **~1.65V DC** (midpoint of 3.3V sine)
2. Wire PB8 → RC filter → PA0
3. The oscilloscope display should show a **1 kHz sine wave**
4. Bottom bar shows: `FG:SIN 1kHz` in orange

### Test 4: External Signal

1. Connect an external signal source to the voltage divider input on PA0
2. Keep signal within **0–36V** range
3. Adjust Time/Div and Volt/Div with buttons to see the waveform clearly
4. Frequency and Vpp measurements appear in the top bar

---

## Project Structure Explained

```
MiniOsci/
│
├── Core/
│   ├── Inc/                          # All header files
│   │   ├── config.h                  # ⭐ ALL constants (change settings here)
│   │   ├── func_gen.h                # Function generator API
│   │   ├── osc_adc.h                 # ADC control API
│   │   ├── osc_display.h             # Display rendering API
│   │   ├── osc_signal.h              # Signal processing API
│   │   ├── osc_ui.h                  # UI/button state API
│   │   ├── fonts.h                   # Bitmap font data
│   │   ├── main.h                    # CubeMX: pin defines, HAL includes
│   │   ├── adc.h / dma.h / gpio.h    # CubeMX: peripheral handles
│   │   ├── spi.h / tim.h / usart.h   # CubeMX: peripheral handles
│   │   ├── stm32f4xx_hal_conf.h      # CubeMX: HAL module enables
│   │   └── stm32f4xx_it.h            # CubeMX: interrupt declarations
│   │
│   ├── Src/                          # All source files
│   │   ├── main.c                    # ⭐ Entry point, init sequence, main loop
│   │   ├── func_gen.c                # Function generator (LUTs + ISR)
│   │   ├── osc_adc.c                 # ADC/DMA start/stop, de-interleave
│   │   ├── osc_display.c             # Grid, waveform, measurement drawing
│   │   ├── osc_signal.c              # Freq, Vpp, trigger, pixel mapping
│   │   ├── osc_ui.c                  # Button debounce + state machine
│   │   ├── stm32f4xx_it.c            # ⭐ All interrupt handlers
│   │   ├── adc.c / dma.c / gpio.c    # CubeMX generated
│   │   ├── spi.c / tim.c / usart.c   # CubeMX generated
│   │   └── system_stm32f4xx.c        # System init (CubeMX)
│   │
│   └── Startup/
│       └── startup_stm32f411ceux.s   # Vector table, reset handler
│
├── Drivers/
│   ├── CMSIS/                        # ARM Cortex-M4 core headers
│   ├── ILI9341/                      # TFT display driver (ili9341.c/h)
│   └── STM32F4xx_HAL_Driver/         # ST HAL library (full)
│
├── docs/                             # Diagrams and images
│   ├── pin_diagram.png
│   └── wiring_diagram.png
│
├── MiniOsci.ioc                      # ⭐ CubeMX config (open to reconfigure)
├── STM32F411CEUX_FLASH.ld            # Linker script (Flash)
├── STM32F411CEUX_RAM.ld              # Linker script (RAM)
├── DOCUMENTATION.md                  # Technical documentation
├── SETUP_GUIDE.md                    # This file
├── README.md                         # Project overview
└── .gitignore                        # Git ignore rules
```

---

## How to Modify the Project

### Change Function Generator Waveform/Frequency

In `main.c`, after `FuncGen_Start()`:
```c
FuncGen_SetWaveform(WAVE_SQUARE);    // WAVE_SINE, WAVE_SQUARE, WAVE_TRIANGLE, WAVE_SAWTOOTH
FuncGen_SetFrequency(5000);          // Set to 5 kHz (range: 100–10000 Hz)
```

### Change Default Oscilloscope Settings

In `config.h`:
```c
#define SAMPLE_BUFFER_SIZE    256     // Samples per channel
#define VREF                  3.3f   // ADC reference voltage
#define VOLTAGE_DIVIDER_RATIO 11.0f  // Change if using different divider
#define BUTTON_DEBOUNCE_MS    200    // Button debounce time
```

### Change Display Colors

In `config.h`, modify the RGB565 color values:
```c
#define COLOR_CH1             COLOR_YELLOW   // Channel 1 waveform color
#define COLOR_CH2             COLOR_CYAN     // Channel 2 waveform color
#define COLOR_GRID            COLOR_DARK_GRAY
#define COLOR_BACKGROUND      COLOR_BLACK
```

### Add a New Time/Div Step

In `config.h`, update the arrays:
```c
#define NUM_TIME_DIVS         8    // Was 7
static const uint32_t TIME_DIV_US[] = { 10, 20, 50, 100, 200, 500, 1000, 2000 };
static const char* TIME_DIV_LABELS[] = {
    "10us", "20us", "50us", "100us", "200us", "500us", "1ms", "2ms"
};
static const uint32_t TIME_DIV_ARR[] = {
    99, 199, 499, 999, 1999, 4999, 9999, 19999
};
```

### Reconfigure Pins in CubeMX

1. Open `MiniOsci.ioc` in STM32CubeIDE (double-click it)
2. Modify pin assignments, clock tree, or peripherals
3. Click **Generate Code** (gear icon)
4. CubeMX will regenerate the peripheral init files
5. Your custom code inside `USER CODE BEGIN/END` blocks is preserved

---

## Common Issues & Troubleshooting

| Issue | Possible Cause | Fix |
|-------|----------------|-----|
| No display | SPI wiring wrong | Check PA5 (SCK), PA7 (MOSI) connections |
| Display white/blank | LED pin not HIGH | Check PB7 is connected to TFT backlight |
| Buttons not working | Missing pull-down resistors | Add 10kΩ from each button pin to GND |
| Erratic button behavior | Debounce too short | Increase `BUTTON_DEBOUNCE_MS` in config.h |
| No waveform on screen | ADC not sampling | Check TIM2 is running (`HAL_TIM_Base_Start`) |
| Flat line on display | No input signal | Connect function generator output to PA0 |
| FuncGen no output | RC filter not connected | Check PB8 → 1kΩ → 10nF → GND wiring |
| Build error: file not found | Include path missing | Add `../Core/Inc` and `../Drivers/ILI9341` to compiler include paths |
| HardFault on boot | Clock config issue | Verify HSE crystal is 25 MHz on your BlackPill board |

---

## License

Educational project for embedded systems lab.
STM32 HAL and CMSIS are licensed by STMicroelectronics/ARM (see `Drivers/*/LICENSE.txt`).
