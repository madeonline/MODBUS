#include <avr/io.h>
#include "Wire.h"
#include <UTFT.h>
#include <UTouch.h>
#include <UTFT_Buttons.h>
#include <RTClib.h>
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <MsTimer2.h> 
#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
#include "MCP23017.h"
#include <avr/pgmspace.h>

MCP23017 mcp_Out1;                                 // Назначение портов расширения MCP23017  4 A - Out, B - Out
MCP23017 mcp_Out2;                                 // Назначение портов расширения MCP23017  6 A - Out, B - Out

#define  ledPin13  13                              // Назначение светодиодов на плате
#define  ledPin12  12                              // Назначение светодиодов на плате

//+++++++++++++++++++ MODBUS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

modbusDevice regBank;
//Create the modbus slave protocol handler
modbusSlave slave;

//+++++++++++++++++++++++ Настройка электронного резистора +++++++++++++++++++++++++++++++++++++
#define address_AD5252   0x2F                      // Адрес микросхемы AD5252  
#define control_word1    0x07                      // Байт инструкции резистор №1
#define control_word2    0x87                      // Байт инструкции резистор №2
byte resistance        = 0x00;                     // Сопротивление 0x00..0xFF - 0Ом..100кОм


//+++++++++++++++++++++++++++++ Внешняя память +++++++++++++++++++++++++++++++++++++++
int deviceaddress        = 80;                     // Адрес микросхемы памяти
unsigned int eeaddress   =  0;                     // Адрес ячейки памяти
byte hi;                                           // Старший байт для преобразования числа
byte low;                                          // Младший байт для преобразования числа

//********************* Настройка монитора ***********************************

// Standard Arduino Mega/Due shield            : <display model>,38,39,40,41
//UTFT          myGLCD(ITDB32S,38,39,40,41);     // Дисплей 3.2"
UTFT        myGLCD(ITDB24E_8,38,39,40,41);   // Дисплей 2.4" !! Внимание! Изменены настройки UTouchCD.h


// Standard Arduino Mega/Due shield            : 6,5,4,3,2
UTouch        myTouch(6,5,4,3,2);
// Finally we set up UTFT_Buttons :)
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

boolean default_colors = true;
uint8_t menu_redraw_required = 0;

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t Dingbats1_XL[];
extern uint8_t SmallSymbolFont[];


//+++++++++++++++++++++++++++ Настройка часов +++++++++++++++++++++++++++++++
uint8_t second = 0;                      //Initialization time
uint8_t minute = 10;
uint8_t hour   = 10;
uint8_t dow    = 2;
uint8_t day    = 15;
uint8_t month  = 3;
uint16_t year  = 16;
RTC_DS1307 RTC;                         // define the Real Time Clock object

int clockCenterX               = 119;
int clockCenterY               = 119;
int oldsec                     = 0;
const char* str[]              = {"MON","TUE","WED","THU","FRI","SAT","SUN"};
const char* str1[]             = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
const char* str_mon[]          = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
unsigned long wait_time        = 0;         // Время простоя прибора
unsigned long wait_time_Old    = 0;         // Время простоя прибора
int time_minute                = 5;         // Время простоя прибора
//------------------------------------------------------------------------------

const unsigned int adr_control_command    PROGMEM       = 40001; // Адрес передачи комманд на выполнение 
const unsigned int adr_reg_count_err      PROGMEM       = 40002; // Адрес счетчика всех ошибок
//-------------------------------------------------------------------------------------------------------
//+++++++++++++++++++++++++++ Порты управления платой Arduino Nano +++++++++++++++++++++++++++++++

#define  kn1Nano   A0                                            // Назначение кнопок управления Nano  A0
#define  kn2Nano   A1                                            // Назначение кнопок управления Nano  A1
#define  kn3Nano   A2                                            // Назначение кнопок управления Nano  A2
#define  kn4Nano   A3                                            // Назначение кнопок управления Nano  A3

#define  kn5Nano   A4                                            // Назначение кнопок управления Nano  A4
#define  kn6Nano   A5                                            // Назначение кнопок управления Nano  A5




//++++++++++++++++++++++++++++ Переменные для цифровой клавиатуры +++++++++++++++++++++++++++++
int x, y, z;
char stCurrent[20]    ="";         // Переменная хранения введенной строки 
char stCurrent1[20];               // Переменная хранения введенной строки 
int stCurrentLen      =0;          // Переменная хранения длины введенной строки 
int stCurrentLen1     =0;          // Переменная временного хранения длины введенной строки  
int stCurrentLen_user =0;          // Переменная  хранения длины введенной строки пароля пользователя
int stCurrentLen_telef=0;          // Переменная  хранения длины введенной строки пароля пользователя
int stCurrentLen_admin=0;          // Переменная  хранения длины введенной строки пароля администратора
char stLast[20]       ="";                // Данные в введенной строке строке.
char stLast1[20]      ="";               // Данные в введенной строке строке.
int ret               = 0;                       // Признак прерывания операции
int lenStr            = 0;                    // Длина строки ZegBee

