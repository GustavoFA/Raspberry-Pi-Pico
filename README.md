# Raspberry Pi Pico 

## Overview 

This repository is an exploration of the Raspberry Pi Pico microcontroller board. It contains C++ and MicroPython projects that demonstrate the capabilities of the microcontroller, including GPIO control, PWM, I2C, SPI, ADC, and more.

For all projects we used a Raspberry Pi Pico W board.

For this projects I am using the VScode with the extesion Raspberry Pi Pico on Linux system (Ubuntu 24.04).

## How to use the Raspberry Pi Pico on VScode 

The following steps will help you to setup the development environment on VScode, build and flash the project to the Raspberry Pi Pico.

### Basic setup

* Install VScode on your system
```bash
sudo apt install code
```
* Install the extesion Raspberry Pi Picon on VScode

* Install GCC
```bash
sudo apt install build-essential
```
Check version
```bash
gcc --version
```

### Pico SDK

For these projects we'll need the Pico-SDK. For this, clone the repository [pico-sdk](https://github.com/raspberrypi/pico-sdk).

* Clone repository

```bash
mkdir -p ~/pico
cd ~/pico
git clone -b master --recursive https://github.com/raspberrypi/pico-sdk.git
```

* Set the environment variable

```bash
echo 'export PICO_SDK_PATH=$HOME/pico/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

* Check if the environment variable is set

```bash
echo $PICO_SDK_PATH
```

### FreeRTOS-Kernel

For these projects we'll need the FreeRTOS-Kernel. For this, clone the repository [FreeRTOS-Kernel]()

```bash
mkdir -p "$HOME/FreeRTOS_Kernel/"
cd "$HOME/FreeRTOS_Kernel/"
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
```

Check if the FreeRTOS-Kernel is cloned

```bash
ls "$HOME/FreeRTOS_Kernel/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040"
```

### projectTemplate

Use this the project template to create your own project. For this, copy the projectTemplate folder and rename it to your project name. 

```bash
cp -r projectTemplate NewProject
```

### Build the project

On the project folder, run the following command to build the project.

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Flash 

Insert the Raspberry Pi Pico to your system holding the BOOTSEL button pressed. The Raspberry Pi Pico will be mounted as a USB drive. Copy the .uf2 file from the build folder to the Raspberry Pi Pico.

```bash
cp your_project/build/src/your_project.uf2 /media/$USER/RPI-RP2
```

### Serial Monitor

You can use the picocom as serial monitor. For this, install the picocom on your system.

Linux

```bash
sudo apt install picocom
```

```bash
picocom -b 115200 /dev/ttyACM0
```

## Projects [UnderDevelopment]

* 001 - Blink


## References

* [Pico Microcontroller boards](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html)

* [Raspberry Pi Pico pinout Diagram](https://pico.pinout.xyz/)