/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "oled.h"
#include "gui.h"
#include "rtthread.h"
#include "stmflash.h"

//����
#define Count 99 //10ms*100=1s;
#define StartCount 97 //����98�μ�����Ϊ֡��ʼλ
#define BPC_EFFECT_NUM 19 //BPC��һ֡������20�Σ�19��Ϊ��Чֵ
#define BPC_EFFECT_DATA 8

//��Ҫ����
//��ȡ������
//*******************
//Buff[0]:֡�ţ�0��ʾ1�룬1��ʾ21�룬2��ʾ41��
//Buff[1]:����
//Buff[2-3]:Сʱ
//Buff[4-6]:����
//Buff[7-8]:����
//Buff[9]:�������ǰ�����ݵ�У��λ
//Buff[10-12]:��
//Buff[13-14]:��
//Buff[15-17]:��
//Buff[18]:��Ķ��������λ�������ݵ�У��λ
//��żУ�飺1����У�� 0��żУ��
//*******************
uint32_t flag1 = 0;
uint32_t flag2 = 0;
uint32_t Buff[BPC_EFFECT_NUM];
uint32_t DATA[BPC_EFFECT_DATA];
uint32_t BPC_First_Check = 9;
uint32_t BPC_Second_Check = 18;

//��ʱ����
uint32_t flag = 0; //�Ƿ��ȡ��֡��ʼλ
uint32_t p = 0; //��¼��ǰ�����ڼ������ݶ�
uint32_t x = 0;
uint32_t y = 0; //y��¼�͵�ƽ����ʱ�䣬���͵�ƽ����100�����ң���Ϊ֡��ʼλ
uint32_t z = 0; 
uint32_t i = 0;
uint32_t j = 0;
uint32_t temp = 0; //��¼�Ľ�����
//��ʱ����

//4���Ʒ�Χ
#define Zero_Low 5
#define Zero_High 14
#define One_Low 15
#define One_High 24
#define Two_Low 25
#define Two_High 34
#define Three_Low 35
#define Three_High 44
//4���Ʒ�Χ

//RTCʱ�����
RTC_TimeTypeDef GetTime;
RTC_DateTypeDef GetDate;
char DATE_Year[5] = "2021";
char DATE_Month[3] = "12";
char DATE_Day[3] = "12";
char DATE_Hour[3] = "21";
char DATE_Minute[3] = "26";
char DATE_Second[3] = "21";
char DATE_Weekday[3] = "Mon";
RTC_TimeTypeDef flashtime;
RTC_TimeTypeDef redtime;
RTC_TimeTypeDef greentime;
RTC_TimeTypeDef yellowtime;
RTC_AlarmTypeDef alarm;
//RTCʱ�����

//FLash����
const u8 TEXT_Buffer_1[]={"193010100802"};
#define SIZE sizeof(TEXT_Buffer_1)
int light=1;//1��ʾ�̵�,2��ʾ�Ƶ�,3��ʾ���
int red;
int green;
int yellow;
//FLash����

//RTthread����
struct rt_thread ShowRTC_thread;
struct rt_thread BPC_thread;
rt_uint8_t rt_showrtc_thread_stack[1024];
rt_uint8_t rt_bpc_thread_stack[1024];
//RTthread����

//��������
void SystemClock_Config(void);
uint32_t BPC_DECODE(uint32_t *Buff, uint32_t *DATA);
uint8_t tenTo16(uint32_t num);
uint32_t BPC_DECODE(uint32_t *Buff, uint32_t *DATA);
void SetSchedule();
void decodeSchedule();
//��������

/**
  * @brief  ��дprintf
  * @retval None
  */
int fputc(int ch, FILE *f){
	HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return ch;	
}

/**
  * @brief  ��Flash��д��ʱ���
  * @retval None
  */
void SetSchedule()
{
	printf("д�����ݳɹ�");
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)TEXT_Buffer_1, SIZE);
	printf("д�����ݳɹ�");
}

/**
  * @brief  ��ȡʱ���
  * @retval None
  */
void deCodeSchedule()
{
	u8 datatemp[SIZE];
	u8 *p = datatemp;
	STMFLASH_Read(FLASH_SAVE_ADDR, (u16*)datatemp, SIZE);
	printf("%s",p);
	//flashtime.Hours = tenTo16((datatemp[0]-'0')*10+(datatemp[1]-'0'));
	//flashtime.Minutes = tenTo16(datatemp[2]*10+datatemp[3]);
	//flashtime.Seconds = tenTo16(datatemp[4]*10+datatemp[5]);
	//red = datatemp[6]*10+datatemp[7];
	//green = datatemp[8]*10+datatemp[9];
	//yellow = datatemp[10]*10+datatemp[11];
}



