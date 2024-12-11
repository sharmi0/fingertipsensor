#include <fingertip.h>
#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "math.h"
#include "ForceSensor.h"
#include "neural_nets.h"
#include "bmp3.h"
#include "bmp3_funcs.h"
#include "neural_nets.h"
#include "VL6180X.h"
#include "math_ops.h"

// new limits for force sensors, need to test these!
#define FT_MIN -20.0f
#define FT_MAX 20.0f
#define FN_MIN -30.0f
#define FN_MAX 30.0f
#define ANG_MIN -45.0f
#define ANG_MAX 45.0f
#define RNG_MAX 255 // this probably won't be necessary

// CAN IDs?
#define CAN_FORCE_1  	5  // left
#define CAN_FORCE_2    	6 // right
#define CAN_TOF_1       7 // left
#define CAN_TOF_2       8 // right

#define PR_1 			0
#define PR_2 			1
#define PR_3 			2
#define PR_4 			3
#define PR_TOF			4

// Variables for force sensor data
int32_t pressure_raw[8];


ForceSensor fingertip;

//Filtering
int range[9];
float range_m[9]; // range in m
// TODO: remove some of this, do filtering upstream?
float range_m_raw[9]; // range in m, non-filtered
int range_status[9];
int range_mode[9];
int range_offsets[] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; //{4, 8, 9, 10, 5, 0, 0, 10, 3}; // offsets in mm to be added to range measurements
uint16_t range_period = 100; //30;
float filt_coef = 0.70f;


// Initialize CAN
CAN_RxHeaderTypeDef rxMsg;
CAN_TxHeaderTypeDef txMsg_t1, txMsg_f1, txMsg_p1, txMsg_p2, txMsg_p3, txMsg_p4 ; // ToF and force for each finger
CAN_FilterTypeDef can_filt;
uint32_t cmb_t1, cmb_f1, cmb_p1, cmb_p2, cmb_p3, cmb_p4; // mailbox for each message?
uint8_t can_rx_buf[100];

uint8_t txMsg_t1_data[8];
uint8_t txMsg_f1_data[8];
uint8_t txMsg_p1_data[8];
uint8_t txMsg_p2_data[8];
uint8_t txMsg_p3_data[8];
uint8_t txMsg_p4_data[8];

void pack_pressure_reply(uint8_t *msg1, uint8_t *msg2, uint8_t *msg3, uint8_t *msg4, ForceSensor * fs){
     msg1[3] = fs->raw_data[0]&0xFF;
     msg1[2] = (fs->raw_data[0]>>8)&0xFF;
     msg1[1] = (fs->raw_data[0]>>16)&0xFF;
     msg1[0] = (fs->raw_data[0]>>24)&0xFF;
     msg1[7] = fs->raw_data[1]&0xFF;
     msg1[6] = (fs->raw_data[1]>>8)&0xFF;
     msg1[5] = (fs->raw_data[1]>>16)&0xFF;
     msg1[4] = (fs->raw_data[1]>>24)&0xFF;

     msg2[3] = fs->raw_data[2]&0xFF;
     msg2[2] = (fs->raw_data[2]>>8)&0xFF;
     msg2[1] = (fs->raw_data[2]>>16)&0xFF;
     msg2[0] = (fs->raw_data[2]>>24)&0xFF;
     msg2[7] = fs->raw_data[3]&0xFF;
     msg2[6] = (fs->raw_data[3]>>8)&0xFF;
     msg2[5] = (fs->raw_data[3]>>16)&0xFF;
     msg2[4] = (fs->raw_data[3]>>24)&0xFF;

     msg3[3] = fs->raw_data[4]&0xFF;
     msg3[2] = (fs->raw_data[4]>>8)&0xFF;
     msg3[1] = (fs->raw_data[4]>>16)&0xFF;
     msg3[0] = (fs->raw_data[4]>>24)&0xFF;
     msg3[7] = fs->raw_data[5]&0xFF;
     msg3[6] = (fs->raw_data[5]>>8)&0xFF;
     msg3[5] = (fs->raw_data[5]>>16)&0xFF;
     msg3[4] = (fs->raw_data[5]>>24)&0xFF;

     msg4[3] = fs->raw_data[6]&0xFF;
     msg4[2] = (fs->raw_data[6]>>8)&0xFF;
     msg4[1] = (fs->raw_data[6]>>16)&0xFF;
     msg4[0] = (fs->raw_data[6]>>24)&0xFF;
     msg4[7] = fs->raw_data[7]&0xFF;
     msg4[6] = (fs->raw_data[7]>>8)&0xFF;
     msg4[5] = (fs->raw_data[7]>>16)&0xFF;
     msg4[4] = (fs->raw_data[7]>>24)&0xFF;
}