//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------------
//Назначение переменных для хранения № опций меню (клавиш)
int but1, but2, but3, but4, but5, but6, but7, but8, but9, but10, butX, butY, butA, butB, butC, butD, but_m1, but_m2, but_m3, but_m4, but_m5, pressed_button;
 //int kbut1, kbut2, kbut3, kbut4, kbut5, kbut6, kbut7, kbut8, kbut9, kbut0, kbut_save,kbut_clear, kbut_exit;
 //int kbutA, kbutB, kbutC, kbutD, kbutE, kbutF;
 int m2 = 1; // Переменная номера меню

 //------------------------------------------------------------------------------------------------------------------
 // Назначение переменных для хранения текстов

 char  txt_menu1_1[]       = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N1";                    // Тест кабель N 1
 char  txt_menu1_2[]       = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N2";                    // Тест кабель N 2
 char  txt_menu1_3[]       = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N3";                    // Тест кабель N 3
 char  txt_menu1_4[]       = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N4";                    // Тест кабель N 4
 char  txt_menu2_1[]       = "menu2_1";                                                // 
 char  txt_menu2_2[]       = "menu2_2";                                                //
 char  txt_menu2_3[]       = "menu2_3";                                                //
 char  txt_menu2_4[]       = "menu2_4";                                                //
 char  txt_menu3_1[]       = "Ta""\x96\xA0\x9D\xA6""a coe""\x99"".";                   // Таблица соед.
 char  txt_menu3_2[]       = "Pe""\x99""a""\x9F\xA4"". ""\xA4""a""\x96\xA0\x9D\xA6";   // Редакт. таблиц
 char  txt_menu3_3[]       = "\x85""a""\x98""py""\x9C"". y""\xA1""o""\xA0\xA7"".";     // Загруз. умолч.
 char  txt_menu3_4[]       = "Bpe""\xA1\xAF"" ""\xA3""poc""\xA4""o""\xAF";             // Время простоя                   //
 char  txt_menu4_1[]       = "C\x9D\xA2yco\x9D\x99""a";                                // Синусоида
 char  txt_menu4_2[]       = "Tpey\x98o\xA0\xAC\xA2\xAB\x9E";                          // Треугольный
 char  txt_menu4_3[]       = "\x89\x9D\xA0oo\x96pa\x9C\xA2\xAB\x9E";                   // Пилообразный
 char  txt_menu4_4[]       = "\x89p\xAF\xA1oy\x98o\xA0\xAC\xA2\xAB\x9E";               // Прямоугольный
 char  txt_menu5_1[]       = " ";// 
 char  txt_menu5_2[]       = " ";//
 char  txt_menu5_3[]       = " ";// 
 char  txt_menu5_4[]       = " ";// 
 char  txt_pass_ok[]       = "Tec\xA4 Pass!";                                           // Тест Pass!
 char  txt_pass_no[]       = "Tec\xA4 NO!";                                             // Тест NO!
 char  txt_info1[]         = "Tec\xA4 ""\x9F""a\x96""e\xA0""e\x9E";                     // Тест кабелей
 char  txt_info2[]         = "Tec\xA4 \x96\xA0o\x9F""a \x98""ap\xA2\x9D\xA4yp";         // Тест блока гарнитур
 char  txt_info3[]         = "Hac\xA4po\x9E\x9F""a c\x9D""c\xA4""e\xA1\xAB";            // Настройка системы
 char  txt_info4[]         = "\x81""e\xA2""epa\xA4op c\x9D\x98\xA2""a\xA0o\x97";        // Генератор сигналов
 char  txt_info5[]         = "Oc\xA6\x9D\xA0\xA0o\x98pa\xA5";                           // Осциллограф
 char  txt_botton_clear[]  = "C\x96poc";                                                // Сброс
 char  txt_botton_otmena[] = "O""\xA4\xA1""e""\xA2""a";                                 // Отмена
 char  txt_system_clear1[] = "B\xA2\x9D\xA1""a\xA2\x9D""e!";                            // Внимание !  
 char  txt_system_clear2[] = "Bc\xAF \xA1\xA2\xA5op\xA1""a""\xA6\xA1\xAF \x96y\x99""e\xA4";  // Вся информация будет 
 char  txt_system_clear3[] = "\x8A\x82""A""\x88""EHA!";                                 // УДАЛЕНА 
 char  txt9[6]             = "B\x97o\x99";                                              // Ввод
 char  txt10[8]            = "O""\xA4\xA1""e""\xA2""a";                                 // "Отмена"
 char  txt_time_wait[]     = "\xA1\x9D\xA2"".""\x97""pe""\xA1\xAF"" ""\xA3""poc""\xA4""o""\xAF";      //  мин. время простоя




 int   temp_buffer[40] ;                                                                // Буфер хранения временной информации
 
 const unsigned int connektN1_default[]    PROGMEM  = { 
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,                                                     // Разъем А
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20                                                      // Разъем B
 }; // 20 x 2 ячеек
 const unsigned int connektN2_default[]    PROGMEM  = { 
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,                                   // Разъем А
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26                                    // Разъем B
 }; // 26 x 2 ячеек
 const unsigned int connektN3_default[]    PROGMEM  = { 
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,  // Разъем А
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37   // Разъем B
 }; // 37 x 2 ячеек
 const unsigned int connektN4_default[]    PROGMEM  = { 
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,           // Разъем А
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34            // Разъем B
 }; // 34 x 2 ячеек

 //++++++++++++++++++ Вариант № 1 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 const unsigned int adr_memN1_1      PROGMEM       =    100;                      // Начальный адрес памяти таблицы соответствия контактов разъемов №1А, №1В
 const unsigned int adr_memN2_1      PROGMEM       =    141;                      // Начальный адрес памяти таблицы соответствия контактов разъемов №2А, №2В
 const unsigned int adr_memN3_1      PROGMEM       =    194;                      // Начальный адрес памяти таблицы соответствия контактов разъемов №3А, №3В
 const unsigned int adr_memN4_1      PROGMEM       =    269;                      // Начальный адрес памяти таблицы соответствия контактов разъемов №4А, №4В
 //++++++++++++++++++ Вариант № 2 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 const unsigned int adr_memN1_2      PROGMEM       =    300;                      // Начальный адрес памяти таблицы соответствия контактов разъемов №1А, №1В
 const unsigned int adr_memN2_2      PROGMEM       =    341;                      // Начальный адрес памяти таблицы соответствия контактов разъемов №2А, №2В
 const unsigned int adr_memN3_2      PROGMEM       =    394;                      // Начальный адрес памяти таблицы соответствия контактов разъемов №3А, №3В
 const unsigned int adr_memN4_2      PROGMEM       =    469;                      // Начальный адрес памяти таблицы соответствия контактов разъемов №4А, №4В

void dateTime(uint16_t* date, uint16_t* time)                  // Программа записи времени и даты файла
{
  DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

void serial_print_date()                           // Печать даты и времени    
{
	  DateTime now = RTC.now();
	  Serial.print(now.day(), DEC);
	  Serial.print('/');
	  Serial.print(now.month(), DEC);
	  Serial.print('/');
	  Serial.print(now.year(), DEC); 
	  Serial.print(' ');
	  Serial.print(now.hour(), DEC);
	  Serial.print(':');
	  Serial.print(now.minute(), DEC);
	  Serial.print(':');
	  Serial.print(now.second(), DEC);
	  Serial.print("  ");
	  Serial.println(str1[now.dayOfWeek()-1]);
}
void clock_read()
{
	DateTime now = RTC.now();
	second = now.second();       
	minute = now.minute();
	hour   = now.hour();
	dow    = now.dayOfWeek();
	day    = now.day();
	month  = now.month();
	year   = now.year();
}

void set_time()
{
	RTC.adjust(DateTime(__DATE__, __TIME__));
	DateTime now = RTC.now();
	second = now.second();       //Initialization time
	minute = now.minute();
	hour   = now.hour();
	day    = now.day();
	day++;
	if(day > 31)day = 1;
	month  = now.month();
	year   = now.year();
	DateTime set_time = DateTime(year, month, day, hour, minute, second); // Занести данные о времени в строку "set_time"
	RTC.adjust(set_time);             
}
void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data )
{
	int rdata = data;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.write(rdata);
	Wire.endTransmission();
	delay(10);
}
byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
	byte rdata = 0xFF;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress,1);
	if (Wire.available()) rdata = Wire.read();
	return rdata;
}
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length )
{
	
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress,length);
	int c = 0;
	for ( c = 0; c < length; c++ )
	if (Wire.available()) buffer[c] = Wire.read();
	
}
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) 
{
	
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddresspage >> 8)); // MSB
	Wire.write((int)(eeaddresspage & 0xFF)); // LSB
	byte c;
	for ( c = 0; c < length; c++)
	Wire.write(data[c]);
	Wire.endTransmission();
	
}

