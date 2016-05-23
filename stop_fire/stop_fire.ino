/*
  Система пожаротушения. Светлодарск... Донецкой обл.


  Применяется Arduino Mega2560 в с клавиатурой ручного управления моторами.

  Применяются  4  датчика газа MQ2, драйвер L298N два коллекторных двигателя с редукторами (на валах привода щелевые IR датчики FC03), реле, светодиод, пьезопищалка.
  По срабатыванию одного из четырех MQ2 должна выполняется следующая программа:
  1) закончен опрос датчиков;
  2) включаем светодиод ;
  3) включаем пьезопищалку – сигнал тревоги;
  4) двигатель А начинает вращение «вперед» до определенного количества оборотов вала привода, например  17 оборотов (импульсов с IR датчика  FC03 ) – двигатель стоп;
  5) двигатель В начинает вращение «влево» до определенного количества оборотов вала привода, например  8 оборотов (импульсов с IR датчика  FC03 ) – двигатель стоп;
  6) включаем реле на 10-20 секунд;
  7) выключаем реле;
  8) выключаем пьезопщалку;
  9) выключаем светодиод;
  10) двигатель В начинает вращение «в право» до 8 оборотов вала привода (импульсов с IR   датчика  FC03 ) – двигатель стоп;
  11) двигатель А начинает вращение «назад» до 17 оборотов вала привода (импульсов с IR датчика  FC03 ) – двигатель стоп;
  12) снова, переходим к опросу всех четырех датчиков газа MQ2.
    Суть проекта заключается в создании макета простой, автоматической противопожарной системы.
	Четыре датчика газа MQ2 опрашиваются до появлений высокого уровня на одном из них.
	Как только один сработал, прекращается опрос датчиков и программа переходит к выполнению одного из четырех вариантов цикла.
    В зависимости от того какой из четырех датчиков газа MQ2 срабатывает - изменяется количество оборотов вала двигателя А
	и исходное  направление вращения двигателя В – куда двигается в начале (влево или вправо).
	Если с начала влево то возвращаясь в исходное состояние двигается в право  .
	Количество оборотов вала двигателя В не изменяется и постоянно = 8, количество оборотов вала двигателя А может быть или 17 или 34.
    Сработал датчик, включается светодиод, включается пьезопищалка. Каретка с  противопожарным раструбом  доставляется
	по адресу сработавшего датчика, на 10-20 секунд включается реле насоса . Происходит «тушение пожара». Выключается светодиод ,
	выключается пьезопищалка. Каретка с раструбом возвращается в исходное положение.
     Датчики газа подключал к аналоговым входам, порог устанавливал 200. Работу счетчика щелевых датчиков FC03 проверял при поданном
	 питании 3.3В ( при 5 В наблюдался дребезг и т.д.) . Сигнал подавал на 2 и 3 цифровые ввода ардуины, считал при помощи buttonPushCounter.
*/

/*
============= Подключение устройств =========================
вход датчика газаMQ2 №1 пин   A0
вход датчика газаMQ2 №2 пин   A1
вход датчика газаMQ2 №3 пин   A2
вход датчика газаMQ2 №4 пин   A3
вход датчика FC03 №1 пин      D2
вход датчика FC03 №2 пин      D3

motorA_en    Подключение пин  D10 к пину 7 на L298N (предварительно убрав перемычку)
motorA_in1   Подключение пин  D9 к пину 8 на L298N    
motorA_in2   Подключение пин  D8 к пину 9 на L298N 

motorB_en    Подключение пин  D5 к пину 12 на L298N (предварительно убрав перемычку)
motorB_in3   Подключение пин  D7 к пину 10 на L298N   
motorB_in4   Подключение пин  D6 к пину 11 на L298N   

stop_motorA                   D22   Датчик исходного положения мотора А. Подключить к выводу 22 и "Общему" проводу. Расположить в начале.
stop_motorB                   D23   Датчик исходного положения мотора B.Подключить к выводу 23 и "Общему" проводу. Расположить справа в крайнем положении

rele1                         D11 Подключение пин D11 к драйверу (усилителю) реле. Высокий уровень - включить.
buzzer                        D4  Подключение пин D4 к драйверу (усилителю) зуммера. Высокий уровень - включить.         

Кнопка управления мотором А "вперед"  D24
Кнопка управления мотором А "назад"   D25
Кнопка управления мотором В "право"   D26
Кнопка управления мотором В "лево"    D27
Кнопка управления реле1               D28

led_ind                      D29     Светодиод индикации. Высокий уровень - включить.  
power_MQ2                    D30     Реле управления прогревом датчиков MQ2. Высокий уровень - включить.  


 */





#include <avr/wdt.h>                           // Библиотека сторожевого таймера

#define sensorMQ2_1   A0                       // Назначение входа датчика газаMQ2 №1 пин Nano A0
#define sensorMQ2_2   A1                       // Назначение входа датчика газаMQ2 №2 пин Nano A1
#define sensorMQ2_3   A2                       // Назначение входа датчика газаMQ2 №3 пин Nano A2
#define sensorMQ2_4   A3                       // Назначение входа датчика газаMQ2 №4 пин Nano A3

