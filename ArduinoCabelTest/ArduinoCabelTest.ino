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
UTFT          myGLCD(ITDB32S,38,39,40,41);
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



//------------------------------------------------------------------------------

//Назначение переменных для хранения № опций меню (клавиш)
int but1, but2, but3, but4, but5, but6, but7, but8, but9, but10, butX, butY, butA, butB, butC, butD, but_m1, but_m2, but_m3, but_m4, but_m5, pressed_button;
 //int kbut1, kbut2, kbut3, kbut4, kbut5, kbut6, kbut7, kbut8, kbut9, kbut0, kbut_save,kbut_clear, kbut_exit;
 //int kbutA, kbutB, kbutC, kbutD, kbutE, kbutF;
 int m2 = 1; // Переменная номера меню

 //------------------------------------------------------------------------------------------------------------------
 // Назначение переменных для хранения текстов

 char  txt_menu1_1[] = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N 1";// Тест кабель N 1
 char  txt_menu1_2[] = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N 2";// Тест кабель N 2
 char  txt_menu1_3[] = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N 3";// Тест кабель N 3
 char  txt_menu1_4[] = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N 4";// Тест кабель N 4

 char  txt_menu2_1[] = "menu2_1"; // //ИНФО СЧЕТЧИКОВ
 char  txt_menu2_2[] = "menu2_2"; //
 char  txt_menu2_3[] = "menu2_3"; //
 char  txt_menu2_4[] = "menu2_4";//
 char  txt_menu3_1[] = "CTEPET\x92 \x8B""A\x87\x89\x91";//
 char  txt_menu3_2[] = "\x8A""c\xA4.N ""\xA4""e\xA0""e\xA5o\xA2""a";// Уст. № телефона
 char  txt_menu3_3[] = "\x8A""c\xA4.Level Gaz";//
 char  txt_menu3_4[] = "\x8A""c\xA4.Level Temp";//
 char  txt_menu4_1[] = "C\x9D\xA2yco\x9D\x99""a";                          // Синусоида
 char  txt_menu4_2[] = "Tpey\x98o\xA0\xAC\xA2\xAB\x9E";                    // Треугольный
 char  txt_menu4_3[] = "\x89\x9D\xA0oo\x96pa\x9C\xA2\xAB\x9E";             // Пилообразный
 char  txt_menu4_4[] = "\x89p\xAF\xA1oy\x98o\xA0\xAC\xA2\xAB\x9E";         // Прямоугольный
 char  txt_menu5_1[] = "";// Инфо ZigBee
 char  txt_menu5_2[] = "Set Adr Coord H";//
 char  txt_menu5_3[] = "Set Adr Coord L";// 
 char  txt_menu5_4[] = "Set Adr Network";// 

 char  txt_pass_ok[] = "Tec\xA4 OK!"; // Тест ОК!
 char  txt_pass_no[] = "Tec\xA4 NO!"; // Тест NO!

 
 char  txt_info1[] = "Tec\xA4 ""\x9F""a\x96""e\xA0""e\x9E";                // Тест кабелей
 char  txt_info2[] = "Tec\xA4 \x96\xA0o\x9F""a \x98""ap\xA2\x9D\xA4yp";    // Тест блока гарнитур
 char  txt_info3[] = "Hac\xA4po\x9E\x9F""a c\x9D""c\xA4""e\xA1\xAB";       // Настройка системы
 char  txt_info4[] = "\x81""e\xA2""epa\xA4op c\x9D\x98\xA2""a\xA0o\x97";   // Генератор сигналов
 char  txt_info5[] = "Oc\xA6\x9D\xA0\xA0o\x98pa\xA5";                      // Осциллограф





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

// Выбор Меню Тексты меню в строках "txt....."
void swichMenu() // Тексты меню в строках "txt....."
	
{
	 m2=1;                           // Устанивить первую странице меню
	 while(1) 
	   {
		  myButtons.setTextFont(BigFont);    // Установить Большой шрифт кнопок  

			if (myTouch.dataAvailable() == true) // Проверить нажатие кнопок
			  {
			    pressed_button = myButtons.checkButtons(); // Если нажата - проверить что нажато
					 if (pressed_button==butX) // Нажата вызов часы
					      {  
							// AnalogClock();
							 myGLCD.clrScr();
							 myButtons.drawButtons(); // Восстановить кнопки
							 print_up();              // Восстановить верхнюю строку
					      }
		 
					 if (pressed_button==but_m1) // Нажата 1 страница меню
						  {
							  myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_BLUE); // Голубой фон меню
							  myButtons.drawButtons();   // Восстановить кнопки
							  default_colors=true;
							  m2=1;                                                // Устанивить первую странице меню
							  myButtons.relabelButton(but1, txt_menu1_1, m2 == 1);
							  myButtons.relabelButton(but2, txt_menu1_2, m2 == 1);
							  myButtons.relabelButton(but3, txt_menu1_3, m2 == 1);
							  myButtons.relabelButton(but4, txt_menu1_4, m2 == 1);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info1, CENTER, 0);            // "Ввод данных"
		
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
							  myGLCD.print(txt_info2, CENTER, 0);            // Информация
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
							  myGLCD.print(txt_info3, CENTER, 0);            // Информация
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
							  myGLCD.print(txt_info4, CENTER, 0);            // 
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
							  myGLCD.print(txt_info5, CENTER, 0);            // 
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
				   if (pressed_button==but4 && m2 == 3) // Четвертый пункт меню 3
				      {
				
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
		    				//set_warm_temp();
						
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
	pinMode(A0, OUTPUT);  
	pinMode(A1, OUTPUT);  
	pinMode(A2, OUTPUT);  
	pinMode(A3, OUTPUT);  
	digitalWrite(A0, HIGH);                        // 
	digitalWrite(A1, HIGH);                        //
	digitalWrite(A2, HIGH);                        //
	digitalWrite(A3, LOW);                         // 

}

void setup()
{
	Serial.begin(9600);                            // Подключение к USB ПК
	Serial1.begin(115200);                         // Подключение к звуковому модулю 
	slave.setSerial(3,57600);                      // Подключение к протоколу MODBUS компьютера Serial3 
	Serial2.begin(115200);                         // 
	Serial.println(" ");
	Serial.println(" ***** Start system  *****");
	Serial.println(" ");
	Wire.begin();
	setup_port();
	setup_mcp();                                   // Настроить порты расширения  
     myGLCD.InitLCD();
	  myGLCD.clrScr();
	  myGLCD.setFont(BigFont);
	  myTouch.InitTouch();
	 // myTouch.setPrecision(PREC_MEDIUM);
	  myTouch.setPrecision(PREC_HI);
 	  myButtons.setTextFont(BigFont);
	  myButtons.setSymbolFont(Dingbats1_XL);
    

	draw_Glav_Menu();

	digitalWrite(ledPin13, HIGH);                  // включение подтягивающего резистора
	Serial.println(" ");                           //
	Serial.println("System initialization OK!.");  // Информация о завершении настройки
}

void loop()
{
 //draw_Glav_Menu();
  swichMenu();
	//delay(1000);
}
