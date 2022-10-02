#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"
#include "lcd_init.h"
#include "lcd.h"
#include "usart.h"
#include "button.h"
#include "flash.h"
#include "led.h"
#include "beep.h"
#include "tim.h"
#include "lm75bd.h"
#include "battery.h"



u8 iconBuff[2048];
u8 ledBuff[576];
u8 buttonEvent=0;
u32 bookPageAddr[500] = {0};
u16 tim2_flag = 0;
u8 BatteryStatus;
u8 pictureBuff[8192];


struct sonMenu {
	u8* sonName; // �Ӳ˵�����
	u8 sonIcon; // �Ӳ˵�ͼ��������
	void (*sonFunc)(void);  // �Ӳ˵��������
};


struct Book {
	u8* name;  // ����
	u32 addr;  // ���ַ
	u32 size;  // ���ֽ���
};


void (*sonFunc)(void);
void badminton(void);
void learn_menu(void);
void timer_1(void);
void temp(void);
void battery(void);
void think(void);
void game(void);
void picture(void);
void beep(void);
void led(void);
void timer_2(void);
void about(void);


const struct sonMenu menuTable[12] = { 
	{"ѧϰ", 7, learn_menu},
	{"��ë��", 14, badminton},
	{"��ʱ��", 5, timer_1},
	{"�¶�", 9, temp},
	{"LED", 22, led},
	{"����", 6, battery},
	{"ͼƬ", 15, picture},
	{"��Ϸ", 13, game},
	{"BEEP", 24, beep},
	{"˼��", 23, think},
	{"��ʱ��", 21, timer_2},
	{"����", 12, about}
};


void led()
{
	u8 ledStatus;
	ledStatus = !GPIO_ReadInputDataBit(LED_GPIO_PORT, LED_GPIO_PIN);  // LED�͵�ƽ��������ȡ��
	LCD_ShowIcon(88, 104, 22, iconBuff);
	LCD_ShowString(120,104, "LED:", WHITE, BLACK, 32, 0);
	if(ledStatus) LCD_ShowString(184,104, "ON ", GREEN, BLACK, 32, 0);
	else LCD_ShowString(184,104, "OFF", RED, BLACK, 32, 0);
	while(1)
	{
		buttonEvent = Return_Button_Event();
		if(buttonEvent == KEY_ENTER_EVENT)
		{
			if(ledStatus)
			{
				LED_OFF;
				LCD_ShowString(184,104, "OFF", RED, BLACK, 32, 0);
				ledStatus = 0;
			}
			else
			{
				LED_ON;
				LCD_ShowString(184,104, "ON ", GREEN, BLACK, 32, 0);
				ledStatus = 1;
			}
		}
		if(buttonEvent == KEY_BACK_EVENT)
		{
			return;
		}
	}
}


void show_menu(u8 index)
{
	u8 i;

	if(index == 1) index=0;
	else if(index == 2) index=6;

	for(i=0; i<6; i++)
	{
		LCD_ShowIcon(10, 4+40*i, menuTable[index].sonIcon, iconBuff);
		LCD_ShowCN_EN(56, 4+40*i, menuTable[index].sonName, WHITE, BLACK, 32, 0);
		index++;
	}

}


struct Book bookList[6] = {
	{"����", BOOK_CHINESE_START_ADDRESS, BOOK_CHINESE_SIZE},
	{"��ѧ", BOOK_MATH_START_ADDRESS, BOOK_MATH_SIZE},
	{"Ӣ��", BOOK_ENGLISH_START_ADDRESS, BOOK_ENGLISH_SIZE},
	{"����", BOOK_ZHENGZHI_START_ADDRESS, BOOK_ZHENGZHI_SIZE},
	{"��ʷ", BOOK_LISHI_START_ADDRESS, BOOK_LISHI_SIZE},
	{"����", BOOK_DILI_START_ADDRESS, BOOK_DILI_SIZE}
};


