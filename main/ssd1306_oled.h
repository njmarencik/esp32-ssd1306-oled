
#ifndef _OLED_SSD1306_H_
#define _OLED_SSD1306_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_system.h"

#include "ssd1306_chars.h"


#define MAX_PAGE 7


#define MAX_FORMATTED_SIZE 32
#define MAX_FORMAT_SPECIFIER_SIZE 4


void spi_write_byte(spi_device_handle_t spi, uint8_t data);
void spi_send_cmd(spi_device_handle_t spi, uint8_t cmd);
void oled_clear_screen(spi_device_handle_t spi);
void oled_test(spi_device_handle_t spi);



void spi_write_byte(spi_device_handle_t spi, uint8_t data){

	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.length = 8 * 1; //Only sending one byte
	transaction.flags = 0; //SPI_TRANS_USE_RXDATA;
	transaction.tx_buffer = &data;

	esp_err_t ret = spi_device_polling_transmit(spi, &transaction);
	if(ret == ESP_OK){
		;
	}
	ESP_ERROR_CHECK(ret);
	//spi_device_release_bus(spi);
}

void spi_send_cmd(spi_device_handle_t spi, uint8_t cmd){
	gpio_set_level(CMD_PIN, 0); // Set DC pin LOW for cmd sending
	spi_write_byte(spi, cmd);
}


void oled_clear_screen(spi_device_handle_t spi){
	int page, col;
	uint8_t p;
	for(page=0; page<8; page++){
		gpio_set_level(CMD_PIN, 0); // Set D/C pin for cmd writing
		p = (uint8_t)(page & 0xff);
		spi_write_byte(spi, 0xB0 | p);
		spi_write_byte(spi, 0x00); // Set the column address
		spi_write_byte(spi, 0x10); // to zero (start at the left)
		gpio_set_level(CMD_PIN, 1); // Set the D/C pin to write data

		// Write zeros to screen data
		for(col=0; col<128; col++){
			spi_write_byte(spi, 0x00); // Write the zeros to the screen RAM
		}
	}
	gpio_set_level(CMD_PIN, 0);
}


void oled_test(spi_device_handle_t spi){

	uint8_t draw[] = {0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
			  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
			  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
			  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa};
	int drawlen = 24, i;
	
	gpio_set_level(CMD_PIN, 0); // Set D/C pin for cmd writing
	spi_write_byte(spi, 0xB0);
	spi_write_byte(spi, 0x00); // Set the column address
	spi_write_byte(spi, 0x10); // to zero (start at the left)
	gpio_set_level(CMD_PIN, 1); // Set the D/C pin to write data
	vTaskDelay(50);

	for(i=0; i<drawlen; i++){	
		spi_write_byte(spi, draw[i]);
	}

	gpio_set_level(CMD_PIN, 0); // reset command pin
}


void oled_tty_printf(spi_device_handle_t spi, uint8_t page, char *str, ...){

	va_list valist;

	char *s = str;
	int i, va_num = 0;
	//const char *arg_chars = "bcdfxX";
	//const int arg_chars_size = 6;

	va_start(valist, str);

	// Set the page
	spi_send_cmd(spi, 0xB0 | (page & 0x0f));
	spi_send_cmd(spi, 0x00);
	spi_send_cmd(spi, 0x10);
	gpio_set_level(CMD_PIN, 1); // Set oled to have data written to it
	char c;
	char modstr[MAX_FORMATTED_SIZE]; // Stores the resulting string of a number
	char fmt[MAX_FORMAT_SPECIFIER_SIZE]; // Stores the formatting code (ex: "%lf")
	fmt[MAX_FORMAT_SPECIFIER_SIZE - 1] = '\0';
	// Loop through string until null byte
	while(*s){
		if(*s == '%'){
			fmt[0] = '%';
			fmt[1] = *(++s);
			fmt[2] = fmt[1] == 'l' ? *(++s) : '\0';
			char t = fmt[1] == 'l' ? fmt[2] : fmt[1];
			int modlen=0;
			printf("fmt: %s\n", fmt);
			switch(t){
				case 'c':
					// Intentional fallthrough
				case 'd':
					// Intentional fallthrough
				case 'x':
					// Intentional fallthrough
				case 'X':
					long val = va_arg(valist, long);
					modlen = sprintf(modstr, fmt, val);
					break;
				case 'f':
					// float is promoted to double through va_arg
					double fval = va_arg(valist, double);
					modlen = sprintf(modstr, fmt, fval);
					break;
				default:
					break;
			}
			s++;
			//printf("modstr: %s\n", modstr); 
			for(int l=0; l<modlen; l++){
				c = modstr[l];
				for(i=0; i<CHAR_WIDTH; i++){
					spi_write_byte(spi, chartex[(int)c][i]);
				}
			}
			continue;
		}
		
		// If newline, increase the page number
		if(*s == '\n'){
			page++;
			if(page > MAX_PAGE){return;}
			spi_send_cmd(spi, 0xB0 | (page & 0x0f));
			spi_send_cmd(spi, 0x00);
			spi_send_cmd(spi, 0x10);
			s++;
			gpio_set_level(CMD_PIN, 1); // Go back to data writing mode
			continue;
		}
		else {
			c = *s;
		}
		// If the character is drawable, write it to the oled
		for(int i=0; i<CHAR_WIDTH; i++){
			spi_write_byte(spi, chartex[(int)c][i]);
		}
		s++;
	}
	gpio_set_level(CMD_PIN, 0); // Reset for command writing

}



