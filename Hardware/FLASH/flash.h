#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32f10x.h"

//#define  sFLASH_ID              0xEF3015   //W25X16
//#define  sFLASH_ID              0xEF4015	 //W25Q16
#define  sFLASH_ID              0XEF4018   //W25Q128
//#define  sFLASH_ID              0XEF4017    //W25Q64

#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256

/*命令定义-开头*******************************/
#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		    0x05 
#define W25X_WriteStatusReg		    0x01 
#define W25X_ReadData			        0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	    0xAB 
#define W25X_DeviceID			        0xAB 
#define W25X_ManufactDeviceID   	0x90 
#define W25X_JedecDeviceID		    0x9F

/* WIP(busy)标志，FLASH内部正在写入 */
#define WIP_Flag                  0x01
#define Dummy_Byte                0xFF
/*命令定义-结尾*******************************/



/*SPI接口定义-开头****************************/
#define      FLASH_SPIx                        SPI2
#define      FLASH_SPI_APBxClock_FUN          RCC_APB1PeriphClockCmd
#define      FLASH_SPI_CLK                     RCC_APB1Periph_SPI2

//CS(NSS)引脚 片选选普通GPIO即可
#define      FLASH_SPI_CS_APBxClock_FUN       RCC_APB2PeriphClockCmd
#define      FLASH_SPI_CS_CLK                  RCC_APB2Periph_GPIOB    
#define      FLASH_SPI_CS_PORT                 GPIOB
#define      FLASH_SPI_CS_PIN                  GPIO_Pin_12

//SCK引脚
#define      FLASH_SPI_SCK_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define      FLASH_SPI_SCK_CLK                 RCC_APB2Periph_GPIOB   
#define      FLASH_SPI_SCK_PORT                GPIOB   
#define      FLASH_SPI_SCK_PIN                 GPIO_Pin_13
//MISO引脚
#define      FLASH_SPI_MISO_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define      FLASH_SPI_MISO_CLK                RCC_APB2Periph_GPIOB    
#define      FLASH_SPI_MISO_PORT               GPIOB 
#define      FLASH_SPI_MISO_PIN                GPIO_Pin_14
//MOSI引脚
#define      FLASH_SPI_MOSI_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define      FLASH_SPI_MOSI_CLK                RCC_APB2Periph_GPIOA    
#define      FLASH_SPI_MOSI_PORT               GPIOB 
#define      FLASH_SPI_MOSI_PIN                GPIO_Pin_15

#define  		SPI_FLASH_CS_LOW()     						GPIO_ResetBits( FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN )
#define  		SPI_FLASH_CS_HIGH()    						GPIO_SetBits( FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN )

/*SPI接口定义-结尾****************************/



/*FLASH文件地址等-开始*********************************************************/

#define GB2312_16_START_ADDRESS         0x000000     // GB2312 16字库开始地址         长度 = 261,698     
#define GB2312_24_START_ADDRESS         0x03FE42     // GB2312 24字库开始地址         长度 = 588,818     588818+0x03FE42=0x0CFA54
#define GB2312_32_START_ADDRESS         0x0CFA54     // GB2312 32字库开始地址         长度 = 1,046,786   0x0CFA54+1046786=0x1CF356
#define LED_0_8_START_ADDRESS           0x1CF356      // LED数码管字库开始地址         长度 = 5,760        0x1CF356+5760=1D09D6
#define ICON_START_ADDRESS              0x1D09D6     // FLASH 小图标                 长度 = 49,152       0x1D09D6+49152=1DC9D6

// 文本开始地址为 0x200000,方便擦除
#define BOOK_CHINESE_START_ADDRESS      0x200000     // 语文书开始地址                长度 = 324,604     
#define BOOK_CHINESE_SIZE               44635

#define BOOK_MATH_START_ADDRESS         0x220000    // 数学书开始地址                长度 = 54,310
#define BOOK_MATH_SIZE                  54310

#define BOOK_ENGLISH_START_ADDRESS      0x240000     // 英语书开始地址                长度 = 78,033
#define BOOK_ENGLISH_SIZE               78033

#define BOOK_ZHENGZHI_START_ADDRESS     0x260000     // 政治书开始地址                长度 = 105,940
#define BOOK_ZHENGZHI_SIZE              105940

#define BOOK_LISHI_START_ADDRESS        0x280000     // 历史书开始地址                长度 = 95,996
#define BOOK_LISHI_SIZE                 95996

#define BOOK_DILI_START_ADDRESS         0x2A0000     // 地理书开始地址                长度 = 53,807
#define BOOK_DILI_SIZE                  53807


// 图片开始地址
#define PICTURE_START_ADDRESS           0x300000      // 长度 = 1,228,320
#define PICTURE_PINK_ADDRESS            0x42C000      // 长度 = 153600
#define PICTURE_GAODA_ADDRESS           0x451800      // 长度 = 153600
#define PICTURE_KAIJI_ADDRESS           0x477000      // 长度 = 153600



// 二维码开始地址
#define QR_BLOG_START_ADDRESS           0x500000      // 长度 = 28800
#define QR_GITHUB_START_ADDRESS         0x507080      // 长度 = 28800



/*FLASH文件地址等-结尾*********************************************************/


void SPI_FLASH_Init(void);
void SPI_FLASH_SectorErase(u32 SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
u32 SPI_FLASH_ReadID(void);
u32 SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_StartReadSequence(u32 ReadAddr);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);


u8 SPI_FLASH_ReadByte(void);
u8 SPI_FLASH_SendByte(u8 byte);
u16 SPI_FLASH_SendHalfWord(u16 HalfWord);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);


#endif /* __SPI_FLASH_H */

