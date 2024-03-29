#ifndef __SERIAL_QUQUE_APP_H
#define __SERIAL_QUQUE_APP_H
/* Includes ------------------------------------------------------------------*/
#include "serial_queue_app.h"

/* 功能介绍：
	 1. 建立了两个队列，分别用于数据的发送和接收
	 2. 建立了两个task，分别用于监控和处理两个队列
	 3. 一个task用于监控队列1，一旦发现队列中有数据，就通过串口发送出去
	 4. 另一个task用于在串口中断中接收到数据后，将其数据内容添加到队列2中
 */



/* 静态变量 ------------------------------------------------------------------*/
TaskHandle_t recTaskHandle;
TaskHandle_t sendTaskHandle;


void recTask(void *pvParameters);
void sendTask(void *pvParameters);
void ModBusCRC16(uint8_t* cmd, int len);
void convertMsgIntoRTU(ArmMachine_TypeDef ArmMachineMsg, uint8_t *data1, uint8_t *data2, uint8_t *data3);


void data_queue_task_init(void)
{
	/* 由于cmsis_os使得FreeRTOS的队列变得不好用，甚至有bug，此处暂且使用FreeRTOS队列功能实现 */
	BaseType_t xReturn;

  /* 创建任务 */
//	xReturn = xTaskCreate((TaskFunction_t)	recTask,		// 任务函数
//												(const char*)			"RecTask",	// 任务名称
//												(uint16_t)				128,											// 任务堆栈大小
//												(void*)						NULL,											// 传递给任务函数的参数
//												(UBaseType_t)			3,												// 任务优先级
//												(TaskHandle_t*)		&recTaskHandle);			// 任务句柄
	xReturn = xTaskCreate(recTask, "RecTask", 256, NULL, 3,	&recTaskHandle);
  if(pdPASS != xReturn)
  {
		printf("recTask create failed\r\n");							
    return;                                      //创建接收任务失败
  }

//	xReturn = xTaskCreate((TaskFunction_t)	sendTask,		// 任务函数
//												(const char*)			"sendTask",	// 任务名称
//												(uint16_t)				128,											// 任务堆栈大小
//												(void*)						NULL,											// 传递给任务函数的参数
//												(UBaseType_t)			3,												// 任务优先级
//												(TaskHandle_t*)		&sendTaskHandle);		// 任务句柄
	xReturn = xTaskCreate(sendTask, "sendTask", 256, NULL, 3,	&sendTaskHandle);
  if(pdPASS != xReturn)
  {
		printf("sendTask create failed: %ld\r\n", xReturn);							
    return;                                      //创建发送任务失败
  }

//	printf("Init --> data_queue_task_init()\r\n");							
}


/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
static void recTask(void *pvParameters)
{

  for(;;)
  {
//		printf("Task --> recTask()\r\n");
		vTaskDelay(1000);
  }
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
static void sendTask(void *pvParameters)
{
  for(;;)
  {
//		printf("Task --> sendTask()\r\n");
		ArmMachineSend_Data(ArmMachineMsg);
		vTaskDelay(1000);
  }
}

/* 用于将指令添加到队列中，等待发送 */
void RTUSend_Data(uint8_t *data1, uint8_t *data2, uint8_t *data3)
{
	// 发送三个步进电机的控制指令
	HAL_UART_Transmit(&huart3, data1, 41, 0xFFFF);
	HAL_UART_Transmit(&huart3, data2, 41, 0xFFFF);
	HAL_UART_Transmit(&huart3, data3, 41, 0xFFFF);
	printf("RTUSend_Data --> success\r\n");
	printf("huart1 status %x\r\n", HAL_UART_GetState(&huart1));
	printf("huart2 status %x\r\n", HAL_UART_GetState(&huart2));
	printf("huart3 status %x\r\n", HAL_UART_GetState(&huart3));
	
}

void ArmMachineSend_Data(ArmMachine_TypeDef ArmMachineMsg)
{
	uint8_t data1[41] = {0};
	uint8_t data2[41] = {0};
	uint8_t data3[41] = {0};

	// 将最新的数据，转化为符合MODBUS-RTU协议以及步进电机对应功能的数组
	convertMsgIntoRTU(ArmMachineMsg, data1, data2, data3);
	RTUSend_Data(data1, data2, data3);
}

// 接收完成回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
	if(huart->Instance == USART3)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
	}
	if(huart->Instance == USART2)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
	}
//	if(huart->Instance == USART1)
//	{
//		printf("UART1 Rx callback\r\n");
//	}
//	if(huart->Instance == USART2)
//	{
//		printf("UART2 Rx callback\r\n");
//	}
//	if(huart->Instance == USART3)
//	{
//		printf("UART3 Rx callback\r\n");
//	}
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
	}
	if(huart->Instance == USART2)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
	}
//	if(huart->Instance == USART1)
//	{
//		printf("UART1 Tx callback\r\n");
//	}
//	if(huart->Instance == USART2)
//	{
//		printf("UART2 Tx callback\r\n");
//	}
//	if(huart->Instance == USART3)
//	{
//		printf("UART3 Tx callback\r\n");
//	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	
	if(huart->Instance == USART1)
	{
		printf("UART1 error callback\r\n");
	}
	if(huart->Instance == USART2)
	{
		printf("UART2 error callback\r\n");
	}
	if(huart->Instance == USART3)
	{
		printf("UART3 error callback\r\n");
	}
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
	
	memcpy(data1,RTU_baseMotor,sizeof(uint8_t)*41);
	memcpy(data2,RTU_updownMotor,sizeof(uint8_t)*41);
	memcpy(data3,RTU_armMotor,sizeof(uint8_t)*41);
//	printf("convertMsgIntoRTU --> success\r\n");
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


