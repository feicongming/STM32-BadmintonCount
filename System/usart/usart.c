#include "usart.h"
__IO uint8_t CocheData[64];            //��ʱ���ݻ���
__IO uint8_t count=0;                  //���ռ���
__IO uint8_t TimeLag = 0;              //����֡�ж�ʱ��
__IO uint8_t FLAG_USART1_IT=0;         //����1�жϱ�־ ��ʾ���յ����� 0��ʾδ�жϣ�1��ʾ�����ж�
__IO uint8_t FLAG_FrameData = 0;       //������ʾһ֡���ݽ������
__IO uint8_t FLAG_FlowData = 0;        //������ģʽ��ʶ
__IO uint8_t FLAG_StartFlowData;       //������ģʽ��ʼ/���������ʶ
__IO uint8_t FLAG_FLASH_WRITE=0;       //flashд���ʶ 1��ʾ����д��һ���ֽڣ���flashд�뺯�����㴮���ж���λ


/*����1��ʼ������*/
void USART_Config(void) {                                               
	GPIO_InitTypeDef GPIOA_InitStructure;                          //����GPIOA��ʼ���ṹ�����
	USART_InitTypeDef USART_InitStructure;                         //���崮�ڳ�ʼ���ṹ�����
	NVIC_InitTypeDef NVIC_InitStructure;                           //�����жϳ�ʼ���ṹ�����

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//ENABLE THE GPIOA    ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//ENABLE USART1      ʹ�ܴ���1ʱ��

	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_9;               //����GPIOA Pin9���� ���ڷ�������
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;         //����ģʽ  �����������
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //����Ƶ��50MHz
	GPIO_Init(GPIOA, &GPIOA_InitStructure);                  //��ʼ��GPIOA

	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_10;             //����GPIOA Pin10���� ���ڽ�������
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //����ģʽ  ��������
	GPIO_Init(GPIOA, &GPIOA_InitStructure);                 //��ʼ��GPIO

	USART_InitStructure.USART_BaudRate = 115200;                          //���ô���1�Ĳ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;           //�������ݳ���
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                //����ֹͣλΪ1λ
	USART_InitStructure.USART_Parity = USART_Parity_No;                   //������żУ��Ϊ��У��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;       //���ý��պʹ���ģʽ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //����Ӳ����ģʽ
	USART_Init(USART1, &USART_InitStructure);                     //��ʼ������1
	USART_Cmd(USART1, ENABLE);                                   //ʹ�ܴ���1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);               //ʹ�ܴ���1�ж�
	USART_ClearFlag(USART1, USART_IT_RXNE);                      //������ջ���ǿ�

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;               //ָ������1���ж�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //ʹ�ܴ���1�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       //��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              //�ж����ȼ�
	NVIC_Init(&NVIC_InitStructure);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);                 //�ж����ȼ�����
}
 
/*���͵����ֽ�*/
void USART1_SendChar(uint8_t dat){
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET){  //�жϷ��ͻ������Ƿ�Ϊ�� TXE�Ƿ��ͻ�������ձ�־
		}
	      USART_SendData(USART1,dat);
}

/*���Ͷ���ֽ�*/
void USART1_SendMulti(uint8_t *dat,uint8_t len){
  uint8_t i;
	for(i=0;i<len;i++){           //���͸���
	    USART1_SendChar(*dat++);
	}
}

/*�����ַ���*/
void USART1_SendString(uint8_t *dat) {
	while (*dat != '\0') {       //����������ֹͣ����
		USART1_SendChar(*dat++);
	}
}

/*����1�жϺ���*/
void USART1_IRQHandler(void) {                                                                   
	/*����1ָ��ģʽ*/
	if (USART_GetFlagStatus(USART1, USART_IT_RXNE)!=RESET) {    //�жϽ��ջ������Ƿ�ǿ�
		CocheData[count] = USART_ReceiveData(USART1);           //�ѽ��յ���������ʱ�洢�����ݻ�������
		FLAG_USART1_IT = 1;                                     //��λ�жϱ�־

		/*����1������ģʽ*/
		if (FLAG_FlowData) {
			FLAG_StartFlowData = 1;                             //��λ������ģʽ��ʼ�����ʶ,��ʾ��ʼ��������ڴ�����
			}
		}
		count++;                                                //ͳ�ƽ��յĸ���
	}


/*�ж��Ƿ������һ֡���� ���ж�������ģʽ�Ƿ����*/
void Judge_ETB(void) {                 
	if (FLAG_USART1_IT) {                //�жϴ����Ƿ����ж�
		FLAG_USART1_IT = 0;              //�����жϣ�����жϱ�־
		TimeLag = 200;                    //�����ж�����֡����ʱ��
	}
	else if (TimeLag>0) {                //û�з����жϿ�ʼ��ʱ30ms
		TimeLag--;                 
		if (TimeLag==0){                 //30ms�󴮿�û�з����жϣ�
			   
			/*����1������ģʽ*/
			if (FLAG_FlowData) {
				FLAG_StartFlowData = 0;        //���������ģʽ��ʼ/���������ʶ�����ߺ���ĳ���Ҫ������ģʽ�Ѿ�������
				return;                        //������ģʽ����Ҫ��λ����֡��־
			}

			FLAG_FrameData = 1;                //��λ����֡��־�����ߺ���ĳ���Ҫ��ʼ������
		}
	}
	
}


