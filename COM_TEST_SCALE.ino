#include <EEPROM.h>
#include <Keyboard.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  //LCD standart
int lcd_key     = 0; //
int lcd_key_prev= 5; //исходное состояние джойстика
int adc_key_in  = 0; //сигнал от джойстика
#define btnRIGHT     0
#define btnUP        1 
#define btnDOWN      2 
#define btnLEFT      3
#define btnSELECT    4
#define btnNONE      5 

#define RS232PORT Serial1
#define RS232BAUDRATE 4800

int weight = 1234; //B1100001001; //777


void setup()
{
 Serial.begin (9600); lcd.begin(20, 4);  Keyboard.begin(); 
 pinMode(10,OUTPUT); digitalWrite(10, 1); //Включаем Подсветка LCD
 Serial1.begin (4800);
 lcd.clear() ; 
}

 

void loop() {
  
byte cData[10];
int nBytesAvail =0;
int nBytes = 0;

key_read ();   //чтение кнопок с защитой от дребезга;
          if (lcd_key == 1){Serial1.write(0x45);}
          if (lcd_key == 2){Serial1.write(0x4A);} 
          if (lcd_key == 0){lcd.clear();} 
          if (lcd_key == 3){
           // for (int a = 0; a <2; a++){
            Serial1.write(weight);
            Serial1.write(weight >>8);
            //} 
          }

if (Serial1.available()>0) {
                lcd.clear() ;// lcd.setCursor(0,1); lcd.print (nBytesAvail); 
                for (int n=0; n < 14; n=n+3){
                nBytes = Serial1.read();  
                lcd.setCursor(n,2); if (nBytes>0)lcd.print(nBytes); 
                delay (50);
                }
 }
}
int key_read ()    //чтение кнопок с защитой от дребезга      
          {
          lcd_key = read_LCD_buttons();  // read the buttons
          delay (10);   
          if (lcd_key != lcd_key_prev) lcd_key_prev = lcd_key; //блокируем повторы
          else lcd_key = 5;
          return lcd_key;
          } 

int read_LCD_buttons() //считываем нажатие джойстика Моя плата
          {  
     //    Serial.println (adc_key_in = analogRead(0)); 
          adc_key_in = analogRead(0);
          if (adc_key_in > 950)  return btnNONE; 
          if (adc_key_in < 50)   return btnUP; 
          if (adc_key_in < 600)  return btnRIGHT; 
          if (adc_key_in < 720)  return btnDOWN;
          if (adc_key_in < 800)  return btnSELECT;        
          if (adc_key_in < 900)  return btnLEFT;   
          return btnNONE;  
          }      
/*


//int n=0;

if (Serial1.available()>0) {
//do 
lcd.clear() ;
for (int n=0; n<15; n=n+3) {
byte a = Serial1.read();   Serial.println(a); 
lcd.setCursor(n,1); lcd.print (a, DEC);
//delay(100);
   }
  }
//while (Serial1.available());

}

*/ 
  


