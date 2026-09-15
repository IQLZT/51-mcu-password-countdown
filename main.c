#include <REGX52.H>

bit	flag_Buzzer = 0;
unsigned char Code = 0;
unsigned char Mode = 0;
unsigned char CodeArray[7];
unsigned char CodeIndex = 0;
unsigned char flag_Mode = 0;

sbit Buzzer = P2^5;
bit  Bomb_Countend = 0;
bit  enBuzz = 0;
bit flag_1s = 0;
bit flag_5s = 0;
bit flag_10ms = 0;
bit flag_500ms = 0;
bit Buzzer_Key250ms = 0;
bit Buzzer_Bomb200ms = 0;
bit Buzzer_Bomb800ms = 0;

unsigned char T0RH;
unsigned char T0RL;

unsigned char T1RH = 0 ;
unsigned char T1RL = 0 ;

sbit KEY_OUT_1 = P1^7;
sbit KEY_OUT_2 = P1^6;
sbit KEY_OUT_3 = P1^5;
sbit KEY_OUT_4 = P1^4;

sbit KEY_IN_1 = P1^3;
sbit KEY_IN_2 = P1^2;
sbit KEY_IN_3 = P1^1;
sbit KEY_IN_4 = P1^0;

unsigned char code MatrixKeyCodeMap[4][4] = {
	{0x31, 0x32, 0x33, 0x26 },
	{0x34, 0x35, 0x36, 0x25 },
	{0x37, 0x38, 0x39, 0x28 },
	{0x30, 0x1B, 0x0D, 0x27 }
};

unsigned char MatrixKeyState[4][4] = {
	{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}
};

unsigned long pdata MatrixKeyDownTime[4][4] =
{
	{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}
};

sbit ADDR2 = P2^4;
sbit ADDR1 = P2^3;
sbit ADDR0 = P2^2;

unsigned char LedBuff[8] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char code NiXie_Tube_Yin[]={
	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
	0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,
	0x76, 0x38, 0x37, 0x3E, 0x73, 0x5C, 0x40, 0x00
};

bit StopwatchRuning  = 0;
bit StopwatchRefresh = 1;
unsigned int Bomb_CountDown = 0;
unsigned int IntegerPart  	= 0;
unsigned char DecimalPart 	= 0;
bit flagStart_Bomb = 0;
unsigned int CountDown = 0;

void ConfigTimer0(unsigned int xms);
void ConfigTimer1(unsigned int xms);
void MatrixKeyScan(void);
void MatrixKeyDriver(void);
void MatrixKeyAction(unsigned char keycode);
void ShowNumber(unsigned int num);
void ShowCode(unsigned char CodeArray[]);
void StopwatchDisplay(void);
void StopwatchCount(void);

void main(void)
{
	EA = 1;
	ConfigTimer1(1);
	ConfigTimer0(10);
	ShowNumber(0);

	while(1)
	{
		MatrixKeyDriver();

		if(StopwatchRuning)
		{
			if(flag_10ms)
			{
				flag_10ms = 0;
				Bomb_CountDown--;
				StopwatchCount();
				StopwatchDisplay();
			}

			if(((Bomb_CountDown / 100) > 15))
			{
				flag_Mode = 1;
				enBuzz  = 1;
				if(flag_5s)
				{
					flag_5s = 0;
					P2_0 = ~P2_0;
					P2_1 = ~P2_1;
					P2_6 = ~P2_6;
					P2_7 = ~P2_7;
					Buzzer_Bomb800ms = 1;
				}
			}

			else if(((Bomb_CountDown / 100) > 5))
			{
				flag_Mode = 2;
				enBuzz  = 1;
				if(flag_1s)
				{
					flag_1s = 0;
					P2_0 = ~P2_0;
					P2_1 = ~P2_1;
					P2_6 = ~P2_6;
					P2_7 = ~P2_7;
					Buzzer_Bomb800ms = 1;
				}
			}

			else if(((Bomb_CountDown / 100) > 0))
			{
				flag_Mode = 3;
				enBuzz  = 1;
				if(flag_500ms)
				{
					flag_500ms = 0;
					P2_0 = ~P2_0;
					P2_1 = ~P2_1;
					P2_6 = ~P2_6;
					P2_7 = ~P2_7;
					Buzzer_Bomb200ms = 1;
				}
			}

			else if(Bomb_CountDown == 0)
			{
				flag_Mode = 4;
				flag_Buzzer = 0;
				StopwatchRuning = 0;
				IntegerPart  	= 0;
				DecimalPart 	= 0;
				StopwatchDisplay();
				enBuzz = 1;
				Bomb_Countend = 1;
				P2  = 0x00;
			}
		}
	}
}

