
/*
Правила хорошего тона при программировании предусматривают сначала указание библиотек
и точко затем назначение переменных. :-)
Это позволит избежать возможных ошибок, которые трудно найти.

Частота ШИМ 244 Гц   Перестройка 0 - 65535
*/

#include <EEPROM.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal.h>;
#include <LiquidCrystal_I2C.h>
//#include <MsTimer2.h>             // Подключаем таймер для опроса валкодера  

            

#define DHTPIN A3                 // Датчик DHT11 подключен к цифровому пину номер A3
#define DHTTYPE DHT11             // DHT 11 
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27,16,2);
// LiquidCrystal lcd(8, 7, 5, 4, 3, 2); //rs, e, d4, d5, d6, d7
 
//--------------------------------------------------------------------------------
                                                 /* ЗАДАЁМ КОНСТАНТЫ */

float umax          = 20.00;                     // максимальное напряжение
float umin          = 0.00;                      // минимальное напряжение
float ah            = 0.0000;                    // Cчетчик Ампер*часов
float Uout          = 0;                         // напряжение на выходе
float Iout          = 0;
float Uout_temp     = 0;
float Iout_temp     = 0;
const int down      = 6;                         // выход валкодера 1/2 В
const int up        = 11;                        // выход валкодера 2/2 А
const int pwm1      = 9;                         // выход ШИМ 1 !! Внимание ** Вывода ШИМ изменять нельзя
const int pwm2      = 10;                        // выход ШИМ 2 !! Внимание ** Вывода ШИМ изменять нельзя
const int power     = 13;                        // управление релюхой 
long previousMillis = 0;                         // храним время последнего обновления дисплея
long maxpwm         = 0;                         // циклы поддержки максимального ШИМ
long interval       = 300;                       // интервал обновления информации на дисплее, мс
int mig             = 0;                         // Для енкодера (0 стоим 1 плюс 2 минус)
volatile unsigned int  level1  = 65535;          // Максимальный "уровень" ШИМ сигнала.  (volatile) указываем что переменная работает с прерываниями
volatile unsigned int  level2  = 65535;          // Максимальный "уровень" ШИМ сигнала.  (volatile) указываем что переменная работает с прерываниями
volatile unsigned int  koef_level1  = 50;        // Коэффициент перестройки ШИМ1 сигнала.  
volatile unsigned int  koef_level2  = 50;        // Коэффициент перестройки ШИМ2 сигнала.  
float com           = 300;
long com2           = 0;
int mode            = 0;                        // режим (0 стабилизация напряжения, 1 стабилизация тока)
volatile float Ioutmax       = 1.0;             // заданный ток.  (volatile) указываем что переменная работает с прерываниями
int set             = 0;                        // пункты меню, отображение защиты...
int knopka_a        = 0;                        // состояние кнопок
int knopka_b        = 0;
int knopka_c        = 0;
int knopka_abc      = 0;
boolean off         = false;
boolean red         = false;
boolean blue        = false;
float counter       = 0;                       // переменная хранит заданное напряжение
int disp            = 0;                       // режим отображения 0 ничего, 1 мощьность, 2 режим, 3 установленный ток, 4 шим уровень
int count_measure   = 0;                       // Счетчик количества измерения для усреднения показателей.
int max_count       = 10;                      // Максимальное количество измерений 
#define kn_menu       12                       // Назначение кнопки "Меню"
#define kn_selection  7                        // Назначение кнопки "Выбор"
#define kn_pwm        A6                       // Назначение кнопки "ВКЛ/Выкл.
int incomingByte;

//++++++++ Новый энкодер +++++++++++++++++++++++++++++++
int encoderPin1 = 2;
int encoderPin2 = 3;

volatile int lastEncoded = 0;
volatile int encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;
//------------------------------------------------------


