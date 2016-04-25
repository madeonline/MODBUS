
/* Режимы меню
 *
 *
 *
 *
 * Команды внешнего управления
 * incomingByte = 97                  // убавляем напряжение
 * incomingByte = 98                  // counter < umax)       counter = counter + 0.1;   //добавляем
 * incomingByte = 99                  // валкодер -
 * incomingByte = 100                 // валкодер +
 * incomingByte = 101 mode = 0;       // ("standart");            // Шим 1
 * incomingByte = 102 mode = 1;       // ("shutdown");            // Защита
 * incomingByte = 103 mode = 2;       // ("    drop");            // Шим 2
 * incomingByte = 104 save();         // сохранить настройки
 * incomingByte = 105                 // включаем реле если оно было выключено и гасим красный светодиод
 * incomingByte = 106  off = true;    // "[   OVERLOAD   ]" 
 * incomingByte = 107  ah = 0;        // Cчетчик Ампер*часов обнулить
 * 
 *
 * Функции валкодера +
 * set = 0   // обычный режим - добавляем напряжение
 * set = 1   // переключаем режим работы вперед
 * set = 2   // настройка тока, добавляем ток
 * set = 3   // сброс счетчика А*ч
 * set = 4   //сохранение текущих настроек в память
 * 
 * Функции валкодера -
 * set = 0   // обычный режим - уменьшаем напряжение
 * set = 1   // переключаем режим работы назад
 * set = 2   // настройка тока, уменьшаем ток
 * 
 * Отображение на дисплее
 * disp = 0  // ничего
 * disp = 1  // мощность
 * disp = 2  // режим БП
 * disp = 3  // максимальный ток
 * disp = 4  // значение ШИМ
 * disp = 5  // значение тока ????
 * 
 * 
 * 
 * 
 */






#include <LiquidCrystal.h>;
#include <EEPROM.h>

LiquidCrystal lcd(8, 7, 5, 4, 3, 2); //rs, e, d4, d5, d6, d7

// задаем константы

float umax          = 20.00;           // максимальное напряжение
float umin          = 0.00;            // минимальное напряжение
float ah            = 0.0000;          // Cчетчик Ампер*часов
const int down      = 10;              // выход валкодера 1/2 В
const int up        = 11;              // выход валкодера 2/2 А
const int pwm1      = 9;               // выход ШИМ1
const int pwm2      = 6;               // выход ШИМ2
const int power     = A5;              // управление релюхой
long previousMillis = 0;               // храним время последнего обновления дисплея
long maxpwm         = 0;               // циклы поддержки максимального ШИМ
long interval       = 500;             // интервал обновления информации на дисплее, мс
int mig             = 0;               // Для енкодера (0 стоим 1 плюс 2 минус)
float level         = 2000;            // "уровень" ШИМ сигнала
float com           = 100;
long com2           = 0;
int mode            = 0;               // режим (0 обычный, спабилизация тока, защита по току)
float Ioutmax       = 1.0;             // заданный ток
int set             = 0;               // пункты меню, отображение защиты...
int knopka_a        = 0;               // состояние кнопок
int knopka_b        = 0;
int knopka_c        = 0;
int knopka_abc      = 0;
boolean off         = false;
boolean red         = false;
boolean blue        = false;
float counter       = 5;               // переменная хранит заданное напряжение
int disp            = 0;               // режим отображения 0 ничего, 1 мощьность, 2 режим, 3 установленный ток, 4 шим уровень
float Uout ;                           // напряжение на выходе
#define kn_menu       12               // Назначение кнопки "Меню"
#define kn_selection  13               // Назначение кнопки "Выбор"
#define kn_pwm        A4               // Назначение кнопки "ШИМ"
#define led_red       A2               // Назначение светодиода "RED"
#define led_blue      A3               // Назначение светодиода "BLUE"

int incomingByte;


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

