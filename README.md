# AtomVM ESP32 Camera Driver

The AtomVM Nif can be used to drive cameras that can be attached to some ESP32 SoCs.

> Note.  At present, this driver only includes support for the [AI Thinker ESP32-cam](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/).  Future versions of this driver may include additional SoCs.

This driver is included as an add-on to the AtomVM base image.  In order to use this driver, you must be able to build the AtomVM virtual machine, which in turn requires installation of the Espressif IDF SDK and tool chain.

> Note.  For build instructions of the AtomVM Virtual machine on the ESP32 platform, see [here](https://github.com/bettio/AtomVM/blob/master/doc/atomvm-esp32.md#adding-custom-nifs-and-third-party-components).

## Getting Started

Clone this repository in the `src/platforms/esp32/components` directory of the AtomVM source tree.

    shell$ cd src/platforms/esp32/components
    shell$ git clone https://github.com/fadushin/atomvm_esp32cam.git

Clone the Espressif `esp32-camera` component in the `src/platforms/esp32/components` directory of the AtomVM source tree.

    shell$ cd src/platforms/esp32/components
    shell$ git clone https://github.com/espressif/esp32-camera.git

Create a `component_nifs.txt` file, if it does not already exist, in `src/platforms/esp32/main`.  The contents of this file should contain a single line for the ESP32 cam driver:

    atomvm_esp32cam

Build and flash AtomVM (typically from `src/platforms/esp32`)

    shell$ cd src/platforms/esp32
    shell$ make flash

Once the AtomVM image is flashed to the ESP32 device, it includes NIFs for interfacing with the ESP32 camera on the device.

> Note.  The `atomvm_esp32cam` Nif is currently only supported on the the [AI Thinker ESP32-cam](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/).

## Erlang API

The AtomVM ESP32 camera driver installs additional nifs into the AtomVM virtual machine, which can be used to interface with the camera installed on the ESP32 device.  These Nifs provide a simple API for initialization of the ESP32 camera, and for capturing JPEG images.

In order to capture images, you must first initialize the camera.  To initialize the camera with all of the defaults, use the `esp32cam:init/0` function:

    ok = esp32cam:init().

You may specify a configuration properties list with any of the following properties:

    -type esp32cam_config_item() ::
        {frame_size, qvga|cif|vga|svga|xga|sxga|uxga}
        | {jpeg_quality, 0..63}.
    -type esp32cam_config() :: [esp32cam_config_item()].

The default frame size is `xga` and the default jpeg quality is `12`  (lower is better quality, but larger size.)

To capture a JPEG image, use the `captture/0` or `capture/1` function.  The `capture/1` function takes a properties list.  Currently, you may specify whether the flash should be on or off (off, by default).

    -type capture_param() :: {flash, on|off}.
    -type capture_params() :: [capture_param()].

## Example Program

This repository contains a sample program in the `examples/esp32cam_example` directory for initializing the ESP32 camera and taking a single capture.

This smaple program uses [`rebar3`](https://www.rebar3.org) to build and flash the application to your ESP32 device.  For example,

    shell$ cd src/platforms/esp32/components/atomvm_esp32cam/examples/esp32cam_example
    shell$ rebar3 flash
