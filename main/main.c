
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_system.h"


static const char *TAG = "main";


#define OLED_SPI_HOST SPI2_HOST


// Define pins before including the oled library
#define LED_PIN 2
#define DATA_PIN 23
#define CLK_PIN 18
#define CMD_PIN 4
#define CS_PIN 5
#define RST_PIN 21

#include "ssd1306_oled.h"


static uint8_t led_state = 0;


void app_main(void){
	
	
	spi_device_handle_t spi = oled_spi_setup();

	oled_init(spi);

	oled_clear_screen(spi);
	oled_test(spi);
	
	//draw_page_string(spi, 0, "!\"#$%&\'()*+,-./01234567\n89:;<=>?@ABCD\nEFGHIJKLMNOP\nQRSTUVWXYZ\n[\\]^_`abcd\nefghijklmnopqrs\ntuvwxyz\n{|}~");
	int status = 125;
	int myhex = 0xa7;
	double bigf = 3.1415;
	float lilf = 2.71;
	oled_tty_printf(spi, 0, "Status: %d\nLittle hex: %x\nHEX: 0x%X\n   Pi: %lf\n   E: %f", status, myhex, myhex, bigf, lilf);
	oled_tty_printf(spi, 5, "A whole new line!\nand char: %c", 'f');

	vTaskDelay(50);

	gpio_set_level(LED_PIN, 1);
	
	vTaskDelay(50);
	while(1){
		vTaskDelay(50);
	}

}