void ConfigTimer1(unsigned int xms)
{
	unsigned long tmp;

	tmp = 1105200 / 12;
	tmp = (tmp * xms) / 1000;
	tmp = 65536 - tmp;
	tmp = tmp + 12;

	T1RH = (unsigned char)(tmp >> 8);
	T1RL = (unsigned char)(tmp);

	TMOD &= 0x0F;
    TMOD |= 0x10;
	TH1 = T1RH;
	TL1 = T1RL;

	ET1 = 1;
	TR1 = 1;
}

void ConfigTimer0(unsigned int xms)
{
	unsigned long tmp;
	tmp = 1105200 / 12;
	tmp = (tmp * xms) / 1000;
	tmp = 65536 - tmp;
	tmp = tmp + 18;
	T0RH = (unsigned char)(tmp >> 8);
	T0RL = (unsigned char)(tmp);

	TMOD &= 0xF0;
	TMOD |= 0x01;

	TH0 = T0RH;
	TL0 = T0RL;
	ET0 = 1;
	TR0 = 1;
}

void MatrixKeyScan(void)
{
	unsigned char i;
	static unsigned char keyout = 0;
	static unsigned char KeyBuff[4][4] = {
		{0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF},
	    {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}
	};

	KeyBuff[keyout][0] = (KeyBuff[keyout][0] << 1) | KEY_IN_1;
	KeyBuff[keyout][1] = (KeyBuff[keyout][1] << 1) | KEY_IN_2;
	KeyBuff[keyout][2] = (KeyBuff[keyout][2] << 1) | KEY_IN_3;
	KeyBuff[keyout][3] = (KeyBuff[keyout][3] << 1) | KEY_IN_4;

	for(i = 0; i < 4;i++)
	{
		if((KeyBuff[keyout][i] & 0x0F) == 0x00)
		{
			MatrixKeyState[keyout][i] = 0;
			MatrixKeyDownTime[keyout][i] += 40;
		}

		else if((KeyBuff[keyout][i] & 0x0F) == 0x0F)
		{
			MatrixKeyState[keyout][i] = 1;
			MatrixKeyDownTime[keyout][i] = 0;
		}
  	}

	keyout++;
	keyout &= 0x03;
	switch(keyout)
	{
		case 0: KEY_OUT_4 = 1; KEY_OUT_1 = 0; break;
		case 1: KEY_OUT_1 = 1; KEY_OUT_2 = 0; break;
		case 2: KEY_OUT_2 = 1; KEY_OUT_3 = 0; break;
		case 3: KEY_OUT_3 = 1; KEY_OUT_4 = 0; break;
		default: break;
	}
}

void MatrixKeyDriver(void)
{
	unsigned char i,j;
	static unsigned char  pdata Keybackup[4][4] = {
		{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}
	};

	static unsigned long pdata TimeThr[4][4] = {
		{1000, 1000, 1000, 1000}, {1000, 1000, 1000, 1000}, {1000, 1000, 1000, 1000}, {1000, 1000, 1000, 1000}
	};

	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			if(Keybackup[i][j] != MatrixKeyState[i][j])
			{
				if(Keybackup[i][j] != 0)
				{
					MatrixKeyAction(MatrixKeyCodeMap[i][j]);
					enBuzz = 1;
					Buzzer_Key250ms = 1;
				}
				Keybackup[i][j] = MatrixKeyState[i][j];
			}

			if(MatrixKeyDownTime[i][j] > 0)
			{
				if(MatrixKeyDownTime[i][j] >= TimeThr[i][j])
				{
					MatrixKeyAction(MatrixKeyCodeMap[i][j]);
					TimeThr[i][j] += 500;
				}
			}

			else
			{
				TimeThr[i][j] = 1000;
			}
		}
	}
}

