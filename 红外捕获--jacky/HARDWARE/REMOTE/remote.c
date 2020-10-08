#include "remote.h"
#include "delay.h"
#include "lcd.h"
#include "led.h"
//#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//����ң�ؽ������� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/7
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved								  
//////////////////////////////////////////////////////////////////////////////////



//����ң�س�ʼ��
//����IO�Լ�TIM3_CH3�����벶��
void Remote_Init(void)    			  
{		
    GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;  
 
 	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //ʹ��PORTBʱ�� 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	//TIM3 ʱ��ʹ�� 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;			// PB0 ���� 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 		// �������� 
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_ResetBits(GPIOB, GPIO_Pin_0);	//��ʼ��GPIOB0
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM3); //GPIOB0����ΪTIM3
						  
 	TIM_TimeBaseStructure.TIM_Period = 0xffff; 			//�趨�������Զ���װֵ ���131ms���  
	TIM_TimeBaseStructure.TIM_Prescaler = (176 - 1); 	//Ԥ��Ƶ��,2us��1.t = (arr+1)*(psc+1)/84000000	   	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;  // ѡ������� IC3ӳ�䵽TI3��
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
    TIM_ICInitStructure.TIM_ICFilter = 0x00;	//IC4F=0011 ���������˲��� 8����ʱ��ʱ�������˲�
    TIM_ICInit(TIM3, &TIM_ICInitStructure);		//��ʼ����ʱ�����벶��ͨ��
	
	TIM_ITConfig( TIM3,TIM_IT_Update|TIM_IT_CC3,ENABLE);//��������ж� ,����CC4IE�����ж�
    TIM_Cmd(TIM3, ENABLE ); 	//ʹ�ܶ�ʱ��3
	
	TIM_ClearFlag(TIM3, TIM_IT_CC3|TIM_IT_Update);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�1��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���	
}



extern u32 IR_Key;
extern u8 flag;
u8 	IR_Sta = 0;	
u8 	IR_Up = 0;	
u32 IR_Code = 0;		// ������յ�������  	  
u16 IR_ThisPulse;		// �½���ʱ��������ֵ
u16 IR_LastPulse;		// �½���ʱ��������ֵ
u16 IR_PulseSub;		// �½���ʱ��������ֵ
u16 LianfaCnt;			// �½���ʱ��������ֵ	   		    
u8  IR_PulseCnt = 0;	// �������µĴ���	 

void TIM3_IRQHandler(void)
{ 		    	 
 
	if(TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
		IR_Up = 0;								// ÿ�ν��벶���жϾ���������������ֵ
		IR_ThisPulse = TIM_GetCapture3(TIM3);	// ��ȡͨ��3�Ĳ���ֵ
		
		if(IR_ThisPulse > IR_LastPulse)			// ��β����ֵ�����ϴβ����ֵ
		{
			IR_PulseSub = IR_ThisPulse - IR_LastPulse;	// �õ�ʱ���
		}
		else									// С��ʱҪ����0xffff
		{
			IR_PulseSub = 0xffff - IR_LastPulse + IR_ThisPulse;	// �õ�ʱ���
		}
		IR_LastPulse = IR_ThisPulse;			// �����εõ�ֵ��Ϊ��һ�α����ǰһ��ֵ
		IR_PulseCnt++;					// ����λ����1
		if(IR_PulseCnt == 2)
		{
			if((IR_PulseSub > 6000) && (IR_PulseSub < 8000))	// �����뷶Χ��13.5ms 13.5ms/2us = 6750
			{
				IR_Sta = 0x01;			// ��־λΪ1�� ��ʾ�������Ѿ��յ�
			}
			else
				IR_PulseCnt = 0;		// ���ʱ���û���������뷶Χ�ڣ��жϴ������㣬���±���
		}
		if((IR_PulseCnt > 2) && (IR_Sta == 0x01))	// �жϴ�������2�������������Ѿ�����
		{
			IR_Code <<= 1;				// �洢�������ļĴ���������һλ�� �Ա��ڴ洢��һλ�ŵ����λ
			if((IR_PulseSub > 450) && (IR_PulseSub < 700))	// ���롮0����Χ�� 1.125ms	1.125/2us = 562.5
			{
				IR_Code |= 0x00;		// �洢0
			}
			else if((IR_PulseSub > 800) && (IR_PulseSub) < 1300) // ���롮1����Χ�� 2.25ms	2.25/2us = 1125
			{
				IR_Code |= 0x01;		// �洢1
			}
			else				// �������0��Ҳ����1�룬 �������㣬���½���
			{
				IR_Sta = 0;
				IR_Code = 0;
				IR_PulseCnt = 0;
			}
			
		}
		if(IR_PulseCnt == 34)		// ��������룬 ��34�ν����ж����ý���һ֡��
		{
			IR_Key = IR_Code;		// ��Ž������
			IR_Sta = 0x02;			// �����������״̬
			flag = 1;				// �������־λ1�����б��봦��
		}
		if((IR_PulseCnt == 36) && (IR_Sta == 0x02)) // 2λ�����룬32λ������
		{
			IR_PulseCnt = 34;			// ���������ֱ���Ϊ����34
			if((IR_PulseSub > 4500) && (IR_PulseSub < 6000))	// ��������״̬�� 11.5ms/2us = 5525
			{
				LianfaCnt++;
				IR_Key = IR_Code;
				flag = 1;
			}
		}		
	}
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);	
		if(RDATA == 1)
		{
			IR_Up++;			// �����Ѿ�̧��
			if(IR_Up >= 2)
			{
				IR_Code = 0;
				IR_Sta = 0;
				IR_PulseCnt = 0;
				LianfaCnt = 0;
			}
		}
	}
	
}
