/**
  * @brief  1.������ÿһ���ȡ��ȡһ��RTC��ʱ��
  * @brief  2.��ʾRTCʱ����߳�
  * @retval None
  */
void ShowRTC_task_entry(void *parameter)
{
	while(1)
	{
	  
	  sprintf(DATE_Year, "%02d", 2000 + GetDate.Year);
	  GUI_ShowString(0,5,(uint8_t *)DATE_Year, 16, 1);
	  GUI_ShowCHinese(33,5,16,"��",1);
	  
	  sprintf(DATE_Month, "%02d", GetDate.Month);
	  GUI_ShowString(48,5, (uint8_t *) DATE_Month, 16, 1);
	  GUI_ShowCHinese(63,5,16,"��",1);
	  
	  sprintf(DATE_Day, "%02d", GetDate.Date);
	  GUI_ShowString(78,5, (uint8_t *) DATE_Day, 16, 1);
	  GUI_ShowCHinese(93,5,16,"��",1);
	  
	  sprintf(DATE_Weekday, "%d", GetDate.WeekDay);
	  GUI_ShowString(30,46,(uint8_t *) DATE_Weekday,16,1);
      /* Display time Format : hh:mm:ss */
      sprintf(DATE_Hour, "%02d:", GetTime.Hours);
	  GUI_ShowString(30,23,(uint8_t *) DATE_Hour, 16, 1);
	  sprintf(DATE_Minute, "%02d:", GetTime.Minutes);
	  GUI_ShowString(55,23,(uint8_t *) DATE_Minute, 16, 1);
	  sprintf(DATE_Second, "%02d", GetTime.Seconds);
	  GUI_ShowString(80,23,(uint8_t *) DATE_Second, 16, 1);
	  
      rt_thread_delay(500);
	  OLED_Clear(0);
	}
}

/**
  * @brief  �粨�ӽ���
  * @retval None
  */
void BPC_task_entry(void *parameter)
{
	while(1){
	if(flag1)
	{
		flag1 = 0;
		  printf("��⵽����ʼλ\r\n");
	}
	else if(flag2)
	  {
		  flag2=0;
		  for(j=0; j<BPC_EFFECT_NUM; j++)
			  printf("%d ", Buff[j]);
			printf("\r\n");
		  j = BPC_DECODE(Buff, DATA);
		  if(j == 1)
		  {
			  printf("������Ч����\r\n");
		  }
		  else if (j == 2)
		  {
			  printf("0-8λ��Ч���ݼ������\r\n");
		  }
		  else if (j == 3)
		  {
			  printf("9-18λ��Ч���ݼ������\r\n");
		  }
		  else if (j == 4)
		  {
			  //�ɹ����ʱ�䣬У׼RTC
			  RTC_TimeTypeDef myTime = {0};
			  RTC_DateTypeDef myDate = {0};
			  //��ʮ����ת��Ϊ16����(���磺12ת��Ϊ0x12)
			  myTime.Hours = tenTo16(DATA[4]?(DATA[1]+12):DATA[1]);
			  myTime.Minutes = tenTo16(DATA[2]);
			  myTime.Seconds = tenTo16((DATA[0]+19)%60);
			  myDate.Year = tenTo16(DATA[7]);
			  myDate.Date = tenTo16(DATA[5]);
			  switch(DATA[6]){
				  case 1:{
					  myDate.Month = RTC_MONTH_JANUARY;
					  break;
				  }
				  case 2:{
					  myDate.Month = RTC_MONTH_FEBRUARY;
					  break;
				  }
				  case 3:{
					  myDate.Month = RTC_MONTH_MARCH;
					  break;
				  }
				  case 4:{
					  myDate.Month = RTC_MONTH_APRIL;
					  break;
				  }
				  case 5:{
					  myDate.Month = RTC_MONTH_MAY;
					  break;
				  }
				  case 6:{
					  myDate.Month = RTC_MONTH_JUNE;
					  break;
				  }
				  case 7:{
					  myDate.Month = RTC_MONTH_JULY;
					  break;
				  }
				  case 8:{
					  myDate.Month = RTC_MONTH_AUGUST;
					  break;
				  }
				  case 9:{
					  myDate.Month = RTC_MONTH_SEPTEMBER;
					  break;
				  }
				  case 10:{
					  myDate.Month = RTC_MONTH_OCTOBER;
					  break;
				  }
				  case 11:{
					  myDate.Month = RTC_MONTH_NOVEMBER;
					  break;
				  }
				  case 12:{
					  myDate.Month = RTC_MONTH_DECEMBER;
					  break;
				  }
			  }
			  switch(DATA[3]){
				  case 1:{
					  myDate.WeekDay = RTC_WEEKDAY_MONDAY;
					  break;
				  }
				  case 2:{
					  myDate.WeekDay = RTC_WEEKDAY_TUESDAY;
					  break;
				  }
				  case 3:{
					  myDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
					  break;
				  }
				  case 4:{
					  myDate.WeekDay = RTC_WEEKDAY_THURSDAY;
					  break;
				  }
				  case 5:{
					  myDate.WeekDay = RTC_WEEKDAY_FRIDAY;
					  break;
				  }
				  case 6:{
					  myDate.WeekDay = RTC_WEEKDAY_SATURDAY;
					  break;
				  }
				  case 7:{
					  myDate.WeekDay = RTC_WEEKDAY_SUNDAY;
					  break;
				  }
			  }
			  if(HAL_RTC_SetTime(&hrtc, &myTime, RTC_FORMAT_BCD)!=HAL_OK){
				  Error_Handler();
			  }
			  if(HAL_RTC_SetDate(&hrtc, &myDate, RTC_FORMAT_BCD)!=HAL_OK){
				  Error_Handler();
			  }
		  }
	  }
	  rt_thread_mdelay(10);
  }
	  
}