//--------------------------------------------------------------------------------
/*
void flash_time()  // Программа обработчик прерывания для опроса состояния валкодера
{ 
  // считываем значения с входа валкодера
  boolean regup   = digitalRead(up);
  boolean regdown = digitalRead(down);

  if(regup<regdown) mig = 1;       // крутится в сторону увеличения
  if(regup>regdown) mig = 2;       // крутится в сторону уменшения
  if(!regup & !regdown)            // момент для переключения
  { 
    if(mig==1) uup();//+
    if(mig==2) udn(); //-
    mig = 0;                       // сбрасываем указатель направления
  }
}
*/
//--------------------------------------------------------------------------------
void EEPROM_float_write(int addr, float val)              // Программа записи в ЕЕPROM
{
  byte *x = (byte *)&val;
  for (byte i = 0; i < 4; i++) EEPROM.write(i + addr, x[i]);
}
//--------------------------------------------------------------------------------
float EEPROM_float_read(int addr)                         // Программа чтения из ЕЕPROM
{
  byte x[4];
  for (byte i = 0; i < 4; i++) x[i] = EEPROM.read(i + addr);
  float *y = (float *)&x;
  return y[0];
}                                 
//--------------------------------------------------------------------------------
                                  /* ФУНКЦИИ ПРИ ВРАЩЕНИИ ЕНКОДЕРА */

void uup()   //енкодер +
{ 
  if(set==0)                      // обычный режим - добавляем напряжения ШИМ1
  {                                
    if(mode == 0)
    {
     level1+=koef_level1;
     if(level1 > 65535) level1 = 65535;
    }

  if(mode == 1)                   // обычный режим - добавляем напряжения ШИМ2
  {               
  level2+=koef_level2;
  if(level2 > 65535) level2 = 65535;
  }
  }
  if(set==1)
  {                                //переключаем режим работы вперед
  mode++;
  if(mode>1) mode=1; 
  }
  if(set==2)
  {                                // настройка тока, добавляем ток
   iplus();
  }
  if(set==3)
  {                                // сброс счетчика А*ч
    ah = 0;
    set = 0;
    disp = 6;
  }
//  if(set==4)
//  {                                // сохранение текущих настроек в память
//   save();
//  }
}
//--------------------------------------------------------------------------------
void udn()                         // валкодер -
{ //валкодер -
  if(set==0)
  {
  if(mode == 0)
  {
    level1-=koef_level1;
    if(level1 < 0) level1 = 0;
  }
  if(mode == 1)
  {
    level2-=koef_level1;
    if(level2 < 0) level2 = 0;
  }

  }

  if(set==1)
  {
   mode--;                         // переключаем режим работы назад
   if(mode<0) mode=0;
  }  

  if(set==2)
  {                                // убавляем ток
   iminus();
  }
}
//--------------------------------------------------------------------------------
void iplus()
{ 
   Ioutmax = Ioutmax+0.01;
   if(Ioutmax>0.2) Ioutmax=Ioutmax+0.04;
   if(Ioutmax>1) Ioutmax=Ioutmax+0.05;
   if(Ioutmax>10.00) Ioutmax=10.00;
}

void iminus(){ 
   Ioutmax = Ioutmax-0.01;
   if(Ioutmax>0.2) Ioutmax=Ioutmax-0.04;
   if(Ioutmax>1) Ioutmax=Ioutmax-0.05;
   if(Ioutmax<0.03) Ioutmax=0.03;
}
//--------------------------------------------------------------------------------
void save()
{
    lcd.clear();                                // Очистить дисплей
    lcd.setCursor (0, 0);                       // Установить курсор в начало
    lcd.print(" S A V E  -  OK ");              // Вывести сообщение

     EEPROM_float_write(0, level1);
     EEPROM_float_write(4, Ioutmax);
     EEPROM_float_write(12, mode);
     EEPROM_float_write(10, disp);
     EEPROM_float_write(20, level2);
                                                // мигаем светодиодами
    digitalWrite(A2, HIGH);                     // Включить красный светодиод
    digitalWrite(A3,HIGH);                      // Включить синий светодиод
    delay(500);                                 // Ждем 0,5 секунды
    digitalWrite(A2,LOW);                       // Отключить красный светодиод
    digitalWrite(A3,LOW);                       // Отключить синий светодиод
    set = 0;                                    // выходим из меню
    delay(100);                                 // Ждем 0,1 секунды
}
//++++++++++++++++ Программа обработки энкодера по аппаратному прерыванию +++++++++++++++++++++++++
void updateEncoder()
{
  int MSB = digitalRead(encoderPin1);
  int LSB = digitalRead(encoderPin2);

  int encoded = (MSB << 1) |LSB;
  int sum  = (lastEncoded << 2) | encoded;

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) 
  {
    encoderValue ++;
    uup();
    
    }
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
  {
    
    encoderValue --;
    udn();
    }

  lastEncoded = encoded;
}


