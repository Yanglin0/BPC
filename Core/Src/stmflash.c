#include "stmflash.h"
 
FLASH_ProcessTypeDef p_Flash; 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];    //????
 
 /**********************************************************************************
  * 函数功能：读取指定地址的半字(16位数据)
  * 输入参数：faddr：读地址
  * 返 回 值：对应数据
  * 说    明：
  */
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
 
#if STM32_FLASH_WREN	//如果使能了写  
 /**********************************************************************************
  * 函数功能：不检查的写入
  * 输入参数：WriteAddr:起始地址、pBuffer:数据指针、NumToWrite:半字(16位)数
  * 返 回 值：无
  * 说    明: 
  */
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//地址增加2
	}  
} 
 /**********************************************************************************
  * 函数功能：从指定地址开始写入指定长度的数据
  * 输入参数：WriteAddr：起始地址(此地址必须为2的倍数)、pBuffer：数据指针、NumToWrite：半字数
  * 返回值：无
  * 说明：
  */
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(16位字计算)
	u16 secremain; //扇区内剩余地址(16位字计算)  
 	u16 i;    
	u32 offaddr;   //去掉0x08000000后的地址
	
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//????
	
	HAL_FLASH_Unlock();					    //解锁
	offaddr=WriteAddr-STM32_FLASH_BASE;		//地址
	secpos=offaddr/STM_SECTOR_SIZE;			
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		
	secremain=STM_SECTOR_SIZE/2-secoff;		
	if(NumToWrite<=secremain)secremain=NumToWrite;
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//?????????
		for(i=0;i<secremain;i++)	
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break; 	  
		}
		if(i<secremain)				//????
		{
			Flash_PageErase(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);	
			FLASH_WaitForLastOperation(FLASH_WAITETIME);            	
			CLEAR_BIT(FLASH->CR, FLASH_CR_PER);							
																		
			for(i=0;i<secremain;i++)//??
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//??????  
		}else 
		{
			FLASH_WaitForLastOperation(FLASH_WAITETIME);       	//????????
			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//???????,??????????. 
		}
		if(NumToWrite==secremain)break;//?????
		else//?????
		{
			secpos++;				//?????1
			secoff=0;				//?????0 	 
		   	pBuffer+=secremain;  	//????
			WriteAddr+=secremain*2;	//?????(16?????,??*2)	   
		   	NumToWrite-=secremain;	//??(16?)???
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//??????????
			else secremain=NumToWrite;//??????????
		}	 
	};	
	HAL_FLASH_Lock();		//??
}
#endif
 /**********************************************************************************
  * ????:????????????????
  * ????:ReadAddr:?????pBuffer:?????NumToWrite:??(16?)?
  * ? ? ?: ?
  * ?    ?: 
  */
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//??2???.
		ReadAddr+=2;//??2???.	
	}
}
 
 /**********************************************************************************
  * ????:????
  * ????:PageAddress:??????
  * ? ? ?: ?
  * ?    ?: 
  */
void Flash_PageErase(uint32_t PageAddress)
{
  /* Clean the error context */
  p_Flash.ErrorCode = HAL_FLASH_ERROR_NONE;
 
#if defined(FLASH_BANK2_END)
  if(PageAddress > FLASH_BANK1_END)
  { 
    /* Proceed to erase the page */
    SET_BIT(FLASH->CR2, FLASH_CR2_PER);
    WRITE_REG(FLASH->AR2, PageAddress);
    SET_BIT(FLASH->CR2, FLASH_CR2_STRT);
  }
  else
  {
#endif /* FLASH_BANK2_END */
    /* Proceed to erase the page */
    SET_BIT(FLASH->CR, FLASH_CR_PER);
    WRITE_REG(FLASH->AR, PageAddress);
    SET_BIT(FLASH->CR, FLASH_CR_STRT);
#if defined(FLASH_BANK2_END)
 
  }
#endif /* FLASH_BANK2_END */
  }