/**
  * @brief  10����ת��16����
  * @retval None
  */
uint8_t tenTo16(uint32_t num){
	uint8_t num1;
	num1 = num%10+(num/10)%10*16;
	return num1;
}

/**
  * @brief  16����ת��10����
  * @retval None
  */
uint32_t sixteenTo10(uint8_t num){
	return (num&0x0f)+(((num&0xf0)>>4)*10);
}

/**
  * @brief  RTthread�������ʼ����ͬʱ����RTC��BPC
  * @retval None
  */
void MX_RT_Thread_Init(void)
{
	rt_thread_init(&ShowRTC_thread,"ShowRTC",ShowRTC_task_entry,RT_NULL,rt_showrtc_thread_stack, sizeof(rt_showrtc_thread_stack),5,10);
	rt_thread_startup(&ShowRTC_thread);
	rt_thread_init(&BPC_thread, "BPC_Thread",BPC_task_entry, RT_NULL, rt_bpc_thread_stack, sizeof(rt_bpc_thread_stack),4, 10);
	rt_thread_startup(&BPC_thread);
}

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_RTC_Init();
	MX_TIM2_Init();
	MX_SPI2_Init();
	MX_USART1_UART_Init();
	HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	rt_thread_mdelay(2000);
	SetSchedule();

  	OLED_Init();
    OLED_Clear(0);
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_RTC_GetTime(&hrtc, &GetTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &GetDate, RTC_FORMAT_BIN);
	u8 datatemp[SIZE];
	u32 data[SIZE];
	u32 i = 0;
	STMFLASH_Read(FLASH_SAVE_ADDR, (u16*)datatemp, SIZE);
	for(i = 0; i < SIZE; i++){
		data[i] = datatemp[i]-'0';
	}
	flashtime.Hours = data[0]*10+data[1];
	//tenTo16((datatemp[0]-'0')*10+(datatemp[1]-'0'));
	flashtime.Minutes = data[2]*10+data[3];
	//tenTo16(datatemp[2]*10+datatemp[3]);
	flashtime.Seconds = data[4]*10+data[5];
	//tenTo16(datatemp[4]*10+datatemp[5]);
	alarm.AlarmTime=flashtime;
	//alarm.AlarmTime.Hours=14;
	//alarm.AlarmTime.Minutes=00;
	//alarm.AlarmTime.Seconds=10;
	alarm.Alarm = RTC_ALARM_A;
	HAL_RTC_SetAlarm_IT(&hrtc,&alarm,RTC_FORMAT_BIN);
	red = data[6]*10+data[7];
	green = data[8]*10+data[9];
	yellow = data[10]*10+data[11];
	MX_RT_Thread_Init();
	while (1)
  {
	  HAL_RTC_GetTime(&hrtc, &GetTime, RTC_FORMAT_BIN);
	  HAL_RTC_GetDate(&hrtc, &GetDate, RTC_FORMAT_BIN);
	  rt_thread_mdelay(1000);
  }
}

