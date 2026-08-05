# Raspberry Pi Pico 

## Overview [NotReady]

For this projects I am using the VScode with the extesion Raspberry Pi Pico

## How to use the Raspberry Pi Pico on VScode [UnderDevelopment]

* Install the extesion Raspberry Pi Pico on VScode

* 

### projectTemplate

Use this the project template to create your own project. 

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

* 001 - 

## Pico SDK

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

## FreeRTOS-Kernel

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