
 // Lab3P1.c
 //
 // 
 // Author : Eugene Rockey
 // 
 
 //
 
 const char MS1[] = "\r\nECE-412 ATMega328PB Tiny OS";
 const char MS2[] = "\r\nby Eugene Rockey Copyright 2022, All Rights Reserved";
 const char MS3[] = "\r\nMenu: (L)CD, (A)DC, (E)EPROM\r\n";
 const char MS4[] = "\r\nReady: ";
 const char MS5[] = "\r\nInvalid Command Try Again...";
 const char MS6[] = "Volts\r";
 
 

void LCD_Init(void);			//external Assembly functions
void UART_Init(void);
void UART_Clear(void);
void UART_Get(void);
void UART_Put(void);
void LCD_Write_Data(void);
void LCD_Write_Command(void);
void LCD_Read_Data(void);
void Mega328P_Init(void);
void ADC_Get(void);
void EEPROM_Read(void);
void EEPROM_Write(void);

unsigned char ASCII;			//shared I/O variable with Assembly
unsigned char DATA;				//shared internal variable with Assembly
char HADC;						//shared ADC variable with Assembly
char LADC;						//shared ADC variable with Assembly

char HIGH;                      //shared EEPROM variable with Assembly
char LOW;                       //shared EEPROM variable with Assembly
char EEPROM_DATA;               //shared EEPROM variable with Assembly

char volts[5];					//string buffer for ADC output
int Acc;						//Accumulator for ADC use

void UART_Puts(const char *str)	//Display a string in the PC Terminal Program
{
	while (*str)
	{
		ASCII = *str++;
		UART_Put();
	}
}

void LCD_Puts(const char *str)	//Display a string on the LCD Module
{
	while (*str)
	{
		DATA = *str++;
		LCD_Write_Data();
	}
}


void Banner(void)				//Display Tiny OS Banner on Terminal
{
	UART_Puts(MS1);
	UART_Puts(MS2);
	UART_Puts(MS4);
}

void HELP(void)						//Display available Tiny OS Commands on Terminal
{
	UART_Puts(MS3);
}

void LCD(void)						//Lite LCD demo
{
	DATA = 0x34;					//Set 8bit 2line 5x10 format
	LCD_Write_Command();
	DATA = 0x08;					//turn display off
	LCD_Write_Command();
	DATA = 0x02;					//return cursor to home
	LCD_Write_Command();
	DATA = 0x06;					//set cursor direction right
	LCD_Write_Command();
	DATA = 0x0f;					//turn on display cursor and cursor blink
	LCD_Write_Command();
    LCD_Puts("Hello World!");
}

void LCD_STR(const char* str)			//Modified LCD to print given string
{
	DATA = 0x34;					//Set 8bit 2line 5x10 format
	LCD_Write_Command();
	DATA = 0x08;					//turn display off
	LCD_Write_Command();
	DATA = 0x02;					//return cursor to home
	LCD_Write_Command();
	DATA = 0x06;					//set cursor direction right
	LCD_Write_Command();
	DATA = 0x0f;					//turn on display cursor and cursor blink
	LCD_Write_Command();
    LCD_Puts(str);
}

void teamname(void){
    char teamname[9] = "Plank :D ";
    char swap;
    for(int j = 0; j < 15; j++){
        LCD_STR(teamname);
        //UART_Puts(teamname);
        //UART_Puts("\r\n");
        swap = teamname[0];
        for(int counter = 1; counter < 9; counter++){
            UART_Puts(MS1);
            teamname[counter-1]=teamname[counter];
        }
        teamname[8]=swap;
        //LCD_Puts("                ");
    }
}