/**
  * @brief  �����ж�ʵ�ֵ��л�
  * @retval None
  */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
   if(light == 1){
	   //��ǰ���õ�Ϊ�̵ƣ����ɻƵ�;
	   printf("======================\r\n");
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	   printf("�̣���\r\n");
	   printf("�ƣ���\r\n");
	   printf("�죺��\r\n");
	   printf("======================\r\n");
	   light = 2;
	   int seconds,minutes, hours;
	   seconds = alarm.AlarmTime.Seconds+2;
	   //sixteenTo10(alarm.AlarmTime.Seconds)+2;
	   minutes = alarm.AlarmTime.Minutes;
	   //sixteenTo10(alarm.AlarmTime.Minutes);
	   hours = alarm.AlarmTime.Hours;
	   //sixteenTo10(alarm.AlarmTime.Hours);
	   if(seconds >= 60){
		   minutes +=1;
		   if(minutes >= 60){
			   hours += 1;
			   alarm.AlarmTime.Hours = hours%60;
			   //tenTo16(hours%60);
		   }
		   alarm.AlarmTime.Minutes = minutes%60;
		   //tenTo16(minutes%60);
	   }
	   alarm.AlarmTime.Seconds = seconds%60;
	   //tenTo16(seconds%60);
   }else if(light == 2){
	   //��ǰ���Ƶƣ����ɺ��
	   printf("======================\r\n");
	   	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	   printf("�̣���\r\n");
	   printf("�ƣ���\r\n");
	   printf("�죺��\r\n");
	   printf("======================\r\n");
	   light = 3;
	   int seconds,minutes, hours;
	   seconds = alarm.AlarmTime.Seconds+10;
	   //sixteenTo10(alarm.AlarmTime.Seconds)+10;
	   minutes = alarm.AlarmTime.Minutes;
	   //sixteenTo10(alarm.AlarmTime.Minutes);
	   hours = alarm.AlarmTime.Hours;
	   //sixteenTo10(alarm.AlarmTime.Hours);
	   if(seconds >= 60){
		   minutes +=1;
		   if(minutes >= 60){
			   hours += 1;
			   alarm.AlarmTime.Hours = hours%60;
			   //tenTo16(hours%60);
		   }
		   alarm.AlarmTime.Minutes = minutes%60;
		   //tenTo16(minutes%60);
	   }
	   alarm.AlarmTime.Seconds = seconds%60;
	   //tenTo16(seconds%60);
   }else if(light == 3){
	   //��ǰ����ƣ������̵�
	   printf("======================\r\n");
	   	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	   printf("�̣���\r\n");
	   printf("�ƣ���\r\n");
	   printf("�죺��\r\n");
	   printf("======================\r\n");
	   light = 1;
	   int seconds,minutes, hours;
	   seconds = alarm.AlarmTime.Seconds+8;
	   //sixteenTo10(alarm.AlarmTime.Seconds)+8;
	   minutes = alarm.AlarmTime.Minutes;
	   //sixteenTo10(alarm.AlarmTime.Minutes);
	   hours = alarm.AlarmTime.Hours;
	   //sixteenTo10(alarm.AlarmTime.Hours);
	   if(seconds >= 60){
		   minutes +=1;
		   if(minutes >= 60){
			   hours += 1;
			   alarm.AlarmTime.Hours = hours%60;
			   //tenTo16(hours%60);
		   }
		   alarm.AlarmTime.Minutes = minutes%60;
		   //tenTo16(minutes%60);
	   }
	   alarm.AlarmTime.Seconds = seconds%60;
	   //tenTo16(seconds%60);
   }
   HAL_RTC_SetAlarm_IT(hrtc,&alarm,RTC_FORMAT_BIN);
}



