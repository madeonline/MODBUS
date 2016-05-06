/*
   Цель: контроль открытия дверей в не рабочее время и праздники (контроль охраны)
   Задача: получать письмо на email при открытии дверей (доп. иметь файл с логами за все время, как пассивная защита от воровства)

*/


#include <Time>
#include <Wire>
#include <DS1307RTC>
#include <SPI>
#include <Ethernet>

int ledPin = 13;                                       // Устанавливает номер входа для лампочки
int inPin = 2;                                         // Устанавливает номер входа для Геркона
int val = 0;                                           // Номер выхода


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   // установить свой
byte ip[]  = {192, 168, 0, 30};
byte server[] = {94, 100, 177, 1};                     //сервер mail.ru

Client client(server, 25);

byte SWval;


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

void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void ethernet()
{
  String time;
  digitalC-- LOCKDisplay();
  time = year() + month() + day() + hour() + minute() + second();
  delay(5000);
  Ethernet.begin(mac, ip);
 // Serial.begin(9600);
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
void setup()
{
  Serial.begin(9600);
  setSyncProvider(RTC.get);                           // Функция получает время с RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
  pinMode(ledPin, OUTPUT);                             //  
  pinMode(inPin, INPUT);                               //  
  digitalWrite(13, HIGH);                              // 
}

void loop()
{
  val = digitalRead(inPin);                           // Считывает значение с геркона
  if (val == HIGH)                                    // Если геркон замкнут
  {
    digitalWrite(ledPin, LOW);                        // Светодиод выключить
    //  digitalC-- LOCKDisplay();
    ethernet();
    delay(1000);
  }
  else
  {
    digitalWrite(ledPin, HIGH);                      // Светодиод включить
    Serial.print("Open");//отправили
  }
}

/*
 * 
 *  Отработка передачи , переработать  mail.ru ввели шифрование, теперь отправить почту можно только на 465 порт с шифрованием SSL.
#include <Base64.h>
#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>

//структура для сообщения
struct Message
{
  String From;//адрес отправителя
  String To;// адрес получателя
  String Subject;// Тема письма
  String Body;// Текст письма
};

//mac адрес устройства (можно не менять)
byte mac[] = { 
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x05 };

//IP адрес адруины (можно не менять)   
IPAddress ip(192,168,0,54);

// IP smtp сервера
IPAddress server(94, 100, 177, 1);// mail.ru
// IP других сереров можно найти в инете 

// логин и пароль к почтовому сервису
char login[] = "ВашЛогин";//без @mail.ru
char password[] = "ВашПароль";

// создаем экземпляр клиента
EthernetClient client;

void setup(){
  Serial.begin(9600);// UART
  Ethernet.begin(mac,ip);// Запускаем шилд
  Serial.println("Start");
}

void loop(){
  while(Serial.available()){
    char inSerialChar = 0;
    inSerialChar = Serial.read();
    if(inSerialChar == 's'){
      SendMessage();// При отправке в дуину 's', отправляем сообщение.
    }
  }
}
// метод подключения к серверу
bool connectToServer(){
  if(client.connected()){
  }
  else{
    if(client.connect(server,25)){
      return true;
    }
    else{
      return false;
    }
  }
}

// метод отправки сообщения
void SendMessage(){
  Serial.println("Please wite...");
  LogPass();
  Message message = {
    "ваш_почтовый_ящик@mail.ru",
    "почтовый_ящик_получателя@gmail.com",
    "Проба пера",//тема
    "Огромный привет от Ардуино!!!!" };//само сообщение
  client.println("MAIL From:<"+message.From+">");
  delay(250);
  client.println("RCPT To:<"+message.To+">");
  delay(250);
  client.println("DATA");
  delay(250);
  client.println("From:"+message.From);
  client.println("To:"+message.To);
  client.println("Subject:"+message.Subject);
  delay(250); 
  client.println(message.Body);
  delay(250);
  client.println(".");// по приходу точки сервер ставит сообщение в очередь 
  delay(250); // на отправку
  client.println("QUIT");// как только закрыли соединение, сообщение улетает
  delay(250);
  Serial.println("Message sending...");
}

// подключение к сереверу и авторизация
void LogPass(){
  char array[64];// массив для кодирования
  if(connectToServer()){
    client.println(F("helo 1.2.3.4"));
    delay(250);
    client.println(F("AUTH LOGIN"));
    delay(250);
    client.println(strToBase64(login));
    delay(250);
    client.println(strToBase64(password));
    delay(250);

  }
  else{
    Serial.println("Server not connected...");
  }

}

// кодировка Base64
String strToBase64(char str[]){
  char charArray[64];
  base64_encode(charArray,str,strlen(str));
  String result = charArray;
  return result;
}

 */