void learn_menu()
{
	u8 bookStr[528];
	u8 i;
	u8 index = 0;
	u16 bookPage = 0;
	u16 readByte = 0;

	// ��ʾ�˵�
	for(i=0; i<6; i++)
	{
		LCD_ShowIcon(10, 4+40*i, 7, iconBuff);
		LCD_ShowCN_EN(56, 4+40*i, bookList[i].name, WHITE, BLACK, 32, 0);
	}
	LCD_ShowIcon(250, 4, 16, iconBuff);  // ��ͷѡ��

	while(1)
	{
		delay_ms(5);
		buttonEvent = Return_Button_Event();
		if(buttonEvent == ENCODER_UP_EVENT & index>0)  // ������������ת
		{
			index--;
			for(i=0;i<8;i++)    // ��ͷ�ƶ����Զ���
			{
				LCD_ShowIcon(250, (4+40*((index+1)%6)-i*5), 16, iconBuff);
				LCD_DrawLine(250, (4+40*((index+1)%6)-i*5)+32, 283, (4+40*((index+1)%6)-i*4)+32, BLACK);
			}
			LCD_ShowCN_EN(56, 4+40*((index+1)%6), bookList[index+1].name, WHITE, BLACK, 32, 0);
			LCD_ShowCN_EN(56, 4+40*(index%6), bookList[index].name, GRAYBLUE, BLACK, 32, 0);
		}
		if(buttonEvent == ENCODER_DOWN_EVENT & index<5)  // ������������ת
		{
			index++;
			for(i=0;i<8;i++)    // ���Զ���
			{
				LCD_ShowIcon(250, (4+40*((index-1)%6)+i*5), 16, iconBuff);
				LCD_DrawLine(250, (4+40*((index-1)%6)+i*5), 283, (4+40*((index-1)%6)+i*4), BLACK);
			}
			LCD_ShowCN_EN(56, 4+40*((index-1)%6), bookList[index-1].name, WHITE, BLACK, 32, 0);
			LCD_ShowCN_EN(56, 4+40*(index%6), bookList[index].name, GRAYBLUE, BLACK, 32, 0);
		}
		if(buttonEvent == KEY_ENTER_EVENT)                      // ȷ�ϼ�������״̬
		{
			bookPageAddr[bookPage] = bookList[index].addr;
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
			SPI_FLASH_BufferRead(bookStr, bookPageAddr[bookPage], 528); // ��ȡ�ı���Ϣ

			LCD_ShowCN_EN(262, 0, "��   ҳ", WHITE, BLACK, 16, 0);
			LCD_ShowCN_EN(278, 0, "  1", GRAYBLUE, BLACK, 16, 0);

			if(BatteryStatus)
			{
				LCD_DrawRectangle(8, 2, 42, 14, WHITE);  // ��ؿ�
				LCD_Fill(42, 5, 45, 11, WHITE);
				LCD_DrawPoint(44, 10, WHITE);
				LCD_Fill(9, 3, 10+8*BatteryStatus, 14, WHITE);
			}
			else
			{
				LCD_DrawRectangle(8, 2, 42, 14, RED);  // ��ؿ�
				LCD_Fill(42, 5, 45, 11, RED);
				LCD_DrawPoint(44, 10, RED);
			}

			readByte = LCD_ShowCN_EN(0, 21, bookStr, WHITE, BLACK, 16, 0);
			// ����״̬
			while(1)
			{
				buttonEvent = Return_Button_Event();
				if(buttonEvent == ENCODER_UP_EVENT & bookPage>0)
				{
					BatteryStatus = ReturnBatteryStatus();
					bookPage--;
					LCD_ShowIntNum(278, 0, bookPage+1, 3, GRAYBLUE, BLACK, 16);
					LCD_Fill(0, 21, LCD_W, LCD_H, BLACK);
					SPI_FLASH_BufferRead(bookStr, bookPageAddr[bookPage], 528); // ��ȡ�ı���Ϣ
					readByte = LCD_ShowCN_EN(0, 21, bookStr, WHITE, BLACK, 16, 0);

					LCD_Fill(9, 3, 42, 14, BLACK);
					if(BatteryStatus)
					{
						LCD_DrawRectangle(8, 2, 42, 14, WHITE);  // ��ؿ�
						LCD_Fill(42, 5, 45, 11, WHITE);
						LCD_DrawPoint(44, 10, WHITE);
						LCD_Fill(9, 3, 10+8*BatteryStatus, 14, WHITE);
					}
					else
					{
						LCD_DrawRectangle(8, 2, 42, 14, RED);  // ��ؿ�
						LCD_Fill(42, 5, 45, 11, RED);
						LCD_DrawPoint(44, 10, RED);
					}

				}
				if(buttonEvent == ENCODER_DOWN_EVENT && bookPageAddr[bookPage]<bookList[index].addr+bookList[index].size)
				{
					BatteryStatus = ReturnBatteryStatus();
					bookPage++;
					LCD_ShowIntNum(278, 0, bookPage+1, 3, GRAYBLUE, BLACK, 16);
					bookPageAddr[bookPage]=bookPageAddr[bookPage-1]+readByte;
					LCD_Fill(0, 21, LCD_W, LCD_H, BLACK);
					SPI_FLASH_BufferRead(bookStr, bookPageAddr[bookPage], 528); // ��ȡ�ı���Ϣ
					readByte = LCD_ShowCN_EN(0, 21, bookStr, WHITE, BLACK, 16, 0);
					if(bookPageAddr[bookPage]>=bookList[index].addr+bookList[index].size)
						LCD_ShowString(100, 104, "THE END", RED ,BLACK, 32, 0);

					LCD_Fill(9, 3, 42, 14, BLACK);
					if(BatteryStatus)
					{
						LCD_DrawRectangle(8, 2, 42, 14, WHITE);  // ��ؿ�
						LCD_Fill(42, 5, 45, 11, WHITE);
						LCD_DrawPoint(44, 10, WHITE);
						LCD_Fill(9, 3, 10+8*BatteryStatus, 14, WHITE);
					}
					else
					{
						LCD_DrawRectangle(8, 2, 42, 14, RED);  // ��ؿ�
						LCD_Fill(42, 5, 45, 11, RED);
						LCD_DrawPoint(44, 10, RED);
					}

				}
				if(buttonEvent == KEY_ENTER_EVENT)
				{

				}
				if(buttonEvent == KEY_BACK_EVENT)
				{
					buttonEvent = 0;
					LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
					break;
				}
			}
			// ��ʾ�˵�
			for(i=0; i<6; i++)
			{
				LCD_ShowIcon(10, 4+40*i, 7, iconBuff);
				LCD_ShowCN_EN(56, 4+40*i, bookList[i].name, WHITE, BLACK, 32, 0);
			}
			LCD_ShowIcon(250, 4+40*(index%6), 16, iconBuff);  // ��ͷѡ��
		}
		if(buttonEvent == KEY_BACK_EVENT) return;    // ������һ��
	}
}