void drawDisplay()
{
  // Clear screen
  myGLCD.clrScr();
  
  // Draw Clockface
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(0, 0, 0);
  for (int i=0; i<5; i++)
  {
	myGLCD.drawCircle(clockCenterX, clockCenterY, 119-i);
  }
  for (int i=0; i<5; i++)
  {
	myGLCD.drawCircle(clockCenterX, clockCenterY, i);
  }
  
  myGLCD.setColor(192, 192, 255);
  myGLCD.print("3", clockCenterX+92, clockCenterY-8);
  myGLCD.print("6", clockCenterX-8, clockCenterY+95);
  myGLCD.print("9", clockCenterX-109, clockCenterY-8);
  myGLCD.print("12", clockCenterX-16, clockCenterY-109);
  for (int i=0; i<12; i++)
  {
	if ((i % 3)!=0)
	  drawMark(i);
  }  
  clock_read();
  drawMin(minute);
  drawHour(hour, minute);
  drawSec(second);
  oldsec=second;

  // Draw calendar
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect(240, 0, 319, 85);
  myGLCD.setColor(0, 0, 0);
  for (int i=0; i<7; i++)
  {
	myGLCD.drawLine(249+(i*10), 0, 248+(i*10), 3);
	myGLCD.drawLine(250+(i*10), 0, 249+(i*10), 3);
	myGLCD.drawLine(251+(i*10), 0, 250+(i*10), 3);
  }

  // Draw SET button
  myGLCD.setColor(64, 64, 128);
  myGLCD.fillRoundRect(260, 200, 319, 239);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect(260, 200, 319, 239);
  myGLCD.setBackColor(64, 64, 128);
  myGLCD.print("SET", 266, 212);
  myGLCD.setBackColor(0, 0, 0);
  
 /* myGLCD.setColor(64, 64, 128);
  myGLCD.fillRoundRect(260, 140, 319, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect(260, 140, 319, 180);
  myGLCD.setBackColor(64, 64, 128);
  myGLCD.print("RET", 266, 150);
  myGLCD.setBackColor(0, 0, 0);*/

}
void drawMark(int h)
{
  float x1, y1, x2, y2;
  
  h=h*30;
  h=h+270;
  
  x1=110*cos(h*0.0175);
  y1=110*sin(h*0.0175);
  x2=100*cos(h*0.0175);
  y2=100*sin(h*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x2+clockCenterX, y2+clockCenterY);
}
void drawSec(int s)
{
  float x1, y1, x2, y2;
  int ps = s-1;
  
  myGLCD.setColor(0, 0, 0);
  if (ps==-1)
  ps=59;
  ps=ps*6;
  ps=ps+270;
  
  x1=95*cos(ps*0.0175);
  y1=95*sin(ps*0.0175);
  x2=80*cos(ps*0.0175);
  y2=80*sin(ps*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x2+clockCenterX, y2+clockCenterY);

  myGLCD.setColor(255, 0, 0);
  s=s*6;
  s=s+270;
  
  x1=95*cos(s*0.0175);
  y1=95*sin(s*0.0175);
  x2=80*cos(s*0.0175);
  y2=80*sin(s*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x2+clockCenterX, y2+clockCenterY);
}
void drawMin(int m)
{
  float x1, y1, x2, y2, x3, y3, x4, y4;
  int pm = m-1;
  
  myGLCD.setColor(0, 0, 0);
  if (pm==-1)
  pm=59;
  pm=pm*6;
  pm=pm+270;
  
  x1=80*cos(pm*0.0175);
  y1=80*sin(pm*0.0175);
  x2=5*cos(pm*0.0175);
  y2=5*sin(pm*0.0175);
  x3=30*cos((pm+4)*0.0175);
  y3=30*sin((pm+4)*0.0175);
  x4=30*cos((pm-4)*0.0175);
  y4=30*sin((pm-4)*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myGLCD.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myGLCD.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myGLCD.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);

  myGLCD.setColor(0, 255, 0);
  m=m*6;
  m=m+270;
  
  x1=80*cos(m*0.0175);
  y1=80*sin(m*0.0175);
  x2=5*cos(m*0.0175);
  y2=5*sin(m*0.0175);
  x3=30*cos((m+4)*0.0175);
  y3=30*sin((m+4)*0.0175);
  x4=30*cos((m-4)*0.0175);
  y4=30*sin((m-4)*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myGLCD.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myGLCD.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myGLCD.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);
}
void drawHour(int h, int m)
{
  float x1, y1, x2, y2, x3, y3, x4, y4;
  int ph = h;
  
  myGLCD.setColor(0, 0, 0);
  if (m==0)
  {
	ph=((ph-1)*30)+((m+59)/2);
  }
  else
  {
	ph=(ph*30)+((m-1)/2);
  }
  ph=ph+270;
  
  x1=60*cos(ph*0.0175);
  y1=60*sin(ph*0.0175);
  x2=5*cos(ph*0.0175);
  y2=5*sin(ph*0.0175);
  x3=20*cos((ph+5)*0.0175);
  y3=20*sin((ph+5)*0.0175);
  x4=20*cos((ph-5)*0.0175);
  y4=20*sin((ph-5)*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myGLCD.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myGLCD.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myGLCD.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);

  myGLCD.setColor(255, 255, 0);
  h=(h*30)+(m/2);
  h=h+270;
  
  x1=60*cos(h*0.0175);
  y1=60*sin(h*0.0175);
  x2=5*cos(h*0.0175);
  y2=5*sin(h*0.0175);
  x3=20*cos((h+5)*0.0175);
  y3=20*sin((h+5)*0.0175);
  x4=20*cos((h-5)*0.0175);
  y4=20*sin((h-5)*0.0175);
  
  myGLCD.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myGLCD.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myGLCD.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myGLCD.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);
}
void printDate()
{
  clock_read();
  myGLCD.setFont(BigFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
	
  myGLCD.print(str[dow-1], 256, 8);
  if (day<10)
	myGLCD.printNumI(day, 272, 28);
  else
	myGLCD.printNumI(day, 264, 28);

  myGLCD.print(str_mon[month-1], 256, 48);
  myGLCD.printNumI(year, 248, 65);
}
void clearDate()
{
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRect(248, 8, 312, 81);
}
void AnalogClock()
{
	int x, y;
	drawDisplay();
	printDate();
	while (true)
	  {
		if (oldsec != second)
		{
		  if ((second == 0) and (minute == 0) and (hour == 0))
		  {
			clearDate();
			printDate();
		  }
		  if (second==0)
		  {
			drawMin(minute);
			drawHour(hour, minute);
		  }
		  drawSec(second);
		  oldsec = second;
		  wait_time_Old =  millis();
		}

		if (myTouch.dataAvailable())
		{
		  myTouch.read();
		  x=myTouch.getX();
		  y=myTouch.getY();
		  if (((y>=200) && (y<=239)) && ((x>=260) && (x<=319))) //установка часов
		  {
			myGLCD.setColor (255, 0, 0);
			myGLCD.drawRoundRect(260, 200, 319, 239);
			setClock();
		  }

		 //  if (((y>=140) && (y<=180)) && ((x>=260) && (x<=319))) //Возврат
		  if (((y>=1) && (y<=239)) && ((x>=1) && (x<=260))) //Возврат
		  {
			//myGLCD.setColor (255, 0, 0);
			//myGLCD.drawRoundRect(260, 140, 319, 180);
			myGLCD.clrScr();
			myGLCD.setFont(BigFont);
			break;
		  }
		 if (((y>=1) && (y<=180)) && ((x>=260) && (x<=319))) //Возврат
		  {
			//myGLCD.setColor (255, 0, 0);
			//myGLCD.drawRoundRect(260, 140, 319, 180);
			myGLCD.clrScr();
			myGLCD.setFont(BigFont);
			break;
		  }
		}
		delay(10);
		clock_read();
	  }
}

void flash_time()                                              // Программа обработчик прерывания 
{ 
	// PORTB = B00000000; // пин 12 переводим в состояние LOW
	slave.run();
	// PORTB = B01000000; // пин 12 переводим в состояние HIGH
}
void serialEvent3()
{
	control_command();
}

void reset_klav()
{
		myGLCD.clrScr();
		myButtons.deleteAllButtons();
		but1 = myButtons.addButton( 10,  20, 250,  35, txt_menu5_1);
		but2 = myButtons.addButton( 10,  65, 250,  35, txt_menu5_2);
		but3 = myButtons.addButton( 10, 110, 250,  35, txt_menu5_3);
		but4 = myButtons.addButton( 10, 155, 250,  35, txt_menu5_4);
		butX = myButtons.addButton(279, 199,  40,  40, "W", BUTTON_SYMBOL); // кнопка Часы 
		but_m1 = myButtons.addButton(  10, 199, 45,  40, "1");
		but_m2 = myButtons.addButton(  61, 199, 45,  40, "2");
		but_m3 = myButtons.addButton(  112, 199, 45,  40, "3");
		but_m4 = myButtons.addButton(  163, 199, 45,  40, "4");
		but_m5 = myButtons.addButton(  214, 199, 45,  40, "5");

}
void txt_pass_no_all()
{
		myGLCD.clrScr();
		myGLCD.setColor(255, 255, 255);
		myGLCD.setBackColor(0, 0, 0);
		myGLCD.print(txt_pass_no, RIGHT, 208);
		delay (1000);
}
void klav123() // ввод данных с цифровой клавиатуры
{
	ret = 0;

	while (true)
	  {
		if (myTouch.dataAvailable())
		{
			  myTouch.read();
			  x=myTouch.getX();
			  y=myTouch.getY();
	  
		if ((y>=10) && (y<=60))  // Upper row
		  {
			if ((x>=10) && (x<=60))  // Button: 1
			  {
				  waitForIt(10, 10, 60, 60);
				  updateStr('1');
			  }
			if ((x>=70) && (x<=120))  // Button: 2
			  {
				  waitForIt(70, 10, 120, 60);
				  updateStr('2');
			  }
			if ((x>=130) && (x<=180))  // Button: 3
			  {
				  waitForIt(130, 10, 180, 60);
				  updateStr('3');
			  }
			if ((x>=190) && (x<=240))  // Button: 4
			  {
				  waitForIt(190, 10, 240, 60);
				  updateStr('4');
			  }
			if ((x>=250) && (x<=300))  // Button: 5
			  {
				  waitForIt(250, 10, 300, 60);
				  updateStr('5');
			  }
		  }

		 if ((y>=70) && (y<=120))  // Center row
		   {
			 if ((x>=10) && (x<=60))  // Button: 6
				{
				  waitForIt(10, 70, 60, 120);
				  updateStr('6');
				}
			 if ((x>=70) && (x<=120))  // Button: 7
				{
				  waitForIt(70, 70, 120, 120);
				  updateStr('7');
				}
			 if ((x>=130) && (x<=180))  // Button: 8
				{
				  waitForIt(130, 70, 180, 120);
				  updateStr('8');
				}
			 if ((x>=190) && (x<=240))  // Button: 9
				{
				  waitForIt(190, 70, 240, 120);
				  updateStr('9');
				}
			 if ((x>=250) && (x<=300))  // Button: 0
				{
				  waitForIt(250, 70, 300, 120);
				  updateStr('0');
				}
			}
		  if ((y>=130) && (y<=180))  // Upper row
			 {
			 if ((x>=10) && (x<=130))  // Button: Clear
				{
				  waitForIt(10, 130, 120, 180);
				  stCurrent[0]='\0';
				  stCurrentLen=0;
				  myGLCD.setColor(0, 0, 0);
				  myGLCD.fillRect(0, 224, 319, 239);
				}
			 if ((x>=250) && (x<=300))  // Button: Exit
				{
				  waitForIt(250, 130, 300, 180);
				  myGLCD.clrScr();
				  myGLCD.setBackColor(VGA_BLACK);
				  ret = 1;
				  stCurrent[0]='\0';
				  stCurrentLen=0;
				  break;
				}
			 if ((x>=130) && (x<=240))  // Button: Enter
				{
				  waitForIt(130, 130, 240, 180);
				 if (stCurrentLen>0)
				   {
				   for (x=0; x<stCurrentLen+1; x++)
					 {
						stLast[x]=stCurrent[x];
					 }
						stCurrent[0]='\0';
						stLast[stCurrentLen+1]='\0';
						//i2c_eeprom_write_byte(deviceaddress,adr_stCurrentLen1,stCurrentLen);
						stCurrentLen1 = stCurrentLen;
						stCurrentLen=0;
						myGLCD.setColor(0, 0, 0);
						myGLCD.fillRect(0, 208, 319, 239);
						myGLCD.setColor(0, 255, 0);
						myGLCD.print(stLast, LEFT, 208);
						break;
					}
				  else
					{
						myGLCD.setColor(255, 0, 0);
						myGLCD.print("\x80\x8A\x8B\x8B""EP \x89\x8A""CTO\x87!", CENTER, 192);//"БУФФЕР ПУСТОЙ!"
						delay(500);
						myGLCD.print("                ", CENTER, 192);
						delay(500);
						myGLCD.print("\x80\x8A\x8B\x8B""EP \x89\x8A""CTO\x87!", CENTER, 192);//"БУФФЕР ПУСТОЙ!"
						delay(500);
						myGLCD.print("                ", CENTER, 192);
						myGLCD.setColor(0, 255, 0);
					}
				 }
			  }
		  }
	   } 
} 
void drawButtons1() // Отображение цифровой клавиатуры
{
// Draw the upper row of buttons
  for (x=0; x<5; x++)
  {
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect (10+(x*60), 10, 60+(x*60), 60);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect (10+(x*60), 10, 60+(x*60), 60);
	myGLCD.printNumI(x+1, 27+(x*60), 27);
  }
// Draw the center row of buttons
  for (x=0; x<5; x++)
  {
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect (10+(x*60), 70, 60+(x*60), 120);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect (10+(x*60), 70, 60+(x*60), 120);
	if (x<4)
	myGLCD.printNumI(x+6, 27+(x*60), 87);
  }

  myGLCD.print("0", 267, 87);
// Draw the lower row of buttons
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (10, 130, 120, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 130, 120, 180);
  myGLCD.print(txt_botton_clear, 25, 147);     //"Сброс"


  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (130, 130, 240, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (130, 130, 240, 180);
  myGLCD.print("B\x97o\x99", 155, 147);       // "Ввод"
  

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (250, 130, 300, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (250, 130, 300, 180);
  myGLCD.print("B""\xAB""x", 252, 147);       // Вых
  myGLCD.setBackColor (0, 0, 0);
}
void updateStr(int val)
{
  if (stCurrentLen<20)
  {
	stCurrent[stCurrentLen]=val;
	stCurrent[stCurrentLen+1]='\0';
	stCurrentLen++;
	myGLCD.setColor(0, 255, 0);
	myGLCD.print(stCurrent, LEFT, 224);
  }
  else
  {   // Вывод строки "ПЕРЕПОЛНЕНИЕ!"
	myGLCD.setColor(255, 0, 0);
	myGLCD.print("\x89""EPE""\x89O\x88HEH\x86""E!", CENTER, 224);// ПЕРЕПОЛНЕНИЕ!
	delay(500);
	myGLCD.print("              ", CENTER, 224);
	delay(500);
	myGLCD.print("\x89""EPE""\x89O\x88HEH\x86""E!", CENTER, 224);// ПЕРЕПОЛНЕНИЕ!
	delay(500);
	myGLCD.print("              ", CENTER, 224);
	myGLCD.setColor(0, 255, 0);
  }
}
void waitForIt(int x1, int y1, int x2, int y2)
{
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
	myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}


void control_command()
{
	/*
	Для вызова подпрограммы проверки необходимо записать номер проверки по адресу adr_control_command (40120) 
	Код проверки
	0 -   Выполнение команды окончено
	1 -   Программа проверки кабеля №1
	2 -   Программа проверки кабеля №2
	3 -   Программа проверки кабеля №3
	4 -   Программа проверки кабеля №4
	5 -   Программа проверки панели гарнитур
	6 -   Записать таблицу проверки №1 по умолчанию
	7 -   Записать таблицу проверки №2 по умолчанию
	8 -   Записать таблицу проверки №3 по умолчанию
	9 -   Записать таблицу проверки №4 по умолчанию
	10 -  Установить уровень сигнала резистором №1
	11 -  Установить уровень сигнала резистором №2
	12 -  Чтение таблиц из EEPROM для передачи в ПК
	13 -  Получить таблицу из ПK и записать в EEPROM
	14 -  
	15 -  
	16 -                   
	17 -  
	18 -  
	19 -  
	20 -  
	21 -  
	22 -  
	23 -  
	24 - 
	25 - 
	26 - 
	27 - 
	28 - 
	29 - 
	30 - 

	*/


	int test_n = regBank.get(adr_control_command);   //адрес  40000
	if (test_n != 0)
	{
		if(test_n != 0) Serial.println(test_n);	
		switch (test_n)
		{
			case 1:
				 test_cabel_N1();             // Программа проверки кабеля №1
				 break;
			case 2:	
				 test_cabel_N2();             // Программа проверки кабеля №2
				 break;
			case 3:
				 test_cabel_N3();             // Программа проверки кабеля №3
				 break;
			case 4:	
				 test_cabel_N4();             // Программа проверки кабеля №4
				 break;
			case 5:
				 test_panel_N1();             // Программа проверки панели гарнитур
				 break;
			case 6:	
				 save_default_N1();           // Записать таблицу проверки №1 по умолчанию
				 break;
			case 7:
				 save_default_N1();           // Записать таблицу проверки №2 по умолчанию
				 break;
			case 8:	
				 save_default_N1();           // Записать таблицу проверки №3 по умолчанию
				 break;
			case 9:
				 save_default_N1();           // Записать таблицу проверки №4 по умолчанию
				 break;
			case 10:
				 set_rezistor1();             // Установить уровень сигнала резистором №1
				 break;
			case 11:
				 set_rezistor2();             // Установить уровень сигнала резистором №1
				 break;
			case 12:
				 mem_byte_trans_read();       // Чтение таблиц из EEPROM для передачи в ПК
				 break;
			case 13:
				 mem_byte_trans_save();       // Получить таблицу из ПK и записать в EEPROM
				 break;
			case 14:
				 //
				 break;
			case 15:
				 //
				 break;
			case 16:
				 //                  
				 break;
			case 17:
				 //
				 break;
			case 18:
				 //
				 break;
			case 19:
				 //
				 break;
			case 20:                                         //  
				 //
				 break;
			case 21:                      		 		     //  
				//
				 break;
			case 22:                                         //  
				 //
				 break;
			case 23: 
				 //      
				 break;
			case 24: 
				 //    
				 break;
			case 25: 
				 //         
				 break;
			case 26: 
				 //    
				 break;
			case 27: 
				 //
				 break;
			case 28:
				 //
				 break;
			case 29:
				 //
				 break;
			case 30:  
				 //
				 break;

			default:
				 regBank.set(adr_control_command,0);        // Установить резистором №1,№2  уровень сигнала
				 break;
		 }

	}
	else
	{
	   regBank.set(adr_control_command,0);
	}
}

void draw_Glav_Menu()
{

  but1   = myButtons.addButton( 10,  20, 250,  35, txt_menu1_1);
  but2   = myButtons.addButton( 10,  65, 250,  35, txt_menu1_2);
  but3   = myButtons.addButton( 10, 110, 250,  35, txt_menu1_3);
  but4   = myButtons.addButton( 10, 155, 250,  35, txt_menu1_4);
  butX   = myButtons.addButton( 279, 199,  40,  40, "W", BUTTON_SYMBOL); // кнопка Часы 
  but_m1 = myButtons.addButton(  10, 199, 45,  40, "1");
  but_m2 = myButtons.addButton(  61, 199, 45,  40, "2");
  but_m3 = myButtons.addButton(  112, 199, 45,  40, "3");
  but_m4 = myButtons.addButton(  163, 199, 45,  40, "4");
  but_m5 = myButtons.addButton(  214, 199, 45,  40, "5");
  myButtons.drawButtons(); // Восстановить кнопки
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_WHITE);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("                      ", CENTER, 0); 

  switch (m2) 
				   {
					case 1:
					      myGLCD.print(txt_info1, CENTER, 0);
					      break;
					 case 2:
                          myGLCD.print(txt_info2, CENTER, 0);
					      break;
					 case 3:
					      myGLCD.print(txt_info3, CENTER, 0);
					      break;
					 case 4:
					      myGLCD.print(txt_info4, CENTER, 0);
					      break;
					 case 5:
					      myGLCD.print(txt_info5, CENTER, 0);
					      break;
					 }
  myButtons.drawButtons();

}

void swichMenu() // Тексты меню в строках "txt....."
{
	 m2=1;                                                         // Устанивить первую странице меню
	 while(1) 
	   {
		  wait_time = millis();                                    // Программа вызова часов при простое 
		  if (wait_time - wait_time_Old > 60000 * time_minute)
		  {
				wait_time_Old =  millis();
				AnalogClock();
				myGLCD.clrScr();
				myButtons.drawButtons();                           // Восстановить кнопки
				print_up();                                        // Восстановить верхнюю строку
		  }

		  myButtons.setTextFont(BigFont);                          // Установить Большой шрифт кнопок  

			if (myTouch.dataAvailable() == true)                   // Проверить нажатие кнопок
			  {
			    pressed_button = myButtons.checkButtons();         // Если нажата - проверить что нажато
				wait_time_Old =  millis();

					 if (pressed_button==butX)                     // Нажата вызов часы
					      {  
							 AnalogClock();
							 myGLCD.clrScr();
							 myButtons.drawButtons();              // Восстановить кнопки
							 print_up();                           // Восстановить верхнюю строку
					      }
		 
					 if (pressed_button==but_m1)                   // Нажата 1 страница меню
						  {
							  myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_BLUE); // Голубой фон меню
							  myButtons.drawButtons();             // Восстановить кнопки
							  default_colors=true;
							  m2=1;                                // Устанивить первую странице меню
							  myButtons.relabelButton(but1, txt_menu1_1, m2 == 1);
							  myButtons.relabelButton(but2, txt_menu1_2, m2 == 1);
							  myButtons.relabelButton(but3, txt_menu1_3, m2 == 1);
							  myButtons.relabelButton(but4, txt_menu1_4, m2 == 1);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info1, CENTER, 0);   // "Тест кабелей"
		
						  }
				    if (pressed_button==but_m2)
						  {
							  myButtons.setButtonColors(VGA_WHITE, VGA_RED, VGA_YELLOW, VGA_BLUE, VGA_TEAL);
							  myButtons.drawButtons();
							  default_colors=false;
							  m2=2;
							  myButtons.relabelButton(but1, txt_menu2_1 , m2 == 2);
							  myButtons.relabelButton(but2, txt_menu2_2 , m2 == 2);
							  myButtons.relabelButton(but3, txt_menu2_3 , m2 == 2);
							  myButtons.relabelButton(but4, txt_menu2_4 , m2 == 2);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info2, CENTER, 0);     // Тест блока гарнитур
						 }

				   if (pressed_button==but_m3)
						 {
							  myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_GREEN);
							  myButtons.drawButtons();
							  default_colors=false;
							  m2=3;
							  myButtons.relabelButton(but1, txt_menu3_1 , m2 == 3);
							  myButtons.relabelButton(but2, txt_menu3_2 , m2 == 3);
							  myButtons.relabelButton(but3, txt_menu3_3 , m2 == 3);
							  myButtons.relabelButton(but4, txt_menu3_4 , m2 == 3);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info3, CENTER, 0);      // Настройка системы
						}
				   if (pressed_button==but_m4)
						{
							  myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_RED);
							  myButtons.drawButtons();
							  default_colors=false;
							  m2=4;
							  myButtons.relabelButton(but1, txt_menu4_1 , m2 == 4);
							  myButtons.relabelButton(but2, txt_menu4_2 , m2 == 4);
							  myButtons.relabelButton(but3, txt_menu4_3 , m2 == 4);
							  myButtons.relabelButton(but4, txt_menu4_4 , m2 == 4);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info4, CENTER, 0);     // Генератор сигналов
						}

				   if (pressed_button==but_m5)
						{
							  myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_NAVY);
							  myButtons.drawButtons();
							  default_colors=false;
							  m2=5;
							  myButtons.relabelButton(but1, txt_menu5_1 , m2 == 5);
							  myButtons.relabelButton(but2, txt_menu5_2 , m2 == 5);
							  myButtons.relabelButton(but3, txt_menu5_3 , m2 == 5);
							  myButtons.relabelButton(but4, txt_menu5_4 , m2 == 5);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info5, CENTER, 0);     // Осциллограф
						}
	
	               //*****************  Меню №1  **************

		           if (pressed_button==but1 && m2 == 1)
			           {
						
								myGLCD.clrScr();   // Очистить экран
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    				//	elektro_save_start(); // если верно - выполнить пункт меню
					
			   				 myGLCD.clrScr();
							 myButtons.drawButtons();
							 print_up();
			           }
	  
		           if (pressed_button==but2 && m2 == 1)
					   {
						
								myGLCD.clrScr();   // Очистить экран
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    				//	gaz_save_start(); // если верно - выполнить пункт меню
					
			   				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					   }
	  
		           if (pressed_button==but3 && m2 == 1)
					   {
						
								myGLCD.clrScr();   // Очистить экран
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    				 //  colwater_save_start(); // если верно - выполнить пункт меню
					
			   				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					   }
		           if (pressed_button==but4 && m2 == 1)
					   {
					
								myGLCD.clrScr();   // Очистить экран
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    		// 
			   				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					   }

		         //*****************  Меню №2  **************


		           if (pressed_button==but1 && m2 == 2)
					  {
						//	print_info();
	        				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
				      }

				  if (pressed_button==but2 && m2 == 2)
					  {
						//   info_nomer_user();
				  			myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					  }
	  
				  if (pressed_button==but3 && m2 == 2)
					  {
					
						 // test_arRequestMod();
							myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					  }
				  if (pressed_button==but4 && m2 == 2)
					  {
						 
						  //  testRemoteAtCommand();
						    myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
				      }
		
		        //*****************  Меню №3  **************
		           if (pressed_button==but1 && m2 == 3) // Первый пункт меню 3
					{
						    myGLCD.clrScr();   // Очистить экран
							myGLCD.print(txt_pass_ok, RIGHT, 208); 
							delay (500);
							//eeprom_clear == 0;
		    		//			system_clear_start(); // если верно - выполнить пункт меню
							 myGLCD.clrScr();
							 myButtons.drawButtons();
							 print_up();
					  }

			 //--------------------------------------------------------------
		           if (pressed_button==but2 && m2 == 3)  // Второй пункт меню 3
				      {
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
						//	set_n_telef();
							myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
				   
					  }

			   //------------------------------------------------------------------

			       if (pressed_button==but3 && m2 == 3)  // Третий пункт меню 3
					  { 
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
		    	//
							myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
				      }

	 //------------------------------------------------------------------
				   if (pressed_button==but4 && m2 == 3)                 // Четвертый пункт меню 3
				      {
				
							myGLCD.clrScr();
							myGLCD.setFont(BigFont);
							myGLCD.setBackColor(0, 0, 255);
							myGLCD.clrScr();
							drawButtons1();                            // Нарисовать цифровую клавиатуру
							myGLCD.printNumI(time_minute, LEFT, 208);
							myGLCD.print(txt_time_wait, 35, 208);   //
							klav123();                                 // Считать информацию с клавиатуры
							if (ret == 1)                              // Если "Возврат" - закончить
								 {
									goto bailout41;                    // Перейти на окончание выполнения пункта меню
								 }
							else                                       // Иначе выполнить пункт меню
								 {
									 time_minute = atol(stLast);
								 }
						    bailout41:                                 // Восстановить пункты меню
						    myGLCD.clrScr();
						    myButtons.drawButtons();
						    print_up();
				      }

                   //*****************  Меню №4  **************

                   if (pressed_button==but1 && m2 == 4) // 
					  {
			
							myGLCD.clrScr();   // Очистить экран
							myGLCD.print(txt_pass_ok, RIGHT, 208); 
							delay (500);
							//butA = myButtons.addButton(279, 20,  40,  35, "W", BUTTON_SYMBOL); // Синусоида
							//if (myButtons.buttonEnabled(butB)) myButtons.deleteButton(butB);
							//if (myButtons.buttonEnabled(butC)) myButtons.deleteButton(butC);
							//if (myButtons.buttonEnabled(butD)) myButtons.deleteButton(butD);
							myButtons.drawButtons();
							print_up();
							//
				   
					  }

				   if (pressed_button==but2 && m2 == 4)
					  {
					
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
		    	//			butB = myButtons.addButton(279, 65, 40,  35, "W", BUTTON_SYMBOL); // Треугольный
							//if (myButtons.buttonEnabled(butA)) myButtons.deleteButton(butA);
							//if (myButtons.buttonEnabled(butC)) myButtons.deleteButton(butC);
							//if (myButtons.buttonEnabled(butD)) myButtons.deleteButton(butD);
							myButtons.drawButtons();
							print_up();
					  }

		           if (pressed_button==but3 && m2 == 4) // 
					  {
				
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
							//butC = myButtons.addButton(279, 110,  40,  35, "W", BUTTON_SYMBOL); // Пилообразный
							//if (myButtons.buttonEnabled(butA)) myButtons.deleteButton(butA);
							//if (myButtons.buttonEnabled(butB)) myButtons.deleteButton(butB);
							//if (myButtons.buttonEnabled(butD)) myButtons.deleteButton(butD);
							myButtons.drawButtons();
							print_up();
					  }
				   if (pressed_button==but4 && m2 == 4) //
					  {
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
							//butD = myButtons.addButton(279, 155,  40,  35, "W", BUTTON_SYMBOL); // Прямоугольный сигнал
							//if (myButtons.buttonEnabled(butB)) myButtons.deleteButton(butB);
							//if (myButtons.buttonEnabled(butC)) myButtons.deleteButton(butC);
							//if (myButtons.buttonEnabled(butA)) myButtons.deleteButton(butA);
							myButtons.drawButtons();
							print_up();
					  }
				    //*****************  Меню №5  **************

                   if (pressed_button==but1 && m2 == 5) // Сброс данных
					  {
							//ZigBee_status();
			   				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					  }
				   if (pressed_button==but2 && m2 == 5)
					  {
							  myGLCD.clrScr();   // Очистить экран
							  myGLCD.print(txt_pass_ok, RIGHT, 208); 
							  delay (500);
		    				 // ZigBee_SetH(); // если верно - выполнить пункт меню
		
							myButtons.drawButtons();
							print_up();
					  }

				   if (pressed_button==but3 && m2 == 5) // Ввод пароля пользователя
					  {
							  myGLCD.clrScr();   // Очистить экран
							  myGLCD.print(txt_pass_ok, RIGHT, 208); 
							  delay (500);
		    				 // ZigBee_SetL(); // если верно - выполнить пункт меню
					
							myButtons.drawButtons();
							print_up();
					  }

			       if (pressed_button==but4 && m2 == 5) // Смена пароля администратора
			          {
				   
					
								myGLCD.clrScr();   // Очистить экран
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    					//ZigBee_Set_Network();
						
							myButtons.drawButtons();
							print_up();
				      }
			
		           if (pressed_button==-1) 
					  {
						//  myGLCD.print("HET", 220, 220);
					  }
				  } 
       }
}
void print_up() // Печать верхней строчки над меню
{
	myGLCD.setColor(0, 255, 0);
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.print("                      ", CENTER, 0); 
	switch (m2) 
	{
 		case 1:
			myGLCD.print(txt_info1, CENTER, 0);
			break;
		case 2:
            myGLCD.print(txt_info2, CENTER, 0);
			break;
		case 3:
			myGLCD.print(txt_info3, CENTER, 0);
			break;
		case 4:
			myGLCD.print(txt_info4, CENTER, 0);
			break;
        case 5:
			myGLCD.print(txt_info5, CENTER, 0);
			break;
    }
}

