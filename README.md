# Smart Plant Care Monitor — STM32G071 Bare-Metal Project

## Overview

Smart Plant Care Monitor is an embedded systems project based on the STM32G071 microcontroller.

The goal of the project is to monitor basic plant conditions, such as soil moisture and light level, and provide a clear status indication using an LCD, LEDs, and UART debug logs.

This project is developed as a portfolio-level embedded project after completing a bare-metal STM32 driver development course.

## Project Overview Diagram

The following diagram shows the high-level idea of the system: sensor inputs, STM32G071 processing, user interface outputs, UART debug logs, and future extensions.

![Smart Plant Care Monitor Overview](Images/system-architecture-diagram.png)

## MVP Features

- Read soil moisture sensor using ADC
- Read light level using ADC
- Process sensor readings and determine plant status
- Display plant status on LCD 16x2
- Indicate status using LEDs
- Print debug logs using UART
- Use a simple state machine for application flow

## Planned Future Features

- BME280 environmental sensor support
- RTC timestamp support
- MicroSD CSV data logging
- Water level monitoring
- Low-power mode
- Automatic watering support

## Target Board

- STM32G071RB / NUCLEO-G071RB

## Project Status

Planning and initial project setup.