void setup()
{

  cli();                                 // Настройка портов микроконтроллера
  DDRB |= 1 << 1 | 1 << 2;
  PORTB &= ~(1 << 1 | 1 << 2);
  TCCR1A = 0b00000010;
  //TCCR1A = 0b10100010;
  TCCR1B = 0b00011001;
  ICR1H = 255;
  ICR1L = 255;
  sei();

  int pwm_rez = 13;
  pwm_rez = pow(2, pwm_rez);               // Вычисляет значение возведенное в заданную степень
  ICR1H   = highByte(pwm_rez);             // Запись старшего байта в Input Capture Register 1 H
  ICR1L   = lowByte(pwm_rez);              // Запись младшего байта в Input Capture Register 1 L

  Serial.begin(9600);                      // Установить скорость серийного порта 9600

  pinMode(pwm1, OUTPUT);                   // На выход 9
  pinMode(pwm2, OUTPUT);                   // На выход 6
  pinMode(down, INPUT);
  pinMode(up, INPUT);
  pinMode(kn_menu, INPUT);
  pinMode(kn_selection, INPUT);
  pinMode(kn_pwm, INPUT);
  pinMode(power, OUTPUT);
  pinMode(led_red, OUTPUT);
  pinMode(led_blue, OUTPUT);
  // поддерживаем высокий уровень на входах от валкодера
  digitalWrite(up, HIGH);
  digitalWrite(down, HIGH);
  //поддерживаем высокий уровень на контактах кнопок
  digitalWrite(kn_menu, HIGH);
  digitalWrite(kn_selection, HIGH);
  digitalWrite(kn_pwm, HIGH);

  //запуск дисплея
  lcd.begin(16, 2);                                     // Дисплей 16 символов, 2 строки
  lcd.print("    WELCOME!    ");

  //загружаем настройки из памяти МК
  counter = EEPROM_float_read(0);
  Ioutmax = EEPROM_float_read(4);
  mode    = EEPROM_float_read(12);
  disp    = EEPROM_float_read(10);
  //Если в памяти еще нет настроек - задаем что нибудь кроме нулей
  if (counter == 0) counter = 5;                         //5 вольт
  if (Ioutmax == 0) Ioutmax = 2;                         //2 ампера

  digitalWrite(power, HIGH);                             //включаем реле

}



//функции при вращении енкодера
void uup()                                           //энкодер +
{
  if (set == 0)                                      //обычный режим - добавляем напряжение
  {
    if (counter < umax)
    {
      counter = counter + 0.1;                       //добавляем
    }
  }
  if (set == 1)                                      //переключаем режим работы вперед
  { 
    mode++;
    if (mode > 2) mode = 2;
  }
  if (set == 2)                                     //настройка тока, добавляем ток
  { 
    iplus();
  }

  if (set == 3)                                      //сброс счетчика А*ч
  { 
    ah = 0;
    set = 0;
    disp = 5;
  }

  if (set == 4)                                     //сохранение текущих настроек в память
  { 
    save();
  }
}

void udn()                                              //валкодер -
{ 
  if (set == 0)
  {
    if (counter > umin + 0.1)counter = counter - 0.1;  // убавляем напряжение
  }
  if (set == 1)
  {
    mode = mode--;                                    // переключаем режим работы назад
    if (mode < 0) mode = 0;
  }
  if (set == 2)                                        // убавляем ток
  { 
    iminus();
  }
}

void iplus()
{
  Ioutmax = Ioutmax + 0.01;
  if (Ioutmax > 0.2) Ioutmax = Ioutmax + 0.04;
  if (Ioutmax > 1) Ioutmax = Ioutmax + 0.05;
  if (Ioutmax > 10.00) Ioutmax = 10.00;
}

void iminus()
{
  Ioutmax = Ioutmax - 0.01;
  if (Ioutmax > 0.2) Ioutmax = Ioutmax - 0.04;
  if (Ioutmax > 1) Ioutmax = Ioutmax - 0.05;
  if (Ioutmax < 0.03) Ioutmax = 0.03;
}

