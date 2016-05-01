
#include <LiquidCrystal.h>;
#include <EEPROM.h>

LiquidCrystal lcd(8, 7, 5, 4, 3, 2); //rs, e, d4, d5, d6, d7

const int down              = 10;              // выход валкодера 1/2 В
const int up                = 11;              // выход валкодера 2/2 А
const int pwm1              = 9;               // выход ШИМ1
const int pwm2              = 6;               // выход ШИМ2
int mig                     = 0;               // Для енкодера (0 стоим 1 плюс 2 минус)

#define kn_menu       12                       // Назначение кнопки "Меню"
#define kn_selection  A5                       // Назначение кнопки "Выбор"
#define kn_pwm        A4                       // Назначение кнопки "ШИМ"

boolean pwm1_2              = true;            // Переключатель true ШИМ 1 / false ШИМ 2
int counter1                = 5;               // переменная хранит заданное напряжение ШИМ1
int counter2                = 5;               // переменная хранит заданное напряжение ШИМ2

int set                     = 0;               // пункты меню, 


void EEPROM_float_write(int addr, float val)              // Программа записи в ЕЕPROM
{
  byte *x = (byte *)&val;
  for (byte i = 0; i < 4; i++) EEPROM.write(i + addr, x[i]);
}
float EEPROM_float_read(int addr)                         // Программа чтения из ЕЕPROM
{
  byte x[4];
  for (byte i = 0; i < 4; i++) x[i] = EEPROM.read(i + addr);
  float *y = (float *)&x;
  return y[0];
}
void valcoder_set()
{
  boolean regup   = digitalRead(up);
  boolean regdown = digitalRead(down);

  if (regup < regdown) mig = 1;                            // крутится в сторону увеличения
  if (regup > regdown) mig = 2;                            // крутится в сторону уменшения
  if (!regup & !regdown)                                   // момент для переключения
  {
    if (mig == 1) uup();                                   //+
    if (mig == 2) udn();                                   //-
    mig = 0;                                               // сбрасываем указатель направления
  }
}

//функции при вращении енкодера
void uup()                                                 //энкодер +
{
    if (pwm1_2)                                            // Если выбран ШИМ1. Код регулирования возможен в диапазоне 0-255    
    {
		counter1++;                                        // Код регулирования возможен в диапазоне 0-255    
       if(counter1 > 255) counter1 = 255;                  // Ограничить счетчик 255, больше нельзя.
       analogWrite(pwm1, counter1);                        // Записать код на выход ШИМ1
	}
    else                                                   // Иначе выбран ШИМ2. Код регулирования возможен в диапазоне 0-255    
    {
       counter2++;                                         // Код регулирования возможен в диапазоне 0-255                                            
       if(counter2 > 255) counter2 = 255;                  // Ограничить счетчик 255, больше нельзя. 
	   analogWrite(pwm2, counter1);                        // Записать код на выход ШИМ1
    }
}
void udn()                                                 //валкодер -
{
   if (pwm1_2)                                             // Если выбран ШИМ1. Код регулирования возможен в диапазоне 0-255  
    {
		counter1--;                                        // Код регулирования возможен в диапазоне 0-255    
       if(counter1 < 0) counter1 = 0;                      // Ограничить счетчик 0, меньше нельзя.
       analogWrite(pwm1, counter1);                        // Записать код на выход ШИМ1
    }
    else                                                   // Иначе выбран ШИМ2. Код регулирования возможен в диапазоне 0-255  
    {
       counter2--;                                         // Код регулирования возможен в диапазоне 0-255                                            
       if(counter2 < 0) counter2 = 0;                      //  Ограничить счетчик 0, меньше нельзя. 
	   analogWrite(pwm2, counter1);                        // Записать код на выход ШИМ1
    }
}
void control_buttons()
{
	 if (digitalRead(kn_menu) == LOW )                     // Переключение режимов меню кнопкой "меню"
	  {
		while (digitalRead(kn_menu) == LOW ) {}            // Ожидаем отпускания кнопки "меню"
		set++;                                             // Следующий пункт меню 
		if (set > 3)  set = 0;                             // Максимальный пункт меню
		lcd.setCursor (0, 0);
		lcd.print("            ");    
	  }
	 if (digitalRead(kn_pwm) == LOW && set == 1)           // Установить канал ШИМ если выбран второй пункт меню
	  {
        while (digitalRead(kn_pwm) == LOW ) {}             // Ожидаем отпускания кнопки ШИМ
 		pwm1_2 = !pwm1_2;                                  // Переключить ШИМ 1/2 ( pwm1_2== true -  ШИМ1,   pwm1_2== false -  ШИМ2)
	  }
}

void display_print()
{
	if (set == 0)                                         //стандартный экран,  выводим установленное напряжение на дисплей
	{
		lcd.setCursor (0, 1);
		lcd.print("U>");
		if (counter1 < 10) lcd.print(" ");                // добавляем пробел, если нужно, чтобы не портить картинку
		lcd.print (counter1, 1);                          // выводим установленное значение напряжения
		lcd.print ("V ");                                 // пишем что это вольты
		lcd.print("          ");    
	}
	if (set == 1)                                         //
	{
		lcd.setCursor (0, 0);
		lcd.print("> MENU 1/3    ");
		lcd.setCursor (0, 1);

		if (pwm1_2)                                                // Если выбран ШИМ1 - регулируем по старой схеме
		{
			lcd.print ("chanal pwm1:");
			lcd.print (counter1, 1);
			lcd.print ("  ");
		}
		else
		{
 			lcd.print ("chanal pwm2:");
			lcd.print (counter2, 1);
			lcd.print ("  ");
		}
	}

  if (set == 2)                                   //спрашиваем хочет ли юзер сбросить настройки
  {
    lcd.setCursor (0, 0);
    lcd.print("> MENU 2/3      ");
    lcd.setCursor (0, 1);
    lcd.print("Reset options?->");
  }

  if (set == 3)                                  //спрашиваем хочет ли юзер сохранить настройки
  {
    lcd.setCursor (0, 0);
    lcd.print("> MENU 3/3      ");
    lcd.setCursor (0, 1);
    lcd.print("Save options? ->");
  }

}

void setup()
{
  Serial.begin(9600);                      // Установить скорость серийного порта 9600

  pinMode(pwm1, OUTPUT);                   // На выход 9
  pinMode(pwm2, OUTPUT);                   // На выход 6
  pinMode(down, INPUT);
  pinMode(up, INPUT);
 
  pinMode(kn_menu, INPUT);
  pinMode(kn_selection, INPUT);
  pinMode(kn_pwm, INPUT);

  digitalWrite(up, HIGH);                  // поддерживаем высокий уровень на входах от валкодера
  digitalWrite(down, HIGH);
 
  digitalWrite(kn_menu, HIGH);             //поддерживаем высокий уровень на контактах кнопок
  digitalWrite(kn_selection, HIGH);
  digitalWrite(kn_pwm, HIGH);

  //запуск дисплея
  lcd.begin(16, 2);                        // Дисплей 16 символов, 2 строки
  lcd.print("    WELCOME!    ");
}

void loop()
{
 valcoder_set();
 control_buttons();
 display_print();
 delay(100);
}