int in_MQ2_1 = 0;                              // Уровень с выхода сенсора газа sensorMQ2_1
int in_MQ2_2 = 0;                              // Уровень с выхода сенсора газа sensorMQ2_2
int in_MQ2_3 = 0;                              // Уровень с выхода сенсора газа sensorMQ2_3
int in_MQ2_4 = 0;                              // Уровень с выхода сенсора газа sensorMQ2_4

int porog_sensorMQ2_1 = 200;                   // Переменная величины порога датчика газа MQ2_1
int porog_sensorMQ2_2 = 200;                   // Переменная величины порога датчика газа MQ2_2
int porog_sensorMQ2_3 = 200;                   // Переменная величины порога датчика газа MQ2_3
int porog_sensorMQ2_4 = 200;                   // Переменная величины порога датчика газа MQ2_4

#define sensorFC03_1  2                        // Назначение входа датчика FC03 №1 пин Nano D2
#define sensorFC03_2  3                        // Назначение входа датчика FC03 №2 пин Nano D3


#define led13         13                       // Применение встроенного светодиода Nano D13


#define motorA_en     10                       // Подключение пин Nano D10 к пину 7 на L298N (предварительно убрав перемычку)
#define motorA_in1     9                       // Подключение пин Nano D9 к пину 8 на L298N    
#define motorA_in2     8                       // Подключение пин Nano D8 к пину 9 на L298N 

#define motorB_en      5                       // Подключение пин Nano D5 к пину 12 на L298N (предварительно убрав перемычку)
#define motorB_in3     7                       // Подключение пин Nano D7 к пину 10 на L298N   
#define motorB_in4     6                       // Подключение пин Nano D6 к пину 11 на L298N   




// Подключить к выводу 22 и "Общему" проводу. Расположить в начале.
#define stop_motorA    22                      // Датчик исходного положения мотора А. Желательно добавить в систему. 


// Подключить к выводу 23 и "Общему" проводу. Расположить справа в крайнем положении
#define stop_motorB    23                      // Датчик исходного положения мотора B  Желательно добавить в систему. 


#define rele1         11                       // Подключение пин Nano D11 к драйверу (усилителю) реле. Высокий уровень - включить.
#define buzzer         4                       // Подключение пин Nano D4 к драйверу (усилителю) зуммера. Высокий уровень - включить.         

#define kn_frontA     24                       // Кнопка управления мотором А "вперед"  D24
#define kn_backA      25                       // Кнопка управления мотором А "назад"   D25
#define kn_rightB     26                       // Кнопка управления мотором В "право"   D26
#define kn_leftB      27                       // Кнопка управления мотором В "лево"    D27
#define kn_rele       28                       // Кнопка управления реле                D28

#define led_ind       29                       // Светодиода индикации
#define power_MQ2     30                       // Реле управления прогревом датчиков


bool  flag_kn_frontA = false;                  // Флаг кнопки управления мотором А "вперед"  D24
bool  flag_kn_backA  = false;                  // Флаг кнопки управления мотором А "назад"   D25
bool  flag_kn_rightB = false;                  // Флаг кнопки управления мотором В "право"   D26
bool  flag_kn_leftB  = false;                  // Флаг кнопки управления мотором В "лево"    D27
bool  flag_kn_rele   = false;                  // Флаг кнопки управления реле                D28

volatile unsigned int pulsesA = 0;             // Счетчик количества импульсов мотора А
volatile unsigned int pulsesB = 0;             // Счетчик количества импульсов мотора B

int int_motorA     = 17;                       // Переменная количества импульсов мотора А
int int_motorB     =  8;                       // Переменная количества импульсов мотора B
int int_centerB    =  8;                       // Переменная количества импульсов мотора B центрального положения
int temp_count     =  0;                       // Переменная для временного хранения количества импульсов мотора

int time_rele_on   = 20;                       // Время включения реле (20 секунд)
unsigned long time_rele_120 = 120;             // Время включения реле (2 минуты)
int time_temp      = 0;                        // Переменная для временного хранения текущего времени

bool left          = false;                    // Направление движения мотора В
bool parkingA      = false;                    // Разрешение парковки мотора А
bool parkingB      = false;                    // Разрешение парковки мотора В
bool powerMQ2_on   = false;                    // Разрешение прогрева датчиков газа 2 минуты



void counterA()                                // Программа подсчета импульсов с мотора А
{
  pulsesA++;
}

void counterB()                                // Программа подсчета импульсов с мотора В
{
  pulsesB++;
}