void save()
{
  lcd.clear();                                             // Очистить дисплей
  lcd.setCursor (0, 0);                                    // Установить курсор в начало
  lcd.print(" S A V E  -  OK ");                           // Вывести сообщение

  EEPROM_float_write(0, counter);
  EEPROM_float_write(4, Ioutmax);
  EEPROM_float_write(12, mode);
  EEPROM_float_write(10, disp);
  //мигаем светодиодами
  digitalWrite(led_red, HIGH);                             // Включить красный светодиод
  digitalWrite(led_blue, HIGH);                            // Включить синий светодиод
  delay(500);                                              // Ждем 0,5 секунды
  digitalWrite(led_red, LOW);                              // Отключить красный светодиод
  digitalWrite(led_blue, LOW);                             // Отключить синий светодиод
  set = 0;                                                 //выходим из меню
}

void loop()                                                //основной цикл работы МК
{

  unsigned long currentMillis = millis();

  // Внешнее управление



  if (Serial.available() > 0)
  { //если есть доступные данные  считываем байт
    incomingByte = Serial.read();
  }
  else
  {
    incomingByte = 0;
  }

  if (incomingByte == 97)
  { //a
    if (counter > umin + 0.1)counter = counter - 0.1;    //убавляем напнряжение

  }
  if (incomingByte == 98) { //b

    if (counter < umax)       counter = counter + 0.1;   //добавляем

  }

  if (incomingByte == 99) { //c
    iminus();
  }

  if (incomingByte == 100) { //d
    iplus();
  }

  if (incomingByte == 101) mode = 0;
  if (incomingByte == 102) mode = 1;
  if (incomingByte == 103) mode = 2;
  if (incomingByte == 104) save();
  if (incomingByte == 105)
  {
    digitalWrite(power, HIGH);                              // врубаем реле если оно было выключено
    delay(100);
    digitalWrite(led_red, LOW);                             // гасим красный светодиод
    Serial.print('t');
    Serial.print(0);
    Serial.print(';');
    off = false;
    set = 0;                                               // выходим из меню
  }

  if (incomingByte == 106) off = true;
  if (incomingByte == 107) ah = 0;

  // конец внешнего управления



  //получаем значение напряжения и тока в нагрузке
  float Ucorr = -0.00;                                            //коррекция напряжения, при желании можно подстроить
  float Uout = analogRead(A1) * ((5.0 + Ucorr) / 1023.0) * 5.0;   //узнаем напряжение на выходе
  float Iout = analogRead(A0) / 100.00;                           // узнаем ток в нагрузке

  if (Iout == 0.01) Iout =  0.03; else if (Iout == 0.02) Iout =  0.04; else if (Iout == 0.03) Iout =  0.05; else if (Iout == 0.04) Iout = 0.06; else if (Iout >= 0.05) Iout = Iout + 0.02;
  if (Iout >= 0.25)Iout = Iout + 0.01;
  //if(Iout>=1)Iout = Iout * 1.02;


  // ЗАЩИТА и выключение

  if (((Iout > (counter + 0.3) * 2.0) | Iout > 10.1  | off) & set<4 & millis()>100 )  // условия защиты

  {
    digitalWrite(power, LOW);                                      // вырубаем реле
    level = 8190;                                                  // убираем ШИМ сигнал
    digitalWrite(led_red, HIGH);                                   // Включаем красный светодиод

    Serial.print('I0;U0;r1;W0;');
    Serial.println(' ');
    set = 6;

  }


  //Зашита от длительного максимального шим
  if (level == 0 & off == false)
  {
    if (set < 4)                                                        // если уже не сработала защита
    {
      maxpwm--;                                                         // добавляем +1 к счетчику
      digitalWrite(led_red, HIGH);                                      // светим красным для предупреждения о максимальном ШИМ
    }
  }
  else                                                                  // шим у нас не максимальный, поэтому поубавим счетчик
  {
    maxpwm++;
    if (maxpwm > 8190)                                                  // если счетчик дошел до нуля
    {
      maxpwm = 8190;                                                    // таким его и держим
      if (set < 4) digitalWrite(led_red, LOW);                          // гасим красный светодиод. Перегрузки нет.
    }
  }


  // ЗАЩИТА КОНЕЦ


  // считываем значения с входа валкодера
  boolean regup   = digitalRead(up);
  boolean regdown = digitalRead(down);

  if (regup < regdown) mig = 1;                                      // крутится в сторону увеличения
  if (regup > regdown) mig = 2;                                      // крутится в сторону уменшения
  if (!regup & !regdown)                                             //момент для переключения
  {
    if (mig == 1) uup(); //+
    if (mig == 2) udn(); //-
    mig = 0;                                                        //сбрасываем указатель направления
  }

  if (mode == 0 | mode == 1)                                        //если управляем только напряжением (не режим стабилизации тока)
  {

    //Сравниваем напряжение на выходе с установленным, и принимаем меры..
    if (Uout > counter)
    {
      float raz = Uout - counter;                                  //на сколько напряжение на выходе больше установленного...
      if (raz > 0.05)
      {
        level = level - raz * 20;                                  //разница большая управляем грубо и быстро!
      } else {
        if (raz > 0.015)  level = level -  raz * 3 ;               //разница небольшая управляем точно
      }
    }
    if (Uout < counter)
    {
      float raz = counter - Uout;                                  //на сколько напряжение меньше чем мы хотим
      if (raz > 0.05)
      {
        level = level + raz * 20; //грубо
      } else {
        if (raz > 0.015)  level = level + raz * 3 ;                //точно
      }
    }

    if (mode == 1 && Iout > Ioutmax)                                //режим защиты по току, и он больше чем мы установили
    {
      digitalWrite(power, LOW);                                     //вырубаем реле
      Serial.print('t');
      Serial.print(2);
      Serial.print(';');

      digitalWrite(led_red, HIGH);                                  // зажигаем красный светодиод
      level = 8190;                                                 // убираем ШИМ сигнал
      set = 5;                                                      // режим ухода в защиту...
    }

  }
  else
  { // режим стабилизации тока

    if (Iout >= Ioutmax)
    {
      //узнаем запас разницу между током в нагрузке и установленным током
      float raz = (Iout - Ioutmax);
      if (raz > 0.3)                                              // очень сильно превышено (ток больше заданного более чем на 0,3А)
      {
        level = level + raz * 20;                                 // резко понижаем ШИМ
      } else {
        if (raz > 0.05)                                           // сильно превышено (ток больше заданного более чем на 0,1А)
        {
          level = level + raz * 5;                                // понижаем ШИМ
        } else {
          if (raz > 0.00) level = level + raz * 2;                // немного превышен (0.1 - 0.01А) понижаем плавно
        }
      }

      digitalWrite(led_blue, HIGH);                               // зажигаем синий светодиод
    } else {                                                      // режим стабилизации тока, но ток у нас в пределах нормы, а значит занимаемся регулировкой напряжения
      digitalWrite(led_blue, LOW);                                // синий светодиод не светится

      //Сравниваем напряжение на выходе с установленным, и принимаем меры..
      if (Uout > counter)
      {
        float raz = Uout - counter;                               //на сколько напряжение на выходе больше установленного...
        if (raz > 0.1)
        {
          level = level + raz * 20;                               //разница большая управляем грубо и быстро!
        } else {
          if (raz > 0.015)  level = level + raz * 5;              //разница небольшая управляем точно
        }
      }
      if (Uout < counter)
      {
        float raz = counter - Uout;                              //на сколько напряжение меньше чем мы хотим
        float iraz = (Ioutmax - Iout); //
        if (raz > 0.1 & iraz > 0.1)
        {
          level = level - raz * 20;                              //грубо
        } else {
          if (raz > 0.015)  level = level - raz ; //точно
        }
      }
    }
  }//конец режима стабилизации тока

  if (off) level = 8190;
  if (level < 0) level = 0;                                             //не опускаем ШИМ ниже нуля
  if (level > 8190) level = 8190;                                       //не поднимаем ШИМ выше 13 бит
  //Все проверили, прощитали и собственно отдаем команду для силового транзистора.
  if (ceil(level) != 255) analogWrite(pwm1, ceil(level));               //подаем нужный сигнал на ШИМ выход (кроме 255, так как там какая-то лажа)


  // УПРАВЛЕНИЕ

  if (digitalRead(kn_menu) == LOW && digitalRead(kn_selection) == LOW && digitalRead(kn_pwm) == LOW && knopka_abc == 0 )
  { // нажата ли кнопка a - б - c  вместе
    knopka_abc = 1;

    //ah = 0.000;

    knopka_abc = 0;
  }

  if (digitalRead(kn_pwm) == LOW && knopka_c == 0)                    // нажата ли кнопка C (disp)
  {
    knopka_c = 1;
    disp = disp + 1;                                                  //поочередно переключаем режим Шим 1 и Шим 2
    if (disp == 6) disp = 0;                                          //дошли до конца, начинаем снова
  }


  if (digitalRead(kn_selection) == LOW && knopka_a == 0) {            // нажата ли кнопка А (disp)
    knopka_a = 1;
    disp = disp + 1;                                                  //поочередно переключаем режим отображения информации
    if (disp == 6) disp = 0;                                          //дошли до конца, начинаем снова
  }

  if (digitalRead(kn_menu) == LOW && knopka_b == 0) {                 // нажата ли кнопка Б (menu)
    knopka_b = 1;
    set = set + 1; //
    if (set > 4 | off)                                                // Задействован один из режимов защиты, а этой кнопкой мы его вырубаем. (или мы просто дошли до конца меню)
    {
      off = false;
      digitalWrite(power, HIGH);                                      // врубаем реле если оно было выключено
      delay(100);
      digitalWrite(led_red, LOW);                                     //гасим красный светодиод
      Serial.print('t');
      Serial.print(0);
      Serial.print(';');
      Serial.print('r');
      Serial.print(0);
      Serial.print(';');
      Serial.println(' ');
      set = 0;                                                        //выходим из меню
    }
    lcd.clear();                                                      //чистим дисплей
  }

  //сбрасываем значения кнопок или чего-то вроде того.
  if (digitalRead(kn_menu)      == HIGH && knopka_b == 1) knopka_b = 0;
  if (digitalRead(kn_selection) == HIGH && knopka_a == 1) knopka_a = 0;
  if (digitalRead(kn_pwm)       == HIGH && knopka_c == 1) knopka_c = 0;




  // COM PORT

  if (currentMillis - com2 > com)
  {
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
    Serial.print(level);
    Serial.print(';');

    Serial.print('c');
    Serial.print(ah);
    Serial.print(';');

    Serial.print('m');
    Serial.print(mode);
    Serial.print(';');

    Serial.print('r');
    Serial.print(digitalRead(led_red));
    Serial.print(';');

    Serial.print('b');
    Serial.print(digitalRead(led_blue));
    Serial.print(';');



    Serial.println(' ');

  }

  // ИНДИКАЦИЯ LCD



  if (set == 0)
  {
    //стандартный экран,  выводим установленное напряжение на дисплей
    lcd.setCursor (0, 1);
    lcd.print("U>");
    if (counter < 10) lcd.print(" ");            // добавляем пробел, если нужно, чтобы не портить картинку
    lcd.print (counter, 1);                      // выводим установленное значение напряжения
    lcd.print ("V ");                            // пишем что это вольты

    //обновление информации
    //проверяем не прошел ли нужный интервал, если прошел то
    // выводим реальные значения на дисплей

    if (currentMillis - previousMillis > interval)
    {
      // сохраняем время последнего обновления
      previousMillis = currentMillis;
      //выводим актуальные значения напряжения и тока на дисплей
      lcd.setCursor (0, 0);
      lcd.print("U=");
      if (Uout < 9.99) lcd.print(" ");
      lcd.print(Uout, 2);
      lcd.print("V I=");
      lcd.print(Iout, 2);
      lcd.print("A ");

      //дополнительная информация
      lcd.setCursor (8, 1);
      if (disp == 0)                                      // ничего
      {
        lcd.print("         ");                           // отображение температуры Датчик DHT11 ???
      }
      if (disp == 1)                                      // мощность
      {
        lcd.print(" ");
        lcd.print (Uout * Iout, 2);
        lcd.print("W   ");
      }
      if (disp == 2)                                      //режим БП
      {
        if (mode == 0)lcd.print  ("standart");            // Шим 1
        if (mode == 1)lcd.print  ("shutdown");            // Защита
        if (mode == 2)lcd.print  ("    drop");            // Шим 2
      }
      if (disp == 3)                                      // максимальный ток
      {
        lcd.print (" I>");
        lcd.print (Ioutmax, 2);
        lcd.print ("A ");
      }
      if (disp == 4)                                     // значение ШИМ
      {
        lcd.print ("pwm1:");
        lcd.print (ceil(level), 0);
        lcd.print ("  ");
      }
      if (disp == 5)                                     // значение ШИМ ????
      {
        if (ah < 1)
        {
          //if(ah<0.001) lcd.print (" ");
          if (ah <= 0.01) lcd.print (" ");
          if (ah <= 0.1) lcd.print (" ");
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
    }
  }

  // ИНДИКАЦИЯ МЕНЮ
  
  if (set == 1) //выбор режима
  {
    lcd.setCursor (0, 0);
    lcd.print("> MENU 1/4    ");
    lcd.setCursor (0, 1);
    lcd.print("mode: ");
    //режим (0 обычный, спабилизация тока, защита по току)
    if (mode == 0)  lcd.print("normal          ");
    if (mode == 1)  lcd.print("shutdown    ");
    if (mode == 2)  lcd.print("drop        ");
  }
  if (set == 2)                                     //настройка тока
  { 
    lcd.setCursor (0, 0);
    lcd.print("> MENU 2/4   ");
    lcd.setCursor (0, 1);
    lcd.print("I out max: ");
    lcd.print(Ioutmax);
    lcd.print("A");
  }
  if (set == 3)                                   //спрашиваем хочет ли юзер сбросить настройки
  { 
    lcd.setCursor (0, 0);
    lcd.print("> MENU 3/4      ");
    lcd.setCursor (0, 1);
    lcd.print("Reset A*h? ->");
  }

  if (set == 4)                                  //спрашиваем хочет ли юзер сохранить настройки
  { 
    lcd.setCursor (0, 0);
    lcd.print("> MENU 4/4      ");
    lcd.setCursor (0, 1);
    lcd.print("Save options? ->");
  }
  
  // ИНДИКАЦИЯ ЗАЩИТЫ
  if (set == 5)                                 //защита. вывод инфы
  { 
    lcd.setCursor (0, 0);
    lcd.print("ShutDown!        ");
    lcd.setCursor (0, 1);
    lcd.print("Iout");
    lcd.print(">Imax(");
    lcd.print(Ioutmax);
    lcd.print("A)");
    level = 0;
    Serial.print('I0;U0;r1;W0;');
    Serial.println(' ');
  }


  if (set == 6)                                 //защита. вывод инфы критическое падение напряжения
  { 
    Serial.print('I0;U0;r1;W0;');
    digitalWrite(led_red, HIGH);
    Serial.println(' ');
    level = 0;
    lcd.setCursor (0, 0);

    if (off == false)
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

}