void badminton()
{
	u8 i;
	u8 winRound[2] = {0, 0};  // ��ʤ����
	u8 finalScore[3][2] = {{0, 0}, {0, 0}, {0, 0}};  // ���շ���
	u8 currentRound = 0;
	u8 pos=0;   // ��ťλ�ã�0������1������ 

	// LCD_ShowCN_EN(4, 4, "����Ʒְ�", WHITE, BLACK, 24, 0);
	LCD_Fill(156, 68, 164, 76, WHITE);    // �м�����
	LCD_Fill(156, 84, 164, 92, WHITE);

	LCD_ShowLED(50, 32, 0, RED, BLACK, ledBuff);  // ����
	LCD_ShowLED(102, 32, 0, RED, BLACK, ledBuff);
	LCD_ShowLED(170, 32, 0, RED, BLACK, ledBuff);
	LCD_ShowLED(222, 32, 0, RED, BLACK, ledBuff);

	LCD_DrawLine(0, 170, LCD_W, 170, WHITE);   // �ָ���
	LCD_DrawLine(160, 170, 160, LCD_H, WHITE);
	LCD_ShowString(168, 189, "1st ROUND", CYAN, BLACK, 32, 0);
	LCD_ShowChar(14, 176, 'L', BRRED, BLACK, 24, 0);
	LCD_ShowChar(14, 211, 'R', LIGHTGREEN, BLACK, 24, 0);
	LCD_DrawLine(0, 205, 160, 205, GRAY);  // ��ǰ�ַ����ָ��� 
	LCD_DrawLine(40, 171, 40, LCD_H, GRAY);
	LCD_DrawLine(80, 171, 80, LCD_H, GRAY);
	LCD_DrawLine(120, 171, 120, LCD_H, GRAY);

	for(i=0; i<3; i++)  // ���ֱ����������ܳ�ʼ�� 0
	{
		LCD_ShowIntNum(48+40*i, 176, 0, 2, GRAY, BLACK, 24);
		LCD_ShowIntNum(48+40*i, 211, 0, 2, GRAY, BLACK, 24);
	}

	LCD_DrawPoint(100, 136, GREEN);   // ѡ��ť�������Զ���
	for(i=1;i<8;i++)
	{
		delay_ms(16);
		Draw_Circle(100, 136, i, GREEN);
	}

	while(1)
	{
		buttonEvent = Return_Button_Event();
		if(buttonEvent == ENCODER_UP_EVENT)  // ������������ת
		{
			finalScore[currentRound][pos]++;
			LCD_ShowLED(50+120*pos, 32, finalScore[currentRound][pos]/10, RED, BLACK, ledBuff);
			LCD_ShowLED(102+120*pos, 32, finalScore[currentRound][pos]%10, RED, BLACK, ledBuff);

			if(finalScore[currentRound][pos]>=21
				&& finalScore[currentRound][pos]-finalScore[currentRound][!pos]>=2 
				|| finalScore[currentRound][pos]==30)   // ���ֻ�ʤ
			{
				LCD_ShowString(96, 3, "ROUND  ,   WINS!", WHITE, BLACK, 16, 0);
				LCD_ShowIntNum(144, 3, currentRound+1, 1,  WHITE, BLACK, 16);
				if(pos) LCD_ShowChar(168, 3, 'R', LIGHTGREEN, BLACK, 16, 0);
				else LCD_ShowChar(168, 3, 'L', BRRED, BLACK, 16, 0);
				LCD_ShowString(80, 21, "Press OK to continue", WHITE, BLACK, 16, 0);

				while(1)
				{
					buttonEvent = Return_Button_Event();
					if(buttonEvent == KEY_BACK_EVENT)
					{
						finalScore[currentRound][pos]--;
						buttonEvent = 0;
						LCD_Fill(0, 0, LCD_W, 37, BLACK);
						LCD_ShowLED(50+120*pos, 32, finalScore[currentRound][pos]/10, RED, BLACK, ledBuff);
						LCD_ShowLED(102+120*pos, 32, finalScore[currentRound][pos]%10, RED, BLACK, ledBuff);
						break;
					}
					if(buttonEvent == KEY_ENTER_EVENT)
					{
						buttonEvent = 0;
						if(pos)
						{
							LCD_ShowIntNum(48+40*currentRound, 176, finalScore[currentRound][!pos], 2, GRAY, BLACK, 24);  // �����²�����������
							LCD_ShowIntNum(48+40*currentRound, 211, finalScore[currentRound][pos], 2, LIGHTGREEN, BLACK, 24);
						}else
						{
							LCD_ShowIntNum(48+40*currentRound, 176, finalScore[currentRound][pos], 2, BRRED, BLACK, 24);  // �����²�����������
							LCD_ShowIntNum(48+40*currentRound, 211, finalScore[currentRound][!pos], 2, GRAY, BLACK, 24);
						}
						winRound[pos]++;
						currentRound++;
						if(winRound[pos] == 2)   // ��ȫʤ��
						{
							LCD_Fill(168, 189, LCD_W, LCD_H, BLACK);
							LCD_ShowString(176, 189, "  WINS!!", CYAN, BLACK, 32, 0);
							if(pos) LCD_ShowChar(176, 189, 'R', LIGHTGREEN, BLACK, 32, 0);
							else LCD_ShowChar(176, 189, 'L', BRRED, BLACK, 32, 0);
							while(1)
							{
								buttonEvent = Return_Button_Event();
								if(buttonEvent == KEY_ENTER_EVENT)
								{
									currentRound = 0;
									for(i=0;i<3;i++)
									{
										finalScore[i][0] = 0;
										finalScore[i][1] = 0;
									}
									winRound[0] = 0;
									winRound[1] = 0;
									for(i=0; i<3; i++)  // ���ֱ����������ܳ�ʼ�� 0
									{
										LCD_ShowIntNum(48+40*i, 176, 0, 2, GRAY, BLACK, 24);
										LCD_ShowIntNum(48+40*i, 211, 0, 2, GRAY, BLACK, 24);
									}
									break;
								}
							}
						}
						LCD_Fill(0, 0, LCD_W, 37, BLACK);
						LCD_ShowLED(50, 32, 0, RED, BLACK, ledBuff);  // ��������
						LCD_ShowLED(102, 32, 0, RED, BLACK, ledBuff);
						LCD_ShowLED(170, 32, 0, RED, BLACK, ledBuff);
						LCD_ShowLED(222, 32, 0, RED, BLACK, ledBuff);

						if(currentRound == 0) LCD_ShowString(168, 189, "1st ROUND", CYAN, BLACK, 32, 0);
						else if(currentRound == 1) LCD_ShowString(168, 189, "2nd", CYAN, BLACK, 32, 0);
						else LCD_ShowString(168, 189, "3rd", CYAN, BLACK, 32, 0);
						break;
					}
				}

			}
		}

		if(buttonEvent == ENCODER_DOWN_EVENT && finalScore[currentRound][pos])  // ������������ת
		{
			finalScore[currentRound][pos]--;
			LCD_ShowLED(50+120*pos, 32, finalScore[currentRound][pos]/10, RED, BLACK, ledBuff);
			LCD_ShowLED(102+120*pos, 32, finalScore[currentRound][pos]%10, RED, BLACK, ledBuff);
		}

		if(buttonEvent == KEY_ENTER_EVENT)  // ȷ��
		{
			pos = !pos;
			for(i=7;i>0;i--)    // ��ʧ����
			{
				Draw_Circle(100+120*!pos, 136, i, BLACK);
				delay_ms(16);
			}
			LCD_DrawPoint(100+120*!pos, 136, BLACK); 

			LCD_DrawPoint(100+120*pos, 136, GREEN);   // ѡ��ťȷ������
			for(i=1;i<8;i++)
			{
				delay_ms(16);
				Draw_Circle(100+120*pos, 136, i, GREEN);
			}
		}

		if(buttonEvent == KEY_BACK_EVENT) return; // ��ť����
	}
}


