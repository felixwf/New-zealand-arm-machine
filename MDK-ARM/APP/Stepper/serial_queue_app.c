#ifndef __SERIAL_QUQUE_APP_H
#define __SERIAL_QUQUE_APP_H
/* Includes ------------------------------------------------------------------*/
#include "serial_queue_app.h"

osMessageQId dataRecQueueHandle;
osMessageQId dataSendQueueHandle;
osThreadId dataRecTaskHandle;
osThreadId dataSendTaskHandle;


void StartDataRecQueueTask(void const * argument);
void StartDataSendQueueTask(void const * argument);
void ModBusCRC16(uint8_t* cmd, int len);
void convertMsgIntoRTU(ArmMachine_TypeDef ArmMachineMsg, uint8_t *data1, uint8_t *data2, uint8_t *data3);



void data_queue_init()
{
  /* definition and creation of dataRecQueue */
	/* 用于存储来自步进电机的消息 */
  osMessageQDef(dataRecQueue, 16, ArmMachine_TypeDef);
  dataRecQueueHandle = osMessageCreate(osMessageQ(dataRecQueue), NULL);

  /* definition and creation of dataSendQueue */
	/* 用于需要发送给步进电机的消息 */
  osMessageQDef(dataSendQueue, 16, ArmMachine_TypeDef);
  dataSendQueueHandle = osMessageCreate(osMessageQ(dataSendQueue), NULL);

	osThreadDef(dataRecTask, StartDataRecQueueTask, osPriorityNormal, 0, 128);
	dataRecTaskHandle = osThreadCreate(osThread(dataRecTask), NULL);

	osThreadDef(dataSendTask, StartDataSendQueueTask, osPriorityNormal, 0, 128);
	dataSendTaskHandle = osThreadCreate(osThread(dataSendTask), NULL);
}


