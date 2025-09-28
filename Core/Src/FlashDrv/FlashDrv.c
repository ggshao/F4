/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name        :  FlashDrv.c
 * Description      :  falsh driver base on stm32f446
 ******************************************************************************
 * @attention
 *
* COPYRIGHT:    Copyright (c) 2024  tangminfei2013@126.com
* DATE:         JUL 05th, 2024
 ******************************************************************************
 */
/* USER CODE END Header */
#include "FlashDrv.h"
#include "stdio.h"
#include "status.h"

 /* Start @ of user Flash area */
#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_2 //flash开始区为2

/* End @ of user Flash area : sector start address + sector size -1 */
#define FLASH_USER_END_ADDR     ADDR_FLASH_SECTOR_7  +  GetSectorSize(ADDR_FLASH_SECTOR_7) -1 //结束为7区＋GetSectorSize(ADDR_FLASH_SECTOR_7)-1


#define  FLASH_TIMEOUT_VALUE     1000  //设置最长等待时间为1000ms

static uint32_t GetSectorSize(uint32_t Sector) //定义每个区的大小
{
  uint32_t sectorsize = 0x00;

  if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3))
  {
    sectorsize = 16 * 1024;
  }
  else if(Sector == FLASH_SECTOR_4)
  {
    sectorsize = 64 * 1024;
  }
  else
  {
    sectorsize = 128 * 1024;
  }
  return sectorsize;
}


static uint32_t GetSector(uint32_t Address) //通过Address分区
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_7) */
  {
    sector = FLASH_SECTOR_7;
  }

  return sector;
}

HAL_StatusTypeDef FlashDrv_Write(uint32_t Addr, uint8_t *source, uint32_t length)//FLASH写数据
{
    uint32_t FirstSector = 0, NbOfSectors = 0;
    HAL_StatusTypeDef FlashStatus = HAL_OK;
    uint32_t  SECTORError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    int trycnt = 0;
    int i;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Get the bank */
    FirstSector = GetSector(Addr);   //定义flash起始区
    NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;//确定现在是哪个区

    /* Fill EraseInit structure  定义擦除FLASH参数*/
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector        = FirstSector;
    EraseInitStruct.NbSectors     = NbOfSectors;

    do
    {
        FlashStatus = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);//擦除
        if( FlashStatus != HAL_OK)//判断是否擦除成功
        {
            /* Infinite loop */
            trycnt++;
            if( trycnt > 3 )
            {
                 HAL_FLASH_Lock();
                return FlashStatus;
            }
        }
        else
            break;
        FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
    }while( trycnt < 3);//定义最大尝试次数，擦除
      //内存管理
    __HAL_FLASH_DATA_CACHE_DISABLE();
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

    __HAL_FLASH_DATA_CACHE_RESET();
    __HAL_FLASH_INSTRUCTION_CACHE_RESET();

    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
    __HAL_FLASH_DATA_CACHE_ENABLE();

    FlashStatus = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE); // 等待上一次擦除完成

    if( FlashStatus == HAL_OK ) //数据写入
    {
        /* Program the user Flash area word by word
        (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
        for ( i = 0; i < length; i += 4) //每次写入4KB
        {
           trycnt = 0;
           do{
                FlashStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Addr + i, *(uint32_t *)(source + i));
           }while( trycnt < 3 && FlashStatus != HAL_OK);
        }
    }
    /* Lock the Flash to disable the flash control register access (recommended
    to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    return  HAL_OK;
}


uint32_t FlashDrv_read(uint32_t Addr, uint8_t *source, uint32_t Length)
{
    int i;

    for ( i = 0; i < Length; i++)
    {
        source[i] =  *(__IO uint8_t *)(Addr + i);// 从Flash地址(Addr + i)读取1字节数据到目标缓冲区
    }

    return  1;
}

//// 写入 StateMachine 结构体到 Flash
//void Flash_WriteStateMachine(StateMachine *sm) {
//    uint8_t buffer[sizeof(StateMachine)];
//    memcpy(buffer, sm, sizeof(StateMachine));
//    FlashDrv_Write(ADDR_FLASH_SECTOR_6, buffer, sizeof(StateMachine));
//}
//
//// 从 Flash 读取 StateMachine 结构体
//void Flash_ReadStateMachine(StateMachine *sm) {
//    uint8_t buffer[sizeof(StateMachine)];//创建与结构体大小相同的缓冲区
//    FlashDrv_read(ADDR_FLASH_SECTOR_6, buffer, sizeof(StateMachine));// 从Flash扇区6读取数据到缓冲区
//    memcpy(sm, buffer, sizeof(StateMachine));// 将缓冲区数据复制回结构体
//}

#define  LEN   2048 //定义了缓冲区的长度
uint8_t buff[LEN];//定义源数据缓冲区
uint8_t rbuff[LEN];//读取数据缓冲区
void debug_testflash( void )
{
    int i;

    for ( i = 0; i < LEN; i++)
    {
       buff[i] = i;
    }

   FlashDrv_Write(ADDR_FLASH_SECTOR_6, buff , LEN );
   FlashDrv_read(ADDR_FLASH_SECTOR_6,rbuff , LEN);

    for ( i = 0; i < LEN; i++)
    {
      if(buff[i] != rbuff[i])
      {
         return;
      }
    }

    ;
}

void debug_testflash_struct( void )
{
    StateMachine sm;
    StateMachine_Init(&sm);

    // 设置一些测试数据
    sm.currentState = OPEN;
    sm.counter = 1234;
    sm.current = 12.34f;
    sm.v_out = 5.67f;
    sm.v_in = 3.21f;

    // 写入到 Flash
    Flash_WriteStateMachine(&sm);

    // 清空结构体
    memset(&sm, 0, sizeof(StateMachine));

    // 从 Flash 读取
    Flash_ReadStateMachine(&sm);

    // 验证读取的数据
    if (sm.currentState == OPEN && sm.counter == 1234 && sm.current == 12.34f && sm.v_out == 5.67f && sm.v_in == 3.21f) {
        ;
    } else {
        ;
    }
}


/* End of this file */