//--------------------------------------------------------------------------------
                                     /* ОСНОВНОЙ ЦИКЛ РАБОТЫ МК */
void setup()                         
{ 
  //++++++++ настройка двух ШИМ в 16 битном режиме. Максимальное число 65535 (+5в) ++++++++++
  cli();
  DDRB |= 1<<1 | 1<<2;         
  PORTB &= ~(1<<1 | 1<<2);
  TCCR1A = 0b00000010; 
  //TCCR1A = 0b10100010;  
  TCCR1B = 0b00011001;  
  ICR1H = 255;
  ICR1L = 255;
  sei(); 
//--------------------------------------------------------------------------------
  Serial.begin(9600);                           // Установить скорость серийного порта 9600

  pinMode(pwm1, OUTPUT);                        // выход ШИМ1 на 9
  pinMode(pwm2, OUTPUT);                        // выход ШИМ2 на 10
  pinMode(down, INPUT);                         // выход валкодера 1/2 В
  pinMode(up, INPUT);                           // выход валкодера 2/2 А
  pinMode(kn_menu, INPUT);                      // вход кнопки "Меню"
  pinMode(kn_selection, INPUT);                 // вход кнопки "Выбор"
  pinMode(kn_pwm, INPUT);                       // вход кнопки "ШИМ"
  pinMode(power, OUTPUT);                       // выход на реле
  pinMode(A2, OUTPUT);                          // выход светодиода "RED"
  pinMode(A3, OUTPUT);                          // выход светодиода "BLUE"
                                                // поддерживаем высокий уровень на входах от валкодера
  digitalWrite(up, HIGH);                       // выход валкодера 1/2 А
  digitalWrite(down, HIGH);                     // выход валкодера 2/2 В
                                                // поддерживаем высокий уровень на контактах кнопок
  digitalWrite(kn_menu, HIGH);                  // Назначение кнопки "Меню"
  digitalWrite(kn_selection, HIGH);             // Назначение кнопки "Выбор"
  digitalWrite(kn_pwm, HIGH);                   // Назначение кнопки "ШИМ"
//--------------------------------------------------------------------------------
                                    /* ЗАПУСК ДИСПЛЕЯ */
                                    
  lcd.begin(16, 2);                // Дисплей 16 символов, 2 строки
  lcd.init();                      // Для <LiquidCrystal_I2C.h>
  lcd.backlight();                 // Для <LiquidCrystal_I2C.h>
  lcd.print("    WELCOME!    ");
  delay(100);
  
//--------------------------------------------------------------------------------
                                    /* ЗАГРУЖАЕМ НАСТРОЙКИ ИЗ ПАМЯТИ МК */
         
  level1  = EEPROM_float_read(0);
  Ioutmax = EEPROM_float_read(4);
  mode    = EEPROM_float_read(12);
  disp    = EEPROM_float_read(10);
  level2  = EEPROM_float_read(20);
  
                         //Если в памяти еще нет настроек - задаем что нибудь кроме нулей
                        
  if(level1==0) counter = 35768; // 2,5 вольт
  if(level2==0) counter = 35768; // 2,5 вольт
  if(Ioutmax==0) Ioutmax = 2;    // 2 ампера                                               
                                                                      
 // MsTimer2::set(20, flash_time);     // Настраиваем таймер 20 ms период таймера прерывания для опроса валкодера (время необходимо подстроить по скорости валкодера) !!
 // MsTimer2::start();                 // Запускаем таймер опроса валкодера каждые 20 мс  !! Рекомендую не уменьшать по возможности.
//++++++++++++++++++++ Настройка нового энкодера +++++++++++++++++++
  pinMode(encoderPin1, INPUT); 
  pinMode(encoderPin2, INPUT);
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);
//---------------------------------------------------------------
 
  digitalWrite(power, HIGH);           // включаем реле
}
//--------------------------------------------------------------------------------

                                      /* ОСНОВНОЙ ЦИКЛ РАБОТЫ МК */