void test_button()
{
  // =========  Кнопка управления мотором А "вперед"  =========
  if ( digitalRead(kn_frontA ) == LOW )
  {
    while (digitalRead(kn_frontA ) == LOW) {}
    flag_kn_frontA = !flag_kn_frontA;
    if (flag_kn_frontA)
    {
      Serial.println("The motorA moves forward");
      digitalWrite(motorA_in1, HIGH);              // запуск двигателя A вперед
      digitalWrite(motorA_in2, LOW);               // запуск двигателя A вперед
      analogWrite(motorA_en, 254);                 // устанавливаем максимальную скорость из доступного диапазона 0~255.

    }
    else
    {
      Serial.println("The motorA  stop");
      analogWrite(motorA_en, 0);                   // Резко останавливаем мотор
      digitalWrite(motorA_in1, LOW);               // Останов двигателя A
      digitalWrite(motorA_in2, LOW);               // Останов двигателя A
    }
  }
  // ==========  Кнопка управления мотором А "назад"   =======================
  if ( digitalRead(kn_backA ) == LOW )
  {
    while (digitalRead(kn_backA ) == LOW) {}
    flag_kn_backA = !flag_kn_backA;
    if (flag_kn_backA)
    {
      Serial.println("The motorA moves back");
      digitalWrite(motorA_in1, LOW);               // Запуск двигателя A назад
      digitalWrite(motorA_in2, HIGH);              // Запуск двигателя A назад
      analogWrite(motorA_en, 254);                 // устанавливаем  скорость из доступного диапазона 0~255.
    }
    else
    {
      Serial.println("The motorA  stop");
      analogWrite(motorA_en, 0);                   // Резко останавливаем мотор
      digitalWrite(motorA_in1, LOW);               // Останов двигателя A
      digitalWrite(motorA_in2, LOW);               // Останов двигателя A
    }
  }
  // ===============  Кнопка управления мотором В "право" ======================================
  if ( digitalRead(kn_rightB ) == LOW )
  {
    while (digitalRead(kn_rightB ) == LOW) {}
    flag_kn_rightB = !flag_kn_rightB;
    if (flag_kn_rightB)
    {
      Serial.println("The motorB moves right");
      digitalWrite(motorB_in3, HIGH);              // Запуск двигателя В вправо
      digitalWrite(motorB_in4, LOW);               // Запуск двигателя В вправо
      analogWrite(motorB_en, 254);                 // устанавливаем  скорость из доступного диапазона 0~255.
    }
    else
    {
      Serial.println("The motorB  stop");
      analogWrite(motorB_en, 0);                   // Резко останавливаем мотор
      digitalWrite(motorB_in3, LOW);               // Останов двигателя B
      digitalWrite(motorB_in4, LOW);               // Останов двигателя B
    }
  }
  // ====================== Кнопка управления мотором В "лево" ================================
  if ( digitalRead(kn_leftB) == LOW )
  {
    while (digitalRead(kn_leftB) == LOW) {}
    flag_kn_leftB = !flag_kn_leftB;
    if (flag_kn_leftB)
    {
      Serial.println("The motorB moves left");
      digitalWrite(motorB_in3, LOW);               // Запуск двигателя В влево
      digitalWrite(motorB_in4, HIGH);              // Запуск двигателя В влево
      analogWrite(motorB_en, 254);                 // устанавливаем  скорость из доступного диапазона 0~255.
    }
    else
    {
      Serial.println("The motorB  stop");
      analogWrite(motorB_en, 0);                   // Резко останавливаем мотор
      digitalWrite(motorB_in3, LOW);               // Останов двигателя B
      digitalWrite(motorB_in4, LOW);               // Останов двигателя B
    }
  }

  // ====================== Кнопка управления реле ================================
  if ( digitalRead(kn_rele) == LOW )
  {
    while (digitalRead(kn_rele) == LOW) {}
    flag_kn_rele = !flag_kn_rele;
    if (flag_kn_rele)
    {
      Serial.println("Rele ON");
      digitalWrite(rele1, HIGH);              // Реле включить
    }
    else
    {
      Serial.println("Rele OFF");
      digitalWrite(rele1, LOW);               // Реле выключить
    }
  }
}