/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDataRecQueueTask(void const * argument)
{

  for(;;)
  {
		osDelay(10);
  }
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDataSendQueueTask(void const * argument)
{

  for(;;)
  {
		osEvent flag = osMessageGet(dataSendQueueHandle, 0xffff) ;
		if(flag.status == osOK)
		{
			uint8_t data1[41] = {0};
			uint8_t data2[41] = {0};
			uint8_t data3[41] = {0};
			convertMsgIntoRTU(ArmMachineMsg, data1, data2, data3);

			HAL_UART_Transmit(&huart2, data1, 41, 0xFFFF);
			HAL_UART_Transmit(&huart2, data2, 41, 0xFFFF);
			HAL_UART_Transmit(&huart2, data3, 41, 0xFFFF);
		}
		osDelay(1);
  }
}

/* 用于将指令添加到队列中，等待发送 */

void RTUSend_Data(ArmMachine_TypeDef ArmMachineMsg)
{
	osMessagePut (dataSendQueueHandle, ArmMachineMsg, 0xffff); 
}




	/* 使用“直接数据运行”
		号机号码： 01h ==> 1
		功能代码： 10h
		写入寄存器起始地址： 00 58h
		写入寄存器数： 00 10h ==>16个
		写入byte数： 20h ==> 32个

		1. 运行数据No.： 00 00 00 00h ==> 0
		2. 方式： 相对定位（00 00 00 02）
		3. 位置： 00 00 21 34h ==> 8500 step
		4. 速度： 00 00 07 D0h ==> 2000Hz
		5. 起动/变速斜率： 00 00 05 DCh ==> 1.5kHz
		6. 停止斜率： 00 00 05 DCh ==> 1.5kHz
		7. 运行电流: 00 00 03 E8h ==> 100.0%

		反映触发： 全部数据反映（00 00 00 01）
		错误检查： 1C 08h

	*/

void convertMsgIntoRTU(ArmMachine_TypeDef ArmMachineMsg, uint8_t *data1, uint8_t *data2, uint8_t *data3)
{
	uint8_t RTU_armMotor[41] = {
		ArmMachineMsg.armMotor.address,
		0x10,
		0x00, 0x58,
		0x00, 0x10,
		0x20,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, ArmMachineMsg.armMotor.mode == 'a' ? 0x01 : 0x02, /* 绝对定位0x01 相对定位0x02*/
		(uint8_t)(ArmMachineMsg.armMotor.targetPosition >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.targetPosition >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.targetPosition >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.targetPosition & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.topSpeed >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.topSpeed >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.topSpeed >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.topSpeed & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.startSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.startSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.startSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.startSlope & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.stopSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.stopSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.stopSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.stopSlope & 0xff),
		0x00, 0x00, 0x03, 0xe8,	// 100.0% full power
		0x00, 0x00, 0x00, 0x01,
		0x00, 0x00
	};
	ModBusCRC16(RTU_armMotor, 39);

	uint8_t RTU_updownMotor[41] = {
		ArmMachineMsg.updownMotor.address,
		0x10,
		0x00, 0x58,
		0x00, 0x10,
		0x20,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, ArmMachineMsg.updownMotor.mode == 'a' ? 0x01 : 0x02, /* 绝对定位0x01 相对定位0x02*/
		(uint8_t)(ArmMachineMsg.updownMotor.targetPosition >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.targetPosition >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.targetPosition >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.targetPosition & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.topSpeed >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.topSpeed >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.topSpeed >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.topSpeed & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.startSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.startSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.startSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.startSlope & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.stopSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.stopSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.stopSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.stopSlope & 0xff),
		0x00, 0x00, 0x03, 0xe8,	// 100.0% full power
		0x00, 0x00, 0x00, 0x01,
		0x00, 0x00
	};
	ModBusCRC16(RTU_updownMotor, 39);

	uint8_t RTU_baseMotor[41] = {
		ArmMachineMsg.baseMotor.address,
		0x10,
		0x00, 0x58,
		0x00, 0x10,
		0x20,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, ArmMachineMsg.baseMotor.mode == 'a' ? 0x01 : 0x02, /* 绝对定位0x01 相对定位0x02*/
		(uint8_t)(ArmMachineMsg.baseMotor.targetPosition >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.targetPosition >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.targetPosition >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.targetPosition & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.topSpeed >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.topSpeed >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.topSpeed >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.topSpeed & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.startSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.startSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.startSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.startSlope & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.stopSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.stopSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.stopSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.stopSlope & 0xff),
		0x00, 0x00, 0x03, 0xe8,	// 100.0% full power
		0x00, 0x00, 0x00, 0x01,
		0x00, 0x00
	};
	ModBusCRC16(RTU_baseMotor, 39);
	
	memcpy(&data1,RTU_baseMotor,sizeof(uint8_t)*41);
	memcpy(&data2,RTU_updownMotor,sizeof(uint8_t)*41);
	memcpy(&data3,RTU_armMotor,sizeof(uint8_t)*41);
//	HAL_UART_Transmit(&huart3, RTU_armMotor, 41, 0xFFFF);		osDelay(100);

//	HAL_UART_Transmit(&huart3, RTU_updownMotor, 41, 0xFFFF);		osDelay(100);

//	HAL_UART_Transmit(&huart3, RTU_baseMotor, 41, 0xFFFF);		osDelay(100);


}




/************************************************
函数名称 ： ModBusCRC16
功    能 ： 进行CRC-ModBus校验，(并将校验结果填入数组末尾)
参    数 ： pvParameters --- 可选参数
返 回 值 ： 无
作    者 ： FelixWu
*************************************************/
void ModBusCRC16(uint8_t* cmd, int len)
{
	uint16_t i, j, tmp, CRC16;

	CRC16 = 0xFFFF;             //CRC寄存器初始值
	for (i = 0; i < len; i++)
	{
		CRC16 ^= cmd[i];
		for (j = 0; j < 8; j++)
		{
			tmp = (uint16_t)(CRC16 & 0x0001);
			CRC16 >>= 1;
			if (tmp == 1)
			{
				CRC16 ^= 0xA001;    //异或多项式
			}
		}
	}
	cmd[i++] = (uint8_t) (CRC16 & 0x00FF);
	cmd[i++] = (uint8_t) ((CRC16 & 0xFF00)>>8);
//	return CRC16;
}





#endif

/*
	uint8_t RTU_armMotor[41] = {
		ArmMachineMsg.armMotor.address,
		0x10,
		0x00, 0x58,
		0x00, 0x10,
		0x20,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, ArmMachineMsg.armMotor.mode == 'a' ? 0x01 : 0x02, // 绝对定位0x01 相对定位0x02
		(uint8_t)(ArmMachineMsg.armMotor.targetPosition >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.targetPosition >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.targetPosition >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.targetPosition & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.topSpeed >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.topSpeed >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.topSpeed >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.topSpeed & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.startSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.startSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.startSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.startSlope & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.stopSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.stopSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.stopSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.armMotor.stopSlope & 0xff),
		0x00, 0x00, 0x03, 0xe8,	// 100.0% full power
		0x00, 0x00, 0x00, 0x01,
		0x00, 0x00
	};
	ModBusCRC16(RTU_armMotor, 39);

	uint8_t RTU_updownMotor[41] = {
		ArmMachineMsg.updownMotor.address,
		0x10,
		0x00, 0x58,
		0x00, 0x10,
		0x20,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, ArmMachineMsg.updownMotor.mode == 'a' ? 0x01 : 0x02, // 绝对定位0x01 相对定位0x02
		(uint8_t)(ArmMachineMsg.updownMotor.targetPosition >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.targetPosition >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.targetPosition >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.targetPosition & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.topSpeed >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.topSpeed >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.topSpeed >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.topSpeed & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.startSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.startSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.startSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.startSlope & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.stopSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.stopSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.stopSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.updownMotor.stopSlope & 0xff),
		0x00, 0x00, 0x03, 0xe8,	// 100.0% full power
		0x00, 0x00, 0x00, 0x01,
		0x00, 0x00
	};
	ModBusCRC16(RTU_armMotor, 39);

	uint8_t RTU_baseMotor[41] = {
		ArmMachineMsg.baseMotor.address,
		0x10,
		0x00, 0x58,
		0x00, 0x10,
		0x20,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, ArmMachineMsg.baseMotor.mode == 'a' ? 0x01 : 0x02, // 绝对定位0x01 相对定位0x02
		(uint8_t)(ArmMachineMsg.baseMotor.targetPosition >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.targetPosition >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.targetPosition >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.targetPosition & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.topSpeed >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.topSpeed >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.topSpeed >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.topSpeed & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.startSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.startSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.startSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.startSlope & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.stopSlope >> 24 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.stopSlope >> 16 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.stopSlope >> 8 & 0xff),
		(uint8_t)(ArmMachineMsg.baseMotor.stopSlope & 0xff),
		0x00, 0x00, 0x03, 0xe8,	// 100.0% full power
		0x00, 0x00, 0x00, 0x01,
		0x00, 0x00
	};
	ModBusCRC16(RTU_armMotor, 39);
	ModBusCRC16(RTU_updownMotor, 39);
	ModBusCRC16(RTU_baseMotor, 39);

*/
