#include <fingertip.h>
#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "math.h"

#include <ForceSensor.h>
#include "bmp3.h"
#include "bmp3_funcs.h"

#include "math_ops.h"

volatile bool INTERRUPT_FLAG = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM2) {
    INTERRUPT_FLAG = 1;
    //
  }
}


// CAN IDs
#define PR_1 			0
#define PR_2 			1
#define PR_3 			2
#define PR_4 			3
#define TME				4

// Initialize CAN
CAN_RxHeaderTypeDef rxMsg;
CAN_TxHeaderTypeDef txMsg_p1, txMsg_p2, txMsg_p3, txMsg_p4, txMsg_t ; // pressure sensor and time
CAN_FilterTypeDef can_filt;
uint32_t cmb_p1, cmb_p2, cmb_p3, cmb_p4, cmb_t; // mailbox for each message?
uint8_t can_rx_buf[100];

// store raw sensor data to send
uint8_t txMsg_p1_data[8];
uint8_t txMsg_p2_data[8];
uint8_t txMsg_p3_data[8];
uint8_t txMsg_p4_data[8];



// Initialize fingertip
ForceSensor fingertip;

// store raw sensor data
int32_t pressure_raw[8];

// Store time values
uint8_t txMsg_t_data[4];
uint32_t loop_time = 0;

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


int fingertip_main(void){
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

	txMsg_t.DLC = 8;
	txMsg_t.IDE = CAN_ID_STD;
	txMsg_t.RTR = CAN_RTR_DATA;
	txMsg_t.StdId = TME;
	txMsg_t.TransmitGlobalTime = DISABLE;

	if ((HAL_CAN_Start(&hcan)) != HAL_OK ){
		while(1);
	}
	// initialize sensors
	fingertip.Initialize();
	HAL_Delay(100);


	while (1){
		if (INTERRUPT_FLAG){
			INTERRUPT_FLAG = 0;



			// Sample sensors
			fingertip.Sample();

			pack_pressure_reply(txMsg_p1_data, txMsg_p2_data, txMsg_p3_data, txMsg_p4_data, &fingertip);

			// measure loop time
			loop_time = __HAL_TIM_GET_COUNTER(&htim2);
			__HAL_TIM_SET_COUNTER(&htim2,0);


			// Pack time buffer
			txMsg_t_data[0] = (loop_time >> 8 * 3) & 0xFF;
			txMsg_t_data[1] = (loop_time >> 8 * 2) & 0xFF;
			txMsg_t_data[2] = (loop_time >> 8 * 1) & 0xFF;
			txMsg_t_data[3] = (loop_time) & 0xFF;


			// Send pressure data
			HAL_CAN_AddTxMessage(&hcan, &txMsg_p1, txMsg_p1_data, &cmb_p1);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_p1));
			HAL_CAN_AddTxMessage(&hcan, &txMsg_p2, txMsg_p2_data, &cmb_p2);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_p2));
			HAL_CAN_AddTxMessage(&hcan, &txMsg_p3, txMsg_p3_data, &cmb_p3);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_p3));
			HAL_CAN_AddTxMessage(&hcan, &txMsg_p4, txMsg_p4_data, &cmb_p4);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_p4));

			// Send loop time data
			HAL_CAN_AddTxMessage(&hcan, &txMsg_t, txMsg_t_data, &cmb_t);
			while(HAL_CAN_IsTxMessagePending(&hcan, cmb_t));

		}
	}
}