void setup_resistor()
{ 
	Wire.beginTransmission(address_AD5252);        // transmit to device
	Wire.write(byte(control_word1));               // sends instruction byte  
	Wire.write(0);                                 // sends potentiometer value byte  
	Wire.endTransmission();                        // stop transmitting
	Wire.beginTransmission(address_AD5252);        // transmit to device
	Wire.write(byte(control_word2));               // sends instruction byte  
	Wire.write(0);                                 // sends potentiometer value byte  
	Wire.endTransmission();                        // stop transmitting
}
void resistor(int resist, int valresist)
{
	resistance = valresist;
	switch (resist)
	{
	case 1:
			Wire.beginTransmission(address_AD5252);     // transmit to device
			Wire.write(byte(control_word1));            // sends instruction byte  
			Wire.write(resistance);                     // sends potentiometer value byte  
			Wire.endTransmission();                     // stop transmitting
			break;
	case 2:				
			Wire.beginTransmission(address_AD5252);     // transmit to device
			Wire.write(byte(control_word2));            // sends instruction byte  
			Wire.write(resistance);                     // sends potentiometer value byte  
			Wire.endTransmission();                     // stop transmitting
			break;
	}
			//Wire.requestFrom(address_AD5252, 1, true);  // Считать состояние движка резистора 
			//level_resist = Wire.read();                 // sends potentiometer value byte  
	// regBank.set(adr_control_command,0);
}
void set_rezistor1()
{
	int mwt1 = regBank.get(40003);             // Адрес хранения величины сигнала резистором № 1
	resistor(1, mwt1);
	regBank.set(adr_control_command,0);
}
void set_rezistor2()
{
	int mwt2 = regBank.get(40004);             // Адрес хранения величины сигнала резистором № 2
	resistor(2, mwt2);
	regBank.set(adr_control_command,0);
}

