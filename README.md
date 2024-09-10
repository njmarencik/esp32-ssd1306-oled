# ESP32 SSD1306 OLED Driver

This project allows an ESP32 microcontroller to interface with an SSD1306, which is an inexpensive and common OLED display. This driver is currently written for the SPI version of the SSD1306, but can be modified to work with the I2C interface with relative ease.

This software was written for use with the ESP-IDF, which is the ESP32 official development framework.

## Usage

### Requirements

In order to compile and upload this software onto an ESP32 board, you will need to have downloaded the [ESP-IDF](https://github.com/espressif/esp-idf), then run the installation script `./install.sh` in Bash (or any of the other scripts applicable to your shell). Next, export variables to your shell using `. ./export.sh`, or whichever script you are told to run by the installer.

### Building

To build, clone the repo and enter into the top directory. Then run:

```bash
idf.py build
```

Now, to upload the code to your ESP32 board, you may need to have the proper permissions first. On Arch Linux, I use the command `sudo dmesg` to find where my board is attatched when I plug it in. Usually, I find it is attatched to `ttyUSB0`, but it may be different for you. Then, I change the USB permissions with:

```bash
sudo chmod 777 /dev/ttyUSB0
```

You may want to be more restrictive with your permissions, but using 777 is fine for my home computer. Finally, I flash the code onto my ESP32 by running:

```bash
idf.py -p /dev/ttyUSB0 flash
```

The ESP-IDF has an issue, and sometimes the upload will fail on the first try. I have always been able to resolve it by re-running the flash command. Sometimes pressing the BOOT reset button on the ESP32 will also help. It seems as though it is just some initial communication error between the IDF and the board.


## Reference

This software was created by referencing the following datasheet:

[https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

The code was tested using an [ESP32](https://a.co/d/1kd4TJb) and an [SPI OLED Display Module](https://a.co/d/44z7wI1).