void MatrixKeyAction(unsigned char keycode)
{
	unsigned char i;

	if(0x30 <= keycode && keycode <= 0x39)
	{
		if(Mode == 0)
		{
			Code =  keycode - '0';
			CodeArray[CodeIndex] = Code;
			ShowCode(CodeArray);
			CodeIndex++;
			CodeIndex %= 7;
		}
	}

	else if(keycode == 0x26)
	{
		if(Mode == 1)
		{
			StopwatchRefresh = 1;
			if (CountDown < 9999)
			{
				CountDown++;
				IntegerPart = CountDown;
				StopwatchDisplay();
			}
		}
	}

	else if(keycode == 0x28)
	{
		if(Mode == 1)
		{
			StopwatchRefresh = 1;
			if (CountDown > 1)
			{
				CountDown--;
				IntegerPart = CountDown;
				StopwatchDisplay();
			}
		}
	}

	else if (keycode == 0x0D)
	{
		Mode++;
		StopwatchDisplay();
		if(Mode == 2)
		{
			Mode %= 3;
			Bomb_CountDown  = CountDown * 100;
			StopwatchRuning = 1;
			flagStart_Bomb  = 1;
		}
	}

	else if (keycode == 0x1B)
	{
		Mode   = 0;
		flag_Mode = 0;
		for(i = 0;i < 7;i++)
		{
			CodeArray[7] = 0;
		}
		enBuzz = 0;
		CodeIndex = 0;
		flagStart_Bomb = 0;
		CountDown = 0;
		IntegerPart  = 0;
		DecimalPart = 0;
		StopwatchRefresh = 0;
		StopwatchRuning = 0;
		ShowNumber(CountDown);
	}
}

void ShowNumber(unsigned int num)
{
	unsigned char buff[8];
	signed char i ;

	for(i = 0; i < 8; i++)
	{
		buff[i] = num % 10;
		num = num / 10;
	}

	for(i = 7; i >= 1; i--)
	{
		if(buff[i] == 0)
		{
			LedBuff[i] = 0x00;
		}
		else
		{
			break;
		}
	}

	for(i = i; i >=0; i--)
	{
		LedBuff[i] = NiXie_Tube_Yin[buff[i]];
	}
}

void ShowCode(unsigned char CodeArray[])
{
	unsigned char buff[8];
	signed char i ;

	for(i = 0; i < 8; i++)
	{
		buff[i] = CodeArray[i];
	}

	for(i = CodeIndex;i >= 0;i--)
	{
		LedBuff[i] = NiXie_Tube_Yin[buff[CodeIndex - i]];
	}

	for(i = 7;i > CodeIndex;i--)
	{
		LedBuff[i] = 0x00;
	}
}

void LedScan(void)
{
	static unsigned char index;

	P0 = 0x00;

	switch(index)
	{
		case(0): ADDR2 = 0; ADDR1 = 0; ADDR0 = 0; index++; P0 = LedBuff[0]; break;
		case(1): ADDR2 = 0; ADDR1 = 0; ADDR0 = 1; index++; P0 = LedBuff[1]; break;
		case(2): ADDR2 = 0; ADDR1 = 1; ADDR0 = 0; index++; P0 = LedBuff[2]; break;
		case(3): ADDR2 = 0; ADDR1 = 1; ADDR0 = 1; index++; P0 = LedBuff[3]; break;
		case(4): ADDR2 = 1; ADDR1 = 0; ADDR0 = 0; index++; P0 = LedBuff[4]; break;
		case(5): ADDR2 = 1; ADDR1 = 0; ADDR0 = 1; index++; P0 = LedBuff[5]; break;
		case(6): ADDR2 = 1; ADDR1 = 1; ADDR0 = 0; index++; P0 = LedBuff[6]; break;
		case(7): ADDR2 = 1; ADDR1 = 1; ADDR0 = 1; index=0; P0 = LedBuff[7]; break;
		default: break;
	}
}

