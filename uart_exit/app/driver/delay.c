/*
 * delay.c
 *
 *  Created on: 2018��8��5��
 *      Author: Marshal
 */
#include "driver/delay.h"
#include "osapi.h"
/*
 * �������ƣ�void DealyUs(unsigned int data)
 * �������ܣ�΢����ʱ����
 * �����βΣ�unsigned int data����ʱʱ��
 * */
void Delay1ms(unsigned int y)
{
	os_delay_us(y * 1000);
}