void loop()                                   
{
  int t = dht.readTemperature();  
  int h = dht.readHumidity();
  unsigned long currentMillis = millis(); 
  
//--------------------------------------------------------------------------------  
                                      /* ВНЕШНЕЕ УПРАВЛЕНИЕ */
                                        
  if (Serial.available() > 0) 
  {                                     //если есть доступные данные                                 
  incomingByte = Serial.read();         // считываем байт
  }
  else
  {
  incomingByte = 0;
  }
  if(incomingByte==97)  // a
  {   
  if(counter>umin+0.1)counter = counter-0.1;   // убавляем напряжение
  }   
  if(incomingByte==98)  // b
  {   
  if(counter<umax)    counter = counter+0.1;   // добавляем напряжение
  }
  if(incomingByte==99)  // c 
  {    
  iminus();
  }
  if(incomingByte==100) // d
  {  
  iplus();
  }
    
  if(incomingByte==101) mode = 0;
  if(incomingByte==102) mode = 1; 
  if(incomingByte==103) mode = 2;
  if(incomingByte==104) save();
  if(incomingByte==105)
  {
    digitalWrite(power,HIGH);    // врубаем реле если оно было выключено
    delay(100);
    digitalWrite(A2,LOW);        // гасим красный светодиод
    Serial.print(0);
    Serial.print(';');
    off = false;
    set = 0;                     // выходим из меню
    lcd.clear();                 // чистим дисплей
  }
   if(incomingByte==106) off = true;
   if(incomingByte==107) ah = 0;
                                  /* конец внешнего управления */
   
//--------------------------------------------------------------------------------    
                                  /* ЗНАЧЕНИЕ НАПРЯЖЕНИЯ И ТОКА В НАГРУЗКЕ */   
                                  
  float Ucorr = 0.00;                                                   // коррекция напряжения, при желании можно подстроить
  Uout_temp += analogRead(A1) * ((5.0 + Ucorr) / 1023.0) * 5.0;         // узнаем напряжение на выходе 
  // Iout_temp += analogRead(A0) / 100.00;                              // узнаем ток в нагрузке  !! уточняем ток фактический
  Iout = analogRead(A0) / 100.00;                                       // узнаем ток в нагрузке  !! уточняем ток фактический. Увеличить  быстродействие
//--------------------------------------------------------------------------------                                                                       
                                  /* ВЫЧИСЛЯЕМ СРЕДНИЙ ПОКАЗАТЕЛЬ */
                                                                        
  count_measure++;                                                      // увеличиваем счетчик измерений
  if (count_measure > max_count)                                        // Проверяем достаточно проивели измерения
  { 
      Uout = Uout_temp/count_measure;                                   // Вычисляем среднее значение
      Iout = Iout_temp/count_measure;                                   // Вычисляем среднее значение
      Uout_temp = 0;
      Iout_temp = 0;
      count_measure = 0;                                                // Обнуляем счетчик измерений
  }
  
//--------------------------------------------------------------------------------
                                 /* ЗАЩИТА и выключение */
                                 
  // Уточните условие защиты, убрал первое условие "(Iout>(level1)*2.0)" . Оставил "Iout>10.1  | off"
 // if (((Iout>(level1)*2.0) | Iout>10.1  | off) & set<4 & millis()>100 ) // условия защиты
  if ( Iout>10.1  | off ) // условия защиты
   {  
    digitalWrite(power, 0);             // вырубаем реле
    level1 = 65535;                     // убираем ШИМ1 сигнал
    level2 = 65535;                     // убираем ШИМ2 сигнал
    digitalWrite(A2, 1);                // Включаем красный светодиод      
    Serial.print('I0;U0;r1;W0;');
    Serial.println(' ');
    set = 5;   
    }   
                                 /* ЗАЩИТА КОНЕЦ */
//--------------------------------------------------------------------------------
                                 /* ОПРОС ВАЛКОДЕРА */
  // Опрос валкодера перенесли в программу таймера                               
  // считываем значения с входа валкодера
/*
  
  boolean regup   = digitalRead(up);
  boolean regdown = digitalRead(down);

  if(regup<regdown) mig = 1;       // крутится в сторону увеличения
  if(regup>regdown) mig = 2;       // крутится в сторону уменшения
  if(!regup & !regdown)            // момент для переключения
  { 
    if(mig==1) uup();//+
    if(mig==2) udn(); //-
    mig = 0;                       // сбрасываем указатель направления
  }

  */
//--------------------------------------------------------------------------------

      if(off) level1 = 0;
      if(level1<0) level1 = 0;                   //не опускаем ШИМ ниже нуля
      if(level1>65535) level1 = 65535;           //не поднимаем ШИМ выше 16 бит
      if(level2<0) level2 = 0;                   //не опускаем ШИМ ниже нуля
      if(level2>65535) level2 = 65535;           //не поднимаем ШИМ выше 16 бит
                                                 // Работа с ШИМ1, ШИМ2
      level1 ? TCCR1A|=1<<7 : TCCR1A&=~(1<<7);   //подаем нужный сигнал на ШИМ1 выход
      OCR1AH = highByte(level1);                 // Записываем старший байт счетчика в ШИМ1
      OCR1AL = lowByte(level1);                  // Записываем младший байт счетчика в ШИМ1 

      level2 ? TCCR1A|=1<<5 : TCCR1A&=~(1<<5);  // подаем нужный сигнал на ШИМ2 выход
      OCR1BH = highByte(level2);                // Записываем старший байт счетчика в ШИМ2
      OCR1BL = lowByte(level2);                 // Записываем младший байт счетчика в ШИМ2

//--------------------------------------------------------------------------------
                                   /* УПРАВЛЕНИЕ */
                                   
  if (digitalRead(kn_menu)==0 && digitalRead(kn_selection)==0 && digitalRead(kn_pwm)==0 && knopka_abc==0 ) 
  {                                  // нажата ли кнопка a - б - c  вместе
  knopka_abc = 1;
  knopka_abc = 0;
  }
  if (digitalRead(kn_selection)==0 && knopka_a==0) 
  {                                  // нажата ли кнопка А (disp)
  knopka_a = 1;
  disp++;                            // поочередно переключаем режим отображения информации
  if(disp==8) disp = 0;              // дошли до конца, начинаем снова
  }
  if (digitalRead(kn_menu)==0 && knopka_b==0) 
  {                                  // нажата ли кнопка Б (menu)
  knopka_b = 1;
  set++; //
  if(set>4 | off) 
  {                                  // Задействован один из режимов защиты, а этой кнопкой мы его вырубаем. (или мы просто дошли до конца меню)
    off = false;
    digitalWrite(power, 1);        // врубаем реле если оно было выключено
    delay(100);
    digitalWrite(A2, 0);           // гасим красный светодиод 
    Serial.print('t');
    Serial.print(0);
    Serial.print(';');
    Serial.print('r');
    Serial.print(0);
    Serial.print(';');
    Serial.println(' ');
    set = 0;                       // выходим из меню
  }
  lcd.clear();                     // чистим дисплей
  }
if (digitalRead(kn_selection)==0 && set==4) 
{
   save();    // Сохранить настройки.
}

                                   //сбрасываем значения кнопок или чего-то вроде того.
  if (digitalRead(kn_menu)      == HIGH && knopka_b == 1) knopka_b = 0;
  if (digitalRead(kn_selection) == HIGH && knopka_a == 1) knopka_a = 0;
  if (digitalRead(kn_pwm)       == HIGH && knopka_c == 1) knopka_c = 0;
  
//--------------------------------------------------------------------------------
                                   /* COM PORT */

    if(currentMillis - com2 > com) 
    {                              // сохраняем время последнего обновления
    com2 = currentMillis;          // Считаем Ампер*часы
    ah = ah + (Iout / 36000);

    Serial.print('U');
    Serial.print(Uout);
    Serial.print(';');
    
    Serial.print('I');
    Serial.print(Iout);
    Serial.print(';');
    
    Serial.print('i');
    Serial.print(Ioutmax);
    Serial.print(';');
    
    Serial.print('u');
    Serial.print(counter);
    Serial.print(';');
    
    Serial.print('W');
    Serial.print(level1);
    Serial.print(';');

    Serial.print('Q');
    Serial.print(level2);
    Serial.print(';');
    
    Serial.print('c');
    Serial.print(ah);
    Serial.print(';');
    
    Serial.print('m');
    Serial.print(mode);
    Serial.print(';');
    
    Serial.print('r');
    Serial.print(digitalRead(A2));
    Serial.print(';');
    
    Serial.print('b');
    Serial.print(digitalRead(A3));
    Serial.print(';');
    
    Serial.println(' ');
    
  }  
//--------------------------------------------------------------------------------  
                                       /* ИНДИКАЦИЯ LCD */

    if(set==0)
{ 
    // выводим реальные значения на дисплей*/
  if(currentMillis - previousMillis > interval) 
  {
    // сохраняем время последнего обновления
    previousMillis = currentMillis;  
    //выводим актуальные значения напряжения и тока на дисплей
   // lcd.clear();                     // чистим дисплей !! может моргать
    lcd.setCursor (0, 0);
    lcd.print("U=");
    if(Uout<9.99) lcd.print(" ");
    lcd.print(Uout,2);
    lcd.print("V I=");
    lcd.print(Iout, 2);
    lcd.print("A ");
  
            // стандартный экран    
    lcd.setCursor (0, 1);
    {       // режим БП
      if(mode==0)lcd.print ("pwm:1   "); // Шим 1
      if(mode==1)lcd.print ("pwm:2   "); // Шим 2
    }  
    //дополнительная информация
    lcd.setCursor (7, 1);
    if(disp==0)    // отображение температуры
    {   
        lcd.print("Temp=");
        lcd.print(t);
        lcd.write(0b11011111);
        lcd.print("C");
    }    
    if(disp==1)   // мощность
    { 
      lcd.print("  ");
      lcd.print (Uout * Iout,2); 
      lcd.print("W   ");
    }  
    if(disp==2)  // режим БП
    {  
      if(mode==0)lcd.print   (" standart"); // Шим 1
      if(mode==1)lcd.print ("    drop");    // Шим 2
    }  
    if(disp==3)  // максимальный ток
    {  
       lcd.print ("  I>"); 
       lcd.print (Ioutmax, 2); 
       lcd.print ("A ");
    }
    if(disp==4)  // значение ШИМ1
    {  
      lcd.print ("pwm1:"); 
      lcd.print (ceil(level1), 0); 
      lcd.print ("   ");
    }
    if(disp==5)  // значение ШИМ2
    {  
      lcd.print ("pwm2:"); 
      lcd.print (ceil(level2), 0); 
      lcd.print ("   ");
    }  
    if(disp==6)  // Счетчик Аh
    {
    if (ah < 1)
     {
    if (ah <= 0.01) lcd.print (" ");
    if (ah <= 0.1) lcd.print ("  ");
       lcd.print (ah * 1000, 1);
       lcd.print ("mAh  ");
    }
     else
       {
    if (ah <= 10) lcd.print (" ");
       lcd.print (ah, 3);
       lcd.print ("Ah  ");
       }
      }
    if (disp==7) // Влажность
      {
       lcd.print("  Hum=");
       lcd.print(h);
       lcd.print("%");
      }
  }
}
//--------------------------------------------------------------------------------
                           /* ИНДИКАЦИЯ МЕНЮ */
                 
  if(set==1)     // выбор режима
  {
   lcd.setCursor (0, 0);
   lcd.print("> MENU 1/4    ");
   lcd.setCursor (0, 1);
   lcd.print("mode: ");
            // режим (0 спабилизация напряжения, 1 стабилизация тока)
   if(mode==0)  lcd.print("  normal >     ");
   if(mode==1)  lcd.print("< drop          ");
  }
   if(set==2)              // настройка тока
  {
   lcd.setCursor (0, 0);
   lcd.print("> MENU 2/4   ");
   lcd.setCursor (0, 1);
   lcd.print("I out max: ");
   lcd.print(Ioutmax);
   lcd.print("A");
  }
  if(set==3)               // спрашиваем хочет ли юзер сбросить A*h
  {
   lcd.setCursor (0, 0);
   lcd.print("> MENU 3/4      ");
   lcd.setCursor (0, 1);
   lcd.print("Reset A*h? ->");
  }
  if(set==4)              // спрашиваем хочет ли юзер сохранить настройки
  {
   lcd.setCursor (0, 0);
   lcd.print("> MENU 4/4      ");
   lcd.setCursor (0, 1);
   lcd.print("Save options? ->");
  }
//--------------------------------------------------------------------------------  
                          /* ИНДИКАЦИЯ ЗАЩИТЫ */
                          
  if(set==5)           //защита. вывод инфы
 {
  lcd.setCursor (0, 0);
  lcd.print("[   OVERLOAD   ]");
  lcd.setCursor (0, 1);
  lcd.print("Iout");
  lcd.print(">Imax(");
  lcd.print(Ioutmax);
  lcd.print("A)"); 
  level1=0;
  level2=0;
  Serial.print('I0;U0;r1;W0;');
  Serial.println(' ');
 }
//--------------------------------------------------------------------------------


}