void save_default_N1()                                          // Запись заводских установок таблицы разъемов №1
{
	int _step_mem = 20*2;                                         // Длина блока с таблицы
	byte _u_konnekt  = 0;                                       // Временное хранения содержимого регистра.
    for (int i = 0; i < _step_mem;i++)                    
		{
			_u_konnekt = pgm_read_word_near(connektN1_default+i);
			i2c_eeprom_write_byte(deviceaddress,i+adr_memN1_1, _u_konnekt); 
		}
	regBank.set(adr_control_command,0);                        // Завершить программу    
	delay(100);
}
void save_default_N2()                                          // Запись заводских установок таблицы разъемов №1
{
	int _step_mem = 26*2;                                         // Длина блока с таблицы
	byte _u_konnekt  = 0;                                       // Временное хранения содержимого регистра.
    for (int i = 0; i < _step_mem;i++)                    
		{
			_u_konnekt = pgm_read_word_near(connektN2_default+i);
			i2c_eeprom_write_byte(deviceaddress,i+adr_memN2_1, _u_konnekt); 
		}
	regBank.set(adr_control_command,0);                        // Завершить программу    
	delay(100);
}
void save_default_N3()                                          // Запись заводских установок таблицы разъемов №1
{
	int _step_mem = 37*2;                                         // Длина блока с таблицы
	byte _u_konnekt  = 0;                                       // Временное хранения содержимого регистра.
    for (int i = 0; i < _step_mem;i++)                    
		{
			_u_konnekt = pgm_read_word_near(connektN3_default+i);
			i2c_eeprom_write_byte(deviceaddress,i+adr_memN3_1, _u_konnekt); 
		}
	regBank.set(adr_control_command,0);                        // Завершить программу    
	delay(100);
}
void save_default_N4()                                          // Запись заводских установок таблицы разъемов №1
{
	int _step_mem = 34*2;                                         // Длина блока с таблицы
	byte _u_konnekt  = 0;                                       // Временное хранения содержимого регистра.
    for (int i = 0; i < _step_mem;i++)                    
		{
			_u_konnekt = pgm_read_word_near(connektN4_default+i);
			i2c_eeprom_write_byte(deviceaddress,i+adr_memN4_1, _u_konnekt); 
		}
	regBank.set(adr_control_command,0);                        // Завершить программу    
	delay(100);
}

