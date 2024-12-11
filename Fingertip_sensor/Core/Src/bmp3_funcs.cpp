#include "bmp3_funcs.h"

// write cs pin on channel 1
void writeLow(uint8_t pin){
    if (pin == 1){
        //cs0.write(0);
        HAL_GPIO_WritePin(SPI1_CS0_GPIO_Port, SPI1_CS0_Pin, GPIO_PIN_RESET);
    }
    else if (pin == 2){
        //cs1.write(0);
        HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin, GPIO_PIN_RESET);
    }
    else if (pin == 3){
        //cs2.write(0);
        HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_RESET);
    }
    else if (pin == 4){
        //cs3.write(0);
        HAL_GPIO_WritePin(SPI1_CS3_GPIO_Port, SPI1_CS3_Pin, GPIO_PIN_RESET);
    }
    else if (pin == 5){
        //cs4.write(0);
        HAL_GPIO_WritePin(SPI1_CS4_GPIO_Port, SPI1_CS4_Pin, GPIO_PIN_RESET);
    }
    else if (pin == 6){
        //cs5.write(0);
        HAL_GPIO_WritePin(SPI1_CS5_GPIO_Port, SPI1_CS5_Pin, GPIO_PIN_RESET);
    }
    else if (pin == 7){
        //cs6.write(0);
        HAL_GPIO_WritePin(SPI1_CS6_GPIO_Port, SPI1_CS6_Pin, GPIO_PIN_RESET);
    }
    else if (pin == 8){
        //cs7.write(0);
        HAL_GPIO_WritePin(SPI1_CS7_GPIO_Port, SPI1_CS7_Pin, GPIO_PIN_RESET);
    }
}

void writeHigh(uint8_t pin){
    if (pin == 1){
        //cs0.write(1);
        HAL_GPIO_WritePin(SPI1_CS0_GPIO_Port, SPI1_CS0_Pin, GPIO_PIN_SET);
    }
    else if (pin == 2){
        //cs1.write(1);
        HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin, GPIO_PIN_SET);
    }
    else if (pin == 3){
        //cs2.write(1);
        HAL_GPIO_WritePin(SPI1_CS2_GPIO_Port, SPI1_CS2_Pin, GPIO_PIN_SET);
    }
    else if (pin == 4){
        //cs3.write(1);
        HAL_GPIO_WritePin(SPI1_CS3_GPIO_Port, SPI1_CS3_Pin, GPIO_PIN_SET);
    }
    else if (pin == 5){
        //cs4.write(1);
        HAL_GPIO_WritePin(SPI1_CS4_GPIO_Port, SPI1_CS4_Pin, GPIO_PIN_SET);
    }
    else if (pin == 6){
        //cs5.write(1);
        HAL_GPIO_WritePin(SPI1_CS5_GPIO_Port, SPI1_CS5_Pin, GPIO_PIN_SET);
    }
    else if (pin == 7){
        //cs6.write(1);
        HAL_GPIO_WritePin(SPI1_CS6_GPIO_Port, SPI1_CS6_Pin, GPIO_PIN_SET);
    }
    else if (pin == 8){
        //cs7.write(1);
        HAL_GPIO_WritePin(SPI1_CS7_GPIO_Port, SPI1_CS7_Pin, GPIO_PIN_SET);
    }
}



// General Read and Write functions for channel 1
// read function: |0x80 done in library, dummy byte taken care of in library
int8_t bmp_spi1_read(uint8_t cspin, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
//    writeLow(cspin);
//    spi1.write(reg_addr); // send read command to chip_id register (reg 0x00)
//    for(int i = 0; i < len; i++){
//        *(reg_data+i) = spi1.write(0x00); // read in 2nd byte = chip_id
//    }
//    writeHigh(cspin);

    writeLow(cspin);
    HAL_SPI_Transmit(&hspi1, &reg_addr, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, reg_data, len, HAL_MAX_DELAY);
    writeHigh(cspin);
    return 0;
}

int8_t bmp_spi1_write(uint8_t cspin, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
//    writeLow(cspin);
//    spi1.write(reg_addr);
//    if (len>1) {
//        for(int i = 0; i < len-1; i++){
//            spi1.write(*(reg_data+i)); // send alternating register address and register bytes in multi write
//        }
//    }
//    else{
//        spi1.write(reg_data[0]);
//    }
//	  writeHigh(cspin);

	writeLow(cspin);
    // HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout)
	// HAL_SPI_Transmit(&hspi1, (uint8_t *)&addr, 1, 100);
    HAL_SPI_Transmit(&hspi1, &reg_addr, 1, HAL_MAX_DELAY);
    if (len>1) {
        for(int i = 0; i < len-1; i++){
			HAL_SPI_Transmit(&hspi1, (reg_data+i), 1, HAL_MAX_DELAY);
		}
	}
	else{
		HAL_SPI_Transmit(&hspi1, reg_data, 1, HAL_MAX_DELAY);
	}
    writeHigh(cspin);
    return 0;
}

// Delay function
void bmp_delay_ms(uint32_t msec){ //delay in milliseconds
    HAL_Delay(msec);
}
