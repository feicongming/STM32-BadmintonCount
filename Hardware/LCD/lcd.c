#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"
#include "spi.h"
#include "dma.h"
#include "flash.h"

/******************************************************************************
      ����˵������ָ�����������ɫ
      ������ݣ�xsta,ysta   ��ʼ����
                xend,yend   ��ֹ����
								color       Ҫ������ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u16 color1[1],t=1;
	u32 num,num1;
	color1[0]=color;
	num=(xend-xsta)*(yend-ysta);
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//������ʾ��Χ
	LCD_CS_Clr();
	SPI1->CR1|=1<<11;//����SPI16λ����ģʽ
	SPI_Cmd(SPI1, ENABLE);//ʹ��SPI
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
			if(DMA_GetFlagStatus(DMA1_FLAG_TC3)!=RESET)//�ȴ�ͨ��4�������
			{
				DMA_ClearFlag(DMA1_FLAG_TC3);//���ͨ��3������ɱ�־
				break;
			}
		}
  }
	LCD_CS_Set();
	SPI1->CR1=~SPI1->CR1;
	SPI1->CR1|=1<<11;
	SPI1->CR1=~SPI1->CR1;//����SPI8λ����ģʽ
	SPI_Cmd(SPI1, ENABLE);//ʹ��SPI
}

/******************************************************************************
      ����˵������ָ��λ�û���
      ������ݣ�x,y ��������
                color �����ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//���ù��λ�� 
	LCD_WR_DATA(color);
} 


/******************************************************************************
      ����˵��������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   �ߵ���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1;
	uRow=x1;//�����������
	uCol=y1;
	if(delta_x>0)incx=1; //���õ������� 
	else if (delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//ˮƽ�� 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//����
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
      ����˵����������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   ���ε���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}


/******************************************************************************
      ����˵������Բ
      ������ݣ�x0,y0   Բ������
                r       �뾶
                color   Բ����ɫ
      ����ֵ��  ��
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
		if((a*a+b*b)>(r*r))//�ж�Ҫ���ĵ��Ƿ��Զ
		{
			b--;
		}
	}
}



/******************************************************************************
      ����˵��������GBK��ģ
      ������ݣ�*pBuffer ����
                c Ҫ��ʾ�ĺ���
				sizey �ֺ�   ��֧��24,32
      ����ֵ��  ��
******************************************************************************/
void GetGB2312Code(u8 *pBuffer, u8 *s, u8 sizey)
{
	u8 High8bit, Low8bit, ByteOfChar;
	u32 pos;

	ByteOfChar = sizey*sizey/8;		// һ��������ռ���ֽ���		
	High8bit= *s;     /* ȡ��8λ���� */
	Low8bit= *(s+1);  /* ȡ��8λ���� */

	Low8bit-=0xA1;
	High8bit-=0xA1;

	pos = ((u32)94*High8bit+Low8bit)*ByteOfChar;   // ���ֺ�GBK��ʼλ�õ�ƫ����

	if(sizey == 16)
		SPI_FLASH_BufferRead(pBuffer, GB2312_16_START_ADDRESS+pos, ByteOfChar);
	if(sizey == 24)
		SPI_FLASH_BufferRead(pBuffer, GB2312_24_START_ADDRESS+pos, ByteOfChar);
	else if(sizey == 32)
		SPI_FLASH_BufferRead(pBuffer, GB2312_32_START_ADDRESS+pos, ByteOfChar);
	else return;
	
}