spi_device_handle_t oled_spi_setup(void){
	
	//Init the spi bus
	spi_device_handle_t spi;

	spi_bus_config_t spibus;
	memset(&spibus, 0, sizeof(spibus));
	spibus.miso_io_num = -1;
	spibus.mosi_io_num = DATA_PIN;
	spibus.sclk_io_num = CLK_PIN;
	spibus.quadwp_io_num = -1;
	spibus.quadhd_io_num = -1;
	//spibus.max_transfer_sz = 32;
	
	//Set up the oled device on spi
	spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(devcfg));
	devcfg.clock_speed_hz = 1 * (1000 * 1000); // 1 MHz clock
	devcfg.mode = 0;
	devcfg.spics_io_num = -1; //CS_PIN;
	devcfg.queue_size = 7;
	devcfg.command_bits = 0;
	devcfg.address_bits = 0;
	devcfg.dummy_bits = 0;
	devcfg.flags = SPI_DEVICE_HALFDUPLEX; //| SPI_DEVICE_3WIRE;
	devcfg.pre_cb = NULL;
	devcfg.post_cb = NULL;


	spi_bus_initialize(OLED_SPI_HOST, &spibus, SPI_DMA_CH_AUTO);
	spi_bus_add_device(OLED_SPI_HOST, &devcfg, &spi);

	return(spi);
}


void oled_init(spi_device_handle_t spi) {

	// Init and set the Data/Command pin low
	gpio_reset_pin(CS_PIN);
	gpio_set_direction(CS_PIN, GPIO_MODE_OUTPUT);
	gpio_set_level(CS_PIN, 0);
	vTaskDelay(10);


	// Init and set the Data/Command pin low
	gpio_reset_pin(CMD_PIN);
	gpio_set_direction(CMD_PIN, GPIO_MODE_OUTPUT);
	gpio_set_level(CMD_PIN, 0);


	//Setup led pin
	gpio_reset_pin(LED_PIN);
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
	gpio_set_level(LED_PIN, 0);

	//Set up the reset pin
	
	gpio_reset_pin(RST_PIN);
	gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);
	gpio_set_level(RST_PIN, 1);

	//Pulse the reset line
	vTaskDelay(50);
	gpio_set_level(RST_PIN, 0);
	vTaskDelay(10);
	gpio_set_level(RST_PIN, 1);
	vTaskDelay(50);

	
	spi_write_byte(spi, 0xA8); //Set mux ratio
	spi_write_byte(spi, 0x3F); // ^^^
	spi_write_byte(spi, 0xD3); //Display offset
	spi_write_byte(spi, 0x00); // ^^^
	spi_write_byte(spi, 0x40); //set disp start ln
	spi_write_byte(spi, 0xA1); //set seg remap
	spi_write_byte(spi, 0xC8); //set COM output scan dir
	spi_write_byte(spi, 0xDA); //set COM pins hardware config
	spi_write_byte(spi, 0x12); // ^^ (used to be 0x02, which interlaced the values)
	spi_write_byte(spi, 0x81); //set contrast conrol
	spi_write_byte(spi, 0x7f); // ^^
	spi_write_byte(spi, 0xA4); // (Disable?) Entire display ON
	spi_write_byte(spi, 0xA6); // Set normal display 
	spi_write_byte(spi, 0xD5); //Set osc freq
	spi_write_byte(spi, 0x80); // ^^
	spi_write_byte(spi, 0x8D); //enable charge pump regulator
	spi_write_byte(spi, 0x14); // ^^
	spi_write_byte(spi, 0xAF); // Display ON

	//spi_write_byte(spi, 0xA6);
	vTaskDelay(50);

	spi_write_byte(spi, 0x20); // Set the addressing mode
	spi_write_byte(spi, 0x02); // to page addressing

	//spi_write_byte(spi, 0x40); //set display start line to 0

	vTaskDelay(50);


}




#endif

