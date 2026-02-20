# Raspberry Pi Pico Projects

This repository contains a collection of my Raspberry Pi Pico projects written in C using PicoSDK.

## Projects

**stopwatch**
  - Simple digital stopwatch

**button_interrupt**
  - LED control using button and GPIO interrupt

Each project is located in its own folder and includes separate README file with detailed informations.

## Platform and tools

- Raspberry Pi Pico
- Pico SDK
- C / CMake
- VS Code

## Build steps

Each project is built using the following steps:
- mkdir build
- cd build
- set PICO_SDK_PATH=C:\VSARM\sdk\pico\pico-sdk
- cmake -G "MinGW Makefiles" ..
- make
