#ifndef __SERIAL_QUEUE_APP_H
#define __SERIAL_QUEUE_APP_H
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "stepper_msg.h"
#include <string.h>
#include "usart.h"

void data_queue_task_init(void);
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
//void RTUSend_Data(ArmMachine_TypeDef ArmMachineMsg);
void ArmMachineSend_Data(ArmMachine_TypeDef ArmMachineMsg);



#endif