void setup()
{
  Serial.begin(9600);                           // инициализация порта
  Serial.println("System start");

  // Настраиваем  выводы как outputs

  pinMode(led13, OUTPUT);
  pinMode(rele1, OUTPUT);
  pinMode(buzzer, OUTPUT);

  pinMode(led_ind, OUTPUT);                     // Светодиод индикации
  pinMode(power_MQ2, OUTPUT);                   // Реле управления прогревом датчиков

  digitalWrite(led13, LOW);                     // выключить светодиод
  digitalWrite(rele1, LOW);                     // выключить реле
  digitalWrite(buzzer, LOW);                    // выключить зуммер
  digitalWrite(led_ind, LOW);                   // выключить светодиод
  digitalWrite(power_MQ2, LOW);                 // выключить реле управления прогревом датчиков

  pinMode(sensorFC03_1, INPUT);                 // Настраиваем вход датчика FC03 №1 пин Nano D2 на ввод
  pinMode(sensorFC03_2, INPUT);                 // Настраиваем вход датчика FC03 №2 пин Nano D3 на ввод

  digitalWrite(sensorFC03_1, HIGH);             // подключить внутренний резистор к +5 вольт.
  digitalWrite(sensorFC03_2, HIGH);             // подключить внутренний резистор к +5 вольт.

  pinMode(kn_frontA, INPUT);                    // Кнопка управления мотором А "вперед"
  pinMode(kn_backA,  INPUT);                    // Кнопка управления мотором А "назад"
  pinMode(kn_rightB, INPUT);                    // Кнопка управления мотором В "право"
  pinMode(kn_leftB , INPUT);                    // Кнопка управления мотором В "лево"
  pinMode(kn_rele,   INPUT);                    // Кнопка управления реле


  digitalWrite(kn_frontA,HIGH);                 // подключить внутренний резистор к +5 вольт.
  digitalWrite(kn_backA, HIGH);                 // подключить внутренний резистор к +5 вольт.
  digitalWrite(kn_rightB,HIGH);                 // подключить внутренний резистор к +5 вольт.
  digitalWrite(kn_leftB, HIGH);                 // подключить внутренний резистор к +5 вольт.
  digitalWrite(kn_rele,  HIGH);                 // подключить внутренний резистор к +5 вольт.

  flag_kn_frontA = false;                       // Флаг кнопки управления мотором А "вперед"
  flag_kn_backA  = false;                       // Флаг кнопки управления мотором А "назад"
  flag_kn_rightB = false;                       // Флаг кнопки управления мотором В "право"
  flag_kn_leftB  = false;                       // Флаг кнопки управления мотором В "лево"
  flag_kn_rele   = false;                       // Флаг кнопки управления мотором В "лево"

  // инициализируем все пины для управления двигателями как outputs
  pinMode(motorA_en,  OUTPUT);
  pinMode(motorB_en,  OUTPUT);
  pinMode(motorA_in1, OUTPUT);
  pinMode(motorA_in2, OUTPUT);
  pinMode(motorB_in3, OUTPUT);
  pinMode(motorB_in4, OUTPUT);

  pinMode(stop_motorA, INPUT);                   // Датчик исходного положения мотора А
  pinMode(stop_motorB, INPUT);                   // Датчик исходного положения мотора B
  digitalWrite(stop_motorA, HIGH);               // подключить внутренний резистор к +5 вольт.
  digitalWrite(stop_motorB, HIGH);               // подключить внутренний резистор к +5 вольт.


  // выключаем двигатели
  digitalWrite(motorA_in1, LOW);
  digitalWrite(motorA_in2, LOW);
  digitalWrite(motorB_in3, LOW);
  digitalWrite(motorB_in4, LOW);

  delay(1000);

//  powerMQ2_on = true;                               // Разрешение прогрева датчика газа. Раскомментировать при использовании
  
  if (powerMQ2_on)
  {
    digitalWrite(power_MQ2, HIGH);                 // включить реле управления прогревом датчиков
    Serial.println("Timer power MQ2 ON: ");
    int time_dec = 0;                              // Десять секунд
    for (int i = 0; i < time_rele_120 + 1; i++)    // Задержка включения реле на N секунд
    {
      delay(1000);                                 // Задержка на 1 секунду
      time_dec++;
      if (time_dec > 9)
      {
          Serial.print(i+1);                      // Вывод в серийный порт количества
          Serial.print(" sec./ ");                // Разделительный символ
          time_dec = 0;
          if(i == 59) Serial.println(" - 1 min"); 
          if(i == 119) Serial.println(" - 2 min"); 
      }

      wdt_reset();                               // говорим собаке что "В Багдаде все спокойно", начинается очередной отсчет 8-и секунд.
    }
    time_temp = 0;                               // Сбросить переменную в 0
  }


  // parkingA = true;                               // Разрешение парковки мотора А. Раскомментировать при использовании датчика парковки

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (parkingA == true)
  {
    if (analogRead(stop_motorA) > 50)
    {
      Serial.println("Motor A parking waiting");   // Вывод в серийный порт сообщения о парковке мотора А
      digitalWrite(motorA_in1, LOW);               // Запуск двигателя A назад
      digitalWrite(motorA_in2, HIGH);              // Запуск двигателя A назад
      analogWrite(motorA_en, 200);                 // устанавливаем  скорость из доступного диапазона 0~255.
      while (analogRead(stop_motorA) > 50) {}      // Ожидаем сигнал с датчика парковки мотора A
      Serial.println("The motorA  stop");
      analogWrite(motorA_en, 0);                   // Резко останавливаем мотор
      digitalWrite(motorA_in1, LOW);               // Останов двигателя A
      digitalWrite(motorA_in2, LOW);               // Останов двигателя A
      Serial.println("Motor A parking");           // Вывод в серийный порт сообщения о парковке мотора A
    }
    else
    {
      Serial.println("Motor A parking");           // Вывод в серийный порт сообщения о парковке мотора A
    }
  }
  //---------------------------------------------------------------------------------------------------


  //   parkingB = true;                               // Разрешение парковки мотора B. Раскомментировать при использовании датчика парковки

  if (parkingB == true)
  {
    if (analogRead(stop_motorB) > 50)
    {
      Serial.println("The motorB moves right");   //
      digitalWrite(motorB_in3, HIGH);              // Запуск двигателя В вправо
      digitalWrite(motorB_in4, LOW);               // Запуск двигателя В вправо
      analogWrite(motorB_en, 254);                 // устанавливаем  скорость из доступного диапазона 0~255.
      while (analogRead(stop_motorB) > 50)         // Ожидаем достижения датчика крайнего положения
      {
        analogWrite(motorB_en, 0);                 // Резко останавливаем мотор
        digitalWrite(motorB_in3, LOW);             // Останов двигателя B
        digitalWrite(motorB_in4, LOW);             // Останов двигателя B
      }
      delay(2000);                                 // 2 секунды на останов системы
      attachInterrupt(digitalPinToInterrupt(sensorFC03_2), counterB, FALLING);       // Включить прерывания по импульсу от датчика sensorFC03_2
      Serial.println("The motorB moves left");
      digitalWrite(motorB_in3, LOW);               // Запуск двигателя В влево
      digitalWrite(motorB_in4, HIGH);              // Запуск двигателя В влево
      analogWrite(motorB_en, 254);                 // устанавливаем  скорость из доступного диапазона 0~255.
      while (pulsesB < int_centerB) {}             // Ожидаем количество импульсов с датчика sensorFC03_2 до центрального положения
      detachInterrupt(1);                          // Отключить прерывания по импульсу от датчика sensorFC03_2
      Serial.println(pulsesB);                     // Вывод в серийный порт количества поступивших импульсов
      pulsesB = 0;                                 // Сбрасываем счетчик импульсов мотора B
      analogWrite(motorB_en, 0);                   // Резко останавливаем мотор
      digitalWrite(motorB_in3, LOW);               // Останов двигателя B
      digitalWrite(motorB_in4, LOW);               // Останов двигателя B
      Serial.println("Motor B parking");           // Вывод в серийный порт сообщения о парковке мотора A
      detachInterrupt(digitalPinToInterrupt(sensorFC03_2));                    // Отключить прерывания по импульсу от датчика sensorFC03_2
    }
    else
    {
      attachInterrupt(digitalPinToInterrupt(sensorFC03_2), counterB, FALLING);       // Включить прерывания по импульсу от датчика sensorFC03_2
      Serial.println("The motorB moves left");
      digitalWrite(motorB_in3, LOW);               // Запуск двигателя В влево
      digitalWrite(motorB_in4, HIGH);              // Запуск двигателя В влево
      analogWrite(motorB_en, 254);                 // устанавливаем  скорость из доступного диапазона 0~255.
      while (pulsesB < int_centerB) {}             // Ожидаем количество импульсов с датчика sensorFC03_2 до центрального положения
      detachInterrupt(1);                          // Отключить прерывания по импульсу от датчика sensorFC03_2
      Serial.println(pulsesB);                     // Вывод в серийный порт количества поступивших импульсов
      pulsesB = 0;                                 // Сбрасываем счетчик импульсов мотора B
      analogWrite(motorB_en, 0);                   // Резко останавливаем мотор
      digitalWrite(motorB_in3, LOW);               // Останов двигателя B
      digitalWrite(motorB_in4, LOW);               // Останов двигателя B
      Serial.println("Motor B parking");           // Вывод в серийный порт сообщения о парковке мотора A
      detachInterrupt(digitalPinToInterrupt(sensorFC03_2));                          // Отключить прерывания по импульсу от датчика sensorFC03_2
    }
  }
  Serial.println();
  Serial.println("System start end");
  Serial.println();
  Serial.println("============================================");

  wdt_enable (WDTO_8S);                  // Сторожевой таймер. При зависании системы - перезагрузит систему. Не рекомендуется устанавливать значение менее 8 сек.
}

