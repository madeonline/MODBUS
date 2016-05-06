#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h> // та же библиотека для датчика, но с новыми функциями. Думаю, функции отображения температуры лучше оставить из старой библиотеки

IRTherm therm; // Объект - термометр

const byte LED_PIN = 8; // просто подключенный к проекту светодиод. Не нужен

float newEmissivity = 0.98; //Коэффициент, который необходимо изменять посредством кнопок. Значение должно измняться от 0,1 до 1 с шагом в 0,01.
                            //Нужно три кнопки, чтобы управлять этой функцией. При нажатии на кнопку "E"(просто название кнопки) на экране должна появляться надпись
                            //`Please set EMC`, и должно отображаться текущее значение (его считывание изложено в скетче далее). Две другие кнопки выполняют просто функцию:
                            //одна из них прибавляет к отображаемому значению 0,01, другая - отнимает 0,01. То есть кнопки вроде "+" и "-". При нажатии на кнопку "Е" после установки значения
                            //должна происходить запись только что установленного коэффициента в память датчика, а само меню перезаписи коэффициента - исчезать. Важно - запись в память датчика должна происходить только при повторном нажатии кнопки "Е"

void setup() 
{
  Serial.begin(9600); // инициализация порта
  Serial.println("Press any key to begin"); //надпись в мониторе порта
  while (!Serial.available()) ;
  therm.begin(); // инициализация сенсора
  therm.setUnit(TEMP_F); // Set the library's units to Farenheit. Надо бы в Цельсиях..

  // Call setEmissivity() to configure the MLX90614's 
  // emissivity compensation:
  therm.setEmissivity(newEmissivity); //собственно установка newEmissivity в качестве коэффициента и его запись в память датчика

  // readEmissivity() can be called to read the device's
  // configured emissivity -- it'll return a value between
  // 0.1 and 1.0. (эта функция вызывает отображение коэффициента в памяти датчика - значения от 0,1 до 1)
  Serial.println("Emissivity: " + String(therm.readEmissivity())); //печать слова + самго значения
  pinMode(LED_PIN, OUTPUT); // LED pin as output. Не нужно
  setLED(LOW); // LED OFF
}

void loop() 
{
  setLED(HIGH); //LED on. Не нужно
  
  // Call therm.read() to read object and ambient temperatures from the sensor.
  if (therm.read()) // On success, read() will return 1, on fail 0.
  {
    // Use the object() and ambient() functions to grab the object and ambient
  // temperatures.
  // They'll be floats, calculated out to the unit you set with setUnit().
    Serial.print("Object: " + String(therm.object(), 2));           //Эти функции схожи с теми, что использовались для чтения температуры в прошлом скетче на базе другой библиотеки.
    Serial.write('°'); // Degree Symbol                             //Желательно оставить те, что были. В идеале можно использовать функции старой библиотеки для вывода температуры,
    Serial.println("F");                                            //а новой - для установки значения коэффициента. Данные, разумеется, нужно выводить на дисплей, а не в порт)
    Serial.print("Ambient: " + String(therm.ambient(), 2));         //Сам алгоритм для максимума и минимума, который был в рошлом скетче, необходимо изменить. Теперь заложенная в прошлом
                                                                    //скетче кнопка еще и запускает процесс измерения, а данные вроде минимума и максимума отображаются как и было. При нажатии на кнопку второй раз Показания замирают, как и ранее.
                                                                    //При нажатии следующий раз, цикл измерений запускается еще раз, а старые данные максимума и минимума стираются. Сами измерения производятся с интервалом 0,25 сек. Но раньше было
                                                                    //установлено количество измерений и конечно время - теперь это время и количество измерений определяется интервалом, в котором мы нажимали кнопку. Сколько времени прошло между
                                                                    //нажатиями на кнопку - столько точек и проверилось. В целом, намного сложнее)
    Serial.write('°'); // Degree Symbol
    Serial.println("F");
    Serial.println();
  }
  setLED(LOW);
  delay(500);
}

void setLED(bool on)                                                //НЕ НУЖНО)
{
  if (on)
    digitalWrite(LED_PIN, LOW);
  else
    digitalWrite(LED_PIN, HIGH);
}

