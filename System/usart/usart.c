#include "usart.h"
__IO uint8_t CocheData[64];            //临时数据缓存
__IO uint8_t count=0;                  //接收计数
__IO uint8_t TimeLag = 0;              //数据帧判断时间
__IO uint8_t FLAG_USART1_IT=0;         //串口1中断标志 表示接收到数据 0表示未中断，1表示发生中断
__IO uint8_t FLAG_FrameData = 0;       //用来表示一帧数据接收完成
__IO uint8_t FLAG_FlowData = 0;        //数据流模式标识
__IO uint8_t FLAG_StartFlowData;       //数据流模式开始/结束传输标识
__IO uint8_t FLAG_FLASH_WRITE=0;       //flash写入标识 1表示可以写入一个字节，由flash写入函数清零串口中断置位


/*串口1初始化函数*/
void USART_Config(void) {                                               
	GPIO_InitTypeDef GPIOA_InitStructure;                          //定义GPIOA初始化结构体变量
	USART_InitTypeDef USART_InitStructure;                         //定义串口初始化结构体变量
	NVIC_InitTypeDef NVIC_InitStructure;                           //定义中断初始化结构体变量

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//ENABLE THE GPIOA    使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//ENABLE USART1      使能串口1时钟

	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_9;               //启用GPIOA Pin9引脚 串口发送引脚
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;         //工作模式  复用推挽输出
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //工作频率50MHz
	GPIO_Init(GPIOA, &GPIOA_InitStructure);                  //初始化GPIOA

	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_10;             //启用GPIOA Pin10引脚 串口接收引脚
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //工作模式  悬空输入
	GPIO_Init(GPIOA, &GPIOA_InitStructure);                 //初始化GPIO

	USART_InitStructure.USART_BaudRate = 115200;                          //设置串口1的波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;           //设置数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                //设置停止位为1位
	USART_InitStructure.USART_Parity = USART_Parity_No;                   //设置奇偶校验为无校验
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;       //启用接收和传输模式
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //设置硬件流模式
	USART_Init(USART1, &USART_InitStructure);                     //初始化串口1
	USART_Cmd(USART1, ENABLE);                                   //使能串口1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);               //使能串口1中断
	USART_ClearFlag(USART1, USART_IT_RXNE);                      //清除接收缓存非空

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;               //指定串口1的中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //使能串口1中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              //中断优先级
	NVIC_Init(&NVIC_InitStructure);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);                 //中断优先级分组
}
 
/*发送单个字节*/
void USART1_SendChar(uint8_t dat){
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET){  //判断发送缓存区是否为空 TXE是发送缓存区清空标志
		}
	      USART_SendData(USART1,dat);
}

/*发送多个字节*/
void USART1_SendMulti(uint8_t *dat,uint8_t len){
  uint8_t i;
	for(i=0;i<len;i++){           //发送个数
	    USART1_SendChar(*dat++);
	}
}

/*发送字符串*/
void USART1_SendString(uint8_t *dat) {
	while (*dat != '\0') {       //遇到结束符停止发送
		USART1_SendChar(*dat++);
	}
}

/*串口1中断函数*/
void USART1_IRQHandler(void) {                                                                   
	/*串口1指令模式*/
	if (USART_GetFlagStatus(USART1, USART_IT_RXNE)!=RESET) {    //判断接收缓冲区是否非空
		CocheData[count] = USART_ReceiveData(USART1);           //把接收到的数据暂时存储到数据缓存数据
		FLAG_USART1_IT = 1;                                     //置位中断标志

		/*串口1数据流模式*/
		if (FLAG_FlowData) {
			FLAG_StartFlowData = 1;                             //置位数据流模式开始传输标识,表示开始传输或是在传输中
			}
		}
		count++;                                                //统计接收的个数
	}


/*判断是否接收完一帧数据 和判断数据流模式是否结束*/
void Judge_ETB(void) {                 
	if (FLAG_USART1_IT) {                //判断串口是否发生中断
		FLAG_USART1_IT = 0;              //发生中断，清除中断标志
		TimeLag = 200;                    //重载判断数据帧结束时间
	}
	else if (TimeLag>0) {                //没有发生中断开始计时30ms
		TimeLag--;                 
		if (TimeLag==0){                 //30ms后串口没有发生中断，
			   
			/*串口1数据流模式*/
			if (FLAG_FlowData) {
				FLAG_StartFlowData = 0;        //清除数据流模式开始/结束传输标识，告诉后面的程序要数据流模式已经结束了
				return;                        //数据流模式不需要置位数据帧标志
			}

			FLAG_FrameData = 1;                //置位数据帧标志，告诉后面的程序要开始处理了
		}
	}
	
}