void ADC(void)						//Lite Demo of the Analog to Digital Converter
{
	volts[0x1]='.';
	volts[0x3]=' ';
	volts[0x4]= 0;
	ADC_Get();
	Acc = (((int)HADC) * 0x100 + (int)(LADC))*0xA;
	volts[0x0] = 48 + (Acc / 0x7FE);
	Acc = Acc % 0x7FE;
	volts[0x2] = ((Acc *0xA) / 0x7FE) + 48;
	Acc = (Acc * 0xA) % 0x7FE;
	if (Acc >= 0x3FF) volts[0x2]++;
	if (volts[0x2] == 58)
	{
		volts[0x2] = 48;
		volts[0x0]++;
	}
	UART_Puts(volts);
	UART_Puts(MS6);
	/*
		Re-engineer this subroutine to display temperature in degrees Fahrenheit on the Terminal.
		The potentiometer simulates a thermistor, its varying resistance simulates the
		varying resistance of a thermistor as it is heated and cooled. See the thermistor
		equations in the lab 3 folder. User must always be able to return to command line.
	*/
	
}

int verify(char address){
	if(address-'0'>=0&&address-'0'<=9){
		return address-'0';
	}else if(address>=65&&address<=70){
		return address-60;
	}else if(address>=97&&address<=102){
		return address-87;
	}
	return -1;
}

void EEPROM_Address(void)
{
	int charconversion;
	int index = 0;
	while(index < 4){
        UART_Puts(MS4);
        while (ASCII == '\0'){UART_Get();}
		charconversion = verify(ASCII);
		if(charconversion!=-1){
			switch(index){
				case 0:
					HIGH = charconversion*16;
					break;
				case 1:
					HIGH += charconversion;
					break;
				case 2:
					LOW = charconversion*16;
					break;
				case 3:
					LOW += charconversion;
					break;
			}
			index++;
		}else{
            UART_Puts(MS5);
        }
        ASCII = '\0';
	}
}

void EEPROM_Value(void){
    int charconversion;
	int index = 0;
	while(index < 2){
        UART_Puts(MS4);
        while (ASCII == '\0'){UART_Get();}
		charconversion = verify(ASCII);
		if(charconversion!=-1){
			switch(index){
				case 0:
					EEPROM_DATA = charconversion*16;
					break;
				case 1:
					EEPROM_DATA += charconversion;
					break;
			}
			index++;
		}else{
            UART_Puts(MS5);
        }
        ASCII = '\0';
	}
}

void EEPROM(void)
{
	UART_Puts("\r\nEEPROM Write and Read.");
	/*
	Re-engineer this subroutine so that a byte of data can be written to any address in EEPROM
	during run-time via the command line and the same byte of data can be read back and verified after the power to
	the Xplained Mini board has been cycled. Ask the user to enter a valid EEPROM address and an
	8-bit data value. Utilize the following two given Assembly based drivers to communicate with the EEPROM. You
	may modify the EEPROM drivers as needed. User must be able to always return to command line.
	*/
	UART_Puts("\r\n");
    EEPROM_Address();
    EEPROM_Value();
	EEPROM_Write();
	UART_Puts("\r\n");
    EEPROM_Address();
	EEPROM_Read();
	UART_Put();
	UART_Puts("\r\n");
}

void Command(void)					//command interpreter
{
	UART_Puts(MS3);
	ASCII = '\0';						
	while (ASCII == '\0')
	{
		UART_Get();
	}
    //UART_Puts("COMMAND RECIEVED: ");
    //UART_Put();
    //UART_Puts("\r\n");
	switch (ASCII)
	{
		case 'L' | 'l': 
            LCD();
            teamname();
            break;
		case 'A' | 'a': ADC();
            break;
		case 'E' | 'e': EEPROM();
            break;
		default:
            UART_Puts(MS5);
            HELP();
            break;  			
//Add a 'USART' command and subroutine to allow the user to reconfigure the 						
//serial port parameters during runtime. Modify baud rate, # of data bits, parity, 							
//# of stop bits.
	}
}

int main(void)
{
	Mega328P_Init();
	Banner();
	while (1)
	{
		Command();				//infinite command loop
	}
}

