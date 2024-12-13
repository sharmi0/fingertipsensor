#ifndef BMP3_FUNCS_H
#define BMP3_FUNCS_H

#include "main.h"
#include "stm32f3xx_hal.h"
#include "gpio.h"
#include "tim.h"
#include <stdio.h>
#include "printing.h"
#include "spi.h"

// function prototypes
void writeLow(uint8_t pin);
void writeHigh(uint8_t pin);

int8_t bmp_spi1_read(uint8_t cspin, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t bmp_spi1_write(uint8_t cspin, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

void bmp_delay_ms(uint32_t msec);

#endif
