#include "lm75bd.h"


void SCL_GPIO_Init(void)   //将SCL引脚配置为推挽输出
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能B端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	  //初始化
 	GPIO_SetBits(GPIOB,GPIO_Pin_6);
}


void SDA_GPIO_Out_Init(void)  //将SDA引脚配置为推挽输出
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能B端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	  //初始化
 	GPIO_SetBits(GPIOB,GPIO_Pin_7);
}

void SDA_GPIO_Read_Init(void)  //将SDA引脚配置为 浮空输入
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能B端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	  //初始化
 	GPIO_SetBits(GPIOB,GPIO_Pin_7);
}

void LM75BD_IIC_Init(void)  //初始化IIC接口
{
  SCL_GPIO_Init();
  SDA_GPIO_Out_Init();
  //SDA_GPIO_Read_Init();
}


void IIC_Start(void)
{
	SDA_GPIO_Out_Init();  //SDA引脚配置为推挽输出模式
	
    SDA_Set();  //置1
	SCL_Set();  //置1
	delay_us(6);
	SDA_Clr();  //置0
	delay_us(6);
	SCL_Clr();  //置0
}


void IIC_Stop(void)
{
	SDA_GPIO_Out_Init();  //SDA引脚配置为推挽输出模式
	
	SDA_Clr();  //置0
	SDA_Set();  //置1
	delay_us(6);  //延时时间自定义
	SDA_Set();  //置1
	delay_us(6);
}



//-------------------------------主机通过IIC发送应答----------------------------//
void IIC_SendAck(unsigned char ackbit)   
{
	SDA_GPIO_Out_Init();   //SDA引脚配置为推挽输出模式
	
	SCL_Clr();  //置0    //在SCL为0时改变SDA
	if(ackbit ==1)   // 0：应答，1：非应答
		SDA_Set();  //置1
	else if(ackbit == 0) 	
		SDA_Clr();  //置0
  
	delay_us(6);
	
	SCL_Set();  //置1 
	delay_us(6);
	SCL_Clr();  //置0
	SDA_Set();  //置1
	delay_us(6);
}


//-----------------------------IIC等待从机应答-----------------------------//
unsigned char IIC_WaitAck(void) 
{
   unsigned char ackbit = 0;
	
   SDA_GPIO_Read_Init(); //将SDA引脚配置为 浮空输入
	
   SCL_Set();  //置1  
   delay_us(6);
   ackbit = SDA_Read();
   SCL_Clr();  //置0
   delay_us(6);
	
   return ackbit;
}


//-------------------------通过I2C总线发送数据------------------------//
void IIC_SendByte(unsigned char byt)
{
    unsigned char i;

    SDA_GPIO_Out_Init();  //SDA引脚配置为推挽输出模式
	
    for(i=0; i<8; i++)
    {
	   SCL_Clr();  //置0           //在SCL为0时改变SDA
	   delay_us(6);
       if(byt & 0x80) 	SDA_Set();  //置1
       else 	SDA_Clr();  //置0
       delay_us(6);
	   SCL_Set();  //置1  
       byt <<= 1;
       delay_us(6);
    }
	  SCL_Clr();  //置0 
}



//-----------------------从I2C总线上接收数据-----------------------------//
unsigned char IIC_RecByte(void)
{
    unsigned char i, da = 0;
	
	SDA_GPIO_Read_Init(); //将SDA引脚配置为 浮空输入
	
    for(i=0; i<8; i++)
    {   
	    SCL_Set();  //置1          //高电平时进行数据读取
        delay_us(6);
	    da <<= 1;
    	if(SDA_Read()) da |= 1;
	    SCL_Clr();  //置0
	    delay_us(6);
    }
    return da;    
}


// unsigned int LM75BD_ReadTemp(void)
// {
// 	unsigned char High_temp = 0;
// 	unsigned char Low_temp  = 0;	
// 	unsigned int temp = 0;
	
// 	unsigned char ack = 0;  //应答信号检查
	
// 	IIC_Start();  //总线开启
	
// 	IIC_SendByte(0x90);  //发送写命令+设备地址  写操作
// 	ack = IIC_WaitAck(); //等待从机发送应答
// 	if(ack == 1)  //从机未产生应答
// 	{
// 		// OLED_ShowString(5,30,"ERROR 1 ack",12,1);  //OLED显示错误
// 		ack = 0;
// 	}
	
//     IIC_SendByte(0x00);     //发送寄存器地址  温度寄存器的地址 0x00
// 	ack = IIC_WaitAck();
// 	if(ack == 1) 
// 	{
// 		// OLED_ShowString(5,40,"ERROR 2 ack",12,1);
// 		ack = 0;
// 	}
	
//     IIC_Start();  //再次开启总线 改变传输方向
	
// 	IIC_SendByte(0x91);  //地址字节 读操作
// 	ack = IIC_WaitAck();
// 	if(ack == 1) 
// 	{
// 		// OLED_ShowString(5,50,"ERROR 3 ack",12,1);
// 		ack = 0;
// 	}
	
// 	High_temp = IIC_RecByte();   //接收高字节
// 	IIC_SendAck(0);   //还未接收完，接收设备主机产生应答 
// 	Low_temp  = IIC_RecByte();  //接收低字节
// 	IIC_SendAck(1);      //已经接收完，接收设备主机产生非应答 
//     IIC_Stop();  //总线停止
	
// 	temp = ((High_temp<<8) | Low_temp);
	
// 	return (temp>>5);  //低五位 为无效数据，由数据手册可知
// }




float LM75BD_ReadTemp(void)
{
	u8 High_temp = 0;
	u8 Low_temp  = 0;	
	u16 tempData = 0;
    float temp = 0;
	
	u8 ack = 0;  //应答信号检查
	
	IIC_Start();  //总线开启
	
	IIC_SendByte(0x90);  //发送写命令+设备地址  写操作
	ack = IIC_WaitAck(); //等待从机发送应答
	if(ack == 1)  //从机未产生应答
	{
		ack = 0;
	}
	
    IIC_SendByte(0x00);     //发送寄存器地址  温度寄存器的地址 0x00
	ack = IIC_WaitAck();
	if(ack == 1) 
	{
		ack = 0;
	}
	
    IIC_Start();  //再次开启总线 改变传输方向
	
	IIC_SendByte(0x91);  //地址字节 读操作
	ack = IIC_WaitAck();
	if(ack == 1) 
	{
		ack = 0;
	}
	
	High_temp = IIC_RecByte();   //接收高字节
	IIC_SendAck(0);   //还未接收完，接收设备主机产生应答 
	Low_temp  = IIC_RecByte();  //接收低字节
	IIC_SendAck(1);      //已经接收完，接收设备主机产生非应答 
    IIC_Stop();  //总线停止
	
	tempData = ((High_temp<<8) | Low_temp);

    if(tempData & 0x8000 == 0x8000) // 负数
        temp = (float)(tempData&0x7FFF >> 5)*(-0.125);    //低五位 为无效数据，由数据手册可知
    else
        temp = (float)(tempData>>5)*0.125;
	
    return temp;
}


