#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"
#include "spi.h"
#include "dma.h"
#include "flash.h"

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u16 color1[1],t=1;
	u32 num,num1;
	color1[0]=color;
	num=(xend-xsta)*(yend-ysta);
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	LCD_CS_Clr();
	SPI1->CR1|=1<<11;//设置SPI16位传输模式
	SPI_Cmd(SPI1, ENABLE);//使能SPI
	while(t)
	{
		if(num>65534)
		{
			num-=65534;
			num1=65534;
		}
		else
		{
			t=0;
			num1=num;
		}
		MYDMA_Config1(DMA1_Channel3,(u32)&SPI1->DR,(u32)color1,num1);
		SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
		MYDMA_Enable(DMA1_Channel3);
		while(1)
		{
			if(DMA_GetFlagStatus(DMA1_FLAG_TC3)!=RESET)//等待通道4传输完成
			{
				DMA_ClearFlag(DMA1_FLAG_TC3);//清除通道3传输完成标志
				break;
			}
		}
  }
	LCD_CS_Set();
	SPI1->CR1=~SPI1->CR1;
	SPI1->CR1|=1<<11;
	SPI1->CR1=~SPI1->CR1;//设置SPI8位传输模式
	SPI_Cmd(SPI1, ENABLE);//使能SPI
}

/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置 
	LCD_WR_DATA(color);
} 


/******************************************************************************
      函数说明：画线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


/******************************************************************************
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}


/******************************************************************************
      函数说明：画圆
      入口数据：x0,y0   圆心坐标
                r       半径
                color   圆的颜色
      返回值：  无
******************************************************************************/
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}



/******************************************************************************
      函数说明：返回GBK字模
      入口数据：*pBuffer 数组
                c 要显示的汉字
				sizey 字号   仅支持24,32
      返回值：  无
******************************************************************************/
void GetGB2312Code(u8 *pBuffer, u8 *s, u8 sizey)
{
	u8 High8bit, Low8bit, ByteOfChar;
	u32 pos;

	ByteOfChar = sizey*sizey/8;		// 一个汉字所占的字节数		
	High8bit= *s;     /* 取高8位数据 */
	Low8bit= *(s+1);  /* 取低8位数据 */

	Low8bit-=0xA1;
	High8bit-=0xA1;

	pos = ((u32)94*High8bit+Low8bit)*ByteOfChar;   // 汉字和GBK起始位置的偏移量

	if(sizey == 16)
		SPI_FLASH_BufferRead(pBuffer, GB2312_16_START_ADDRESS+pos, ByteOfChar);
	if(sizey == 24)
		SPI_FLASH_BufferRead(pBuffer, GB2312_24_START_ADDRESS+pos, ByteOfChar);
	else if(sizey == 32)
		SPI_FLASH_BufferRead(pBuffer, GB2312_32_START_ADDRESS+pos, ByteOfChar);
	else return;
	
}


