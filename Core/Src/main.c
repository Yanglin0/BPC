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

//常量
#define Count 99 //10ms*100=1s;
#define StartCount 97 //持续98次及以上为帧起始位
#define BPC_EFFECT_NUM 19 //BPC中一帧数据有20段，19段为有效值
#define BPC_EFFECT_DATA 8

//重要变量
//读取缓冲区
//*******************
//Buff[0]:帧号：0表示1秒，1表示21秒，2表示41秒
//Buff[1]:保留
//Buff[2-3]:小时
//Buff[4-6]:分钟
//Buff[7-8]:星期
//Buff[9]:上下午和前面数据的校验位
//Buff[10-12]:日
//Buff[13-14]:月
//Buff[15-17]:年
//Buff[18]:年的二进制最高位后面数据的校验位
//奇偶校验：1：奇校验 0：偶校验
//*******************
uint32_t flag1 = 0;
uint32_t flag2 = 0;
uint32_t Buff[BPC_EFFECT_NUM];
uint32_t DATA[BPC_EFFECT_DATA];
uint32_t BPC_First_Check = 9;
uint32_t BPC_Second_Check = 18;

//临时变量
uint32_t flag = 0; //是否读取到帧起始位
uint32_t p = 0; //记录当前读到第几个数据段
uint32_t x = 0;
uint32_t y = 0; //y记录低电平持续时间，若低电平持续100次左右，则为帧起始位
uint32_t z = 0; 
uint32_t i = 0;
uint32_t j = 0;
uint32_t temp = 0; //记录四进制数
//临时变量

//4进制范围
#define Zero_Low 5
#define Zero_High 14
#define One_Low 15
#define One_High 24
#define Two_Low 25
#define Two_High 34
#define Three_Low 35
#define Three_High 44
//4进制范围

//RTC时间变量
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
//RTC时间变量

//FLash数据
const u8 TEXT_Buffer_1[]={"193010100802"};
#define SIZE sizeof(TEXT_Buffer_1)
int light=1;//1表示绿灯,2表示黄灯,3表示红灯
int red;
int green;
int yellow;
//FLash数据

//RTthread变量
struct rt_thread ShowRTC_thread;
struct rt_thread BPC_thread;
rt_uint8_t rt_showrtc_thread_stack[1024];
rt_uint8_t rt_bpc_thread_stack[1024];
//RTthread变量

//函数声明
void SystemClock_Config(void);
uint32_t BPC_DECODE(uint32_t *Buff, uint32_t *DATA);
uint8_t tenTo16(uint32_t num);
uint32_t BPC_DECODE(uint32_t *Buff, uint32_t *DATA);
void SetSchedule();
void decodeSchedule();
//函数声明

/**
  * @brief  重写printf
  * @retval None
  */
int fputc(int ch, FILE *f){
	HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return ch;	
}

/**
  * @brief  向Flash中写入时间表
  * @retval None
  */
void SetSchedule()
{
	printf("写入数据成功");
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)TEXT_Buffer_1, SIZE);
	printf("写入数据成功");
}

/**
  * @brief  读取时间表
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
  * @brief  1.主函数每一秒获取获取一次RTC的时间
  * @brief  2.显示RTC时间的线程
  * @retval None
  */
