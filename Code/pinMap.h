// temp sensor connected to PE5 - DS18B20
#define temp_Pin 3

//current sensor connected to PF0 - SCT013-70A-1V
#define CT_pin A0

//presure sensors connected to PF1-PF4 - 4-20ma- HOTH0040FGCK
#define presure_pin1 A1
#define presure_pin2 A2
#define presure_pin3 A3
#define presure_pin4 A4

//led blink connected to PB5 - output
#define LED_pin 11
#define LED_setup DDRB = DDRB | ( 1 << 5 )
#define LED_ON  PORTB = PORTB | ( 1 << 5 )
#define LED_OFF PORTB = PORTB & ~( 1 << 5 )

//serial debug port connected to PH0-PH1
#define Serial_debug Serial2

//GSM module (sim800c) connect to PORTE
#define GSM_Serial Serial
#define GSM_PWR 2
#define GSM_setup DDRE = DDRE | ( 1 << GSM_PWR )
#define GSM_ON  PORTE = PORTE | ( 1 << GSM_PWR )
#define GSM_OFF PORTE = PORTE & ~( 1 << GSM_PWR )
#define GSM_NETlight 5
#define GSM_status 2

//SD card connected to SPI port - CS to PB4
#define SD_CSpin 10

//RTC DS3231 connected to PD0-PD1
#define RTC_SDA 20
#define RTC_SCL 21

//Bluetooth HC05 connect to serial port 1
#define Blutooth_port Serial1

//4*4 keypad connected to PORTC
#define key1 37
#define key2 36
#define key3 35
#define key4 34
#define key5 33
#define key6 32
#define key7 31
#define key8 30

// 240*128 GLCD toshiba 6369
#define LCD_WR 41 //PG0
#define LCD_RD 40 //PG1
#define LCD_CE 15 //PJ0
#define LCD_CD 14 //PJ1
#define LCD_BL A15 //PK7 output
#define LCD_BL_ON  PORTK = PORTK | ( 1 << 7 )  //PH5
#define LCD_BL_OFF PORTK = PORTK & ~( 1 << 7 ) //PH5
#define LCD_RST 39 //PG2
#define LCD_DB1 29 //PA7
#define LCD_DB2 28 //PA6
#define LCD_DB3 27 //PA5
#define LCD_DB4 26 //PA4
#define LCD_DB5 25 //PA3
#define LCD_DB6 24 //PA2
#define LCD_DB7 23 //PA1
#define LCD_DB8 22 //PA0

//relays output
#define relays_setup DDRH = DDRH | ( 1 << 5 ) |  ( 1 << 4 ) | ( 1 << 3 ) | ( 1 << 2 )
#define relay1_ON  PORTH = PORTH | ( 1 << 5 )  //PH5
#define relay1_OFF PORTH = PORTH & ~( 1 << 5 ) //PH5

#define relay2_ON  PORTH = PORTH | ( 1 << 4 )  //PH4
#define relay2_OFF PORTH = PORTH & ~( 1 << 4 ) //PH4

#define relay3_ON  PORTH = PORTH | ( 1 << 3 )  //PH3
#define relay3_OFF PORTH = PORTH & ~( 1 << 3 ) //PH3

#define relay4_ON  PORTH = PORTH | ( 1 << 2 )  //PH2
#define relay4_OFF PORTH = PORTH & ~( 1 << 2 ) //PH2