void timer_1()    // ��ʱ��
{
	u8 i;
	u8 timeCount[2] = {0};
	u8 LR_Count[2] = {0};
	u8 index = 1;
	u8 rat=0;
	LCD_Fill(156, 86, 164, 94, WHITE);    // �м�����
	LCD_Fill(156, 102, 164, 110, WHITE);

	LCD_ShowLED(50, 50, 0, WHITE, BLACK, ledBuff);  // LEDʱ��
	LCD_ShowLED(102, 50, 0, WHITE, BLACK, ledBuff);
	LCD_ShowLED(170, 50, 0, LIGHTBLUE, BLACK, ledBuff);
	LCD_ShowLED(222, 50, 0, LIGHTBLUE, BLACK, ledBuff);

	LCD_ShowString(120, 169, "START", WHITE, BLACK, 32, 0);  // START��ť
	LCD_DrawLine(115, 200, 205, 200, WHITE);
	LCD_DrawLine(115, 202, 205, 202, WHITE);


	while(1)
	{
		buttonEvent = Return_Button_Event();
		if(buttonEvent == ENCODER_UP_EVENT)
		{
			if(index == 1)
			{
				index = 0;
				LCD_ShowLED(170, 50, LR_Count[1]/10, WHITE, BLACK, ledBuff);
				LCD_ShowLED(222, 50, LR_Count[1]%10, WHITE, BLACK, ledBuff);
				LCD_ShowLED(50, 50, LR_Count[0]/10, LIGHTBLUE, BLACK, ledBuff);  // LEDʱ��
				LCD_ShowLED(102, 50, LR_Count[0]%10, LIGHTBLUE, BLACK, ledBuff);
			}
			else if(index == 0)
			{
				index = 2;
				LCD_ShowLED(50, 50, LR_Count[0]/10, WHITE, BLACK, ledBuff);  // LEDʱ��
				LCD_ShowLED(102, 50, LR_Count[0]%10, WHITE, BLACK, ledBuff);
				LCD_ShowString(120, 169, "START", LIGHTBLUE, BLACK, 32, 0);  // START��ť
				LCD_DrawLine(115, 200, 205, 200, LIGHTBLUE);
				LCD_DrawLine(115, 202, 205, 202, LIGHTBLUE);
			}
			else if(index == 2)
			{
				index = 1;
				LCD_ShowString(120, 169, "START", WHITE, BLACK, 32, 0);  // START��ť
				LCD_DrawLine(115, 200, 205, 200, WHITE);
				LCD_DrawLine(115, 202, 205, 202, WHITE);
				LCD_ShowLED(170, 50, LR_Count[1]/10, LIGHTBLUE, BLACK, ledBuff);  // LEDʱ��
				LCD_ShowLED(222, 50, LR_Count[1]%10, LIGHTBLUE, BLACK, ledBuff);
			}
		}
		if(buttonEvent == ENCODER_DOWN_EVENT)
		{
			if(index == 1)
			{
				index = 2;
				LCD_ShowLED(170, 50, LR_Count[1]/10, WHITE, BLACK, ledBuff);
				LCD_ShowLED(222, 50, LR_Count[1]%10, WHITE, BLACK, ledBuff);
				LCD_ShowString(120, 169, "START", LIGHTBLUE, BLACK, 32, 0);  // START��ť
				LCD_DrawLine(115, 200, 205, 200, LIGHTBLUE);
				LCD_DrawLine(115, 202, 205, 202, LIGHTBLUE);
			}
			else if(index == 0)
			{
				index = 1;
				LCD_ShowLED(50, 50, LR_Count[0]/10, WHITE, BLACK, ledBuff);  // LEDʱ��
				LCD_ShowLED(102, 50, LR_Count[0]%10, WHITE, BLACK, ledBuff);
				LCD_ShowLED(170, 50, LR_Count[1]/10, LIGHTBLUE, BLACK, ledBuff);
				LCD_ShowLED(222, 50, LR_Count[1]%10, LIGHTBLUE, BLACK, ledBuff);
				
			}
			else if(index == 2)
			{
				index = 0;
				LCD_ShowString(120, 169, "START", WHITE, BLACK, 32, 0);  // START��ť
				LCD_DrawLine(115, 200, 205, 200, WHITE);
				LCD_DrawLine(115, 202, 205, 202, WHITE);
				LCD_ShowLED(50, 50, LR_Count[0]/10, LIGHTBLUE, BLACK, ledBuff);  // LEDʱ��
				LCD_ShowLED(102, 50, LR_Count[0]%10, LIGHTBLUE, BLACK, ledBuff);
			}
		}
		if(buttonEvent == KEY_ENTER_EVENT)
		{
			if(index==0 || index==1)
			{
				LCD_DrawPoint(100+120*index, 152, GREEN); 
				for(i=1;i<6;i++)
				{
					delay_ms(16);
					Draw_Circle(100+120*index, 152, i, GREEN);
				}

				LCD_ShowLED(50+120*index, 50, LR_Count[index]/10, GBLUE, BLACK, ledBuff);  // LEDʱ��
				LCD_ShowLED(102+120*index, 50, LR_Count[index]%10, GBLUE, BLACK, ledBuff);
				while(1)
				{
					buttonEvent = Return_Button_Event();
					if(buttonEvent == ENCODER_UP_EVENT && LR_Count[index]<59 ||
						buttonEvent == ENCODER_DOWN_EVENT && LR_Count[index]>0)  // ��������ת
					{
						if(buttonEvent == ENCODER_UP_EVENT) LR_Count[index]++;
						else LR_Count[index]--;
						LCD_ShowLED(50+120*index, 50, LR_Count[index]/10, GBLUE, BLACK, ledBuff);  // LEDʱ��
						LCD_ShowLED(102+120*index, 50, LR_Count[index]%10, GBLUE, BLACK, ledBuff);
					}
					else if(buttonEvent == KEY_BACK_EVENT || buttonEvent == KEY_ENTER_EVENT)    // ����ѡ��
					{
						buttonEvent = 0;
						LCD_ShowLED(50+120*index, 50, LR_Count[index]/10, LIGHTBLUE, BLACK, ledBuff);  // LEDʱ��
						LCD_ShowLED(102+120*index, 50, LR_Count[index]%10, LIGHTBLUE, BLACK, ledBuff);

						for(i=5;i>0;i--)    // ��ʧ����
						{
							Draw_Circle(100+120*index, 152, i, BLACK);
							delay_ms(16);
						}
						LCD_DrawPoint(100+120*index, 152, BLACK); 
						break;
					}
				}
			}

			else if(index == 2)   // ��ʼ����ʱ
			{
				timeCount[0] = LR_Count[0];
				timeCount[1] = LR_Count[1];
				for(i=0;i<26;i++)   // ����
				{
					LCD_DrawLine(115-3*i, 202-i, 205+3*i, 202-i, WHITE);
					LCD_DrawLine(115-3*i, 203-i, 205+3*i, 203-i, BLACK);
				}
				LCD_Fill(39, 173, 45, 176, LIGHTBLUE);
				TIM2_Init();
				TIM_Cmd(TIM2, ENABLE);
				while(1)
				{
					buttonEvent = Return_Button_Event();
					if(!(LR_Count[0]|LR_Count[1]))    // ��ʱ��ɺ�Ĳ���
					{
						TIM_Cmd(TIM2, DISABLE);
						LCD_Fill(30, 172, 300, 178, BLACK);

						LCD_ShowCN_EN(46, 170, "  ��  ���Ѽ�ʱ���!", WHITE, BLACK, 24, 0);
						LCD_ShowIntNum(46, 170, timeCount[0], 2, WHITE, BLACK, 24);
						LCD_ShowIntNum(94, 170, timeCount[1], 2, WHITE, BLACK, 24);
						for(i=0;i<6;i++)   // LED��˸��BEEP����
						{
							buttonEvent = Return_Button_Event();
							if(buttonEvent==KEY_BACK_EVENT||buttonEvent==KEY_ENTER_EVENT) 
							{
								buttonEvent = 0;
								break;
							}
							LED_ON;
							BEEP(2000, 50, 10);
							delay_ms(300);
							buttonEvent = Return_Button_Event();
							if(buttonEvent==KEY_BACK_EVENT||buttonEvent==KEY_ENTER_EVENT)
							{
								LED_OFF;
								buttonEvent = 0;
								break;
							}
							BEEP(2000, 50, 10);
							LED_OFF;
							delay_ms(700);
						}
						while(1)
						{
							buttonEvent = Return_Button_Event();
							if(buttonEvent==KEY_BACK_EVENT||buttonEvent==KEY_ENTER_EVENT)
							{
								buttonEvent = 0;
								break;
							}
						}

						LCD_Fill(20, 165, 300, 203, BLACK);
						LCD_ShowString(120, 169, "START", WHITE, BLACK, 32, 0);  // START��ť
						LCD_DrawLine(115, 200, 205, 200, WHITE);
						LCD_DrawLine(115, 202, 205, 202, WHITE);
						buttonEvent = 0;
						break;
					}
					else if(tim2_flag)   // ��λ����
					{
						if(LR_Count[1]==0)
						{
							LR_Count[1] = 59;
							LR_Count[0]--;
						}
						else LR_Count[1]--;
						LCD_ShowLED(50, 50, LR_Count[0]/10, WHITE, BLACK, ledBuff);  // LEDʱ��
						LCD_ShowLED(102, 50, LR_Count[0]%10, WHITE, BLACK, ledBuff);
						LCD_ShowLED(170, 50, LR_Count[1]/10, WHITE, BLACK, ledBuff); 
						LCD_ShowLED(222, 50, LR_Count[1]%10, WHITE, BLACK, ledBuff);

						rat = (u8)((1 - (((float)LR_Count[0]*60+LR_Count[1]) / (timeCount[0]*60+timeCount[1]))) * 239);
						LCD_Fill(39+rat, 173, 45+rat, 176, LIGHTBLUE);
						
						tim2_flag = 0;
					}
					if(buttonEvent == KEY_BACK_EVENT)
					{
						buttonEvent = 0;
						TIM_Cmd(TIM2, DISABLE);
						LR_Count[1] = 0;
						LR_Count[0] = 0;
						index = 1;
						LCD_Fill(0, 140, LCD_W, LCD_H, BLACK);
						LCD_ShowLED(50, 50, 0, WHITE, BLACK, ledBuff);  // LEDʱ��
						LCD_ShowLED(102, 50, 0, WHITE, BLACK, ledBuff);
						LCD_ShowLED(170, 50, 0, LIGHTBLUE, BLACK, ledBuff);
						LCD_ShowLED(222, 50, 0, LIGHTBLUE, BLACK, ledBuff);

						LCD_ShowString(120, 169, "START", WHITE, BLACK, 32, 0);  // START��ť
						LCD_DrawLine(115, 200, 205, 200, WHITE);
						LCD_DrawLine(115, 202, 205, 202, WHITE);
						break;
					}
				}
			}
		}
		if(buttonEvent == KEY_BACK_EVENT) return;
	}
}


