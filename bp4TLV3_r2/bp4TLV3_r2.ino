
#include <EEPROM.h>
#include <DHT.h>
#define DHTPIN A4                 // Датчик DHT11 подключен к цифровому пину номер A2
#define DHTTYPE DHT11             // DHT 11 
#include <Wire.h>
#include <LiquidCrystal.h>;
#include <MsTimer2.h>             // Подключаем таймер для опроса валкодера               



DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(8, 7, 5, 4, 3, 2); //rs, e, d4, d5, d6, d7

//  #include <LiquidCrystal_I2C.h>
//  LiquidCrystal_I2C lcd(0x27,16,2);

// задаем константы

float umax          = 20.00;                     // максимальное напряжение
float umin          = 0.00;                      // минимальное напряжение
float ah            = 0.0000;                    // Cчетчик Ампер*часов
const int down      = 6;                         // выход валкодера 1/2 В
const int up        = 11;                        // выход валкодера 2/2 А
const int pwm1      = 9;                         // выход ШИМ 1
const int pwm2      = 10;                        // выход ШИМ 2
const int power     = 13;                        // управление релюхой 
long previousMillis = 0;                         // храним время последнего обновления дисплея
long maxpwm         = 0;                         // циклы поддержки максимального ШИМ
long interval       = 500;                       // интервал обновления информации на дисплее, мс
int mig             = 0;                         // Для енкодера (0 стоим 1 плюс 2 минус)
volatile byte  level1        = 255;              // "уровень" ШИМ сигнала.  (volatile) указываем что переменная работает с прерываниями
volatile byte  level2        = 255;              // "уровень" ШИМ сигнала.  (volatile) указываем что переменная работает с прерываниями
float com           = 100;
long com2           = 0;
int mode            = 0;                        // режим (0 обычный, спабилизация тока, защита по току)
volatile float Ioutmax       = 1.0;             // заданный ток.  (volatile) указываем что переменная работает с прерываниями
int set             = 0;                        // пункты меню, отображение защиты...
int knopka_a        = 0;                        // состояние кнопок
int knopka_b        = 0;
int knopka_c        = 0;
int knopka_abc      = 0;
boolean off         = false;
boolean red         = false;
boolean blue        = false;
boolean pwm1_2      = false;                   // Переключатель ШИМ 1 / ШИМ 2
float counter       = 0;                       // переменная хранит заданное напряжение
int disp            = 0;                       // режим отображения 0 ничего, 1 мощьность, 2 режим, 3 установленный ток, 4 шим уровень
float Uout ;                                   // напряжение на выходе

#define kn_menu       12                       // Назначение кнопки "Меню"
#define kn_selection  A5                        // Назначение кнопки "Выбор"
#define kn_pwm        A6                       // Назначение кнопки "ШИМ"

int incomingByte;


void flash_time()                                              // Программа обработчик прерывания для опроса состояния валкодера
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

