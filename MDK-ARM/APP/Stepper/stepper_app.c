#include "stepper_app.h"

osThreadId stepperTaskHandle;
void StepperTask(void const * argument);



void StepperInit(void)
{
	osThreadDef(stepperTask, StepperTask, osPriorityNormal, 0, 128);
	stepperTaskHandle = osThreadCreate(osThread(stepperTask), NULL);
//	printf("Init --> StepperInit()\r\n");
}

void StepperTask(void const * argument)
{
	msg_init(&ArmMachineMsg);
//	convertMsgIntoRTU(ArmMachineMsg);
	for(;;)
	{
//		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
//		convertMsgIntoRTU(ArmMachineMsg);
//		stepper_test();
//		printf("Task --> StepperTask()\r\n");
		osDelay(1000);
	}
}



void msg_init(ArmMachine_TypeDef *ArmMachineMsg)
{
	StepperMsg_TypeDef baseMotor;
	StepperMsg_TypeDef updownMotor;
	StepperMsg_TypeDef armMotor;

	// baseMotor
	baseMotor.startSlope = 1500;		// 1.5kHz
	baseMotor.stopSlope = 1500;
	baseMotor.targetPosition = 0;
	baseMotor.topSpeed = 2000;
	baseMotor.mode = 'a';
	baseMotor.alarmCode = 0x00;

	baseMotor.address = 0x03;
	baseMotor.reductionRatio = 4;		// gearRatio = 1 : 4
	baseMotor.stepPerRound = 1000;
	baseMotor.step2angular = 360.0 / baseMotor.stepPerRound / baseMotor.reductionRatio;		//
	baseMotor.step2distance = 0.0;

	// updownMotor
	updownMotor.startSlope = 1500;
	updownMotor.stopSlope = 1500;
	updownMotor.targetPosition = 200;
	updownMotor.topSpeed = 2000;
	updownMotor.mode = 'a';
	updownMotor.alarmCode = 0x00;

	updownMotor.address = 0x02;
	updownMotor.reductionRatio = 1;		// NA
	updownMotor.stepPerRound = 1000;
	updownMotor.step2angular = 0.0;
	updownMotor.step2distance = 1.0 / updownMotor.stepPerRound;		// 导程1mm，步进电机1圈对应1mm距离变化

	// armMotor
	armMotor.startSlope = 100;
	armMotor.stopSlope = 100;
	armMotor.targetPosition = 0;
	armMotor.topSpeed = 1000;
	armMotor.mode = 'a';
	armMotor.alarmCode = 0x00;

	armMotor.address = 0x01;
	armMotor.reductionRatio = 1;		// NA
	armMotor.stepPerRound = 1000;
	armMotor.step2angular = 0.0;
	armMotor.step2distance = 3.1415927 * 27 / armMotor.stepPerRound;		// 齿轮直径27mm

	ArmMachineMsg->armMotor = armMotor;
	ArmMachineMsg->baseMotor = baseMotor;
	ArmMachineMsg->updownMotor = updownMotor;
}