void temp()
{
	float nowTemp, preTemp, yTemp;
	u8 i;
	u8 tempList[24];
	preTemp = LM75BD_ReadTemp();  // ��ȡ�¶�Ϊ��׼ֵ
	yTemp = preTemp;
	for(i=0;i<24;i++)
	{
		tempList[i] = 105;
	}
	LCD_DrawRectangle(40, 30, 280, 180, WHITE);
	LCD_ShowCN_EN(100, 209, "�¶�:  . C", WHITE, BLACK, 24, 0);
	LCD_ShowString(276, 185, "0", LIGHTGREEN, BLACK, 16, 0);
	LCD_ShowString(216, 185, "-6", LIGHTGREEN, BLACK, 16, 0);
	LCD_ShowString(148, 185, "-12", LIGHTGREEN, BLACK, 16, 0);
	LCD_ShowString(88, 185, "-18", LIGHTGREEN, BLACK, 16, 0);
	LCD_ShowString(28, 185, "-24", LIGHTGREEN, BLACK, 16, 0);
	LCD_ShowString(40, 10, "Temp(C)", BRRED, BLACK, 16, 0);
	LCD_ShowString(284, 164, "t(s)", LIGHTGREEN, BLACK, 16, 0);

	for(i=1;i<8;i++)   // ��������
		LCD_DrawLine(40+30*i, 31, 40+30*i, 179, GRAY);
	for(i=1;i<5;i++)
		LCD_DrawLine(41, 30+30*i, 279, 30+30*i, GRAY);

	for(i=0;i<6;i++)
	{
		LCD_ShowIntNum(2, 172-30*i, (u8)(yTemp-4.16666666666666), 2, BRRED, BLACK, 16);
		LCD_ShowChar(18, 172-30*i, '.', BRRED, BLACK, 16, 0);
		LCD_ShowIntNum(26, 172-30*i, (u8)((yTemp-4.16666666666666)*10)%10, 1, BRRED, BLACK, 16);
		yTemp += 1.666666666;
	}

	TIM2_Init();
	TIM_Cmd(TIM2, ENABLE);

	while(1)
	{
		buttonEvent = Return_Button_Event();
		if(buttonEvent==KEY_BACK_EVENT)
		{
			buttonEvent = 0;
			TIM_Cmd(TIM2, DISABLE);
			tim2_flag = 0;
			break;
		}

		if(tim2_flag)
		{
			nowTemp = LM75BD_ReadTemp();  // ��ȡ��ǰ�¶�

			for(i=0;i<23;i++)   // �����һ������
				LCD_DrawLine(i*10+45, tempList[i], (i+1)*10+45, tempList[i+1], BLACK);

			for(i=0;i<23;i++)    // ��������
				tempList[i] = tempList[i+1];

			tempList[23] = (u8)((preTemp-nowTemp)*18+105);


			LCD_Fill(284, 30, 317, 163, BLACK);
			for(i=0;i<23;i++)   // ����ͼ��
				LCD_DrawLine(i*10+45, tempList[i], (i+1)*10+45, tempList[i+1], RED);
			LCD_ShowIntNum(284, tempList[23]-4, (u8)nowTemp, 2, RED, BLACK, 16);
			LCD_ShowChar(300, tempList[23]-4, '.', RED, BLACK, 16, 0);
			LCD_ShowIntNum(308, tempList[23]-4, (u8)(nowTemp*10)%100, 1, RED, BLACK, 16);
			
			for(i=1;i<8;i++)   // ����
				LCD_DrawLine(40+30*i, 31, 40+30*i, 179, GRAY);
			for(i=1;i<5;i++)
				LCD_DrawLine(41, 30+30*i, 279, 30+30*i, GRAY);

			LCD_ShowIntNum(160, 209, (u8)nowTemp, 2, WHITE, BLACK, 24);     // �ײ��¶���ʾ
			LCD_ShowIntNum(196, 209, (u8)(nowTemp*10)%100, 1, WHITE, BLACK, 24);
			tim2_flag = 0;

		}
	}
	
}


