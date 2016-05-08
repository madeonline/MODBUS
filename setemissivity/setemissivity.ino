
/*
  Коэффициент, который необходимо изменять посредством кнопок. Значение должно измняться от 0,1 до 1 с шагом в 0,01.
  Нужно три кнопки, чтобы управлять этой функцией. При нажатии на кнопку "E"(просто название кнопки) на экране должна появляться надпись
  `Please set EMC`, и должно отображаться текущее значение (его считывание изложено в скетче далее). Две другие кнопки выполняют просто функцию:
  одна из них прибавляет к отображаемому значению 0,01, другая - отнимает 0,01. То есть кнопки вроде "+" и "-". При нажатии на кнопку "Е" после установки значения
  должна происходить запись только что установленного коэффициента в память датчика, а само меню перезаписи коэффициента - исчезать.
  Важно - запись в память датчика должна происходить только при повторном нажатии кнопки "Е"*

*/


#include <Wire.h>                                                   // I2C library, required for MLX90614
#include <SparkFunMLX90614.h>                                       // та же библиотека для датчика, но с новыми функциями. Думаю, функции отображения температуры лучше оставить из старой библиотеки
#include <Adafruit_GFX.h>                                           // подключаем общую библиотеку дисплея
#include <Adafruit_PCD8544.h>                                       // подключаем библиотеку дисплея PCD8544
#include <EEPROM.h>                                                 // подключаем библиотеку памяти EEPROM

Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);         // присваиваем имя дисплею - display. В дальнейшем так будем к нему обращаться
IRTherm therm;                                                      // Объект - термометр

float newEmissivity = 0.98;                                         // Коэффициент, который необходимо изменять посредством кнопок. Значение должно измняться от 0,1 до 1 с шагом в 0,01.
#define kn_freeze                   2                               // Назначение кнопки "Остановить показания дисплея". Подключаем один конец кнопки к выводу 2 Ардуино, второй конец на землю !!Изменять нельзя - подключено прерывание
#define kn_E                        8                               // Назначение кнопки "Set EMC".                      Подключаем один конец кнопки к выводу 3 Ардуино, второй конец на землю
#define kn_plus                     9                               // Назначение кнопки "+".                            Подключаем один конец кнопки к выводу 4 Ардуино, второй конец на землю
#define kn_minus                   10                               // Назначение кнопки "-".                            Подключаем один конец кнопки к выводу 5 Ардуино, второй конец на землю
#define LED_PIN                    13                               // Назначение светодиода для индикации заморозки                                                           


unsigned long currentMillis = 0;                                    // Переменная для отсчета периода измерения
long interval               = 250;                                  // интервал обновления информации на дисплее, мс  ( 0.5 скунды)
boolean off_display         = true;                                 // флаг разрешения остановки измерений
boolean freeze_display      = false;                                // флаг разрешения остановки измерений
boolean start_display       = true;                                 // Первый запуск системы
boolean set_EMC             = false;                                // Признак записи коэффициента в память
//boolean change_EMC          = false;                                // Признак изменения коэффициента
boolean set_mem             = false;                                // Признак записи коэффициента в память
boolean read_MLX90614       = false;                                // Признак подключения MLX90614
float tek_temp              = 0;                                    // Текущее значение температуры
float max_temp              = 0;                                    // Максимальное значение температуры
float min_temp              = 1000;                                 // Миниимальное значение температуры
float max_tmp               = 0;                                    // Временное хранение максимальное значение температуры
float min_tmp               = 1000;                                 // Временное хранение миниимальное значение температуры
float max_display           = 0;                                    // Отображение максимальное значение температуры
float min_display           = 1000;                                 // Отображение миниимальное значение температуры
unsigned int counter        = 0;                                    // Счетчик подсчета количества измерений для вычисления максимального и минимального значения
float emissivity_level      = 0;                                    // Коэффициент компенсации
float emissivity_level_tmp  = 0;                                    // Коэффициент компенсации



void EEPROM_float_write(int addr, float val)                        // Программа записи в ЕЕPROM чисел с плавающей запятой
{
  byte *x = (byte *)&val;
  for (byte i = 0; i < 4; i++) EEPROM.write(i + addr, x[i]);
}