/******************************************************************************
      ����˵������ʾ����16x16����
      ������ݣ�x,y��ʾ����
                CNChar Ҫ��ʾ�ĺ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 mode)
{
	u8 pBuffer[32];
	u8 i,j,m=0;
	u16 x0=x;

	GetGB2312Code(pBuffer, s, 16);
	LCD_Address_Set(x,y,x+16-1,y+16-1);
	for(i=0;i<32;i++)                 // 72Ϊ24*24��ģ��ռ�ֽ���
	{
		for(j=0;j<8;j++)
		{	
			if(!mode)//�ǵ��ӷ�ʽ
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
			else//���ӷ�ʽ
			{
				if(pBuffer[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
      ����˵������ʾ����24x24����
      ������ݣ�x,y��ʾ����
                CNChar Ҫ��ʾ�ĺ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 mode)
{
	u8 pBuffer[72];
	u8 i,j,m=0;
	u16 x0=x;

	GetGB2312Code(pBuffer, s, 24);
	LCD_Address_Set(x,y,x+24-1,y+24-1);
	for(i=0;i<72;i++)                 // 72Ϊ24*24��ģ��ռ�ֽ���
	{
		for(j=0;j<8;j++)
		{	
			if(!mode)//�ǵ��ӷ�ʽ
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
			else//���ӷ�ʽ
			{
				if(pBuffer[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
      ����˵������ʾ����32x32����
      ������ݣ�x,y��ʾ����
                CNChar Ҫ��ʾ�ĺ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 mode)
{
	u8 pBuffer[128];
	u8 i,j,m=0;
	u16 x0=x;

	GetGB2312Code(pBuffer, s, 32);
	LCD_Address_Set(x,y,x+32-1,y+32-1);
	for(i=0;i<128;i++)                 // 128Ϊ32*32��ģ��ռ�ֽ���
	{
		for(j=0;j<8;j++)
		{	
			if(!mode)//�ǵ��ӷ�ʽ
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
			else//���ӷ�ʽ
			{
				if(pBuffer[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
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
      ����˵������ʾ�����ַ�
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ���ַ�
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�  ��֧��24��32
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 temp,sizex,t,m=0;
	u16 i,TypefaceNum;//һ���ַ���ռ�ֽڴ�С
	u16 x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //�õ�ƫ�ƺ��ֵ
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //���ù��λ�� 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==16)temp=ascii_168[num][i];		       //����8x16����
		else if(sizey==24)temp=ascii_2412[num][i];		   //����12x24����
		else if(sizey==32)temp=ascii_3216[num][i];		   //����16x32����
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//�ǵ���ģʽ
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
			else//����ģʽ
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//��һ����
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
      ����˵������ʾ�ַ���
      ������ݣ�x,y��ʾ����
                *p Ҫ��ʾ���ַ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ� ��֧��24��32
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
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
      ����˵������ʾ����
      ������ݣ�m������nָ��
      ����ֵ��  ��
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      ����˵������ʾ��������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ��������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
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
      ����˵������ʾ��λС������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾС������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
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
      ����˵������ʾͼƬ
      ������ݣ�x,y�������
                length ͼƬ����
                width  ͼƬ���
                pic[]  ͼƬ����    
      ����ֵ��  ��
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
			if(DMA_GetFlagStatus(DMA1_FLAG_TC3)!=RESET)//�ȴ�ͨ��4�������
			{
				DMA_ClearFlag(DMA1_FLAG_TC3);//���ͨ��4������ɱ�־
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
			if(DMA_GetFlagStatus(DMA1_FLAG_TC3)!=RESET)//�ȴ�ͨ��4�������
			{
				DMA_ClearFlag(DMA1_FLAG_TC3);//���ͨ��4������ɱ�־
				break; 
			}
		}
		picAddr+=8192;
		SPI_FLASH_BufferRead(flashBuff, picAddr, 8192);
	}
	LCD_CS_Set();
}




/******************************************************************************
      ����˵������ʾ��Ӣ���ַ���
      ������ݣ�x,y��ʾ����
                *s Ҫ��ʾ�ĺ��ִ�
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ� ��ѡ 16 24 32
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  д����ֽ���
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
		if(*s == 0x0D)     // �س���
		{
			y+=sizey+5;
			x=0;
			s+=2;

			if(y>=LCD_H-sizey)
			{
				break;
			}
		}
		else if(*s <= 126)     // Ӣ���ַ�
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
      ����˵������ʾСͼ��
      ������ݣ�x,y�������
	  		   index  Сͼ��������		 1.github  2.����  3.��ά��  4.��  5.���
			   							6.�����  7.��ǩ  8.ͼ���ļ�  9.�¶ȼ�
										10.�ļ�  11.�ļ���  12.��Ϣ  13.��Ϸ
										14.��ë��  15.��Ƭջ  16.�����ͷ 17.����
										18.��ͣ  19.ֹͣ����  20.����  21.©��
										22.С����  23.����  24.����
				iconBuff  ͼƬ���� ӦΪ2048�ֽ�
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowIcon(u16 x,u16 y,u8 index, u8 iconBuff[])
{
	u16 pos=(index-1)*2048;
	SPI_FLASH_BufferRead(iconBuff, ICON_START_ADDRESS+pos, 2048);
	LCD_ShowPicture(x, y, 32, 32, iconBuff);
}



/******************************************************************************
      ����˵������ʾLED�����
      ������ݣ�x,y�������
	  		   num  ����������С��10  
			   iconBuff  ͼƬ���� ӦΪ576�ֽ�   96*48/8
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowLED(u16 x, u16 y, u8 num, u16 fc, u16 bc, u8 ledBuff[])
{
	u8 t,m=0;
	u16 i;

	SPI_FLASH_BufferRead(ledBuff, LED_0_8_START_ADDRESS+num*576, 576);
	LCD_Address_Set(x,y,x+48-1,y+96-1);  //���ù��λ�� 
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