void StopwatchDisplay(void)
{
	signed char i;
	unsigned char buff[4];

	LedBuff[0] = NiXie_Tube_Yin[DecimalPart % 10];
	LedBuff[1] = NiXie_Tube_Yin[DecimalPart / 10];

	buff[0] = (IntegerPart % 10);
	buff[1] = (IntegerPart / 10) % 10;
	buff[2] = (IntegerPart / 100) % 10;
	buff[3] = (IntegerPart / 1000) % 10;

	LedBuff[7] = 0x00;
	LedBuff[6] = 0x00;

	for(i = 3; i >= 1; i--)
	{
		if(buff[i] == 0)
		{
			LedBuff[i + 2] = 0x00;
		}
		else
		{
			break;
		}
	}

	for(i = i; i >=0; i--)
	{
		LedBuff[i+2] = NiXie_Tube_Yin[buff[i]];
	}

	LedBuff[2] |= 0x80;
}

void StopwatchCount(void)
{
	if(StopwatchRuning && (Bomb_CountDown > 0))
	{
		DecimalPart = Bomb_CountDown % 100;
		IntegerPart = Bomb_CountDown / 100;

		StopwatchRefresh = 1;
	}
}

void Interrupt_Timer0(void) interrupt 1
{
	static unsigned int  tmr10ms = 0;
	static unsigned char tmr1s   = 1;
	static unsigned char tmr_250ms = 0;
	static unsigned char tmr_500ms = 0;

	TH0 = T0RH;
	TL0 = T0RL;

	flag_10ms = 1;
	LedScan();
	MatrixKeyScan();

	if(Buzzer_Key250ms)
	{
		tmr_250ms++;
		if(tmr_250ms >= 25)
		{
			tmr_250ms = 0;
			Buzzer_Key250ms = 0;
			enBuzz = 0;
		}
	}

	if( (flag_Mode == 1) && (flag_5s == 0))
	{
		tmr10ms++;
		if(tmr10ms >= 1000)
		{
			tmr10ms = 0;
			flag_5s = 1;
		}
	}

	if( (flag_Mode == 2) && (flag_1s == 0))
	{
		tmr10ms++;
		if(tmr10ms >= 500)
		{
			tmr10ms = 0;
			flag_1s = 1;
		}
	}

	if( (flag_Mode == 3) && (flag_500ms == 0))
	{
		tmr10ms++;
		if(tmr10ms >= 200)
		{
			tmr10ms = 0;
			flag_500ms = 1;
		}
	}
}

void Interrupt_Timer1(void)   interrupt 3
{
	static  unsigned int tmr1_500ms = 0;
	static  unsigned int tmr1_800ms = 0;

	TH1 = T1RH;
	TL1 = T1RL;

	if(enBuzz == 1)
	{
		if(Buzzer_Key250ms)
		{
			Buzzer = ~Buzzer;
		}

		else if(Buzzer_Bomb800ms)
		{
			tmr1_800ms++;
			if(tmr1_800ms >= 800)
			{
				enBuzz = 0;
				tmr1_800ms = 0;
				Buzzer_Bomb800ms = 0;
			}
			Buzzer = ~Buzzer;
		}

		else if(Buzzer_Bomb200ms)
		{
			tmr1_500ms++;
			if(tmr1_500ms >= 500)
			{
				enBuzz = 0;
				tmr1_500ms = 0;
				Buzzer_Bomb200ms = 0;
			}
			Buzzer = ~Buzzer;
		}

		else if(Bomb_Countend)
		{
			Buzzer = ~Buzzer;
		}

		else
		{
			enBuzz = 0;
		}
	}
	else
	{
		Buzzer = 1;
	}
}