void mem_byte_trans_read()                                      //  Чтение таблиц из EEPROM для передачи в ПК
{
	unsigned int _adr_reg = regBank.get(40005)+40000;           //  Адрес блока регистров для передачи в ПК таблиц.
	unsigned int _adr_mem = regBank.get(40006);                 //  Адрес блока памяти для передачи в ПК таблиц.
	unsigned int _size_block = regBank.get(40007);              //  Адрес длины блока таблиц

	for (unsigned int x_mem = 0;x_mem < _size_block;x_mem++)
	{
		regBank.set(_adr_reg+x_mem,i2c_eeprom_read_byte(deviceaddress,_adr_mem + x_mem));
	}
	regBank.set(adr_control_command,0);                         // Завершить программу    
	delay(200);
}
void mem_byte_trans_save()                                      //  Получить таблицу из ПK и записать в EEPROM
{
	unsigned int _adr_reg = regBank.get(40005);                 //  Адрес блока регистров для передачи в ПК таблиц.
	unsigned int _adr_mem = regBank.get(40006);                 //  Адрес блока памяти для передачи в ПК таблиц.
	unsigned int _size_block = regBank.get(40007);              //  Адрес длины блока таблиц

	for (unsigned int x_mem = 0;x_mem < _size_block;x_mem++)
	{
		i2c_eeprom_write_byte(deviceaddress, _adr_mem + x_mem, regBank.get(_adr_reg+x_mem));
	}
	regBank.set(adr_control_command,0);                         // Завершить программу    
	delay(200);
}

