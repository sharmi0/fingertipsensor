#include "ForceSensor.h"
#include "math_ops.h"

ForceSensor::ForceSensor(){

    // initialize other values
    sensor_comp = 1; //| uint8_t(1<<1); // sensor_comp = BMP3_PRESS | BMP3_TEMP;
    
    }
    
void ForceSensor::Initialize(){     
    
	// configure sensor devices
	// start with writing all CS pins high
	writeHigh(1);
	writeHigh(2);
	writeHigh(3);
	writeHigh(4);
	writeHigh(5);
	writeHigh(6);
	writeHigh(7);
	writeHigh(8);

	//printf("Initializing force sensor.\n\r");

	s1.dev_id = 1;  // tells which cs pin associated with device
	config_dev(&s1);
	s2.dev_id = 2;  // tells which cs pin associated with device
	config_dev(&s2);
	s3.dev_id = 3;  // tells which cs pin associated with device
	config_dev(&s3);
	s4.dev_id = 4;  // tells which cs pin associated with device
	config_dev(&s4);
	s5.dev_id = 5;  // tells which cs pin associated with device
	config_dev(&s5);
	s6.dev_id = 6;  // tells which cs pin associated with device
	config_dev(&s6);
	s7.dev_id = 7;  // tells which cs pin associated with device
	config_dev(&s7);
	s8.dev_id = 8;  // tells which cs pin associated with device
	config_dev(&s8);

}
    
    
void ForceSensor::config_dev(struct bmp3_dev *dev){
    
	int8_t rslt=0;//BMP3_OK; // get error with rslt = BMP3_OK;

    dev -> intf = BMP3_SPI_INTF;
    dev -> read = &bmp_spi1_read;
    dev -> write = &bmp_spi1_write;

    dev -> delay_ms = &bmp_delay_ms;
    rslt = bmp3_init(dev);
    //printf("* initialize sensor result = 0x%x *\r\n", rslt);
    HAL_Delay(250);

    // ***** Configuring settings of sensor
    // Normal Mode - bmp3_set_op_mode
    // Temp En, Press En
    // OSR = no oversampling temp, press
    // ODR = 200Hz temp, press
    // IRR = no IRR filter
    // ^^^all 4 above =  bmp3_set_sensor_settings

    // Set sensor settings (press en, temp en, OSR, ODR, IRR)
    dev -> settings.press_en = 0x01; // BMP3_ENABLE
    dev -> settings.temp_en = 0x01; //BMP3_ENABLE
    dev -> settings.odr_filter.press_os = 0x00; //BMP3_NO_OVERSAMPLING
    dev -> settings.odr_filter.temp_os = 0x00; //BMP3_NO_OVERSAMPLING
    dev -> settings.odr_filter.odr = 0x00; //BMP3_ODR_200_HZ
    dev -> settings.odr_filter.iir_filter = 0x00; //BMP3_IIR_Filter_disable

    uint16_t settings_sel;
    //settings_sel = BMP3_PRESS_EN_SEL | BMP3_TEMP_EN_SEL | BMP3_PRESS_OS_SEL | BMP3_TEMP_OS_SEL | BMP3_IIR_FILTER_SEL | BMP3_ODR_SEL;
    settings_sel = uint16_t(1 << 1) | uint16_t(1 << 2) | uint16_t(1 << 4) | uint16_t(1 << 5) | uint16_t(1 << 6) | uint16_t(1 << 7);
    //settings_sel = uint16_t(1 << 1) | uint16_t(1 << 2);
    rslt = bmp3_set_sensor_settings(settings_sel, dev);

    // Set operating (power) mode
    dev -> settings.op_mode = 0x03; /// normal mode = 0x03
    rslt = bmp3_set_op_mode(dev);

    // Check settings
    rslt = bmp3_get_sensor_settings(dev);

}
    
void ForceSensor::Sample(){
    
//    // get data from every sensor
//	for(int i = 0;i<8;i++){
//		raw_data[i] = pressure_raw[i];
//	}
    
	// get data from every sensor
	bmp3_get_sensor_data(sensor_comp, &data1, &s1);
	bmp3_get_sensor_data(sensor_comp, &data2, &s2);
	bmp3_get_sensor_data(sensor_comp, &data3, &s3);
	bmp3_get_sensor_data(sensor_comp, &data4, &s4);
	bmp3_get_sensor_data(sensor_comp, &data5, &s5);
	bmp3_get_sensor_data(sensor_comp, &data6, &s6);
	bmp3_get_sensor_data(sensor_comp, &data7, &s7);
	bmp3_get_sensor_data(sensor_comp, &data8, &s8);

	// store data
	raw_data[0] = int(data1.pressure)-100000; // pressure is returned in Pa, could subtract actual sea level pressure here
	raw_data[1] = int(data2.pressure)-100000;
	raw_data[2] = int(data3.pressure)-100000;
	raw_data[3] = int(data4.pressure)-100000;
	raw_data[4] = int(data5.pressure)-100000;
	raw_data[5] = int(data6.pressure)-100000;
	raw_data[6] = int(data7.pressure)-100000;
	raw_data[7] = int(data8.pressure)-100000;

    // could combine this with previous step
    offset_data[0] = raw_data[0]-offsets[0];
    offset_data[1] = raw_data[1]-offsets[1];
    offset_data[2] = raw_data[2]-offsets[2];
    offset_data[3] = raw_data[3]-offsets[3];
    offset_data[4] = raw_data[4]-offsets[4];
    offset_data[5] = raw_data[5]-offsets[5];
    offset_data[6] = raw_data[6]-offsets[6];
    offset_data[7] = raw_data[7]-offsets[7];
    
    
}
    
