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
#define RS232BAUDRATE 9600
int weight = 0;



void setup()
{
 Serial.begin (9600); lcd.begin(20, 4);  Keyboard.begin(); 
 pinMode(10,OUTPUT); digitalWrite(10, 1); //Включаем Подсветка LCD
 Serial1.begin (9600);
 lcd.clear() ; 
}

 

void loop() {
  
String Parm[20] = ""; 
byte cData[64] = {0};
int nBytesAvail = 0;

key_read ();   //чтение кнопок с защитой от дребезга;
          if (lcd_key == 1){Serial1.write(0x45);}
          if (lcd_key == 2){Serial1.write(0x4A);} 
          if (lcd_key == 0){lcd.clear();} 


if (Serial1.available()>2) {
  delay(100);
  nBytesAvail = Serial1.available();
  for (int i=0; i<20; i++){
  delay(500);
  Serial1.readBytesUntil(0x2c, &cData[0], nBytesAvail);
  lcd.clear(); lcd.setCursor (0,0);
  Parm[i] = &cData[0];  
  lcd.print("P");  lcd.print(i); lcd.print(" = "); lcd.print(Parm[i]);  
  //Serial.println();
  Serial.write (&cData[0], nBytesAvail);  
  Serial.println();
  if (Parm[i] == 13) {i=20;}
  Parm[i] = "   ";
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

  