void battery()
{
	u8 i;
	float Voltage;
	u8 ChargeStatus;

	BatteryStatus = ReturnBatteryStatus();

	Charge_Init();

	for(i=0;i<7;i++) LCD_DrawRectangle(30+i, 50+i, 110-i, 210-i, WHITE);
	LCD_Fill(50, 35, 90, 51, WHITE);
	
	if(!BatteryStatus)   // С��25
	{
		for(i=0;i<7;i++) LCD_DrawRectangle(30+i, 50+i, 110-i, 210-i, RED);
		LCD_Fill(50, 35, 90, 51, RED);
	}
	else
		LCD_Fill(36, 204-37*BatteryStatus, 104, 204, GREEN);
	
	LCD_ShowCN_EN(140, 80, "״̬:", WHITE, BLACK, 24, 0);
	LCD_ShowCN_EN(140, 120, "��ѹ: .  V", WHITE, BLACK, 24, 0);
	LCD_ShowCN_EN(140, 160, "����:", WHITE, BLACK, 24, 0);

	switch(BatteryStatus)
	{
		case 0:
		LCD_ShowCN_EN(200, 160, "С��25%", WHITE, BLACK, 24, 0);
		break;
		case 1:
		LCD_ShowCN_EN(200, 160, "Լ25%", WHITE, BLACK, 24, 0);
		break;
		case 2:
		LCD_ShowCN_EN(200, 160, "Լ50%", WHITE, BLACK, 24, 0);
		break;
		case 3:
		LCD_ShowCN_EN(200, 160, "Լ75%", WHITE, BLACK, 24, 0);
		break;
		case 4:
		LCD_ShowCN_EN(200, 160, "Լ100%", WHITE, BLACK, 24, 0);
		break;
	}

	while(1)
	{
		buttonEvent = Return_Button_Event();
		if(buttonEvent==KEY_BACK_EVENT)
		{
			buttonEvent=0;
			break;
		}

		Voltage =  AD_GetVoltage();
		ChargeStatus = ReturnChargeStatus();

		if(ChargeStatus) LCD_ShowCN_EN(200, 80, "�����", WHITE, BLACK, 24, 0);
		else LCD_ShowCN_EN(200, 80, "�ŵ���", WHITE, BLACK, 24, 0);

		LCD_ShowFloatNum1(200, 120, Voltage, 3, WHITE, BLACK, 24);
		// LCD_ShowIntNum(200, 120, (unsigned int)Voltage, 1, WHITE, BLACK, 24);
		// LCD_ShowIntNum(224, 120, (unsigned int)(Voltage*100)%100, 2, WHITE, BLACK, 24);

	}
	
}


void think()
{
	u8 i;

	Draw_Circle(160, 120, 2, WHITE);
	delay_ms(500);
	Draw_Circle(160, 120, 80, WHITE);
	delay_ms(500);
	LCD_DrawLine(160, 40, 230, 160, WHITE);
	delay_ms(500);
	LCD_DrawLine(230, 160, 90, 160, WHITE);
	delay_ms(500);
	LCD_DrawLine(90, 160, 160, 40, WHITE);
	delay_ms(500);

	LCD_ShowChinese24x24(56, 68, "��", WHITE, BLACK, 0);
	delay_ms(300);
	LCD_ShowChinese24x24(240, 68, "��", WHITE, BLACK, 0);
	delay_ms(300);
	LCD_ShowChinese24x24(148, 207, "��", WHITE, BLACK, 0);
	delay_ms(500);
	

	for(i=0;i<8;i++)
	{
		delay_ms(20);
		LCD_DrawLine(160, 40+10*i, 160, 47+10*i, WHITE);
	}
	for(i=0;i<8;i++)
	{
		delay_ms(20);
		LCD_DrawLine((u8)(160+8.75*i), (u8)(120+5*i), (u8)(166.125+8.75*i), (u8)(123.5+5*i),WHITE);
	}
	for(i=0;i<8;i++)
	{
		delay_ms(20);
		LCD_DrawLine((u8)(160+8.75*i), (u8)(121.5-5*i), (u8)(166.125+8.75*i), (u8)(117-5*i),WHITE);
	}
	LCD_ShowChar(146, 120, 'O', WHITE, BLACK, 16, 0);
	delay_ms(500);

	LCD_DrawLine(190, 93, 197, 89, WHITE);   // ֱ��
	LCD_DrawLine(197, 89, 201, 97, WHITE);

	LCD_DrawLine(99, 147, 106, 148, WHITE);  // �������
	LCD_DrawLine(106, 148, 107, 155, WHITE);
	LCD_DrawLine(107, 155, 105, 160, WHITE);
	delay_ms(500);
	LCD_ShowString(112, 141, "60", WHITE, BLACK, 16, 0);

	delay_ms(500);
	LCD_DrawLine(220, 144, 214, 146, WHITE);   // �������
	LCD_DrawLine(214, 146, 213, 150, WHITE);
	delay_ms(500);
	LCD_ShowString(196, 129, "30", WHITE, BLACK, 16, 0);
	delay_ms(500);

	LCD_ShowString(155, 167, "3r", WHITE, BLACK, 16, 0);   // ����3
	delay_ms(500);
	LCD_DrawLine(144, 177, 147, 173, WHITE);
	LCD_DrawLine(147, 173, 151, 177, WHITE);
	LCD_DrawLine(151, 177, 155, 167, WHITE);
	LCD_DrawLine(155, 167, 163, 167, WHITE);

	delay_ms(500);
	LCD_ShowChar(176, 139, 'r', WHITE, BLACK, 16, 0);
	
	delay_ms(500);
	LCD_ShowChar(166, 96, '2', WHITE, BLACK, 16, 0);
	LCD_ShowChar(166, 79, '1', WHITE, BLACK, 16, 0);
	LCD_DrawLine(166, 96, 174, 96, WHITE); 
	LCD_ShowChar(174, 87, 'r', WHITE, BLACK, 16, 0);
	delay_ms(500);

	LCD_ShowChar(211, 93, 'h', WHITE, BLACK, 16, 0);

	delay_ms(500);
	
	LCD_ShowString(124, 4, "h=1/2r", WHITE, BLACK, 24, 0);
	while(1)
	{
		buttonEvent = Return_Button_Event();
		if(buttonEvent==KEY_BACK_EVENT) break;
	}
}


