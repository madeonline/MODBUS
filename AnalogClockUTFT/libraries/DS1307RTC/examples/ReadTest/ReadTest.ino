#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

#include <LiquidCrystal.h>
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);


void setup() 
  {
  Serial.begin(9600);
  lcd.begin(16, 2);
  }

void loop() 
  {
  tmElements_t tm;

  if (RTC.read(tm)) 
    {
    print2digits(tm.Hour,0,0);
    lcd.print(':');
    print2digits(tm.Minute,3,0);
    lcd.print(':');
    print2digits(tm.Second,6,0);
    Serial.print(", Date (D/M/Y) = ");
    print2digits(tm.Day,0,1);
    lcd.print('/');
    print2digits(tm.Month,3,1);
    lcd.print('/');
    lcd.print(tmYearToCalendar(tm.Year));
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
    } 
  else 
    {
    if (RTC.chipPresent()) 
      {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("DS1307 is stopped");   
      }
    else 
      {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("DS1307 read error");   
      }
   delay(9000);
   }
  delay(1000);
}

void print2digits(int number,int col, int str) {
  lcd.setCursor(col, str);
  if (number >= 0 && number < 10) 
    {lcd.print("0");}   
  lcd.print(number);

}
