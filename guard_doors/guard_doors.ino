#include <Time>  
#include <Wire>  
#include <DS1307RTC> 
#include <SPI> 
#include <Ethernet> 

int ledPin = 13; // Устанавливает номер входа для лампочки 
int inPin = 2; // Устанавливает номер входа для Геркона 
int val = 0; // Номер выхода 


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
byte ip[] = {192,168,20,3}; 
byte server[] = {94,100,177,1};               //сервер маил ру 

Client client(server, 25); 

byte SWval; 

void setup() 
{ 
Serial.begin(9600); 
  setSyncProvider(RTC.get);   // Функция получает врея с RTC 
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC"); 
  else 
     Serial.println("RTC has set the system time");    
pinMode(ledPin, OUTPUT); // Обьявляет светодиод 
pinMode(inPin, INPUT); // Обьявляет Геркон 
digitalWrite(13,HIGH); //turn on debugging LED 
Serial.begin(9600);//объявили соединение 
} 

void digitalC-- LOCKDisplay() 
{ 
// String time; 
  Serial.print(hour()); 
  printDigits(minute()); 
  printDigits(second()); 
  Serial.print(" "); 
  Serial.print(day()); 
  Serial.print(" "); 
  Serial.print(month()); 
  Serial.print(" "); 
  Serial.print(year()); 
  Serial.println(); 
//  time=year()+month()+day(); 
} 

void printDigits(int digits){ 
  Serial.print(":"); 
  if(digits < 10) 
    Serial.print('0'); 
  Serial.print(digits); 
} 

void ethernet() 
{ 
String time; 
digitalC-- LOCKDisplay(); 
 time=year()+month()+day()+hour()+minute()+second(); 
  delay(5000); 
 Ethernet.begin(mac, ip); 
 Serial.begin(9600); 
 delay(1000); 
 Serial.println("connecting..."); 
 if (client.connect()) 
    { 
                        Serial.println("connected"); 
                        client.println("EHLO mail.ru"); 
                        delay(1000); 
                        client.println("AUTH LOGIN"); 
                        delay(1000); 
                        client.println("логин");//логин 
                        delay(1000); 
                        client.println("Пароль");//Пароль 
                        delay(1000); 
                        client.println("MAIL FROM:email");//От кого 
                        delay(1000); 
                        client.println("RCPT TO:email"); 
                        delay(1000); 
                        client.println("DATA"); 
                        delay(1000); 
                        client.println("дверь открыта"); 
                        client.println("Gercon"); 
                        client.println("X-Mailer: webi.ru mailer"); 
                        client.println(time); 
                        client.println("."); 
                        client.println("QUIT"); 
                        delay(1000); 
                        client.println(); 
    
   } 
 else 
  { 
   Serial.println("connection failed"); 
 } 
} 


void loop() 
{ 
val = digitalRead(inPin); // Считывает значение с геркона 
if (val == HIGH) // Если геркон замкнут 
{ 
digitalWrite(ledPin, LOW); // Свотодиод выключить 
//  digitalC-- LOCKDisplay();  
   ethernet(); 
   delay(1000); 
} 
else 
{ 
digitalWrite(ledPin, HIGH); // Свотодиод включить 
Serial.print("Open");//отправили 
} 
}