//функции при вращении енкодера
void uup()   //енкодер +
{ 
  if(set==0)                      // обычный режим - добавляем напряжения ШИМ1
  {                                
    if(mode == 0)
    {
     level1++;
     if(level1 > 255) level1 = 255;
    }

  if(mode == 1)                   // обычный режим - добавляем напряжения ШИМ2
  {               
  level2++;
  if(level2 > 255) level2 = 255;
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
  if(set==4)
  {                                // сохранение текущих настроек в память
   save();
  }
}

void udn()                         // валкодер -
{ //валкодер -
  if(set==0)
  {
  if(mode == 0)
  {
    level1--;
    if(level1 < 0) level1 = 0;
  }
  if(mode == 1)
  {
    level2--;
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

void save()
{
    lcd.clear();                                // Очистить дисплей
    lcd.setCursor (0, 0);                       // Установить курсор в начало
    lcd.print(" S A V E  -  OK ");              // Вывести сообщение

     EEPROM.write(12, mode);
     EEPROM.write(10, disp);
     EEPROM.write(4, Ioutmax);
     EEPROM.write(15, level1);
    // EEPROM.write(16, level2);

                                       // мигаем светодиодами
    digitalWrite(A2, HIGH);                     // Включить красный светодиод
    digitalWrite(A3,HIGH);                      // Включить синий светодиод
    delay(500);                                 // Ждем 0,5 секунды
    digitalWrite(A2,LOW);                       // Отключить красный светодиод
    digitalWrite(A3,LOW);                       // Отключить синий светодиод
    set = 0;                                    // выходим из меню
}

void setup()                            // основной цикл работы МК
{ 
  Serial.begin(9600);                   // Установить скорость серийного порта 9600

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
                                                // запуск дисплея
  lcd.begin(16, 2);                             // Дисплей 16 символов, 2 строки
  //  lcd.init(); 
  //  lcd.backlight();  
  lcd.print("    WELCOME!    ");
                                                // загружаем настройки из памяти МК
  Ioutmax = EEPROM.read(4);
  mode = EEPROM.read(12);
  disp = EEPROM.read(10);
  level1 = EEPROM.read(15);
 // level2 = EEPROM.read(16);
                                               // Если в памяти еще нет настроек - задаем что нибудь кроме нулей
  if(Ioutmax==0) Ioutmax = 2;                  //2 ампера
                                                                        
  digitalWrite(power, HIGH);                   // включаем реле

  MsTimer2::set(100, flash_time);              // Настраиваем таймер 100ms период таймера прерывания для опроса валкодера (время необходимо подстроить по скорости валкодера)
  MsTimer2::start();                           // Запускаем таймер опроса валкодера каждые 100 мс
  
}
void loop()                                   // основной цикл работы МК
{
  int t = dht.readTemperature();  
  int h = dht.readHumidity();
  unsigned long currentMillis = millis(); 
  
                                        /* Вншнее управление */
  if (Serial.available() > 0) 
  {                                     //если есть доступные данные                                 
     incomingByte = Serial.read();      // считываем байт
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
                                  
  // получаем значение напряжения и тока в нагрузке             
  float Ucorr = -0.00;                                            // коррекция напряжения, при желании можно подстроить
  float Uout = analogRead(A1) * ((5.0 + Ucorr) / 1023.0) * 5.0;   // узнаем напряжение на выходе 
  float Iout = analogRead(A0) / 100.00;                           // узнаем ток в нагрузке
  
  if(Iout==0.01) Iout =  0.03; else 
  if(Iout==0.02) Iout =  0.04; else
  if(Iout==0.03) Iout =  0.05; else
  if(Iout==0.04) Iout = 0.06; else
  if(Iout>=0.05) Iout = Iout + 0.02;
  if(Iout>=0.25) Iout = Iout + 0.01;
  
                                 /* ЗАЩИТА и выключение */
  
  if (((Iout>(counter+0.3)*2.0) | Iout>10.1  | off) & set<4 & millis()>100 ) // условия защиты
   {  
    digitalWrite(power, 0);           // вырубаем реле
    level1 = 255;                     // убираем ШИМ1 сигнал
    level2 = 255;                     // убираем ШИМ2 сигнал
    digitalWrite(A2, 1);              // Включаем красный светодиод      
    Serial.print('I0;U0;r1;W0;');
    Serial.println(' ');
    set = 5;   
    }   
                                /* ЗАЩИТА КОНЕЦ */

 // Опрос валкодера перенесли в программу таймера                               
 /* 
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
*/
analogWrite(pwm1, level1);         // подаем нужный сигнал на ШИМ1 выход
analogWrite(pwm2, level2);         // подаем нужный сигнал на ШИМ2 выход

                                   /* УПРАВЛЕНИЕ */
if (digitalRead(kn_menu)==0 && digitalRead(kn_selection)==0 && digitalRead(kn_pwm)==0 && knopka_abc==0 ) 
{                                  // нажата ли кнопка a - б - c  вместе
  knopka_abc = 1;
  knopka_abc = 0;
}
/*
if (digitalRead(kn_pwm) == LOW && set == 5 && knopka_c == 0)          
                                   // нажата ли кнопка C (pwm)
{
    pwm1_2 = !pwm1_2;                                                   // Переключить ШИМ 1/2 ( pwm1_2== true -  ШИМ1,   pwm1_2== false -  ШИМ2)
    while (digitalRead(kn_pwm) == LOW )  {  }                           // Ожидаем отпускания кнопки ШИМ
}
*/
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

                                   //сбрасываем значения кнопок или чего-то вроде того.
  if (digitalRead(kn_menu)      == HIGH && knopka_b == 1) knopka_b = 0;
  if (digitalRead(kn_selection) == HIGH && knopka_a == 1) knopka_a = 0;
  if (digitalRead(kn_pwm)       == HIGH && knopka_c == 1) knopka_c = 0;

                                   /* COM PORT */

    if(currentMillis - com2 > com) {
                                   // сохраняем время последнего обновления
    com2 = currentMillis;  
                                   //Считаем Ампер*часы
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
  
                                       /* ИНДИКАЦИЯ LCD */

    if(set==0)
{ 
    // выводим реальные значения на дисплей*/
  if(currentMillis - previousMillis > interval) 
  {
    // сохраняем время последнего обновления
    previousMillis = currentMillis;  
    //выводим актуальные значения напряжения и тока на дисплей
  
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
      if(mode==0)lcd.print ("pwm:1"); // Шим 1
      if(mode==1)lcd.print ("pwm:2"); // Шим 2
    }  
    //дополнительная информация
    lcd.setCursor (7, 1);
    if(disp==0)    // отображение температуры
  {   
        lcd.print("Temp=");
        lcd.print(t);
        lcd.write(0b11011111);
        lcd.print("C");
     // lcd.print("         ");   // температура
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
      if(mode==1)lcd.print ("    drop");   // Шим 2
    }  
    if(disp==3)  // максимальный ток
    {  
       lcd.print ("  I>"); 
       lcd.print (Ioutmax, 2); 
       lcd.print ("A ");
    }
    if(disp==4)  // значение ШИМ1
    {  
      lcd.print (" pwm1:"); 
      lcd.print (ceil(level1), 0); 
      lcd.print ("  ");
    }
    if(disp==5)  // значение ШИМ2
    {  
      lcd.print (" pwm2:"); 
      lcd.print (ceil(level2), 0); 
      lcd.print ("  ");
    }
      
    if(disp==6)                        // Счетчик Аh
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
      if (disp==7)                                     // Влажность
      {
       lcd.print("  Hum=");
       lcd.print(h);
       lcd.print("%");
      }
  }
}

                 /* ИНДИКАЦИЯ МЕНЮ */
                 
  if(set==1)     // выбор режима
  {
   lcd.setCursor (0, 0);
   lcd.print("> MENU 1/4    ");
   lcd.setCursor (0, 1);
   lcd.print("mode: ");
            // режим (0 спабилизация напряжения, 1 стабилизация тока)
   if(mode==0)  lcd.print("normal          ");
   if(mode==1)  lcd.print("drop        ");
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
                          /* ИНДИКАЦИЯ ЗАЩИТЫ */
                          
  if(set==5)           //защита. вывод инфы
 {
  lcd.setCursor (0, 0);
 // lcd.print("ShutDown!        ");
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
  
 /*  if(set==6)          // защита. вывод инфы критическое падение напряжения
 {
  Serial.print('I0;U0;r1;W0;');
  digitalWrite(A2, true);
  Serial.println(' ');
  level1=0;
  level2=0;
  lcd.setCursor (0, 0);
  if (off==false)
    { 
    lcd.print("[   OVERLOAD   ]");
    lcd.setCursor (0, 1);
   //и обьясняем юзеру что случилось
    if ((Iout > (counter + 0.3) * 2.0) | Iout > 10.0)
    {
    Serial.print('t');
    Serial.print(1);
    Serial.print(';');
    lcd.print("  Iout >= Imax  ");
    }    
 }
 else
 {
 lcd.print("[      OFF     ]");
 lcd.setCursor (0, 1);
 Serial.print('t');
 Serial.print(4);
 Serial.print(';');
 }
 }  
*/ 
}