/**
  * @brief  ÿ10ms����һ�θ����жϣ���¼�ߵ͵�ƽ����
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim2){
		if(flag){              //�Ѿ��ҵ�֡��ʼλ����ʼ��ȡ����������Ч����
			if(p < BPC_EFFECT_NUM)
			{
				if(z < Count)
				{
					z++;
					if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
						i++; //��ʾ����ߵ�ƽ�����˶��ٴ�
				}
				else         //����һ��1s�ڵĸߵ�ƽ����ʱ��󣬽�����ת��Ϊ4����
				{
					printf("i = %d\r\n", i);
					if(i >= Zero_Low && i <= Zero_High)
					{
						temp = 0; //�Ľ���"0"
					}
					else if(i >= One_Low && i <= One_High)
					{
						temp = 1; //�Ľ���"1"
					}
					else if(i >= Two_Low && i <= Two_High)
					{
						temp = 2; //�Ľ���"2"
					}
					else if(i >= Three_Low && i <= Three_High)
					{
						temp = 3; //�Ľ���"3"
					}
					else
					{
						temp = 4; //������
					}
					Buff[p] = temp;
					p++;
					z = 0;
					i = 0;
				}
			}
			else
			{
				p = 0;
				flag = 0; //��ʼ��һ֡��ʼλ��ȡ
				flag2 = 1; //��ȡ��һ֡�����������
			}
			
		}
		else                  //�ȴ�֡��ʼ��־λ
		{
			if(x < Count){
				x++;
				if(!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
					y++;
			}
			else
			{

				printf("y = %d\r\n", y);
				if(y >= StartCount){
					flag = 1; //�Ѿ��ҵ���֡��ʼλ
					flag1 = 1;
				}
				//��x,y��0��������һ����ʼλ
				y = 0;
				x = 0;
			}
		}
	}
}





/**
  * @brief  ����
  * @retval None
  */
uint32_t BPC_DECODE(uint32_t *Buff, uint32_t *DATA)
{
	uint32_t i;
	uint32_t x = 0;
	//��ս���ʱ�仺����
	for(i = 0; i < BPC_EFFECT_DATA; i++){
		DATA[i]= 0;
	}
	//�ж��Ƿ�����Ч���ݣ������򷵻�
	for(i = 0; i < BPC_EFFECT_NUM; i++)
	{
		if(Buff[i] == 4)
			return 1;
	}
	
	//У���һ�׶����ݣ���¼ǰ������1�ĸ�����������żУ��
		for(i = 0; i < BPC_First_Check; i++)
	{
		if(Buff[i] == 1 || Buff[i] == 2)
			x+=1; //һ��1
		else if(Buff[i] == 3)
			x+=2; //����1
	}
	if(x%2 == 0) //��ż����1
	{
		if(Buff[BPC_First_Check] == 1 || Buff[BPC_First_Check] == 3)
			return 2; //ǰ0-8λУ��ʧ��
	}
	else
	{
		if(Buff[BPC_First_Check] == 0 || Buff[BPC_First_Check] == 2)
			return 2; //ǰ0-8λУ��ʧ��
	}
	x = 0;
	
	//У��ڶ��׶����ݣ���¼ǰ������1�ĸ�����������żУ��
	for(i = 9; i < BPC_Second_Check; i++)
	{
		if(Buff[i] == 1 || Buff[i] == 2)
			x+=1;
		else if(Buff[i] == 3)
			x+=2;
	}
	if(x%2 == 0)
	{
		if(Buff[BPC_Second_Check] == 1 || Buff[BPC_Second_Check] == 3)
		 return 3;
	}
	else
	{
		if(Buff[BPC_Second_Check] == 0 || Buff[BPC_Second_Check] == 2)
			return 3; //��9-18λУ��ʧ��
	}
	
	//У���޴���ʼ����
	//��
	if(Buff[0] == 0)
	{
		DATA[0]=1;
	}
	else if(Buff[0] == 1)
	{
		DATA[0]=21;
	}
	else if(Buff[0] == 2)
	{
		DATA[0]=41;
	}
	
	//ʱ
	DATA[1]=Buff[2]*4+Buff[3];
	
	//��
	DATA[2]=Buff[4]*16+Buff[5]*4+Buff[6];
	
	//����
	DATA[3]=Buff[7]*4+Buff[8];
	
	//������
	if(Buff[9] == 0 || Buff[9] == 1)
	{
		DATA[4] = 0;
	}else if(Buff[9] == 2 || Buff[9] == 3)
	{
		DATA[4] = 1;
	}
	
	//��
	DATA[5] = Buff[10]*16+Buff[11]*4+Buff[12];
	
	//��
	DATA[6] = Buff[13]*4+Buff[14];
	
	//��
	if(Buff[18] == 0 || Buff[18] == 1)
	{
		DATA[7] = Buff[15]*16+Buff[16]*4+Buff[17];
	}
	else if(Buff[18] == 2 || Buff[18] == 3)
	{
		DATA[7] = 64+Buff[15]*16+Buff[16]*4+Buff[17];
	}
	
	return 4;
	
}




/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