float EEPROM_float_read(int addr)                                   // Программа чтения из ЕЕPROM чисел с плавающей запятой
{
  byte x[4];
  for (byte i = 0; i < 4; i++) x[i] = EEPROM.read(i + addr);
  float *y = (float *)&x;
  return y[0];
}

void freeze_process()                                                // Программа обработки прерывания принажатии на кнопку заморозки
{
  if (!set_EMC)                                                       // Заморозка возможна когда не работаем с EMC
  {
    //++++++++++++++++++++ Проверяем кнопку заморозки ++++++++++++++++++++++++++++++++++++++++++++++
    if (digitalRead(kn_freeze) == LOW )                               // Проверяем нажата ли кнопка
    {
      while (digitalRead(kn_freeze) == LOW ) {}                       // Если нажата - ожидаем отпускания кнопки
      off_display = !off_display;                                     // изменяем флаг на противоположный
      freeze_display = true;
    }
    // --------------------Конец проверки ----------------------------------------------------------
  }
}



void setup()
{
  Serial.begin(9600);                                                // инициализация порта
  therm.begin();                                                     // инициализация сенсора
  therm.setUnit(TEMP_C);                                             // Set the library's units to  Цельсия

  // Call setEmissivity() to configure the MLX90614's
  // emissivity compensation:
  // therm.setEmissivity(newEmissivity);                                //собственно установка newEmissivity в качестве коэффициента и его запись в память датчика

  // readEmissivity() can be called to read the device's
  // configured emissivity -- it'll return a value between
  // 0.1 and 1.0. (эта функция вызывает отображение коэффициента в памяти датчика - значения от 0,1 до 1)
  // Serial.println("Emissivity: " + String(therm.readEmissivity())); //печать слова + самого значения
  display.begin();                                                   // инициализация дисплея
  display.setContrast(60);                                           // установка констрастности дисплея
  delay(1000);                                                       // задержка на 1 секунду для завершения инициализации
  display.clearDisplay();                                            // очистка экрана дисплея
  display.setTextSize(1);                                            // выбор размера текста
  display.setTextColor(BLACK);                                       // выбор цвета текста
  pinMode(kn_freeze, INPUT);                                         // настроить вывод кнопки на ввод
  pinMode(kn_E,      INPUT);                                         // настроить вывод кнопки на ввод
  pinMode(kn_plus,   INPUT);                                         // настроить вывод кнопки на ввод
  pinMode(kn_minus,  INPUT);                                         // настроить вывод кнопки на ввод
  digitalWrite(kn_freeze, HIGH);                                     // поддерживаем высокий уровень(+5в) на контакте кнопки
  digitalWrite(kn_E,      HIGH);                                     // поддерживаем высокий уровень(+5в) на контакте кнопки
  digitalWrite(kn_plus,   HIGH);                                     // поддерживаем высокий уровень(+5в) на контакте кнопки
  digitalWrite(kn_minus,  HIGH);                                     // поддерживаем высокий уровень(+5в) на контакте кнопки
  pinMode(LED_PIN, OUTPUT);                                          // настроить вывод светодиода на вывод
  digitalWrite(LED_PIN, LOW);                                        // выключить светодиод
  emissivity_level  = EEPROM_float_read(0);                          // Чтение коэффициента из памяти EEPROM
  if (emissivity_level == 0)                                         // Если память пустая записываем начальное значение коэффициента
  {
    therm.setEmissivity(newEmissivity);                              // собственно установка newEmissivity в качестве коэффициента и его запись в память датчика
  }
  else                                                               // В памяти есть сохранненая информация о коэффициенте
  {
    if (emissivity_level > 0 && emissivity_level < 1)                // Если информация находится в разрешенном диапазоне от 0 до 1 записываем в память датчика
    {
      therm.setEmissivity(emissivity_level);                         // собственно установка newEmissivity в качестве коэффициента и его запись в память датчика
    }
    else                                                             // Информация вне диапазона, запишем начальное состояние на всякий случай.
    {
      therm.setEmissivity(newEmissivity);                            // установка newEmissivity в качестве коэффициента и его запись в память датчика
    }
  }

  display.setCursor(18, 20);
  display.print("MLX90614");
  display.display();
  delay(1000);
  display.clearDisplay();
  attachInterrupt(0, freeze_process, LOW);                          // Включаем прерывания на реакцию кнопки заморозки
}