void ForceSensor::Evaluate(){
    // scales raw input data, evaluates neural network, scales and stores output data
        
    // scale sensor data
    for (int i=0; i<8; i++){
        input_data[i] = 0.0f;
        input_data[i] = (((float)offset_data[i]) - (fingertip_net.minims[i+5]))/(fingertip_net.maxims[i+5]-fingertip_net.minims[i+5]); // / fingertip_net.max_pressure;
        // check that inputs are between 0 and 1?
    }

    // decode sensor data here....521*4 operations (multiply,add,activation,add)
    // reset values
    for (int i = 0; i<12; i++){
        l1[i] = 0.0f;
    }
    for (int i = 0; i<64; i++){ //i<25
        l2[i] = 0.0f;
        l3[i] = 0.0f;
    }
    for (int i = 0; i<5; i++){
        l4[i] = 0.0f;
    }
        
    // layer 1
    for(int i = 0; i<12; i++){ // for each node in the next layer
        for(int j = 0; j<8; j++){ // add contribution of node in prev. layer
            l1[i] +=  (fingertip_net.w1[j][i]*input_data[j]);
        }
        l1[i] += fingertip_net.b1[i]; // add bias
        l1[i] = fmaxf(0.0f, l1[i]); // relu activation
    }
        
    // layer 2
    for(int i = 0; i<64; i++){ // for each node in the next layer
        for(int j = 0; j<12; j++){ // add contribution of node in prev. layer
            l2[i] += (fingertip_net.w2[j][i]*l1[j]);
        }
        l2[i] += fingertip_net.b2[i]; // add bias
        l2[i] = fmaxf(0.0f, l2[i]); // relu activation
    }   
    
    // layer 3 // added for larger network architecture
    for(int i = 0; i<64; i++){ // for each node in the next layer
        for(int j = 0; j<64; j++){ // add contribution of node in prev. layer
            l3[i] += (fingertip_net.w3[j][i]*l2[j]);
        }
        l3[i] += fingertip_net.b3[i]; // add bias
        l3[i] = fmaxf(0.0f, l3[i]); // relu activation
    }

    // layer 4
    for(int i = 0; i<5; i++){ // for each node in the next layer
        for(int j = 0; j<64; j++){ // add contribution of node in prev. layer
            l4[i] += fingertip_net.w4[j][i]*l3[j];
        }
        l4[i] += fingertip_net.b4[i];// add bias
        l4[i] = fmaxf(0.0f, l4[i]); // relu activation
    }

    // post-process, re-scale decoded data
    for (int i=0; i<5; i++) {
        output_data[i] = 0.0f;
        output_data[i] = (l4[i]*(fingertip_net.maxims[i]-fingertip_net.minims[i])) + fingertip_net.minims[i]; // - abs(fingertip_net.minims[i]);
    
    }      
    
}
    











//void ForceSensor::Calibrate(){
//
//    printf("Calculating sensor offsets.\n\r");
//    float temp_offsets[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
//    int num_samples = 10;
//    for (int i=0; i<num_samples; i++){
//        Sample();
//        for (int j=0; j<8; j++){
//            temp_offsets[j] += ((float)raw_data[j])/((float)num_samples);
//        }
//        wait_us(10000); // wait for 1/200 s for next sample
//    }
//    printf("Saved offsets: ");
//    for (int i=0; i<8; i++){
//        offsets[i] = (int)temp_offsets[i];
//        if (i<7) {
//            printf("%d, ", offsets[i]);
//        } else {
//            printf("%d\r\n", offsets[i]);
//        }
//    }
//    wait_us(100000); // wait for 1/200 s for next sample
//
//}
    
    




    





//void calibrateSensor(uint16_t* offsets){
//    
//    float temp_offsets[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
//    int samps = 10;
//    
//    for (int i=0; i<samps; i++){
//        for (int j=0; j<8; j++){
//            temp_offsets[j] += (float)spi3.binary(j);
//        }
//        wait_ms(1);
//    }
//    
//    for (int i=0; i<8; i++){
//        temp_offsets[i] = temp_offsets[i]/((float)samps); // get overall offset
//        offsets[i] = (uint16_t)temp_offsets[i]; // convert to int
//    }
//
//    }