void setup_mcp()
{
	// Настройка расширителя портов
 
  mcp_Out1.begin(1);                               //  Адрес (4) второго  расширителя портов
  mcp_Out1.pinMode(0, OUTPUT);                     //  1A1
  mcp_Out1.pinMode(1, OUTPUT);                     //  1B1  
  mcp_Out1.pinMode(2, OUTPUT);                     //  1C1
  mcp_Out1.pinMode(3, OUTPUT);                     //  1D1  
  mcp_Out1.pinMode(4, OUTPUT);                     //  1A2
  mcp_Out1.pinMode(5, OUTPUT);                     //  1B2
  mcp_Out1.pinMode(6, OUTPUT);                     //  1C2
  mcp_Out1.pinMode(7, OUTPUT);                     //  1D2
  
  mcp_Out1.pinMode(8, OUTPUT);                     //  1E1   U13
  mcp_Out1.pinMode(9, OUTPUT);                     //  1E2   U17
  mcp_Out1.pinMode(10, OUTPUT);                    //  1E3   U23
  mcp_Out1.pinMode(11, OUTPUT);                    //  1E4   U14
  mcp_Out1.pinMode(12, OUTPUT);                    //  1E5   U19
  mcp_Out1.pinMode(13, OUTPUT);                    //  1E6   U21 
  mcp_Out1.pinMode(14, OUTPUT);                    //  1E7   Свободен  
  mcp_Out1.pinMode(15, OUTPUT);                    //  1E8   Свободен

	
  mcp_Out2.begin(2);                               //  
  mcp_Out2.pinMode(0, OUTPUT);                     //  2A1  
  mcp_Out2.pinMode(1, OUTPUT);                     //  2B1  
  mcp_Out2.pinMode(2, OUTPUT);                     //  2C1
  mcp_Out2.pinMode(3, OUTPUT);                     //  2D1  
  mcp_Out2.pinMode(4, OUTPUT);                     //  2A2
  mcp_Out2.pinMode(5, OUTPUT);                     //  2B2
  mcp_Out2.pinMode(6, OUTPUT);                     //  2C2
  mcp_Out2.pinMode(7, OUTPUT);                     //  2D2
  
  mcp_Out2.pinMode(8, OUTPUT);                     //  2E1   U15
  mcp_Out2.pinMode(9, OUTPUT);                     //  2E2   U18
  mcp_Out2.pinMode(10, OUTPUT);                    //  2E3   U22
  mcp_Out2.pinMode(11, OUTPUT);                    //  2E4   U16
  mcp_Out2.pinMode(12, OUTPUT);                    //  2E5   U20     
  mcp_Out2.pinMode(13, OUTPUT);                    //  2E6   U24       
  mcp_Out2.pinMode(14, OUTPUT);                    //  2E7   Реле №1, №2
  mcp_Out2.pinMode(15, OUTPUT);                    //  2E8   Свободен
  for(int i=0;i<16;i++)
  {
	  mcp_Out1.digitalWrite(i, LOW); 
	  mcp_Out2.digitalWrite(i, LOW); 
  }
}
void setup_port()
{
	pinMode(ledPin13, OUTPUT);   
	pinMode(ledPin12, OUTPUT);  
	digitalWrite(ledPin12, LOW);                   // 
	digitalWrite(ledPin13, LOW);                   // 
	pinMode(kn1Nano, OUTPUT);  
	pinMode(kn2Nano, OUTPUT);  
	pinMode(kn3Nano, OUTPUT);  
	pinMode(kn4Nano, OUTPUT);  
	pinMode(kn5Nano, OUTPUT);  
	pinMode(kn6Nano, OUTPUT);  

	digitalWrite(kn1Nano, HIGH);                        // 
	digitalWrite(kn2Nano, HIGH);                        //
	digitalWrite(kn3Nano, HIGH);                        //
	digitalWrite(kn4Nano, LOW);                         // 
	digitalWrite(kn5Nano, HIGH);                        // 
	digitalWrite(kn6Nano, HIGH);                        //
}
void test_cabel_N1()
{

}
void test_cabel_N2()
{

}
void test_cabel_N3()
{

}
void test_cabel_N4()
{

}
void test_panel_N1()
{

}

