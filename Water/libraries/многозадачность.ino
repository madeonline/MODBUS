include <Servo.h>
 
class Flasher
{
  // Переменные - члены класса
  // Инициализируются при запуске
  int ledPin; // номер пина со светодиодом
  long OnTime; // время включения в миллисекундах
  long OffTime; // время, когда светодиод выключен
 
  // Текущее состояние
  int ledState; // состояние ВКЛ/ВЫКЛ
  unsigned long previousMillis; // последний момент смены состояния
 
  // Конструктор создает экземпляр Flasher и инициализирует 
  // переменные-члены класса и состояние
  public:
  Flasher(int pin, long on, long off)
  {
    ledPin = pin;
    pinMode(ledPin, OUTPUT);
 
    OnTime = on;
    OffTime = off;
 
    ledState = LOW;
    previousMillis = 0;
  }
 
  void Update()
  {
    // выясняем не настал ли момент сменить состояние светодиода
 
    unsigned long currentMillis = millis(); // текущее время в миллисекундах
 
    if((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))
    {
      ledState = LOW; // выключаем
      previousMillis = currentMillis; // запоминаем момент времени
      digitalWrite(ledPin, ledState); // реализуем новое состояние
    }
    else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime))
    {
      ledState = HIGH; // выключаем
      previousMillis = currentMillis ; // запоминаем момент времени
      digitalWrite(ledPin, ledState); // реализуем новое состояние
    }
  }
};
 
class Sweeper
{
  Servo servo; // сервопривод
  int pos; // текущее положение сервы 
  int increment; // увеличиваем перемещение на каждом шаге
  int updateInterval; // промежуток времени между обновлениями
  unsigned long lastUpdate; // последнее обновление положения 
 
  public: 
  Sweeper(int interval)
  {
    updateInterval = interval;
    increment = 1;
  }
 
  void Attach(int pin)
  {
    servo.attach(pin);
  }
 
  void Detach()
  {
    servo.detach();
  }
 
  void Update()
  {
    if((millis() - lastUpdate) > updateInterval) // время обновлять
    {
      lastUpdate = millis();
      pos += increment;
      servo.write(pos);
      Serial.println(pos);
      if ((pos >= 180) || (pos <= 0)) // конец вращения
      {
        // обратное направление
        increment = -increment;
      }
   }
 }
};
 
Flasher led1(11, 123, 400);
Flasher led2(12, 350, 350);
Flasher led3(13, 200, 222);
 
Sweeper sweeper1(15);
Sweeper sweeper2(25);
 
void setup() 
{ 
  Serial.begin(9600);
  sweeper1.Attach(9);
  sweeper2.Attach(10);
} 
 
 
void loop() 
{ 
  sweeper1.Update();
  if (digitalRead(2) == HIGH)
  {
    sweeper2.Update();
    led1.Update();
  }
 
  led2.Update();
  led3.Update
}