/******************************************************************************
      函数说明：显示单个16x16汉字
      入口数据：x,y显示坐标
                CNChar 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 mode)
{
	u8 pBuffer[32];
	u8 i,j,m=0;
	u16 x0=x;

	GetGB2312Code(pBuffer, s, 16);
	LCD_Address_Set(x,y,x+16-1,y+16-1);
	for(i=0;i<32;i++)                 // 72为24*24字模所占字节数
	{
		for(j=0;j<8;j++)
		{	
			if(!mode)//非叠加方式
			{
				if(pBuffer[i]&(0x01<<j))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%16==0)
				{
					m=0;
					break;
				}
			}
			else//叠加方式
			{
				if(pBuffer[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==16)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}
}



/******************************************************************************
      函数说明：显示单个24x24汉字
      入口数据：x,y显示坐标
                CNChar 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 mode)
{
	u8 pBuffer[72];
	u8 i,j,m=0;
	u16 x0=x;

	GetGB2312Code(pBuffer, s, 24);
	LCD_Address_Set(x,y,x+24-1,y+24-1);
	for(i=0;i<72;i++)                 // 72为24*24字模所占字节数
	{
		for(j=0;j<8;j++)
		{	
			if(!mode)//非叠加方式
			{
				if(pBuffer[i]&(0x01<<j))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%24==0)
				{
					m=0;
					break;
				}
			}
			else//叠加方式
			{
				if(pBuffer[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==24)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}
}

/******************************************************************************
      函数说明：显示单个32x32汉字
      入口数据：x,y显示坐标
                CNChar 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 mode)
{
	u8 pBuffer[128];
	u8 i,j,m=0;
	u16 x0=x;

	GetGB2312Code(pBuffer, s, 32);
	LCD_Address_Set(x,y,x+32-1,y+32-1);
	for(i=0;i<128;i++)                 // 128为32*32字模所占字节数
	{
		for(j=0;j<8;j++)
		{	
			if(!mode)//非叠加方式
			{
				if(pBuffer[i]&(0x01<<j))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%32==0)
				{
					m=0;
					break;
				}
			}
			else//叠加方式
			{
				if(pBuffer[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==32)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}
}



/******************************************************************************
      函数说明：显示单个字符
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号  仅支持24，32
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 temp,sizex,t,m=0;
	u16 i,TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==16)temp=ascii_168[num][i];		       //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		   //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		   //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}


/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号 仅支持24，32
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}


/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp;
	u8 enshow=0;
	u8 sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 


/******************************************************************************
      函数说明：显示两位小数变量
      入口数据：x,y显示坐标
                num 要显示小数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp,sizex;
	u16 num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}


/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
{
	u8 t=1;
	u32 num=length*width*2,num1;
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	LCD_CS_Clr();
	while(t)
	{
	  if(num>65534)
		{
			num-=65534;
			num1=65534;
		}
		else
		{
			t=0;
			num1=num;
		}
		MYDMA_Config(DMA1_Channel3,(u32)&SPI1->DR,(u32)pic,num1);
		SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
		MYDMA_Enable(DMA1_Channel3);
		while(1)
		{
			if(DMA_GetFlagStatus(DMA1_FLAG_TC3)!=RESET)//等待通道4传输完成
			{
				DMA_ClearFlag(DMA1_FLAG_TC3);//清除通道4传输完成标志
				break; 
			}
		}
		pic+=65534;
	}
	LCD_CS_Set();
}



void LCD_ShowFlashPicture(u16 x,u16 y,u16 length,u16 width, u8 flashBuff[], u32 picAddr)
{
	u8 t=1;
	u32 num=length*width*2,num1;
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	SPI_FLASH_BufferRead(flashBuff, picAddr, 8192);
	LCD_CS_Clr();
	while(t)
	{
	  if(num>8192)
		{
			num-=8192;
			num1=8192;
		}
		else
		{
			t=0;
			num1=num;
		}
		MYDMA_Config(DMA1_Channel3,(u32)&SPI1->DR,(u32)flashBuff,num1);
		SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
		MYDMA_Enable(DMA1_Channel3);
		while(1)
		{
			if(DMA_GetFlagStatus(DMA1_FLAG_TC3)!=RESET)//等待通道4传输完成
			{
				DMA_ClearFlag(DMA1_FLAG_TC3);//清除通道4传输完成标志
				break; 
			}
		}
		picAddr+=8192;
		SPI_FLASH_BufferRead(flashBuff, picAddr, 8192);
	}
	LCD_CS_Set();
}




/******************************************************************************
      函数说明：显示中英文字符串
      入口数据：x,y显示坐标
                *s 要显示的汉字串
                fc 字的颜色
                bc 字的背景色
                sizey 字号 可选 16 24 32
                mode:  0非叠加模式  1叠加模式
      返回值：  写入的字节数
******************************************************************************/
u16 LCD_ShowCN_EN(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 *Temp = s;
	while(*s!='\0' && *s != 0xFF)
	{
		// if(y>=LCD_H-sizey)
		// {
		// 	break;
		// }
		if(*s == 0x0D)     // 回车符
		{
			y+=sizey+5;
			x=0;
			s+=2;

			if(y>=LCD_H-sizey)
			{
				break;
			}
		}
		else if(*s <= 126)     // 英文字符
		{
			if(x+sizey/2 > LCD_W)
			{
				y+=sizey+5;
				x=0;
			}

			if(y>=LCD_H-sizey)
			{
				break;
			}

			LCD_ShowChar(x, y, *s, fc, bc, sizey, mode);
			x+=sizey/2;
			s++;
		}
		else
		{
			if(x+sizey > LCD_W)
			{
				y+=sizey+5;
				x=0;
			}

			if(y>=LCD_H-sizey)
			{
				break;
			}

			if(sizey == 16) LCD_ShowChinese16x16(x, y, s, fc, bc, mode);
			else if(sizey == 24) LCD_ShowChinese24x24(x, y, s, fc, bc, mode);
			else LCD_ShowChinese32x32(x, y, s, fc, bc, mode);
			x+=sizey;
			s+=2;
		}

		// if(y>=LCD_H-sizey)
		// {
		// 	break;
		// }
	}
	return (u16)(s-Temp);
}


// u8 iconBuff[2048];
/******************************************************************************
      函数说明：显示小图标
      入口数据：x,y起点坐标
	  		   index  小图标索引号		 1.github  2.错误  3.二维码  4.码  5.秒表
			   							6.闪光灯  7.书签  8.图像文件  9.温度计
										10.文件  11.文件夹  12.信息  13.游戏
										14.羽毛球  15.照片栈  16.向左箭头 17.播放
										18.暂停  19.停止盘旋  20.解锁  21.漏斗
										22.小灯泡  23.星星  24.铃铛
				iconBuff  图片数组 应为2048字节
      返回值：  无
******************************************************************************/
void LCD_ShowIcon(u16 x,u16 y,u8 index, u8 iconBuff[])
{
	u16 pos=(index-1)*2048;
	SPI_FLASH_BufferRead(iconBuff, ICON_START_ADDRESS+pos, 2048);
	LCD_ShowPicture(x, y, 32, 32, iconBuff);
}



/******************************************************************************
      函数说明：显示LED数码管
      入口数据：x,y起点坐标
	  		   num  整数变量，小于10  
			   iconBuff  图片数组 应为576字节   96*48/8
      返回值：  无
******************************************************************************/
void LCD_ShowLED(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 ledBuff[])
{
	u8 t,m=0;
	u16 i;

	SPI_FLASH_BufferRead(ledBuff, LED_0_8_START_ADDRESS+num*576, 576);
	LCD_Address_Set(x,y,x+48-1,y+96-1);  //设置光标位置 
	for(i=0;i<576;i++)
	{ 
		for(t=0;t<8;t++)
		{
			if(ledBuff[i]&(0x01<<t))LCD_WR_DATA(fc);
			else LCD_WR_DATA(bc);
			m++;
			if(m%48==0)
			{
				m=0;
				break;
			}
		}
	}
}