void menu()
{
	u8 i;
	u8 index = 0;
	show_menu(1);
	LCD_ShowIcon(250, 4, 16, iconBuff);  // ��ͷѡ��
	LCD_ShowCN_EN(56, 4+40*index, menuTable[index].sonName, GRAYBLUE, BLACK, 32, 0);
	LCD_Fill(296, 30, 308, 210, GRAY);  // ��ҳ����������
	LCD_Fill(300, 34, 304, 120, BLUE);  // ��ҳ������

	while(1)
	{
		delay_ms(5);
		buttonEvent = Return_Button_Event();
		if(buttonEvent == ENCODER_UP_EVENT)  // ������������ת
		{
			if(index == 6)  // ���Ϸ�ҳ
			{
				index--;
				LCD_Fill(0,0,160,LCD_H,BLACK);  // ����������ֲ˵�
				LCD_Fill(250,4,282,36,BLACK);  // �����ͷ
				for(i=0;i<88;i++)
				{
					LCD_DrawLine(296, 208-i, 308, 208-i, GRAY);
					LCD_DrawLine(300, 122-i, 304, 122-i, BLUE);
					delay_ms(1);
				}
				show_menu(1);
				LCD_ShowIcon(250, 4+40*(index%6), 16, iconBuff);  // ��ͷѡ��
				LCD_ShowCN_EN(56, 4+40*(index%6), menuTable[index].sonName, GRAYBLUE, BLACK, 32, 0);
			}
			else if(index>0)
			{
				index--;
				for(i=0;i<8;i++)    // ��ͷ�ƶ����Զ���
				{
					LCD_ShowIcon(250, (4+40*((index+1)%6)-i*5), 16, iconBuff);
					LCD_DrawLine(250, (4+40*((index+1)%6)-i*5)+32, 283, (4+40*((index+1)%6)-i*4)+32, BLACK);
				}
				LCD_ShowCN_EN(56, 4+40*((index+1)%6), menuTable[index+1].sonName, WHITE, BLACK, 32, 0);
				LCD_ShowCN_EN(56, 4+40*(index%6), menuTable[index].sonName, GRAYBLUE, BLACK, 32, 0);
			}
		}
		if(buttonEvent == ENCODER_DOWN_EVENT)  // ������������ת
		{
			if(index == 5)  // ���·�ҳ
			{
				index++;
				LCD_Fill(0,0,160,LCD_H,BLACK);
				LCD_Fill(250,204,282,236,BLACK);
				for(i=0;i<88;i++)
				{
					LCD_DrawLine(296, 30+i, 308, 30+i, GRAY);
					LCD_DrawLine(300, 118+i, 304, 118+i, BLUE);
					delay_ms(1);
				}
				show_menu(2);
				LCD_ShowIcon(250, 4+40*(index%6), 16, iconBuff);  // ��ͷѡ��
				LCD_ShowCN_EN(56, 4+40*(index%6), menuTable[index].sonName, GRAYBLUE, BLACK, 32, 0);
			}
			else if(index<11)
			{
				index++;
				for(i=0;i<8;i++)    // ���Զ���
				{
					LCD_ShowIcon(250, (4+40*((index-1)%6)+i*5), 16, iconBuff);
					LCD_DrawLine(250, (4+40*((index-1)%6)+i*5), 283, (4+40*((index-1)%6)+i*4), BLACK);
				}
				LCD_ShowCN_EN(56, 4+40*((index-1)%6), menuTable[index-1].sonName, WHITE, BLACK, 32, 0);
				LCD_ShowCN_EN(56, 4+40*(index%6), menuTable[index].sonName, GRAYBLUE, BLACK, 32, 0);
			}
		}
		if(buttonEvent == KEY_ENTER_EVENT)  // ȷ�ϼ�
		{
			LCD_Fill(0,0,LCD_W,LCD_H,BLACK);

			// �����Ӳ˵��������
			sonFunc = menuTable[index].sonFunc;
			(*sonFunc)();

			LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
			if(index<6) show_menu(1);
			else show_menu(2);
			LCD_Fill(296, 30, 308, 210, GRAY);  // ��ҳ����������
			LCD_Fill(300, 34, 304, 120, BLUE);  // ��ҳ������
			LCD_ShowIcon(250, 4+40*(index%6), 16, iconBuff);  // ��ͷѡ��
			LCD_ShowCN_EN(56, 4+40*(index%6), menuTable[index].sonName, GRAYBLUE, BLACK, 32, 0);
		}
	}
}


void game()
{
	u8 i;
	LCD_ShowCN_EN(100, 104, "Loading...", WHITE, BLACK, 24, 0);
	for(i=0;i<3;i++)
		LCD_DrawRectangle(80+i, 130+i, 240-i, 150-i, WHITE);

	for(i=0;i<110;i++)
	{
		delay_ms(30);
		LCD_DrawLine(82+i, 132, 82+i, 148, WHITE);
	}
	delay_ms(1000);
	for(i=110;i<156;i++)
	{
		delay_ms(50);
		LCD_DrawLine(82+i, 132, 82+i, 148, WHITE);
	}
	delay_ms(1000);
	buttonEvent = 0;

	LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);

	LCD_ShowCN_EN(96, 88, "���û��", WHITE, BLACK, 32, 0);
	LCD_ShowCN_EN(149, 204, "��", WHITE, BLACK, 24, 0);

	while(1)
	{
		buttonEvent = Return_Button_Event();
		if(buttonEvent==KEY_BACK_EVENT) break;
		else if(buttonEvent==ENCODER_DOWN_EVENT)
		{
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
			LCD_ShowCN_EN(80, 88, "�����û��", WHITE, BLACK, 32, 0);
		}
	}


}


void picture()
{
	u8 index=0;
	u8 picIndex[10] = {8, 9, 0, 1, 2, 3, 4, 5, 6, 7};     // ͼƬ˳��
	LCD_ShowFlashPicture(0, 0, 320, 240, pictureBuff, PICTURE_START_ADDRESS+picIndex[index]*153600);
	while(1)
	{
		buttonEvent=Return_Button_Event();
		if(buttonEvent==ENCODER_UP_EVENT && index>0)
		{
			index--;
			if( index==1 || index==5 || index==8 || index==9 )
			{
				LCD_DrawLine(319, 0, 319, 240, BLACK);
				LCD_ShowFlashPicture(0, 0, 319, 240, pictureBuff, PICTURE_START_ADDRESS+picIndex[index]*153600);
			}
			else
				LCD_ShowFlashPicture(0, 0, 320, 240, pictureBuff, PICTURE_START_ADDRESS+picIndex[index]*153600);
		}
		else if(buttonEvent==ENCODER_DOWN_EVENT && index<9)
		{
			index++;
			if( index==1 || index==5 || index==8 || index==9 )
			{
				LCD_DrawLine(319, 0, 319, 240, BLACK);
				LCD_ShowFlashPicture(0, 0, 319, 240, pictureBuff, PICTURE_START_ADDRESS+picIndex[index]*153600);
			}
			else
				LCD_ShowFlashPicture(0, 0, 320, 240, pictureBuff, PICTURE_START_ADDRESS+picIndex[index]*153600);
		}
		else if(buttonEvent==KEY_BACK_EVENT) return;
	}
}


void beep()
{
	LCD_ShowIcon(76, 104, 24, iconBuff);
	LCD_ShowString(112, 104, "BEEP:", WHITE, BLACK, 32, 0);
	LCD_ShowString(192, 104, "OFF", RED, BLACK, 32, 0);
	while(1)
	{
		buttonEvent=Return_Button_Event();
		if(buttonEvent==KEY_ENTER_EVENT)
		{
			LCD_ShowString(192, 104, "ON ", GREEN, BLACK, 32, 0);
			while(1)
			{
				buttonEvent = Return_Button_Event();
				if(buttonEvent==KEY_ENTER_EVENT) 
				{
					buttonEvent = 0;
					LCD_ShowString(192, 104, "OFF", RED, BLACK, 32, 0);
					break;
				}
				else if(buttonEvent==KEY_BACK_EVENT) return;

				// LED_ON;
				BEEP(2000, 50, 10);
				delay_ms(300);
				BEEP(2000, 50, 10);
				// LED_OFF;
				delay_ms(700);
			}
		}
		else if(buttonEvent==KEY_BACK_EVENT) return;
	}
}


