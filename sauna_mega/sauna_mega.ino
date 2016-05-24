#include <EEPROM.h>
#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include <SPI.h>
#include <SD.h> 
#include <Ethernet2.h>


#define REQ_BUF_SZ 20
#define BUFFER_SIZE 256

byte second, minute, hour, dayOfWeek, dayOfMonth;
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
byte addr1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37};
File webFile;
boolean flag = 0;
String t;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  
byte ip[] = { 192, 168, 1, 25 };            
EthernetServer server(80);  
char HTTP_req[REQ_BUF_SZ] = {0};
char req_index = 0; 
byte c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14   = 0;

byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}
void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth)
{
  Wire.beginTransmission(0x68);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(0x68, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
}
void setup() {
  Wire.begin();
  Serial.begin(9600);
  SD.begin(4);
  pinMode(9, OUTPUT);
  pinMode(6, OUTPUT);
  Ethernet.begin(mac, ip);
  server.begin();
  EEPROM.write(42, 25);
}


void loop() {
  EthernetClient client = server.available();
  byte buff[BUFFER_SIZE];
  
  byte h = htu.readHumidity();
  byte tt = htu.readTemperature();
  EEPROM.write(38, tt);
  if (client) {
    boolean currentIsBlank = true;
    while (client.connected()) {   
      if (client.available()) {
        char c = client.read();
        if (t.length() < 25) {
          t += c;
        }
        if (req_index < (REQ_BUF_SZ - 1)) {
              HTTP_req[req_index] = c;
              req_index++;
        }
        Serial.print(c);
        String s = t.substring(5,9);
         if (c == '\n' && currentIsBlank) {
           if (StrContains(HTTP_req, "GET / ") || StrContains(HTTP_req, "GET / index.htm") || StrContains(HTTP_req, "GET /index.htm")) {
             client.println("HTTP/1.1 200 OK");
             client.println("Content-Type: text/html");
             client.println("Connnection: close");
             client.println();
             webFile = SD.open("index.htm");
           }else if (StrContains(HTTP_req, "ajax_flame")) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: keep-alive");
            client.println();
            client.print(EEPROM.read(38));
            client.print(":");
            client.print(h);
            client.print(":");
            client.print(EEPROM.read(39));
            client.print(":");
            client.print(EEPROM.read(50));
          }else if (StrContains(HTTP_req, "able")) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connnection: close");
            client.println();
              int week = t.substring(9,10).toInt();//dayOfWeek
              if (week == 1) {
              int hourOn = t.substring(11,13).toInt();
              int minOn = t.substring(14,16).toInt();
              int hourOff = t.substring(17,19).toInt();
              int minOff = t.substring(20,22).toInt();
              EEPROM.write(addr1[0], week);      
              EEPROM.write(addr1[1], hourOn);
              EEPROM.write(addr1[2], minOn);
              EEPROM.write(addr1[3], hourOff);
              EEPROM.write(addr1[4], minOff);      
              }
              if (week == 2) {
              int hourOn = t.substring(11,13).toInt();
              int minOn = t.substring(14,16).toInt();
              int hourOff = t.substring(17,19).toInt();
              int minOff = t.substring(20,22).toInt();
              EEPROM.write(addr1[5], week);      
              EEPROM.write(addr1[6], hourOn);
              EEPROM.write(addr1[7], minOn);
              EEPROM.write(addr1[8], hourOff);
              EEPROM.write(addr1[9], minOff);  
              }
              if (week == 3) {
              int hourOn = t.substring(11,13).toInt();
              int minOn = t.substring(14,16).toInt();
              int hourOff = t.substring(17,19).toInt();
              int minOff = t.substring(20,22).toInt();
              EEPROM.write(addr1[10], week);      
              EEPROM.write(addr1[11], hourOn);
              EEPROM.write(addr1[12], minOn);
              EEPROM.write(addr1[13], hourOff);
              EEPROM.write(addr1[14], minOff);  
              }
              if (week == 4) {
              int hourOn = t.substring(11,13).toInt();
              int minOn = t.substring(14,16).toInt();
              int hourOff = t.substring(17,19).toInt();
              int minOff = t.substring(20,22).toInt();
              EEPROM.write(addr1[15], week);      
              EEPROM.write(addr1[16], hourOn);
              EEPROM.write(addr1[17], minOn);
              EEPROM.write(addr1[18], hourOff);
              EEPROM.write(addr1[19], minOff);
              }
              if (week == 5) {
              int hourOn = t.substring(11,13).toInt();
              int minOn = t.substring(14,16).toInt();
              int hourOff = t.substring(17,19).toInt();
              int minOff = t.substring(20,22).toInt();
              EEPROM.write(addr1[20], week);      
              EEPROM.write(addr1[21], hourOn);
              EEPROM.write(addr1[22], minOn);
              EEPROM.write(addr1[23], hourOff);
              EEPROM.write(addr1[24], minOff); 
              }  
              if (week == 6) {
              int hourOn = t.substring(11,13).toInt();
              int minOn = t.substring(14,16).toInt();
              int hourOff = t.substring(17,19).toInt();
              int minOff = t.substring(20,22).toInt();
              EEPROM.write(addr1[25], week);      
              EEPROM.write(addr1[26], hourOn);
              EEPROM.write(addr1[27], minOn);
              EEPROM.write(addr1[28], hourOff);
              EEPROM.write(addr1[29], minOff);   
              }   if (week == 7) {
              int hourOn = t.substring(11,13).toInt();
              int minOn = t.substring(14,16).toInt();
              int hourOff = t.substring(17,19).toInt();
              int minOff = t.substring(20,22).toInt();
              EEPROM.write(addr1[30], week);      
              EEPROM.write(addr1[31], hourOn);
              EEPROM.write(addr1[32], minOn);
              EEPROM.write(addr1[33], hourOff);
              EEPROM.write(addr1[34], minOff);
              }
          }else if (StrContains(HTTP_req, "time")) {
             client.println("HTTP/1.1 200 OK");
             client.println("Content-Type: text/html");
             client.println("Connnection: close");
             client.println();
             c1 = 0;
             c2 = 0;
             c3 = 0;
             c4 = 0;
             c5 = 0;
             c6 = 0;
             c7 = 0;
             c8 = 0;
             c9 = 0;
             c10 = 0;
             c11 = 0;
             c12 = 0;
             c13 = 0;
             c14 = 0;
              boolean f = false;
              boolean a = false;
              boolean b = false;
              boolean c = false;
              boolean d = false;
              boolean r = false;
              boolean y = false;
            for (int p = 5; p <= 9; p++){
               if (EEPROM.read(addr1[p]) == 255) f = false; 
               else a = true;
             }
             for (int p = 10; p <= 14; p++){
               if (EEPROM.read(addr1[p]) == 255) f = false; 
               else b = true;
             }
             for (int p = 15; p <= 19; p++){
               if (EEPROM.read(addr1[p]) == 255) f = false; 
               else c = true;
             }
             for (int p = 20; p <= 24; p++){
               if (EEPROM.read(addr1[p]) == 255) f = false; 
               else d = true;
             }
             for (int p = 25; p <= 29; p++){
               if (EEPROM.read(addr1[p]) == 255) f = false; 
               else r = true;
             }
             for (int p = 30; p <= 34; p++){
               if (EEPROM.read(addr1[p]) == 255) f = false; 
               else y = true;
             }
             for (int p = 0; p <= 4; p++){
               if (EEPROM.read(addr1[p]) == 255) f = false; 
               else f = true;
             }String per[35] = "";
               for(int p=1;p<=4;p++){if ( EEPROM.read(addr1[p]) < 10 ) { per[p] = "0" + String(EEPROM.read(addr1[p])); } else per[p]=String(EEPROM.read(addr1[p]));}
               for(int p=6;p<=9;p++){if ( EEPROM.read(addr1[p]) < 10 ) { per[p] = "0" + String(EEPROM.read(addr1[p])); } else per[p]=String(EEPROM.read(addr1[p]));}
               for(int p=11;p<=14;p++){if ( EEPROM.read(addr1[p]) < 10 ) { per[p] = "0" + String(EEPROM.read(addr1[p])); } else per[p]=String(EEPROM.read(addr1[p]));}
               for(int p=16;p<=19;p++){if ( EEPROM.read(addr1[p]) < 10 ) { per[p] = "0" + String(EEPROM.read(addr1[p])); } else per[p]=String(EEPROM.read(addr1[p]));}
               for(int p=21;p<=24;p++){if ( EEPROM.read(addr1[p]) < 10 ) { per[p] = "0" + String(EEPROM.read(addr1[p])); } else per[p]=String(EEPROM.read(addr1[p]));}
               for(int p=25;p<=29;p++){if ( EEPROM.read(addr1[p]) < 10 ) { per[p] = "0" + String(EEPROM.read(addr1[p])); } else per[p]=String(EEPROM.read(addr1[p]));}
               for(int p=31;p<=34;p++){if ( EEPROM.read(addr1[p]) < 10 ) { per[p] = "0" + String(EEPROM.read(addr1[p])); } else per[p]=String(EEPROM.read(addr1[p]));}
               String str1 = "<p id='text'>Monday On: " + per[6] + ":" + per[7] + "  Off: " + per[8] + ":" + per[9] + "</p>";
               String str2 = "<p id='text'>Tuesday On: " + per[11] + ":" + per[12] + "  Off: " + per[13] + ":" + per[14] + "</p>";
               String str3 = "<p id='text'>Wednesday On: " + per[16] + ":" + per[17] + "  Off: " + per[18] + ":" + per[19] + "</p>";
               String str4 = "<p id='text'>Thursday On: " + per[21] + ":" + per[22] + "  Off: " + per[23] + ":" + per[24] + "</p>";
               String str5 = "<p id='text'>Friday On: " + per[26] + ":" + per[27] + "  Off: " + per[28] + ":" + per[29] + "</p>";
               String str6 = "<p id='text'>Saturday On: " + per[31] + ":" + per[32] + "  Off: " + per[33] + ":" + per[34] + "</p>";
               String str7 = "<p id='text'>Sunday On: " + per[1] + ":" + per[2] + "  Off: " + per[3] + ":" + per[4] + "</p>";
                      
          
           if (a)client.println(str1); 
           if (b)client.println(str2);
           if (c)client.println(str3); 
           if (d)client.println(str4); 
           if (r)client.println(str5); 
           if (y)client.println(str6);     
           if (f)client.println(str7); 
             }else if (StrContains(HTTP_req, "emp")) {
             client.println("HTTP/1.1 200 OK");
             client.println("Content-Type: text/html");
             client.println("Connnection: close");
             client.println();
             String t1 = t.substring(8,11);
             int temp1 = t1.toInt();
             EEPROM.write(addr1[35], temp1);
           }else if (StrContains(HTTP_req, "utton2")) {
             EEPROM.write(39, 0);
             client.println("HTTP/1.1 200 OK");
             client.println("Content-Type: text/html");
             client.println("Connnection: close");
             client.println();
           }else if (StrContains(HTTP_req, "utton1")) {
             readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth);
             EEPROM.write(42, hour);
             EEPROM.write(39, 1);
             client.println("HTTP/1.1 200 OK");
             client.println("Content-Type: text/html");
             client.println("Connnection: close");
             client.println();
           }if (webFile){
             while(true) {
               client.write(webFile.read());
               int n = webFile.read((char*)buff, BUFFER_SIZE);
               if (!n) break;
               client.write(buff, n);
             }
             webFile.close();
           }
           req_index = 0; 
           t = "";
           StrClear(HTTP_req, REQ_BUF_SZ);
           break;
      }
           if (c == '\n') {
             currentIsBlank = true;
           }else if (c != '\r') {
           currentIsBlank = false;
         }
       }
    }
    delay(1);
    client.stop();
    readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth);
    if (1  == dayOfWeek  &&  EEPROM.read(addr1[1]) == hour && EEPROM.read(addr1[2]) == minute && c1 == 0) {
      flag = 1;
      EEPROM.write(39, 1);
      c1++;
      c2 = 0;
    }
    if (1  == dayOfWeek  &&  EEPROM.read(addr1[3]) == hour && EEPROM.read(addr1[4]) == minute && c2 == 0) {
      EEPROM.write(39, 0);
      c2++;
      flag = 0;
      c1 = 0;
    }
    if (2  == dayOfWeek  &&  EEPROM.read(addr1[6]) == hour && EEPROM.read(addr1[7]) == minute && c3 == 0) {
      EEPROM.write(39, 1);
      c3++;
      flag = 1;
      c4 = 0;
    }
    if (2  == dayOfWeek  &&  EEPROM.read(addr1[8]) == hour && EEPROM.read(addr1[9]) == minute && c4 == 0) {
      flag = 0;
      EEPROM.write(39, 0);
      c4++;
      c3 = 0;
    }
    if (3  == dayOfWeek  &&  EEPROM.read(addr1[11]) == hour && EEPROM.read(addr1[12]) == minute && c5 == 0) {
      EEPROM.write(39, 1);
      c5++;
      flag = 1;
      c6 = 0;
    }
    if (3  == dayOfWeek  &&  EEPROM.read(addr1[13]) == hour && EEPROM.read(addr1[14]) == minute && c6 == 0) {
      flag = 0;
      EEPROM.write(39, 0);
      c6++;
      c5 = 0;
    }
    if (4  == dayOfWeek  &&  EEPROM.read(addr1[16]) == hour && EEPROM.read(addr1[17]) == minute && c7 == 0) {
      EEPROM.write(39, 1);
      c7++;
      flag = 1;
      c8 = 0;
    }
    if (4  == dayOfWeek  &&  EEPROM.read(addr1[18]) == hour && EEPROM.read(addr1[19]) == minute && c8 == 0) {
      EEPROM.write(39, 0);
      c8++;
      flag = 0;
      c7 = 0;
    }
    if (5  == dayOfWeek  &&  EEPROM.read(addr1[21]) == hour && EEPROM.read(addr1[22]) == minute && c9 == 0) {
      flag = 1;
      EEPROM.write(39, 1);
      c9++;
      c10 = 0;
    }
    if (5  == dayOfWeek  &&  EEPROM.read(addr1[23]) == hour && EEPROM.read(addr1[24]) == minute && c10 == 0) {
     flag = 0;
     EEPROM.write(39, 0);
     c10++;
     c9 = 0;
    }
    if (6  == dayOfWeek  &&  EEPROM.read(addr1[26]) == hour && EEPROM.read(addr1[27]) == minute && c11 == 0) {
      flag = 1;
      EEPROM.write(39, 1);
      c11++;
      c12 = 0;
    }
    if (6  == dayOfWeek  &&  EEPROM.read(addr1[28]) == hour && EEPROM.read(addr1[29]) == minute && c12 == 0) {
      flag = 0;
      EEPROM.write(39, 0);
      c12++;
      c11 = 0;
    }
    if (7  == dayOfWeek  &&  EEPROM.read(addr1[31]) == hour && EEPROM.read(addr1[32]) == minute && c13 == 0) {
      flag = 1;
      EEPROM.write(39, 1);
      c13++;
      c14 = 0;
    }
    if (7  == dayOfWeek  &&  EEPROM.read(addr1[33]) == hour && EEPROM.read(addr1[34]) == minute && c14== 0) {
      flag = 0;
      EEPROM.write(39, 0);
      c14++;
      c13 = 0;
    }
    if(EEPROM.read(39) == 1){
      digitalWrite(9, HIGH);
    }else{
      digitalWrite(9, LOW);
    }
    if (EEPROM.read(38) > EEPROM.read(addr1[35])) {
       digitalWrite(6, LOW);
       EEPROM.write(50, 0);
     }
     if (EEPROM.read(38)  < (EEPROM.read(addr1[35]) - 5)) {
       digitalWrite(6, HIGH);
       EEPROM.write(50, 1);
     }
     if (EEPROM.read(42)+2 == hour && flag == 0) {
       digitalWrite(9, LOW);
       EEPROM.write(39, 0);
       EEPROM.write(42, 25);
     }
     Serial.println(minute);
     Serial.println(EEPROM.read(addr1[16]));
     Serial.println(EEPROM.read(addr1[17]));
}else{
    readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth);
    if (1  == dayOfWeek  &&  EEPROM.read(addr1[1]) == hour && EEPROM.read(addr1[2]) == minute && c1 == 0) {
      EEPROM.write(39, 1);
      c1++;
      c2 = 0;
    }
    if (1  == dayOfWeek  &&  EEPROM.read(addr1[3]) == hour && EEPROM.read(addr1[4]) == minute && c2 == 0) {
      EEPROM.write(39, 0);
      c2++;
      c1 = 0;
    }
    if (2  == dayOfWeek  &&  EEPROM.read(addr1[6]) == hour && EEPROM.read(addr1[7]) == minute && c3 == 0) {
      EEPROM.write(39, 1);
      c3++;
      c4 = 0;
    }
    if (2  == dayOfWeek  &&  EEPROM.read(addr1[8]) == hour && EEPROM.read(addr1[9]) == minute && c4 == 0) {
      EEPROM.write(39, 0);
      c4++;
      c3 = 0;
    }
    if (3  == dayOfWeek  &&  EEPROM.read(addr1[11]) == hour && EEPROM.read(addr1[12]) == minute && c5 == 0) {
      EEPROM.write(39, 1);
      c5++;
      c6 = 0;
    }
    if (3  == dayOfWeek  &&  EEPROM.read(addr1[13]) == hour && EEPROM.read(addr1[14]) == minute && c6 == 0) {
      EEPROM.write(39, 0);
      c6++;
      c5 = 0;
    }
    if (4  == dayOfWeek  &&  EEPROM.read(addr1[16]) == hour && EEPROM.read(addr1[17]) == minute && c7 == 0) {
      EEPROM.write(39, 1);
      c7++;
      c8 = 0;
    }
    if (4  == dayOfWeek  &&  EEPROM.read(addr1[18]) == hour && EEPROM.read(addr1[19]) == minute && c8 == 0) {
      EEPROM.write(39, 0);
      c8++;
      c7 = 0;
    }
    if (5  == dayOfWeek  &&  EEPROM.read(addr1[21]) == hour && EEPROM.read(addr1[22]) == minute && c9 == 0) {
      EEPROM.write(39, 1);
      c9++;
      c10 = 0;
    }
    if (5  == dayOfWeek  &&  EEPROM.read(addr1[23]) == hour && EEPROM.read(addr1[24]) == minute && c10 == 0) {
      EEPROM.write(39, 0);
      c10++;
      c9 = 0;
    }
    if (6  == dayOfWeek  &&  EEPROM.read(addr1[26]) == hour && EEPROM.read(addr1[27]) == minute && c11 == 0) {
      EEPROM.write(39, 1);
      c11++;
      c12 = 0;
    }
    if (6  == dayOfWeek  &&  EEPROM.read(addr1[28]) == hour && EEPROM.read(addr1[29]) == minute && c12 == 0) {
      EEPROM.write(39, 0);
      c12++;
      c11 = 0;
    }
    if (7  == dayOfWeek  &&  EEPROM.read(addr1[31]) == hour && EEPROM.read(addr1[32]) == minute && c13 == 0) {
      EEPROM.write(39, 1);
      c13++;
      c14 = 0;
    }
    if (7  == dayOfWeek  &&  EEPROM.read(addr1[33]) == hour && EEPROM.read(addr1[34]) == minute && c14== 0) {
      EEPROM.write(39, 0);
      c14++;
      c13 = 0;
    }
    if (EEPROM.read(38) > EEPROM.read(addr1[35])) {
       digitalWrite(6, LOW);
       EEPROM.write(41, 0);
     }
     if (EEPROM.read(38)  <= (EEPROM.read(addr1[35]) - 5)) {
       digitalWrite(6, HIGH);
       EEPROM.write(41, 1);
     }
     if(EEPROM.read(39) == 1){
      digitalWrite(9, HIGH);
    }else{
      digitalWrite(9, LOW);
    }
}


}

void StrClear(char *str, char length){
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}
char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;
    len = strlen(str);
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }
    return 0;
}