void loop()
{
  //if (1 == 1)                                                         // Отключение проверки датчика
  if (therm.read())                                                   // Проверяем подключение датчика On success, read() will return 1, on fail 0.
  {
    read_MLX90614 = true;                                             // Признак успешного чтения датчика MLX90614

    // +++++++++++++++++++ Проверяем кнопки коррекции ++++++++++++++++++++++++++++++++++++++++++++++
    if (digitalRead(kn_E) == LOW && off_display )                    // Проверяем нажата ли кнопка "Set EMC"
    {
      while (digitalRead(kn_E) == LOW ) {}                            // Если нажата - ожидаем отпускания кнопки
      emissivity_level_tmp = emissivity_level;                        // Сохраняем текущее знчение EMC
      display.clearDisplay();                                         // очистка экрана дисплея
      display.println("  Please set");                                // Выводим на дисплей "  Please set"
      display.println("     EMC");                                    // Выводим на дисплей "     EMC"
      emissivity_level = therm.readEmissivity();                      // Получить данные коррикции из устройства
      display.println("Emissivity: ");                                // Выводим на дисплей "Emissivity: "
      display.println(emissivity_level);                              // Выводим на дисплей коэффициент
      display.display();                                              // Запускаем отображение на дисплей
      delay(200);                                                     // Ждем отрыва пальца (от кнопки), и не дрожим
      set_EMC = true;                                                 // Сообщаем что работаем с EMC
      do {
        if (digitalRead(kn_E) == LOW )                                // Проверяем нажата ли кнопка "Set EMC"
        {
          while (digitalRead(kn_E) == LOW ) {}                        // Если нажата - ожидаем отпускания кнопки
          if ( emissivity_level != emissivity_level_tmp)              // Проверяем были ли изменения EMC
          {
            therm.setEmissivity(emissivity_level);                    // собственно установка newEmissivity в качестве коэффициента и его запись в память датчика
            EEPROM_float_write(0, emissivity_level);                  // Записываем в память EEPROM. При повторном старте эти данные будут записаны в датчик
            display.println("");
            display.println("");
            display.print("   EMC set");
            display.display();
            delay(1000);
          }
          else
          {
            display.println("");
            display.print("  EMC no set");
            display.display();
            delay(1000);
          }
          set_mem = true;                                           // Флаг выполнения операции записи в память EEPROM.
          counter = 0;                                              // Новый счет измерений
          max_display = 0;                                          // Информацию об максимальном результате обнуляем, выводим на дисплей
          min_display = 0;                                          // Информацию об минимальном результате обнуляем, выводим на дисплей
          min_tmp = 1000;                                           // Переменную минимального результата устанавливаем в исходное состояние
          max_tmp = 0;

          set_EMC = false;                                            // Сообщаем чтозавершили работу с EMC
        }
        if (digitalRead(kn_plus) == LOW )                             // Проверяе нажата ли кнопка "Set EMC"
        {
          while (digitalRead(kn_plus) == LOW ) {}                     // Если нажата - ожидаем отпускания кнопки
          display.setTextColor(WHITE);
          display.setCursor(0, 24);
          display.print(emissivity_level);
          display.setTextColor(BLACK);
          emissivity_level += 0.01;                                   // Прибавляем 0,01
          if (emissivity_level > 1)  emissivity_level = 1;            // Если больше 1 - остановить прибавление
          display.setCursor(0, 24);
          display.print(emissivity_level);                            // Выводим на дисплей коэффициент
          display.display();
        }
        if (digitalRead(kn_minus) == LOW )                            // Проверяе нажата ли кнопка "Set EMC"
        {
          while (digitalRead(kn_minus) == LOW ) {}                    // Если нажата - ожидаем отпускания кнопки
          display.setTextColor(WHITE);
          display.setCursor(0, 24);
          display.print(emissivity_level);
          display.setTextColor(BLACK);
          emissivity_level -= 0.01;                                   // Прибавляем 0,01
          if (emissivity_level < 0)  emissivity_level = 0;            // Если меньше - остановить уменьшение
          display.setCursor(0, 24);
          display.print(emissivity_level);                            // Выводим на дисплей коэффициент
          display.display();
        }
      } while (!set_mem);

      set_mem = false;                                                // Новые данные сохранены, дальше сохранять не нужно
    }
    // --------------------Конец проверки ----------------------------------------------------------
    // +++++++++++++++++++ Измерение температуры +++++++++++++++++++++++++++++++++++++++++++++++++++
    if (millis() - currentMillis > interval)                           // проверяем прошло ли 0.25 секунды, если время наступило - произвести измерения
    {
      digitalWrite(LED_PIN, HIGH);                                     // включить светодиод
      //if (1 == 1)                                                         // Отключение проверки датчика
      if (therm.read())                                                // Повторно проверяем подключения к датчику, может отключился?
      {
        read_MLX90614 = true;                                          // Признак успешного чтения датчика MLX90614
        currentMillis = millis();                                      // сохраняем время последнего обновления
        digitalWrite(LED_PIN, HIGH);                                   // включить светодиод
        tek_temp = therm.object();                                     // Текущее значение температуры
        // tek_temp = random(10.25, 65.99);                               // Строка для отладки без датчика температуры
        min_tmp = min(min_tmp, tek_temp);                              // Вычисление минимального значения температуры
        max_tmp = max(max_tmp, tek_temp);                              // Вычисление максимального значения температуры
        min_display = min_tmp;                                         // Результат готов, выводим на дисплей
        max_display = max_tmp;                                         // Результат готов, выводим на дисплей
        counter++;
        if (counter >= 65535) counter = 0;
      }
      else
      {
        read_MLX90614 = false;                                         // Признак неудачи чтения датчика MLX90614
      }
    }

    if (off_display)                                                  // проверяем прошло ли 0,25 секунды и разрешено обновление показаний дисплея
    {
      display.clearDisplay();                                         // очистка дисплея
      display.setTextColor(BLACK);
      display.println(counter);
      display.print("Tek  ");                                         // значок
      display.print(tek_temp);                                        // вывод значения температуры в C
      display.println("C");                                           // значок С
      display.print("Max  ");                                         // значок
      display.print(max_display);                                     // вывод максимального значения температуры в C
      display.println("C");                                           // значок С
      display.print("Min  ");                                         // значок
      display.print(min_display);                                     // вывод минимального значения температуры в C
      display.println("C");                                           // значок С
      if (!read_MLX90614)
      {
        display.println("  MLX90614");
        display.println("not connected");
      }
      display.display();                                             // отображение данных
      delay(300);                                                      // немного посветить светодиодом
      digitalWrite(LED_PIN, LOW);                                      // выключить светодиод
    }

    //+++++++++++++++++++++++ Проверяем "заморозку" дисплея ++++++++++++++++++++++++++++
    if (freeze_display)
    {
      display.println();
      display.println("    Freeze");                                   //
      display.display();                                               // отображение данных
      counter = 0;                                                     // Новый счет измерений
      max_display = 0;                                                 // Информацию об максимальном результате обнуляем, выводим на дисплей
      min_display = 0;                                                 // Информацию об минимальном результате обнуляем, выводим на дисплей
      min_tmp = 1000;                                                  // Переменную минимального результата устанавливаем в исходное состояние
      max_tmp = 0;                                                     // Переменную максимального результата устанавливаем в исходное состояние
      freeze_display      = false;                                     // Подтверждаем выполнение программы "заморозки" показаний
    }
    // ------------------- Конец программы  "заморозки" дисплея --------------------------------------
  }
  else                                                                 // Датчик не подключен, измерять невозможно.
  {
    read_MLX90614 = false;                                             // Признак успешного чтения датчика MLX90614, Датчик отсутствует
    display.clearDisplay();                                            // очистка дисплея
    display.setCursor(18, 20);
    display.println("MLX90614");
    display.println("not connected");
    display.display();
  }
  delay(20);
}


