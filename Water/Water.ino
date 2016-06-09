

/* скетч для Nano 3.0 Atmega328  или ATmega32U4  
c использованием millis 
c внешними прерываниями ,ButtonWC,SW3
с программной защитой от дребезга контакта .
время мин и сек условны 
вот примерный алгоритм для скетча .      

 // все пункты должны быть в одном скетче


А)  управление двумя реле ,одной кнопкой с led индикацией .    // переключение источников водоснабжения. задержка в 2сек используется для сброса давления в водопроводе при переключении ел.кранов
    LedЕСО - (аналог) LED на кнопке ЕСО по умолчанию не горит.
    ButtonECO - если кнопка ECO  нажата то включает на 5мин + реле R1 +R2 (с задержкой в 2 сек) и Led на кнопке ЕСО // кнопка ButtonECO ,без фиксации NO
    на последней минуте Led на кнопке ЕСО   начинает плавно мигать показывая что время 5 мин заканчивается.
    досрочное принудительное  выключение  R2 +R1 происходит удержанием   кнопки ECO   2 сек. это выключает  реле R2 +R1(с задержкой в 2 сек)   и Led на кнопке ЕСО  
                                                             
 Б)  управление сервомотором на 60* одной кнопкой с led индикацией           // слив воды в бочке унитаза 
    LedWC  - это (аналог) LED на кнопке WC по умолчанию  горит.
    ButtonWC - это кнопка WC   нажата серва поворачивается  на 50* ,задержка 2сек и возврат на 10*временное блокирование кнопки на 10 сек
    Led на кнопке WC плавно мигает 2 мин .
С)  pin SW1 HIGH вкл R3 на 30сек                                                                         //  сигнал от датчика влажности вкл вентиляцию
Д)  pin SW2 HIGH вкл R3 и R4 на 10 сек                                                                   // сигнал от датчика движения вкл вентиляцию и освещение
Е)  pin SW3 HIGH вкл R4 на 90сек + вкл плавно(1сек) Led на 60сек если SW3 LOW выкл плавно(1сек) Led      //сигнал от датчика движения вкл   освещение  и подсветку (аналог) LED 
 
*/

#define Rele_R1  A1                             // Реле R1
#define Rele_R2  A2                             // Реле R2
#define Rele_R3  A3                             // Реле R3
#define Rele_R4  A4                             // Реле R4
#define Rele_R5  A5                             // Реле R4

#define ledECO 8                                // Светодиод на кнопке ECO
#define ledWC  10                               // Светодиод на кнопке WC
#define ButtonECO 7                             // Кнопка ECO
#define ButtonWC  9                             // Кнопка WC






void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
