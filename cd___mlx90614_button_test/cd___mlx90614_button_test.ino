
#include <Adafruit_GFX.h>                                     // подключаем общую библиотеку дисплея
#include <Adafruit_PCD8544.h>                                 // подключаем библиотеку дисплея PCD8544
#include <Wire.h>                                             // подключаем библиотеку протокола 1Ware для работы по протоколу I2C
#include <Adafruit_MLX90614.h>                                // подключаем библиотеку термодатчика  MLX90614


Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);   // присваиваем имя дисплею - display. В дальнейшем так будем к нему обращаться
                                                              // подключаем к выводам Ардуино 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();                  // присваиваем имя термодатчику - mlx. В дальнейшем так будем к нему обращаться
                                                              // термодатчик подключен по шине I2C, вывода в Arduino Nano  A4 (SDA) и A5 (SCL)
                                                              // Arduino Mega2560 20 (SDA) и 21 (SCL)
unsigned long currentMillis = 0;                              // Переменная для отсчета периода измерения
long interval               = 500;                            // интервал обновления информации на дисплее, мс  ( 3 скунды)
#define kn_freeze             2                               // Назначение кнопки "Остановить показания дисплея". Подключаем один конец кнопки к выводу 2 Ардуино, второй конец на землю
#define led                  13                               // Назначение светодиода для индикации заморозки
boolean off_display         = true;                           // флаг разрешения остановки измерений   
boolean freeze_display      = false;                          // флаг разрешения остановки измерений    
boolean start_display       = true;   
float tek_temp              = 0;                              // Текущее значение температуры  
float max_temp              = 0;                              // Максимальное значение температуры
float min_temp              = 1000;                           // Миниимальное значение температуры                                              
float max_tmp               = 0;                              // Временное хранение максимальное значение температуры
float min_tmp               = 1000;                           // Временное хранение миниимальное значение температуры    
float max_display           = 0;                              // Отображение максимальное значение температуры
float min_display           = 1000;                           // Отображение миниимальное значение температуры      
unsigned int counter        = 0;                              // Счетчик подсчета количества измерений для вычисления максимольного и минимального значения 



   
void setup()                                                  // настройка параметров
{
  mlx.begin();                                                // инициализация термодатчика
  display.begin();                                            // инициализация дисплея
  display.setContrast(60);                                    // установка констрастности дисплея
  delay(1000);                                                // задержка на 1 секунду для завершения инициализации
  display.clearDisplay();                                     // очистка экрана дисплея
  display.setTextSize(1);                                     // выбор размера текста
  display.setTextColor(BLACK);                                // выбор цвета текста
  pinMode(kn_freeze, INPUT);                                  // настроить вывод кнопки на ввод
  digitalWrite(kn_freeze, HIGH);                              // поддерживаем высокий уровень(+5в) на контакте кнопки 
  pinMode(led, OUTPUT);                                       // настроить вывод светодиода на вывод
  digitalWrite(led, LOW);                                     // выключить светодиод
}

void loop()
{
  if (digitalRead(kn_freeze) == LOW )                         // Проверяе нажата ли кнопка
  {
     while (digitalRead(kn_freeze) == LOW ) {}                // Если нажата - ожидаем отпускания кнопки 
     off_display = !off_display;                              // изменяем флаг на противоположный
     freeze_display = true;
  }

 if (millis() - currentMillis > interval)                     // проверяем прошло ли 0.5 секунды 
    {
      currentMillis = millis();                               // сохраняем время последнего обновления
      digitalWrite(led, HIGH);                                // включить светодиод
    //  tek_temp = mlx.readObjectTempC();                       // Текущее значение температуры  
      tek_temp = random(10.25, 65.99);
      min_tmp = min(min_tmp,tek_temp);                        // Вычисление минимального значения температуры
      max_tmp = max(max_tmp,tek_temp);                        // Вычисление максимального значения температуры
      counter ++;                                             // Считаем количество измерений
      if(counter > 10)                                        // Провели 10 измерений
       {
         counter = 0;                                         // Новый счет измерений
         max_display = max_tmp;                               // Результат готов, выводим на дисплей
         min_display = min_tmp;                               // Результат готов, выводим на дисплей
         min_tmp = 1000;
         max_tmp = 0;
         start_display = false;
       }     
     }
 if (off_display)                                             // проверяем прошло ли 3 секунды и разрешено обновление показаний дисплея
    {
  //  digitalWrite(led, HIGH);                                // включить светодиод
      display.clearDisplay();                                 // очистка дисплея
      display.println(counter);
      display.print("Tek  ");                                 // значок 
      display.print(tek_temp);                                // вывод значения температуры в C
      display.println("C");                                   // значок С
     if(start_display)                                        // первый запуск дисплея
      {
      display.print("Max  ");                                 // значок 
      display.print(0.00);                                    // вывод 0.00 значения температуры в C
      display.println("C");                                   // значок С
      display.print("Min  ");                                 // значок 
      display.print(0.00);                                    // вывод 0.00 значения температуры в C
      display.println("C");                                   // значок С
      display.display();                                      // отображение данных
      }
      else
      {
      display.print("Max  ");                                 // значок 
      display.print(max_display);                             // вывод максимального значения температуры в C
      display.println("C");                                   // значок С
      display.print("Min  ");                                 // значок 
      display.print(min_display);                             // вывод минимального значения температуры в C
      display.println("C");                                   // значок С
      display.display();                                      // отображение данных
      }
      delay(300);                                             // немного посветить светодиодом   
      digitalWrite(led, LOW);                                 // выключить светодиод
    }
 if(freeze_display) 
    {
        display.println(); 
        display.println("    Freeze");                        // 
        display.display();                                    // отображение данных
        freeze_display      = false;  
    }
     delay(200);
}