void pack_force_reply(uint8_t * msg, ForceSensor * fs){

     /// limit data to be within bounds ///
     float fx_temp = fminf(fmaxf(FT_MIN, fs->output_data[0]), FT_MAX);
     float fy_temp = fminf(fmaxf(FT_MIN, fs->output_data[1]), FT_MAX);
     float fz_temp = fminf(fmaxf(FN_MIN, fs->output_data[2]), FN_MAX);
     float theta_temp = fminf(fmaxf(ANG_MIN, fs->output_data[3]), ANG_MAX);
     float phi_temp = fminf(fmaxf(ANG_MIN, fs->output_data[4]), ANG_MAX);
     /// convert floats to unsigned ints ///
     uint16_t fx_int = float_to_uint(fx_temp, FT_MIN, FT_MAX, 12);
     uint16_t fy_int = float_to_uint(fy_temp, FT_MIN, FT_MAX, 12);
     uint16_t fz_int = float_to_uint(fz_temp, FN_MIN, FN_MAX, 12);
     uint16_t theta_int = float_to_uint(theta_temp, ANG_MIN, ANG_MAX, 12);
     uint16_t phi_int = float_to_uint(phi_temp, ANG_MIN, ANG_MAX, 12);
     /// pack ints into the can buffer ///
     msg[0] = (fs->_channel<<4)|(fx_int>>8);
     msg[1] = fx_int&0xFF;
     msg[2] = fy_int>>4;
     msg[3] = ((fy_int&0x0F)<<4)|(fz_int>>8);
     msg[4] = fz_int&0xFF;
     msg[5] = theta_int>>4;
     msg[6] = ((theta_int&0x0F)<<4)|(phi_int>>8);
     msg[7] = phi_int&0xFF;
     }

void pack_tof_reply(uint8_t * msg){
    /// pack ints into the can buffer ///
    msg[0] = range[0]; // top left
    msg[1] = range[1]; // front left
    msg[2] = range[2]; // top right
    msg[3] = range[3]; // front right
    msg[4] = range[4]; // back
}