void setup_regModbus()
{
    regBank.setId(1);    // Slave ID 1

  	regBank.add(1);      //  
	regBank.add(2);      //  
	regBank.add(3);      //  
	regBank.add(4);      //  
	regBank.add(5);      //  
	regBank.add(6);      //  
	regBank.add(7);      //  
	regBank.add(8);      //  

	regBank.add(10001);  //  
	regBank.add(10002);  //  
	regBank.add(10003);  //  
	regBank.add(10004);  //  
	regBank.add(10005);  //  
	regBank.add(10006);  //  
	regBank.add(10007);  //  
	regBank.add(10008);  //  

	regBank.add(30001);  //  
	regBank.add(30002);  //  
	regBank.add(30003);  //  
	regBank.add(30004);  //  
	regBank.add(30005);  //  
	regBank.add(30006);  //  
	regBank.add(30007);  //  
	regBank.add(30008);  //  

	regBank.add(40001);  //  Адрес передачи комманд на выполнение 
	regBank.add(40002);  //  Адрес счетчика всех ошибок
	regBank.add(40003);  //  Адрес хранения величины сигнала резистором № 1
	regBank.add(40004);  //  Адрес хранения величины сигнала резистором № 2
	regBank.add(40005);  //  Адрес блока регистров для передачи в ПК таблиц.
	regBank.add(40006);  //  Адрес блока памяти для передачи в ПК таблиц.
	regBank.add(40007);  //  Адрес длины блока таблиц
	regBank.add(40008);  //  
	regBank.add(40009);  //  

	regBank.add(40010);  //  Регистры временного хранения для передачи таблицы
	regBank.add(40011);   
	regBank.add(40012);   
	regBank.add(40013);   
	regBank.add(40014);    
	regBank.add(40015);   
	regBank.add(40016);    
	regBank.add(40017);   
	regBank.add(40018);     
	regBank.add(40019);   

	regBank.add(40020);                            
	regBank.add(40021);   
	regBank.add(40022);   
	regBank.add(40023);   
	regBank.add(40024);    
	regBank.add(40025);   
	regBank.add(40026);    
	regBank.add(40027);   
	regBank.add(40028);     
	regBank.add(40029); 

	regBank.add(40030);                            
	regBank.add(40031);   
	regBank.add(40032);   
	regBank.add(40033);   
	regBank.add(40034);    
	regBank.add(40035);   
	regBank.add(40036);    
	regBank.add(40037);   
	regBank.add(40038);     
	regBank.add(40039); 

	regBank.add(40040);                            
	regBank.add(40041);   
	regBank.add(40042);   
	regBank.add(40043);   
	regBank.add(40044);    
	regBank.add(40045);   
	regBank.add(40046);    
	regBank.add(40047);   
	regBank.add(40048);     
	regBank.add(40049); 


						 // Текущее время 
	regBank.add(40050);  // адрес день модуля часов контроллера
	regBank.add(40051);  // адрес месяц модуля часов контроллера
	regBank.add(40052);  // адрес год модуля часов контроллера
	regBank.add(40053);  // адрес час модуля часов контроллера
	regBank.add(40054);  // адрес минута модуля часов контроллера
	regBank.add(40055);  // адрес секунда модуля часов контроллера
 
						 // Установка времени в контроллере
	regBank.add(40056);  // адрес день
	regBank.add(40057);  // адрес месяц
	regBank.add(40058);  // адрес год
	regBank.add(40059);  // адрес час
	regBank.add(40060);  // адрес минута
	regBank.add(40061);  // 
	regBank.add(40062);  // 
	regBank.add(40063);  // 

}

void setup()
{
	Serial.begin(9600);                                    // Подключение к USB ПК
	Serial1.begin(115200);                                 // Подключение к 
	slave.setSerial(3,57600);                              // Подключение к протоколу MODBUS компьютера Serial3 
	Serial2.begin(115200);                                 // Подключение к 
	Wire.begin();
	if (!RTC.begin())                                      // Настройка часов 
		{
			Serial.println("RTC failed");
			while(1);
		};
	//DateTime set_time = DateTime(16, 3, 15, 10, 19, 0);  // Занести данные о времени в строку "set_time" год, месяц, число, время...
	//RTC.adjust(set_time);                                // Записать дату
	Serial.println(" ");
	Serial.println(" ***** Start system  *****");
	Serial.println(" ");
	//set_time();
	serial_print_date();
	pinMode(ledPin13, OUTPUT);   
	Wire.begin();
	setup_port();
	setup_mcp();                                          // Настроить порты расширения  
	setup_resistor();                                     // Начальные установки резистора
	MsTimer2::set(300, flash_time);                       // 300ms период таймера прерывани
	resistor(1, 200);                                     // Установить уровень сигнала
	resistor(2, 200);                                     // Установить уровень сигнала
	setup_regModbus();
	myGLCD.InitLCD();
	myGLCD.clrScr();
	myGLCD.setFont(BigFont);
	myTouch.InitTouch();
	// myTouch.setPrecision(PREC_MEDIUM);
	myTouch.setPrecision(PREC_HI);
	myButtons.setTextFont(BigFont);
	myButtons.setSymbolFont(Dingbats1_XL);
    

	draw_Glav_Menu();
	wait_time_Old =  millis();
	digitalWrite(ledPin13, HIGH);                       // 
	Serial.println(" ");                                //
	Serial.println("System initialization OK!.");       // Информация о завершении настройки
}

void loop()
{
	//draw_Glav_Menu();
	swichMenu();
	//delay(1000);
}