void loop()
{
  pulsesA = 0;                           // Сбрасываем счетчик импульсов мотора A
  pulsesB = 0;                           // Сбрасываем счетчик импульсов мотора B

  test_button();                         // Проверить кнопки ручного управления

  in_MQ2_1 = analogRead(sensorMQ2_1);    // Считываем показания сенсора газа sensorMQ2_1
  in_MQ2_2 = analogRead(sensorMQ2_2);    // Считываем показания сенсора газа sensorMQ2_1
  in_MQ2_3 = analogRead(sensorMQ2_3);    // Считываем показания сенсора газа sensorMQ2_1
  in_MQ2_4 = analogRead(sensorMQ2_4);    // Считываем показания сенсора газа sensorMQ2_1

  // Проверяем не превышен ли порог любого датчика газа
  if (in_MQ2_1 > porog_sensorMQ2_1 || in_MQ2_2 > porog_sensorMQ2_2 || in_MQ2_3 > porog_sensorMQ2_3 || in_MQ2_4 > porog_sensorMQ2_4)
  {
    if (in_MQ2_1 > porog_sensorMQ2_1)
    {
      int_motorA = 20;                   // Настроить количество импульсов перемещения мотора А на датчик 1
      int_motorB = 8;                    // Настроить количество импульсов перемещения мотора B на датчик 1
      left = true;                       // Датчик находится слева
      Serial.print("MQ2_1 alarm - ");    // Вывод в серийный порт сообщения о превышении загазованности с датчика MQ2_1
      Serial.println(in_MQ2_1);          // Вывод в серийный порт сообщения об относительной величины загазованности
      Serial.print("Impulse A - ");      // Вывод в серийный порт сообщения
      Serial.println(int_motorA);        // Вывод в серийный порт сообщения
      Serial.print("Impulse B - ");      // Вывод в серийный порт сообщения
      Serial.println(int_motorB);        // Вывод в серийный порт сообщения
      in_MQ2_1 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_2 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_3 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_4 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
    }
    if (in_MQ2_2 > porog_sensorMQ2_2)
    {
      int_motorA = 20;                   // Настроить количество импульсов перемещения мотора А на датчик 2
      int_motorB = 8;                    // Настроить количество импульсов перемещения мотора B на датчик 2
      left = false;                      // Датчик находится справа
      Serial.print("MQ2_2 alarm - ");    // Вывод в серийный порт сообщения о превышении загазованности с датчика MQ2_2
      Serial.println(in_MQ2_2);          // Вывод в серийный порт сообщения об относительной величины загазованности
      Serial.print("Impulse A - ");      // Вывод в серийный порт сообщения
      Serial.println(int_motorA);        // Вывод в серийный порт сообщения
      Serial.print("Impulse B - ");      // Вывод в серийный порт сообщения
      Serial.println(int_motorB);        // Вывод в серийный порт сообщения
      in_MQ2_1 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_2 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_3 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_4 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
    }
    if (in_MQ2_3 > porog_sensorMQ2_3)
    {
      int_motorA = 10;                   // Настроить количество импульсов перемещения мотора А на датчик 3
      int_motorB = 8;                    // Настроить количество импульсов перемещения мотора B на датчик 3
      left = true;                       // Датчик находится слева
      Serial.print("MQ2_3 alarm - ");    // Вывод в серийный порт сообщения о превышении загазованности с датчика MQ2_3
      Serial.println(in_MQ2_3);          // Вывод в серийный порт сообщения об относительной величины загазованности
      Serial.print("Impulse A - ");      // Вывод в серийный порт сообщения
      Serial.println(int_motorA);        // Вывод в серийный порт сообщения
      Serial.print("Impulse B - ");      // Вывод в серийный порт сообщения
      Serial.println(int_motorB);        // Вывод в серийный порт сообщения
      in_MQ2_1 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_2 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_3 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_4 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
    }
    if (in_MQ2_4 > porog_sensorMQ2_4)
    {
      int_motorA = 10;                   // Настроить количество импульсов перемещения мотора А на датчик 4
      int_motorB = 8;                    // Настроить количество импульсов перемещения мотора B на датчик 4
      left = false;                      // Датчик находится справа
      Serial.print("MQ2_4 alarm - ");    // Вывод в серийный порт сообщения о превышении загазованности с датчика MQ2_4
      Serial.println(in_MQ2_4);          // Вывод в серийный порт сообщения об относительной величины загазованности
      Serial.print("Impulse A - ");      // Вывод в серийный порт сообщения
      Serial.println(int_motorA);        // Вывод в серийный порт сообщения
      Serial.print("Impulse B - ");      // Вывод в серийный порт сообщения
      Serial.println(int_motorB);        // Вывод в серийный порт сообщения
      in_MQ2_1 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_2 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_3 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
      in_MQ2_4 = 0;                      // Сбрасываем показания сенсора газа sensorMQ2_1
    }

    // Порог превышен - выполним программу воздействия
    digitalWrite(led13, HIGH);             // включаем светодиод
    digitalWrite(led_ind, HIGH);           // включаем светодиод
    digitalWrite(buzzer, HIGH);            // включаем зуммер
    Serial.println("Activate motor A");    // Вывод в серийный порт сообщения "включить мотор A"

    attachInterrupt(digitalPinToInterrupt(sensorFC03_1), counterA, FALLING); // Включить прерывания по импульсу от датчика sensorFC03_1
    digitalWrite(motorA_in1, HIGH);        // запуск двигателя A вперед
    digitalWrite(motorA_in2, LOW);         // запуск двигателя A вперед
    //analogWrite(motorA_en, 254); 	       // устанавливаем максимальную скорость из доступного диапазона 0~255. При плавном пуске необходимо закомментировать строку

    //++++++++++++++  Начало фрагмента ++++++++++++++++++++++++++++++++++++++
    for (int i = 0; i < 255; i++)          // Плавный запуск двигателя.
    { // Для отключения плавного пуска необходимо закомментировать фрагмент
      analogWrite(motorA_en, i);           // И раскомментировать верхнюю строку "analogWrite(enA, 254);    //устанавливаем максимальную скорость из доступного диапазона 0~255"
      delay(20);
    }
    //----------------- Конец фрагмента ------------------------------------
    Serial.print("pulsesA - ");            // Вывод в серийный порт количества поступивших импульсов
    pulsesA    = 0;                        // Сбрасываем счетчик импульсов мотора А
    while (pulsesA < int_motorA)           // Ожидаем количество импульсов с датчика sensorFC03_1
    {
      wdt_reset();                         // говорим собаке что "В Багдаде все спокойно", начинается очередной отсчет 8-и секунд.
      if (pulsesA != temp_count )
      {
        Serial.print(pulsesA);             // Вывод в серийный порт количества поступивших импульсов
        Serial.print("/");                 // Разделительный символ
        temp_count = pulsesA;
      }
    }
    detachInterrupt(digitalPinToInterrupt(sensorFC03_1));                    // Отключить прерывания по импульсу от датчика sensorFC03_1

    pulsesA    = 0;                        // Сбрасываем счетчик импульсов мотора А
    temp_count = 0;                        // Сбрасываем временный счетчик импульсов
    Serial.println();
    for (int i = 255; i >= 0; --i)         // Плавно останавливаем двигатель
    {
      analogWrite(motorA_en, i);           //
      delay(20);
    }
    // analogWrite(motorA_en, 0); 	       // Резко останавливаем мотор

    digitalWrite(motorA_in1, LOW);         // Останов двигателя A
    digitalWrite(motorA_in2, LOW);         // Останов двигателя A
    Serial.println("Switch off the motor A"); // Вывод в серийный порт сообщения "выключить мотор A"

    //==============================  Запуск двигателя В вправо ===============================
    Serial.println("Activate motor B");    // Вывод в серийный порт сообщения "включить мотор B"
    Serial.print("pulsesB - ");            // Вывод в серийный порт количества поступивших импульсов
    attachInterrupt(digitalPinToInterrupt(sensorFC03_2), counterB, FALLING); // Включить прерывания по импульсу от датчика sensorFC03_2
    if (left == false)                     // Датчик находится справа)
    {
      digitalWrite(motorB_in3, HIGH);      // Запуск двигателя В вправо
      digitalWrite(motorB_in4, LOW);       // Запуск двигателя В вправо
      Serial.println("motor B -->");       // Вывод в серийный порт сообщения  направление движения
    }
    else
    {
      digitalWrite(motorB_in3, HIGH);      // Запуск двигателя В влево
      digitalWrite(motorB_in4, LOW);       // Запуск двигателя В влево
      Serial.println("motor B <--");       // Вывод в серийный порт сообщения  направление движения
    }

    //analogWrite(motorB_en, 254);         // устанавливаем максимальную скорость из доступного диапазона 0~255. При плавном пуске необходимо закомментировать строку

    //++++++++++++++  Начало фрагмента ++++++++++++++++++++++++++++++++++++++
    for (int i = 0; i < 256; i++)          // Плавный запуск двигателя.
    { // Для отключения плавного пуска необходимо закомментировать фрагмент
      analogWrite(motorB_en, i);           // И раскомментировать верхнюю строку "analogWrite(motorB_en, 254);    //устанавливаем максимальную скорость из доступного диапазона 0~255"
      delay(10);                           // Настроить скорость
    }
    //----------------- Конец фрагмента ------------------------------------

    pulsesB    = 0;                        // Сбрасываем счетчик импульсов мотора А
    while (pulsesB < int_motorB)           // Ожидаем количество импульсов с датчика sensorFC03_2
    {
      if (pulsesB != temp_count)
      {
        Serial.print(pulsesB);             // Вывод в серийный порт количества поступивших импульсов
        Serial.print("/");                 // Разделительный символ
        temp_count = pulsesB;
      }
      wdt_reset();                         // говорим собаке что "В Багдаде все спокойно", начинается очередной отсчет 8-и секунд.
    }
    detachInterrupt(digitalPinToInterrupt(sensorFC03_2));                    // Отключить прерывания по импульсу от датчика sensorFC03_2
    Serial.print(pulsesB);                 // Вывод в серийный порт количества поступивших импульсов
    pulsesB = 0;                           // Сбрасываем счетчик импульсов мотора B
    temp_count = 0;                        // Сбрасываем временный счетчик импульсов
    for (int i = 255; i >= 0; --i)         // Плавно останавливаем двигатель
    {
      analogWrite(motorB_en, i);           //
      delay(10);                           // Настроить скорость
    }
    // analogWrite(motorB_en, 0); 	       // Резко останавливаем мотор

    digitalWrite(motorB_in3, LOW);         // Останов двигателя B
    digitalWrite(motorB_in4, LOW);         // Останов двигателя B

    Serial.println();
    Serial.println("Switch OFF the motor B"); // Вывод в серийный порт сообщения "выключить мотор B"
    Serial.println("Switch ON the rele");  // Вывод в серийный порт сообщения "включить реле"
    digitalWrite(rele1, HIGH);             // включить реле

    Serial.print("Timeout sec - ");        // Разделительный символ
    time_temp = 0;                         // Сбросить переменную в 0
    for (int i = 0; i < time_rele_on + 1; i++) // Задержка включения реле на N секунд
    {
      delay(1000);                         // Задержка на 1 секунду
      if ( i != time_temp)
      {
        Serial.print(i);                   // Вывод в серийный порт количества
        Serial.print("s/ ");               // Разделительный символ
        time_temp = i;                     // Сохранить текущее значение
      }
      wdt_reset();                         // говорим собаке что "В Багдаде все спокойно", начинается очередной отсчет 8-и секунд.
    }
    time_temp = 0;                         // Сбросить переменную в 0
    Serial.println();
    Serial.println("Switch OFF the rele"); // Вывод в серийный порт сообщения "выключить реле"
    //+++++++++++++++++++++++++++ Окончание работы автоматической противопожарной системы -----------------------------------------

    digitalWrite(led13,  LOW);             // выключить светодиод
    digitalWrite(led_ind, LOW);            // выключить светодиод
    digitalWrite(rele1,  LOW);             // выключить реле
    digitalWrite(buzzer, LOW);             // выключить зуммер


    //==============================  Возврат мотораВ в исходное положение ===============================
    Serial.println("Return to the starting position motor B");  // Вывод в серийный порт сообщения "Возврат  в исходное положение мотора B"
    Serial.print("pulsesB - ");            // Вывод в серийный порт количества поступивших импульсов
    attachInterrupt(digitalPinToInterrupt(sensorFC03_2), counterB, FALLING); // Включить прерывания по импульсу от датчика sensorFC03_2

    if (left == false)                     // Датчик находится справа)
    {
      digitalWrite(motorB_in3, HIGH);      // Запуск двигателя В влево
      digitalWrite(motorB_in4, LOW);       // Запуск двигателя В влево
      Serial.println("motor B <--");       // Вывод в серийный порт сообщения направление движения
    }
    else
    {
      digitalWrite(motorB_in3, HIGH);      // Запуск двигателя В вправо
      digitalWrite(motorB_in4, LOW);       // Запуск двигателя В вправо
      Serial.println("motor B -->");       // Вывод в серийный порт сообщения направление движения
    }

    //analogWrite(motorB_en, 254);         // устанавливаем максимальную скорость из доступного диапазона 0~255. При плавном пуске необходимо закомментировать строку

    //++++++++++++++  Начало фрагмента ++++++++++++++++++++++++++++++++++++++
    for (int i = 0; i < 255; i++)          // Плавный запуск двигателя.
    { // Для отключения плавного пуска необходимо закомментировать фрагмент
      analogWrite(motorB_en, i);           // И раскомментировать верхнюю строку "analogWrite(motorB_en, 254);    //устанавливаем максимальную скорость из доступного диапазона 0~255"
      delay(20);                           // Настроить скорость
    }
    //----------------- Конец фрагмента ------------------------------------
    pulsesB    = 0;                       // Сбрасываем счетчик импульсов мотора А
    while (pulsesB < int_motorB)          // Ожидаем количество импульсов с датчика sensorFC03_2
    {
      if (pulsesB != temp_count)
      {
        Serial.print(pulsesB);             // Вывод в серийный порт количества поступивших импульсов
        Serial.print("/");                 // Разделительный символ
        temp_count = pulsesB;
      }
      wdt_reset();                        // говорим собаке что "В Багдаде все спокойно", начинается очередной отсчет 8-и секунд.
      if (digitalRead(stop_motorB) == LOW) break; // Остановить мотор раньше времени
    }
    detachInterrupt(digitalPinToInterrupt(sensorFC03_2));                   // Отключить прерывания по импульсу от датчика sensorFC03_2
    Serial.print(pulsesB);                // Вывод в серийный порт количества поступивших импульсов
    pulsesB = 0;                          // Сбрасываем счетчик импульсов мотора B
    temp_count = 0;                       // Сбрасываем временный счетчик импульсов
    for (int i = 255; i >= 0; --i)        // Плавно останавливаем двигатель
    {
      analogWrite(motorB_en, i);          //
      delay(20);                          // Настроить скорость
    }
    // analogWrite(motorB_en, 0);          // Резко останавливаем мотор

    digitalWrite(motorB_in3, LOW);         // Останов двигателя B
    digitalWrite(motorB_in4, LOW);         // Останов двигателя B

    //================================ Возвращаем моторы в исходное положение ==============================
    Serial.println("Return to the starting position motor A");  // Вывод в серийный порт сообщения "Возврат  в исходное положение мотора А"
    Serial.print("pulsesA - ");            // Вывод в серийный порт количества поступивших импульсов
    attachInterrupt(digitalPinToInterrupt(sensorFC03_1), counterA, FALLING); // Включить прерывания по импульсу от датчика sensorFC03_1
    digitalWrite(motorA_in1, LOW);         // запуск двигателя A назад
    digitalWrite(motorA_in2, HIGH);        // запуск двигателя A назад
    //analogWrite(motorA_en, 254); 	       // устанавливаем максимальную скорость из доступного диапазона 0~255. При плавном пуске необходимо закомментировать строку

    //++++++++++++++  Начало фрагмента ++++++++++++++++++++++++++++++++++++++
    for (int i = 0; i < 255; i++)          // Плавный запуск двигателя.
    { // Для отключения плавного пуска необходимо закомментировать фрагмент
      analogWrite(motorA_en, i);           // И раскомментировать верхнюю строку "analogWrite(enA, 254);    //устанавливаем максимальную скорость из доступного диапазона 0~255"
      delay(10);
    }
    //------------------------ Конец фрагмента ------------------------------------
    pulsesA    = 0;                        // Сбрасываем счетчик импульсов мотора А
    while (pulsesA < int_motorA)           // Ожидаем количество импульсов с датчика sensorFC03_1
    {
      if (pulsesA != temp_count )
      {
        Serial.print(pulsesA);             // Вывод в серийный порт количества поступивших импульсов
        Serial.print("/");                 // Разделительный символ
        temp_count = pulsesA;
      }
      wdt_reset();                         // говорим собаке что "В Багдаде все спокойно", начинается очередной отсчет 8-и секунд.
      if (digitalRead(stop_motorA) == LOW) break; // Остановить мотор раньше времени
    }
    detachInterrupt(digitalPinToInterrupt(sensorFC03_1));                    // Отключить прерывания по импульсу от датчика sensorFC03_1
    Serial.println(pulsesA);               // Вывод в серийный порт количества поступивших импульсов
    pulsesA = 0;                           // Сбрасываем счетчик импульсов мотора А
    temp_count = 0;                        // Сбрасываем временный счетчик импульсов
    for (int i = 255; i >= 0; --i)         // Плавно останавливаем мотор
    {
      analogWrite(motorA_en, i);           //
      delay(10);
    }
    // analogWrite(motorA_en, 0); 	       // Резко останавливаем мотор

    digitalWrite(motorA_in1, LOW);         // Останов двигателя A
    digitalWrite(motorA_in2, LOW);         // Останов двигателя A


    Serial.println();
    Serial.println("Job completed");       // Вывод в серийный порт сообщения "Работа выполнена"
    Serial.println("====================================================================");
    Serial.println();
  }
  delay (50);                              // Проверяем 1 раз в 0,05 секунду. Разрешено не более 7 секунд  иначе перейдет в сброс.
  wdt_reset();                             // говорим собаке что "В Багдаде все спокойно", начинается очередной отсчет 8-и секунд.
}