// main CPP loop
int fingertip_main(void){


	//printf("Hello from fingertip CPP main.\n\r");

	HAL_Delay(1000);

	// initialize CAN messages
	can_filt.FilterBank = 0;
	can_filt.FilterMode = CAN_FILTERMODE_IDMASK;
	can_filt.FilterFIFOAssignment = CAN_RX_FIFO0;
	can_filt.FilterIdHigh = 0;
	can_filt.FilterIdLow = 0;
	can_filt.FilterMaskIdHigh = 0;
	can_filt.FilterMaskIdLow = 0;
	can_filt.FilterScale = CAN_FILTERSCALE_32BIT;
	can_filt.FilterActivation = ENABLE;
	can_filt.SlaveStartFilterBank = 14;

	txMsg_f1.DLC = 8;
	txMsg_f1.IDE = CAN_ID_STD;
	txMsg_f1.RTR = CAN_RTR_DATA;
	txMsg_f1.StdId = CAN_FORCE_1; // TODO: chamge this?
	txMsg_f1.TransmitGlobalTime = DISABLE;

	txMsg_t1.DLC = 8;
	txMsg_t1.IDE = CAN_ID_STD;
	txMsg_t1.RTR = CAN_RTR_DATA;
	txMsg_t1.StdId = PR_TOF; //CAN_TOF_1;
	txMsg_t1.TransmitGlobalTime = DISABLE;

	txMsg_p1.DLC = 8;
	txMsg_p1.IDE = CAN_ID_STD;
	txMsg_p1.RTR = CAN_RTR_DATA;
	txMsg_p1.StdId = PR_1;
	txMsg_p1.TransmitGlobalTime = DISABLE;

	txMsg_p2.DLC = 8;
	txMsg_p2.IDE = CAN_ID_STD;
	txMsg_p2.RTR = CAN_RTR_DATA;
	txMsg_p2.StdId = PR_2;
	txMsg_p2.TransmitGlobalTime = DISABLE;

	txMsg_p3.DLC = 8;
	txMsg_p3.IDE = CAN_ID_STD;
	txMsg_p3.RTR = CAN_RTR_DATA;
	txMsg_p3.StdId = PR_3;
	txMsg_p3.TransmitGlobalTime = DISABLE;

	txMsg_p4.DLC = 8;
	txMsg_p4.IDE = CAN_ID_STD;
	txMsg_p4.RTR = CAN_RTR_DATA;
	txMsg_p4.StdId = PR_4;
	txMsg_p4.TransmitGlobalTime = DISABLE;



//	HAL_CAN_ConfigFilter(&hcan,&can_filt); //Initialize CAN Filter
//	HAL_CAN_Start(&hcan); //Initialize CAN Bus
	if ((HAL_CAN_Start(&hcan)) != HAL_OK )
	{
		//printf("Failed to start CAN.\n\r");
		while(1);
	}
	//HAL_CAN_ActivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING);// Initialize CAN Bus Rx Interrupt

	HAL_Delay(100);

	// initialize force sensor
	fingertip.Initialize();

	// initialize time of flight sensors
    HAL_GPIO_WritePin(I2C1_TE1_GPIO_Port, I2C1_TE1_Pin, GPIO_PIN_RESET); // disable all sensors
    HAL_GPIO_WritePin(I2C1_TE2_GPIO_Port, I2C1_TE2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(I2C1_TE3_GPIO_Port, I2C1_TE3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(I2C1_TE4_GPIO_Port, I2C1_TE4_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(I2C1_TE5_GPIO_Port, I2C1_TE5_Pin, GPIO_PIN_RESET);

	//printf("Sensor 1...\n\r");
	HAL_GPIO_WritePin(I2C1_TE1_GPIO_Port, I2C1_TE1_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	if(!tof1.begin(&hi2c1)){
		//printf("Sensor 1 init failed.\n\r");
	}
	HAL_Delay(1);
	//printf("Changing address 1\n\r");
	tof1.setNewAddress(taddr1);
	HAL_Delay(1);
	//printf("Range mode: %d\n\r",tof1.readRangeMode());
	if(tof1.readRangeMode()==0){
		tof1.startRangeContinuous(range_period);
	}
	HAL_Delay(10);

	//printf("Sensor 2...\n\r");
	HAL_GPIO_WritePin(I2C1_TE2_GPIO_Port, I2C1_TE2_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	if(!tof2.begin(&hi2c1)){
		//printf("Sensor 2 init failed.\n\r");
	}
	HAL_Delay(1);
	//printf("Changing address 2\n\r");
	tof2.setNewAddress(taddr2);
	HAL_Delay(1);
	//printf("Range mode: %d\n\r",tof2.readRangeMode());
	if(tof2.readRangeMode()==0){
		tof2.startRangeContinuous(range_period);
	}
	HAL_Delay(10);

	//printf("Sensor 3...\n\r");
	HAL_GPIO_WritePin(I2C1_TE3_GPIO_Port, I2C1_TE3_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	if(!tof3.begin(&hi2c1)){
		//printf("Sensor 3 init failed.\n\r");
	}
	HAL_Delay(1);
	//printf("Changing address 3\n\r");
	tof3.setNewAddress(taddr3);
	HAL_Delay(1);
	//printf("Range mode: %d\n\r",tof3.readRangeMode());
	if(tof3.readRangeMode()==0){
		tof3.startRangeContinuous(range_period);
	}
	HAL_Delay(10);

	//printf("Sensor 4...\n\r");
	HAL_GPIO_WritePin(I2C1_TE4_GPIO_Port, I2C1_TE4_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	if(!tof4.begin(&hi2c1)){
		//printf("Sensor 4 init failed.\n\r");
	}
	HAL_Delay(1);
	//printf("Changing address 4\n\r");
	tof4.setNewAddress(taddr4);
	HAL_Delay(1);
	//printf("Range mode: %d\n\r",tof4.readRangeMode());
	if(tof4.readRangeMode()==0){
		tof4.startRangeContinuous(range_period);
	}
	HAL_Delay(10);

	//printf("Sensor 5...\n\r");
	HAL_GPIO_WritePin(I2C1_TE5_GPIO_Port, I2C1_TE5_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	if(!tof5.begin(&hi2c1)){
		//printf("Sensor 5 init failed.\n\r");
	}
	HAL_Delay(1);
	//printf("Changing address 5\n\r");
	tof5.setNewAddress(taddr5);
	HAL_Delay(1);
	//printf("Range mode: %d\n\r",tof5.readRangeMode());
	if(tof5.readRangeMode()==0){
		tof5.startRangeContinuous(range_period);
	}
	HAL_Delay(1000);

	uint16_t eval_time = 0;
	uint16_t loop_time = 0;

	while (1) {
		/* Super loop */

		// Notes:
		// - Loop is set to run at 100Hz, or every 10000us
		// - Sampling pressure sensors takes 1160us
		// - Evaluating neural net takes 5990us (!!!) (with this in loop, real loop frequency is close to 100Hz)
		// - Sampling ToF sensors takes from 5*167=835us to 5*465=2325us, depending on how many have new results
		// - Packing and sending CAN messages takes ~610us (~120us each)
		// - At 200Hz loop timing, sampling typically takes ~2500-3100us

		if (sample_flag==1){

//			printf("Still in fingertip CPP main.\n\r");
//			HAL_Delay(1);

			// reset interrupt flag
			sample_flag = 0;

			loop_time = __HAL_TIM_GET_COUNTER(&htim15);
			__HAL_TIM_SET_COUNTER(&htim15,0);  // set the counter value a 0

			// sample and evaluate pressure sensors
			fingertip.Sample(); // TODO: look into what makes this function take so long?
//			fingertip.Evaluate();

			// sample time of flight sensors
			delay_us(10);
			if (tof1.isRangeComplete()){
				// if it is ready, get range status, mode, and result
				delay_us(10);
				range[0] = tof1.readRangeResult();
				delay_us(10);
			}
			delay_us(10);
			if (tof2.isRangeComplete()){
				// if it is ready, get range status, mode, and result
				delay_us(10);
				range[1] = tof2.readRangeResult();
				delay_us(10);
			}
			delay_us(10);
			if (tof3.isRangeComplete()){
				// if it is ready, get range status, mode, and result
				delay_us(10);
				range[2] = tof3.readRangeResult();
				delay_us(10);
			}
			delay_us(10);
			if (tof4.isRangeComplete()){
				// if it is ready, get range status, mode, and result
				delay_us(10);
				range[3] = tof4.readRangeResult();
				delay_us(10);
			}
			delay_us(10);
			if (tof5.isRangeComplete()){
				// if it is ready, get range status, mode, and result
				delay_us(10);
				range[4] = tof5.readRangeResult();
				delay_us(10);
			}

	        // pack and send CAN messages
	        pack_pressure_reply(txMsg_p1_data, txMsg_p2_data, txMsg_p3_data, txMsg_p4_data, &fingertip);
	        pack_force_reply(txMsg_f1_data, &fingertip);
	        pack_tof_reply(txMsg_t1_data);

	    	// sending CAN messages
	        // TODO: select between force and tof or pressure sensor data
//	        HAL_CAN_AddTxMessage(&hcan, &txMsg_f1, txMsg_f1_data, &cmb_f1);
//			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_f1));
			HAL_CAN_AddTxMessage(&hcan, &txMsg_t1, txMsg_t1_data, &cmb_t1);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_t1));

			HAL_CAN_AddTxMessage(&hcan, &txMsg_p1, txMsg_p1_data, &cmb_p1);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_p1));
	        HAL_CAN_AddTxMessage(&hcan, &txMsg_p2, txMsg_p2_data, &cmb_p2);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_p2));
	        HAL_CAN_AddTxMessage(&hcan, &txMsg_p3, txMsg_p3_data, &cmb_p3);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_p3));
	        HAL_CAN_AddTxMessage(&hcan, &txMsg_p4, txMsg_p4_data, &cmb_p4);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_p4));


			// Print data for testing
			eval_time = __HAL_TIM_GET_COUNTER(&htim15);
//			printf("%d, %d\n\r", eval_time, loop_time);
			printf("Pressure: %03d,%03d,%03d,%03d,%03d,%03d,%03d,%03d \n\r", fingertip.raw_data[0],fingertip.raw_data[1],fingertip.raw_data[2],
						fingertip.raw_data[3],fingertip.raw_data[4],fingertip.raw_data[5],fingertip.raw_data[6],fingertip.raw_data[7]);
			printf("TOF: %03d,%03d,%03d,%03d,%03d\n\r", range[0], range[1], range[2], range[3], range[4]);
			printf("\n\r\n\r");
//			printf("Force: %d,%d,%d\n\r", (int)(1000.0f*fingertip.output_data[0]), (int)(1000.0f*fingertip.output_data[1]), (int)(1000.0f*fingertip.output_data[2]));
//			printf("Angle: %d,%d\n\r\n\r", (int)(fingertip.output_data[3]), (int)(fingertip.output_data[4]));

		}

	}

}


