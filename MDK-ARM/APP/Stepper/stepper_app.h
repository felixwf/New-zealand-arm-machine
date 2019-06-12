#ifndef __STEPPER_APP_H
#define __STEPPER_APP_H
#include "stepper_drv.h"
#include "modbus_rtu.h"


void StepperInit(void);
void StepperTask(void const * argument);
void msg_init(ArmMachine_TypeDef *ArmMachineMsg);

#endif

