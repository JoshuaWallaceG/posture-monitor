# Posture Monitor

A chair-based posture monitor that detects uneven sitting pressure and alerts you when you start to slouch.

## Features
- Measures seating pressure across three sensor locations
- Visualizes real-time weight distribution on an LCD display
- Detects posture imbalance and poor sitting habits
- Provides immediate feedback when posture thresholds are exceeded

## Wiring Diagram
<img width="65%" alt="FinalCircuit" src="https://github.com/user-attachments/assets/3629cf42-3abc-446f-a9d8-ec593a09d5d6" />

Final wiring diagram for the STM32, force sensors, and ST7735 display.

## Photos
<img width="70%" alt="nochair" src="https://github.com/user-attachments/assets/c03e9527-be26-40aa-9280-924b1816b71f" />

Current hardware prototype before chair installation.

## How to Run
Build and flash the firmware to the STM32 Nucleo board using STM32CubeIDE.

## Technical Concepts
- Circular DMA for continuous multi-channel ADC sensor acquisition
- Interrupt-driven firmware architecture
- SPI communication with custom ST7735 LCD driver
- RAM frame buffer and bitmap font rendering
- Sensor calibration and linearization for pressure metrics

## Built With
- C
- STM32 Nucleo-L476RG
- ST7735 LCD Display
- STM32CubeIDE/MX
