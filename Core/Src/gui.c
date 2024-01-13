//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//测试硬件：单片机STM32F103RCT6,正点原子MiniSTM32开发板,主频72MHZ，晶振12MHZ
//QDtech-OLED液晶驱动 for STM32
//xiao冯@ShenZhen QDtech co.,LTD
//公司网站:www.qdtft.com
//淘宝网站：http://qdtech.taobao.com
//wiki技术网站：http://www.lcdwiki.com
//我司提供技术支持，任何技术问题欢迎随时交流学习
//固话(传真) :+86 0755-23594567 
//手机:15989313508（冯工） 
//邮箱:lcdwiki01@gmail.com    support@lcdwiki.com    goodtft@163.com
//技术支持QQ:3002773612  3002778157
//技术交流QQ群:324828016
//创建日期:2018/8/27
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 深圳市全动电子技术有限公司 2018-2028
//All rights reserved
/****************************************************************************************************
//=========================================电源接线================================================//
// OLED模块               STM32单片机
//   VCC         接       DC 5V/3.3V      //OLED屏电源正
//   GND         接          GND          //OLED屏电源地
//=======================================液晶屏数据线接线==========================================//
//本模块默认数据总线类型为4线制SPI
// OLED模块               STM32单片机
//   D1          接          PB15        //OLED屏SPI写信号
//=======================================液晶屏控制线接线==========================================//
// OLED模块               STM32单片机
//   CS          接          PB11        //OLED屏片选控制信号
//   RES         接          PB12        //OLED屏复位控制信号
//   DC          接          PB10        //OLED屏数据/命令选择控制信号
//   D0          接          PB13        //OLED屏SPI时钟信号
//=========================================触摸屏接线=========================================//
//本模块不带触摸功能，所以不需要触摸屏接线
****************************************************************************************************/	
/***************************************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
****************************************************************************************************/	
#include "oled.h"
#include "string.h"
#include "gui.h"
#include "oledfont.h"  

/*****************************************************************************
 * @name       :void GUI_ShowChar( uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size, uint8_t mode)
 * @date       :2018-08-27 
 * @function   :Display a single English character
 * @parameters :x:the bebinning x coordinate of the Character display position
                y:the bebinning y coordinate of the Character display position
								chr:the ascii code of display character(0~94)
								Char_Size:the size of display character(8,16)
								mode:0-white background and black character
								     1-black background and white character
 * @retvalue   :None
******************************************************************************/ 
void GUI_ShowChar( uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size, uint8_t mode)
{      	
	  unsigned char c=0,i=0,tmp,j=0;	
		c=chr-' ';//得到偏移后的值			
		if(x>WIDTH-1){x=0;y=y+2;}
		if(Char_Size ==16)
		{
			for(i=0;i<16;i++)
			{
			  if(mode)
				{
					tmp = F8X16[c*16+i];
				}
				else
				{
					tmp = ~(F8X16[c*16+i]);
				}
				for(j=0;j<8;j++)
				{
					if(tmp&(0x80>>j))
					{
						OLED_Set_Pixel(x+j, y+i,1);
					}
					else
					{
						OLED_Set_Pixel(x+j, y+i,0);
					}
				}
			}
		}
	else
	{
		return;
	}
	OLED_Display();
}

/*****************************************************************************
 * @name       :void GUI_ShowString( uint8_t x, uint8_t y, uint8_t *chr, uint8_t Char_Size, uint8_t mode)
 * @date       :2018-08-27 
 * @function   :Display English string
 * @parameters :x:the bebinning x coordinate of the English string
                y:the bebinning y coordinate of the English string
								chr:the start address of the English string
								Char_Size:the size of display character
								mode:0-white background and black character
								     1-black background and white character
 * @retvalue   :None
******************************************************************************/   	  
void GUI_ShowString( uint8_t x, uint8_t y, uint8_t *chr, uint8_t Char_Size, uint8_t mode)
{
	unsigned char j=0,csize;
	if(Char_Size == 16)
  {
	  csize = Char_Size/2;
	}
  else if(Char_Size == 8)
  {
	  csize = Char_Size/2+2;
	}
	else
	{
		return;
	}
	while (chr[j]!='\0')
	{		
		GUI_ShowChar(x,y,chr[j],Char_Size,mode);
		x+=csize;
		if(x>120)
		{
			x=0;
			y+=Char_Size;
		}
		j++;
	}
}
/*****************************************************************************
 * @name       :void GUI_ShowFont16( uint8_t x, uint8_t y, uint8_t *s, uint8_t mode)
 * @date       :2018-08-27 
 * @function   :Display a single 16x16 Chinese character
 * @parameters :x:the bebinning x coordinate of the Chinese character
                y:the bebinning y coordinate of the Chinese character
								s:the start address of the Chinese character
								mode:0-white background and black character
								     1-black background and white character
 * @retvalue   :None
******************************************************************************/ 
void GUI_ShowFont16( uint8_t x, uint8_t y, uint8_t *s, uint8_t mode)
{
	 uint8_t i,j,k,tmp;
	 uint16_t num;
	num = sizeof(cfont16)/sizeof(typFNT_GB16);
  for(i=0;i<num;i++)
	{
		if((cfont16[i].Index[0]==*s)&&(cfont16[i].Index[1]==*(s+1)))
		{
			for(j=0;j<32;j++)
			{
				if(mode)
				{
					tmp = cfont16[i].Msk[j];
				}
				else
				{
					tmp = ~(cfont16[i].Msk[j]);
				}
				for(k=0;k<8;k++)
				{
					if(tmp&(0x80>>k))
					{
						OLED_Set_Pixel(x+(j%2)*8+k, y+j/2,1);
					}
					else
					{
						OLED_Set_Pixel(x+(j%2)*8+k, y+j/2,0);
					}
				}
			}	
			break;
		}	
	}
	OLED_Display();
}
/*****************************************************************************
 * @name       :void GUI_ShowCHinese( uint8_t x, uint8_t y, uint8_t hsize, uint8_t *str, uint8_t mode)
 * @date       :2018-08-27 
 * @function   :Display Chinese strings
 * @parameters :x:the bebinning x coordinate of the Chinese strings
                y:the bebinning y coordinate of the Chinese strings
								size:the size of Chinese strings
								str:the start address of the Chinese strings
								mode:0-white background and black character
								     1-black background and white character
 * @retvalue   :None
******************************************************************************/	   		   
void GUI_ShowCHinese( uint8_t x, uint8_t y, uint8_t hsize, uint8_t *str, uint8_t mode)
{ 
	while(*str!='\0')
	{
		if(hsize == 16)
		{
			GUI_ShowFont16(x,y,str,mode);
		}
		else if(hsize == 24)
		{
			//GUI_ShowFont24(x,y,str,mode);
		}
		else if(hsize == 32)
		{
			//GUI_ShowFont32(x,y,str,mode);
		}
		else
		{
			return;
		}
		x+=hsize;
		if(x>WIDTH-hsize)
		{
			x=0;
			y+=hsize;
		}
		str+=2;
	}			
}
