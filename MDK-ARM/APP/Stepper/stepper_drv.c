/* Includes ------------------------------------------------------------------*/
#include "stepper_drv.h"


/*
 * 本文件包含了：
 * 1. 步进电机所使用的RS485串口的端口初始化
 
 
 * This file contains:
 * 1. Init of the serial port used by Stepper

*/

void stepper_send_data(void)
{
	
}


void stepper_test(void)
{
/* 使用“直接数据运行”
	号机号码： 01h ==> 1
	功能代码： 10h
	写入寄存器起始地址： 00 58h
	写入寄存器数： 00 10h ==>16个
	写入byte数： 20h ==> 32个
	
	1. 运行数据No.： 00 00 00 00h ==> 0
	2. 方式： 相对定位（00 00 00 02）
	3. 位置： 00 00 21 24h ==> 8500 step
	4. 速度： 00 00 07 D0h ==> 2000Hz
	5. 起动/变速斜率： 00 00 05 DCh ==> 1.5kHz
	6. 停止斜率： 00 00 05 DCh ==> 1.5kHz
	7. 运行电流: 00 00 03 E8h ==> 100.0%
	
	反映触发： 全部数据反映（00 00 00 01）
	错误检查： 1C 08h
	
*/
	uint8_t test_data[41] = {
	0x01, 
	0x10, 
	0x00, 0x58, 
	0x00, 0x10, 
	0x20, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x02,
	0x00, 0x00, 0x21, 0x34,
	0x00, 0x00, 0x07, 0xd0,
	0x00, 0x00, 0x05, 0xdc,
	0x00, 0x00, 0x05, 0xdc,
	0x00, 0x00, 0x03, 0xe8,
	0x00, 0x00, 0x00, 0x01,
	0x1c, 0x08
	};
	
	HAL_UART_Transmit(&huart2, test_data, 41, 0xFFFF);
}