void timer_2()
{
	u16 LR_Count[2]={0, 0};
	LCD_Fill(156, 86, 164, 94, WHITE);    // �м�����
	LCD_Fill(156, 102, 164, 110, WHITE);

	LCD_ShowLED(50, 50, 0, WHITE, BLACK, ledBuff);  // LEDʱ��
	LCD_ShowLED(102, 50, 0, WHITE, BLACK, ledBuff);
	LCD_ShowLED(170, 50, 0, WHITE, BLACK, ledBuff);
	LCD_ShowLED(222, 50, 0, WHITE, BLACK, ledBuff);

	LCD_ShowString(120, 169, "START", GREEN, BLACK, 32, 0);  // START��ť
	LCD_DrawLine(115, 200, 205, 200, GREEN);
	LCD_DrawLine(115, 202, 205, 202, GREEN);

	while(1)
	{
		buttonEvent=Return_Button_Event();
		if(buttonEvent==KEY_ENTER_EVENT)
		{
			buttonEvent=0;
			TIM2_Init();
			TIM_Cmd(TIM2, ENABLE);
			LCD_ShowString(120, 169, "PAUSE", BRRED, BLACK, 32, 0);  // START��ť
			LCD_DrawLine(115, 200, 205, 200, BRRED);
			LCD_DrawLine(115, 202, 205, 202, BRRED);
			while(1)
			{
				buttonEvent=Return_Button_Event();
				if(buttonEvent==KEY_ENTER_EVENT)   // ��ͣ
				{
					buttonEvent=0;
					TIM_Cmd(TIM2, DISABLE);
					tim2_flag=0;
					LCD_ShowString(120, 169, "Go on", GREEN, BLACK, 32, 0);  // START��ť
					LCD_DrawLine(115, 200, 205, 200, GREEN);
					LCD_DrawLine(115, 202, 205, 202, GREEN);
					while(1)
					{
						buttonEvent=Return_Button_Event();
						if(buttonEvent==KEY_ENTER_EVENT)  // ����
						{
							TIM_Cmd(TIM2, ENABLE);
							break;
						}
						else if(buttonEvent==KEY_BACK_EVENT) return;
					}
					LCD_ShowString(120, 169, "PAUSE", BRRED, BLACK, 32, 0);  // START��ť
					LCD_DrawLine(115, 200, 205, 200, BRRED);
					LCD_DrawLine(115, 202, 205, 202, BRRED);
				}
				if(tim2_flag)
				{
					if(LR_Count[1]==59)
					{
						LR_Count[1] = 0;
						LR_Count[0]++;
					}
					else if(LR_Count[1]==59&&LR_Count[0]==59)
					{
						LR_Count[0] = 0;
						LR_Count[1] = 0;
					}
					else LR_Count[1]++;
					LCD_ShowLED(50, 50, LR_Count[0]/10, WHITE, BLACK, ledBuff);  // LEDʱ��
					LCD_ShowLED(102, 50, LR_Count[0]%10, WHITE, BLACK, ledBuff);
					LCD_ShowLED(170, 50, LR_Count[1]/10, WHITE, BLACK, ledBuff); 
					LCD_ShowLED(222, 50, LR_Count[1]%10, WHITE, BLACK, ledBuff);
					
					tim2_flag = 0;
				}
			}
		}
		else if(buttonEvent==KEY_BACK_EVENT) return;
	}
}


void about()
{
	u8 i;
	u8 sizeOfStr;
	u8 *p;
	u8 *info[5][2] = {{"�ͺ�", "S22fLZT"}, {"Ӳ���汾", "v1.3"}, {"����汾", "v0.2"}, {"����", "STM32F103"}, {"��������", "2022/9/30"}};
	for(i=0;i<4;i++)
		LCD_DrawLine(16, 48*(i+1), 302, 48*(i+1), GRAY);
	for(i=0;i<5;i++)
	{
		sizeOfStr = 0;
		p = info[i][1];
		while(*p != '\0')   // �ж��ַ���ռ�����ֽ�
		{
			sizeOfStr++;
			p++;
		}
		LCD_ShowCN_EN(20, 12+48*i, info[i][0], WHITE, BLACK, 24, 0);
		LCD_ShowCN_EN(298-sizeOfStr*12, 12+48*i, info[i][1], LIGHTBLUE, BLACK, 24, 0);
	}

	LCD_Fill(307, 30, 317, 210, GRAY);  // ��ҳ����������
	LCD_Fill(311, 34, 313, 120, BLUE);  // ��ҳ������
	while(1)
	{
		buttonEvent=Return_Button_Event();
		if(buttonEvent==ENCODER_DOWN_EVENT)
		{
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
			LCD_ShowFlashPicture(30, 30, 120, 120, pictureBuff, QR_BLOG_START_ADDRESS);
			LCD_ShowFlashPicture(170, 30, 120, 120, pictureBuff, QR_GITHUB_START_ADDRESS);
			LCD_ShowString(66, 156, "Blog", WHITE, BLACK, 24, 0);
			LCD_ShowString(194, 156, "Github", WHITE, BLACK, 24, 0);
			LCD_ShowCN_EN(42, 186, "Ӳ����Դ", WHITE, BLACK, 24, 0);
			LCD_ShowCN_EN(182, 186, "�����Դ", WHITE, BLACK, 24, 0);
			while(1)
			{
				buttonEvent = Return_Button_Event();
				if(buttonEvent==KEY_BACK_EVENT || buttonEvent==KEY_ENTER_EVENT) return;
			}
		}
		else if(buttonEvent==KEY_BACK_EVENT) return;
	}


}


int main(void)
{
	u8 i;

	delay_init();

	// LCD��ʼ��
	LCD_Init(); 
	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);

	SPI_FLASH_Init();

	// ��ʼ������
	USART_Config();

	// LED��ʼ��
	LED_Init();

	// BEEP��ʼ��
	BEEP_Init();
	
	Button_Init();

	LM75BD_IIC_Init();

	AD_Init();

	
	// ��������
	LCD_ShowFlashPicture(0, 0, 320, 240, pictureBuff, PICTURE_KAIJI_ADDRESS);
	for(i=0;i<3;i++)
		LCD_DrawRectangle(80+i, 208+i, 240-i, 228-i, WHITE);

	for(i=0;i<110;i++)   // �������ض��������߿�
	{
		delay_ms(30);
		LCD_DrawLine(82+i, 210, 82+i, 226, WHITE);
	}
	delay_ms(900);
	for(i=110;i<156;i++)   // �������ض���
	{
		delay_ms(50);
		LCD_DrawLine(82+i, 210, 82+i, 226, WHITE);
	}
	delay_ms(700);
	LCD_ShowFlashPicture(0, 0, 320, 240, pictureBuff, PICTURE_PINK_ADDRESS);
	delay_ms(1200);
	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);  // ����

	// ����˵�
	menu();

}