void ShowRTC_task_entry(void *parameter)
{
	while(1)
	{
	  
	  sprintf(DATE_Year, "%02d", 2000 + GetDate.Year);
	  GUI_ShowString(0,5,(uint8_t *)DATE_Year, 16, 1);
	  GUI_ShowCHinese(33,5,16,"年",1);
	  
	  sprintf(DATE_Month, "%02d", GetDate.Month);
	  GUI_ShowString(48,5, (uint8_t *) DATE_Month, 16, 1);
	  GUI_ShowCHinese(63,5,16,"月",1);
	  
	  sprintf(DATE_Day, "%02d", GetDate.Date);
	  GUI_ShowString(78,5, (uint8_t *) DATE_Day, 16, 1);
	  GUI_ShowCHinese(93,5,16,"日",1);
	  
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
  * @brief  电波钟解码
  * @retval None
  */
void BPC_task_entry(void *parameter)
{
	while(1){
	if(flag1)
	{
		flag1 = 0;
		  printf("检测到了起始位\r\n");
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
			  printf("包含无效数据\r\n");
		  }
		  else if (j == 2)
		  {
			  printf("0-8位有效数据检验错误\r\n");
		  }
		  else if (j == 3)
		  {
			  printf("9-18位有效数据检验错误\r\n");
		  }
		  else if (j == 4)
		  {
			  //成功解出时间，校准RTC
			  RTC_TimeTypeDef myTime = {0};
			  RTC_DateTypeDef myDate = {0};
			  //将十进制转换为16进制(例如：12转换为0x12)
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
  * @brief  10进制转换16进制
  * @retval None
  */
uint8_t tenTo16(uint32_t num){
	uint8_t num1;
	num1 = num%10+(num/10)%10*16;
	return num1;
}

/**
  * @brief  16进制转换10进制
  * @retval None
  */
uint32_t sixteenTo10(uint8_t num){
	return (num&0x0f)+(((num&0xf0)>>4)*10);
}

/**
  * @brief  RTthread多任务初始化，同时开启RTC和BPC
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
  * @brief  闹钟中断实现灯切换
  * @retval None
  */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
   if(light == 1){
	   //当前亮得灯为绿灯，换成黄灯;
	   printf("======================\r\n");
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	   printf("绿：灭\r\n");
	   printf("黄：亮\r\n");
	   printf("红：灭\r\n");
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
	   //当前亮黄灯，换成红灯
	   printf("======================\r\n");
	   	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	   printf("绿：灭\r\n");
	   printf("黄：灭\r\n");
	   printf("红：亮\r\n");
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
	   //当前亮红灯，换成绿灯
	   printf("======================\r\n");
	   	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	   printf("绿：亮\r\n");
	   printf("黄：灭\r\n");
	   printf("红：灭\r\n");
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
  * @brief  每10ms进入一次更新中断，记录高低电平次数
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim2){
		if(flag){              //已经找到帧起始位，开始获取输入引脚有效数据
			if(p < BPC_EFFECT_NUM)
			{
				if(z < Count)
				{
					z++;
					if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
						i++; //表示这个高电平持续了多少次
				}
				else         //读完一个1s内的高电平持续时间后，将次数转换为4进制
				{
					printf("i = %d\r\n", i);
					if(i >= Zero_Low && i <= Zero_High)
					{
						temp = 0; //四进制"0"
					}
					else if(i >= One_Low && i <= One_High)
					{
						temp = 1; //四进制"1"
					}
					else if(i >= Two_Low && i <= Two_High)
					{
						temp = 2; //四进制"2"
					}
					else if(i >= Three_Low && i <= Three_High)
					{
						temp = 3; //四进制"3"
					}
					else
					{
						temp = 4; //无意义
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
				flag = 0; //开始下一帧起始位读取
				flag2 = 1; //读取完一帧后开启处理过程
			}
			
		}
		else                  //等待帧起始标志位
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
					flag = 1; //已经找到了帧起始位
					flag1 = 1;
				}
				//将x,y清0方便找下一次起始位
				y = 0;
				x = 0;
			}
		}
	}
}





/**
  * @brief  解码
  * @retval None
  */
uint32_t BPC_DECODE(uint32_t *Buff, uint32_t *DATA)
{
	uint32_t i;
	uint32_t x = 0;
	//清空解码时间缓冲区
	for(i = 0; i < BPC_EFFECT_DATA; i++){
		DATA[i]= 0;
	}
	//判断是否有无效数据，若有则返回
	for(i = 0; i < BPC_EFFECT_NUM; i++)
	{
		if(Buff[i] == 4)
			return 1;
	}
	
	//校验第一阶段数据，记录前面数据1的个数，进行奇偶校验
		for(i = 0; i < BPC_First_Check; i++)
	{
		if(Buff[i] == 1 || Buff[i] == 2)
			x+=1; //一个1
		else if(Buff[i] == 3)
			x+=2; //两个1
	}
	if(x%2 == 0) //有偶数个1
	{
		if(Buff[BPC_First_Check] == 1 || Buff[BPC_First_Check] == 3)
			return 2; //前0-8位校验失败
	}
	else
	{
		if(Buff[BPC_First_Check] == 0 || Buff[BPC_First_Check] == 2)
			return 2; //前0-8位校验失败
	}
	x = 0;
	
	//校验第二阶段数据，记录前面数据1的个数，进行奇偶校验
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
			return 3; //后9-18位校验失败
	}
	
	//校验无错，开始解码
	//秒
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
	
	//时
	DATA[1]=Buff[2]*4+Buff[3];
	
	//分
	DATA[2]=Buff[4]*16+Buff[5]*4+Buff[6];
	
	//星期
	DATA[3]=Buff[7]*4+Buff[8];
	
	//上下午
	if(Buff[9] == 0 || Buff[9] == 1)
	{
		DATA[4] = 0;
	}else if(Buff[9] == 2 || Buff[9] == 3)
	{
		DATA[4] = 1;
	}
	
	//日
	DATA[5] = Buff[10]*16+Buff[11]*4+Buff[12];
	
	//月
	DATA[6] = Buff[13]*4+Buff[14];
	
	//年
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
