#ifndef FORCESENSOR_H
#define FORCESENSOR_H

#include "stm32f3xx_hal.h"
#include <stdint.h>
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include <stdio.h>
#include "printing.h"

#include "neural_nets.h"
#include "bmp3.h"
#include "bmp3_funcs.h"

extern int32_t pressure_raw[8];
extern const NeuralNet fingertip_net;

class ForceSensor{
public:
    ForceSensor();
    void Sample();
    void Initialize();
//    void Calibrate();
    void Evaluate();
    
    int _channel;
    int raw_data[8];
    int offsets[8];
    int offset_data[8];
    float input_data[8];
    float output_data[5];
    
    
private:
    void config_dev(struct bmp3_dev *dev);
//    const NeuralNet *_net;
    uint8_t sensor_comp;
    
    float l1[12]; // to be evaluated on-line
    float l2[64]; //[25];
    float l3[64]; //[5];
    float l4[5];

    // TODO: convert this into a struct array?
    struct bmp3_dev s1; // sets up dev as a 'bmp3_dev structure' w/ associated variables
    struct bmp3_dev s2;
    struct bmp3_dev s3;
    struct bmp3_dev s4;
    struct bmp3_dev s5;
    struct bmp3_dev s6;
    struct bmp3_dev s7;
    struct bmp3_dev s8;
    // TODO: convert this into a struct array?
    struct bmp3_data data1; // structs to store sensor data
    struct bmp3_data data2;
    struct bmp3_data data3;
    struct bmp3_data data4;
    struct bmp3_data data5;
    struct bmp3_data data6;
    struct bmp3_data data7;
    struct bmp3_data data8;


};

#endif
