/////////////////////////////////////////////////////////////
//   LogisticSonar 3d v.3 base + manual + весы Масса-к     //
//   Leonardo + моя плата v.2.1 + BAT sensors              //
//   DIY-pragmatiс konakovskiy@gmail.com                   //
//   02.11.2017                                            //
//   3D Сканер размеров упаковки.                          //
/////////////////////////////////////////////////////////////
//Базовая версия с прямой калибровкой размеров,подключение весов через COM-порт
//Передача данных в COM порт по запросу с РС
//Выход на исходную позицию для сканирования
//Размещаем коробку с известными размерами и настраиваем их отображение наиболее точно.
//Максимальные размеры  x  x 0 cm
//макс. вес:  40 кг (выводятся 3 разряда для целых кг, 3 разряда для граммов)
//вывод:      размеры в см (3 разряда), 1 знак после запятой
//            вес в кг, 3 знака после запятой 
// десятичный разделитель - запятая 
// порядок и разделитель: L TAB W TAB H TAB Wg ENTER
uint8_t tochki[8] = {B0, B00000, B0, B0, B0, B0, B10101}; //...
uint8_t bukva_P[8] = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11}; //П
uint8_t bukva_Ya[8] = {B01111, B10001, B10001, B01111, B00101, B01001, B10001};//Я
uint8_t bukva_L[8] = {B00111, B01001, B01001, B01001, B01001, B01001, B10001};//Л
uint8_t bukva_Lm[8] = {0, 0, B01111, B00101, B00101, B10101, B01001};//л
uint8_t bukva_Mz[8] = {0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E};//Ь1нгг
uint8_t bukva_I[8] = {B10001, B10001, B10001, B10011, B10101, B11001, B10001};//И
uint8_t bukva_D[8] = {B01110, B01010, B01010, B01010, B01010, B11111, 0x11};//Д
uint8_t bukva_G[8] = {B11111, B10001, B10000, B10000, B10000, B10000, B10000};//Г
uint8_t bukva_g[8] = {B00000, B00000, B11100, B10000, B10000, B10000, B10000};//г
uint8_t bukva_IY[8] = {B01110, B00000, B10001, B10011, B10101, B11001, B10001};//Й
uint8_t bukva_Z[8] = {B01110, B10001, B00001, B00010, B00001, B10001, B01110};//З
uint8_t bukva_ZH[8] = {B10101, B10101, B10101, B11111, B10101, B10101, B10101};//Ж
uint8_t bukva_Y[8] = {B10001, B10001, B10001, B01010, B00100, B01000, B10000};//У
uint8_t bukva_B[8] = {B11110, B10000, B10000, B11110, B10001, B10001, B11110};//Б
uint8_t bukva_CH[8] = {B10001, B10001, B10001, B01111, B00001, B00001, B00001};//Ч
uint8_t bukva_IYI[8] = {B10001, B10001, B10001, B11001, B10101, B10101, B11001};//Ю
uint8_t bukva_TS[8] = {B10010, B10010, B10010, B10010, B10010, B10010, B11111, B00001};//Щ
uint8_t bukva_RR[8] = {B10001, B10001, B11101, B10011, B10011, B10011, B11101};//Ы
uint8_t bukva_SH[8] = {B10001, B10101, B10101, B10101, B10101, B10101, B11111};//Ш
//#include "HX711.h"
#include <EEPROM.h>
#include <Keyboard.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  //LCD standart
//HX711 scale;

int tmp_1 = 0;
boolean lang =  1; //0- русский язык, true - английский
int print_btn = 13; // Кнопка передачи данных
boolean scale_flag = false; //флаг стабилизации веса
boolean print_stat = HIGH ; //текущее состояние кнопки передачи данных/при нажатии переходит в 0
boolean print_stat_prew = LOW; // предыдущее состояние кнопки
boolean menu = LOW; //признак активированного меню
long Size = 0; //текущее измерение расстояни до цели
long SumSize = 0; //сумма измерений сторон
long SumBase_M = 0; //сумма ручного ввода базовых расстояний
//описание меню
int menupos = 0;//код пункта меню
int menupos_end = 4; //крайний пункт меню - EXIT, 0 - главное меню
boolean outputtype = 0;  //тип вывода данных в буфер ручн/автом
//
long SumSizeMin = 60; //порог чувствительности
long SizeGate = 9 ;  //чувствительность к дребезгу измерения по сумме осей 
//Канал ШИРИНА
int TR_PIN_W = 21; //A3;      //trigger pin on the ultrasonic sensor
int EC_PIN_W = 20; //A2;      //echo pin on the ultrasonic sensor
long Base_W_M = 0; //базовое расстояние, введенное руками
long Base_W = 0;
long Size_W = 0;
//Канал ДЛИНА
int TR_PIN_L = A1; //A1;       
int EC_PIN_L = 2; //D2;         
long Base_L_M = 0;
long Base_L = 0;
long Size_L = 0;
//Канал ВЫСОТА
int TR_PIN_H = 23; //A5     
int EC_PIN_H = 22; //A4         
long Base_H_M = 0;
long Base_H = 0;
long Size_H = 0;
// Определяем кнопки джойстика
int lcd_key     = 0; //
int lcd_key_prev= 5; //исходное состояние джойстика
int adc_key_in  = 0; //сигнал от джойстика
#define btnRIGHT     0
#define btnUP        1 
#define btnDOWN      2 
#define btnLEFT      3
#define btnSELECT    4
#define btnNONE      5   
//#define btnEncodeOK  6 
//Взвешивание
float weight = 0; //полученный вес
float weight_key = 26; //стартовый коэффициент для калибровки весов
int weight_gate = 20; //минимально взвешиваемый вес
int prec = 0; //точность вывода веса 0 - без знаков после запятой
//int DOUT = 11;// - pin #D11
//int SCK = 12; //- pin #D12


void setup()
{
  Serial.begin (9600); 
  Serial1.begin (4800, SERIAL_8E1); 
  lcd.begin(20, 4);  Keyboard.begin(); 
  pinMode(TR_PIN_H,OUTPUT); pinMode(EC_PIN_H,INPUT_PULLUP);    // set TP output for trigger, EP input for echo
  pinMode(TR_PIN_W,OUTPUT); pinMode(EC_PIN_W,INPUT_PULLUP);    
  pinMode(TR_PIN_L,OUTPUT); pinMode(EC_PIN_L,INPUT_PULLUP); 
  //EEPROM.get(6, weight_key); //Serial.println(weight_key);
  //if ((weight_key <= 0)||(weight_key >1000000)) {weight_key = 26;}   
  //EEPROM.put(6, 26); 
//  scale.begin(11, 12); scale.set_scale(weight_key);   
  pinMode (print_btn, INPUT_PULLUP);
  pinMode(10,OUTPUT); digitalWrite(10, 1); //Включаем Подсветка LCD
  display_welcome();    // приглашение
  Base_H = EEPROM.read(10) | (EEPROM.read(11) << 8);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
  Base_W = EEPROM.read(12) | (EEPROM.read(13) << 8);
  Base_L = EEPROM.read(14) | (EEPROM.read(15) << 8);
  SumBase_M = (Base_H + Base_W + Base_L);
     if (SumBase_M == 0) { // если значения 0 - автокалибруем 
//      calibration(); //основная калибровка
     }
     else { // берем базовые значения из памяти
        lcd.clear();
         if (lang) {lcd.setCursor(0, 0);  lcd.print("    From memory");}
         if (!lang) { lcd.clear(); lcd.createChar(1, bukva_I); lcd.createChar(2, bukva_Z); lcd.createChar(3,bukva_Ya); lcd.createChar(4,bukva_P);
                    lcd.setCursor(0, 0);lcd.print("    ");lcd.write(1);lcd.write(2);lcd.print(" ");lcd.write(4);lcd.print("AM");lcd.write(3);lcd.print("T");lcd.write(1);}
        lcd.setCursor(3, 2);  lcd.print("H    W    L");
        lcd.setCursor(3, 3);  lcd.print(Base_H);
        lcd.setCursor(8, 3);  lcd.print(Base_W);
        lcd.setCursor(13, 3); lcd.print(Base_L);
     }
//        scale.tare();   //начальная тарировка весов
         delay (1000);
        //taring();  
        
 Serial1.read(); 
}
void loop()
{
    key_read ();   //чтение кнопок с защитой от дребезга;
     outputtype = EEPROM.read(4);
     weighting();
     if ((lcd_key == 5)&&(menu==LOW)) 
        {  
        sizing(); //запуск измерениe
           //weighting(); Serial.print("fast weight"); Serial.println(weight);      // вылавливал дрейф тары
          print_stat = digitalRead (print_btn);
        if (((Size_H + Size_W + Size_L) < SumSizeMin)&&(!print_stat_prew && print_stat)) {
           if (tmp_1 == 0) { //впомогательный счетчик, чтобы избавится от мигания экрана в цикле loop
            scale_flag = false;  //сбрасываем весы
            display_ready();// размеры меньше порога и кнопка в исходном состоянии, 
            tmp_1 ++;
            if (tmp_1 > 5) tmp_1 = 0;
           }
        }
        else if (((Size_H + Size_W + Size_L) >= SumSizeMin)&&(!print_stat_prew && print_stat)) { //размеры выше порога, кнопка в исходном состоянии, отображаем размеры
              if ((SumSize == 0) || (SumSize <= (Size_H + Size_W + Size_L - SizeGate)) || (SumSize >= (Size_H + Size_W + Size_L + SizeGate))){ //убираем "дребезг" измерения
                  delay (600) ; 
                  if (!scale_flag) {
                    display_w_process(); //экран процесса взвешивания
                     weighting_1();  //запуск взвешивание со стабилизацией
                }
                if (weight !=0){
                 sizing(); //запуск измерениe    
                  display_ok(); //отображение измерений
                }
                else {display_ready();}

                tmp_1 = 0;
               }
                SumSize = (Size_H + Size_W + Size_L); //запоминаем сумму размеров
            } 
        
        else if (((Size_H + Size_W + Size_L) >= SumSizeMin)&&(!print_stat_prew && !print_stat)) {
                 printing (); // передача данных
                    scale_flag = false; //сбрасываем весы
                 print_stat_prew = HIGH;   //запрет на передачу данных, пока не сброшены размеры
                 }
        else if (((Size_H + Size_W + Size_L) < SumSizeMin)&&(print_stat_prew && print_stat)) print_stat_prew = LOW; //перезапуск в режим измерений       
        }
//обработка нажатия кнопок в режиме меню 
     else if ((lcd_key == 4)&&(menu == LOW )) menuposition(lcd_key) ;//главное меню          
     else if ((lcd_key == 2)&&(menu == HIGH)) menuposition (lcd_key); //меню вниз
     else if ((lcd_key == 1)&&(menu == HIGH)) menuposition (lcd_key); //меню вверх
     else if ((lcd_key == 0)&&(menu == HIGH)) {
          if (menupos == 0) {
            calibration();  //Калибровка
            for (int i = 10 ; i < EEPROM.length() ; i++) EEPROM.write(i, 0);// очистка памяти начиная с 10-й ячейки, не трогаем 8 и 9 там тарировка
          }
          if (menupos == 1) manual_calibration();  //Ручная Калибровка 
          if (menupos == 2) weighting_request();//taring();  //Тарировка-------------------
          if (menupos == 3) set_outputtype(); 
          if (menupos == menupos_end) {
            menu = LOW; 
            display_ready();   // Exit
          }
      }
  }   //конец цикла loop

//вызываемые функции
//запрос веса
float weighting_request(){
   Serial1.read(); 
     Serial1.write(0x4A); 
}
// получение веса
float weighting(){
  byte cData[5] = {0};
  int nBytesAvail = 0;
       
         if (Serial1.available()>2) {
           delay(15);
            lcd.clear(); lcd.setCursor (0,0); lcd.print(" 1  2  3  4  5"); 
            nBytesAvail = Serial1.available();
              for (int i=0; i<5; i++) {
                Serial1.readBytes(&cData[0], nBytesAvail); 
                lcd.setCursor(i*3,1); 
                lcd.print(cData[i],HEX); 
                delay(100);}
         }
     // delay (5000);
//     if ((weight/weight_gate)<1) {Serial.print(weight); Serial.println(" gate "); weight = 0; } //вылавливаю дрейф тары
//     weight = ((round(weight/10))*10);
  //return weight;
}
//сброс веса тары ---------------------- сделать
void taring() {
   lcd.clear();  lcd.setCursor(0, 0);  lcd.print("    Scale Reset");  
   Serial1.write(0x0E);
   delay(3000);
   menu = LOW;  lcd.clear();   display_ready();//выход в меню 
}

boolean set_outputtype()
      {
        outputtype = EEPROM.read(4);
         display_outputtype ();
        do { 
              key_read ();  //Serial.println (adc_key_in = analogRead(0)); // чтение кнопок
              // движение фокуса по значку осей
              if (lcd_key == 0) {outputtype = true  ; display_outputtype();}
              if (lcd_key == 3) {outputtype = false ; display_outputtype();}
        }
        while (lcd_key != 4); //Сохранение данных по Центральной кнопке ДЖОЙСТИКА
              if (lcd_key == 4) {
                EEPROM.write(4, outputtype); 
              }
           menu = LOW; display_ready();
        return outputtype;
      }

void display_outputtype()    //дисплей выбора типа вывода
        {
        lcd.clear(); 
            lcd.setCursor(0, 1);
            if (lang){
              lcd.setCursor(0, 0);  lcd.print("    Output type");
              if (outputtype)  {lcd.setCursor(0, 2);  lcd.print(" Manual     >> Auto  ");}
              if (!outputtype) {lcd.setCursor(0, 2);  lcd.print(" Manual <<     Auto  ");}
            }
            if (!lang){
              lcd.createChar(1, bukva_I); lcd.createChar(2, bukva_P); lcd.createChar(3, bukva_D);lcd.createChar(7, bukva_RR);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(6, bukva_IY);
              lcd.setCursor(0, 0);  lcd.print("     T"); lcd.write(1);lcd.write(2); lcd.print(" B"); lcd.write(7);lcd.print("BO");lcd.write(3); lcd.print("A");
              if (outputtype) {lcd.setCursor(0, 2);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.write("HO"); lcd.write(6); lcd.print("   >>"); lcd.write(" ABTO");}
              if (!outputtype){lcd.setCursor(0, 2);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.write("HO"); lcd.write(6); lcd.print(" <<  "); lcd.write(" ABTO");}
            }  
         }


void display_w_process() //экран процесса взвешивания
        {
        lcd.clear();  
        if (lang) {
            lcd.setCursor(0, 1);  lcd.print("    Weighting...");
            }         
        if (!lang) { // на русском
            lcd.createChar(1, bukva_SH);  lcd.createChar(2, bukva_Z); lcd.createChar(3, bukva_B); lcd.createChar(4, bukva_I); 
            lcd.setCursor(0, 1);  lcd.print("   B"); lcd.write(2); lcd.print("BE"); lcd.write(1); lcd.write(4); lcd.print("BAH"); lcd.write(4); lcd.print("E...");
            } 
        }       
void display_ready() //экран готовности к измерению
        {
        lcd.clear();  
        if (lang) {
            lcd.setCursor(0, 0);  lcd.print("     Ready for");
            lcd.setCursor(0, 1);  lcd.print("    measurement");
            }         
        if (!lang) { // на русском
            lcd.createChar(1, bukva_G); lcd.createChar(2, bukva_B); 
            lcd.setCursor(0, 1);  lcd.print("   "); lcd.write(1); lcd.print("OTOB K PA"); lcd.write(2); lcd.print("OTE");
            } 
        }        


//запрос веса --------------------------------сделать
float weighting_1 (){
 
  long i=0; //делаем стабилизацию веса
  float weight_tmp = 0; float weight_tmp_next = 0; 
//  weight_tmp = scale.get_units(4), 0;  //среднее от 5 измерений
     if ((weight_tmp / weight_gate)<1) {
     weight = 0; Serial.print(weight_tmp); Serial.println(" gate "); 
     return weight;
  }
    do {
//         weight_tmp = scale.get_units(4), 0;  //среднее от 5 измерений
          //delay (15);
//           weight_tmp_next = scale.get_units(5), 0; 
            Serial.print("tmp"); Serial.print(weight_tmp); Serial.print(" next"); Serial.print (weight_tmp_next); Serial.print(" k= "); Serial.println(weight_tmp_next/weight_tmp);
             if (((weight_tmp_next / weight_tmp)<1.05)&&((weight_tmp_next/weight_tmp))>0.95) {i++;}
             else {i--;}
             //Serial.print("i = "); Serial.println(i);
     }
    
  while ((i<3)&&(i>-3)); //добиваемся стабильного результата 3 взвешивания в диапазоне +-5% и защищаемся от сбоев (отриц. счетчик)
    if (i==3) {weight = ((round(weight_tmp_next/10))*10);}
    else {weight = 0; lcd.clear(); lcd.setCursor(0,1); lcd.print ("      Error... "); delay(1500); }
 // Serial.println (weight);
  scale_flag = true;
  
   return weight;
}


//Функция MANUAL калибровки, измеряем 3 раза, находим среднее арифметическое, оставляем 
// на экране, делаем редактирование.     

int manual_calibration() 
        {
        //извлекаем данные из памяти
        Base_H = EEPROM.read(10) | (EEPROM.read(11) << 8);
        Base_W = EEPROM.read(12) | (EEPROM.read(13) << 8);
        Base_L = EEPROM.read(14) | (EEPROM.read(15) << 8);
        SumBase_M = (Base_H + Base_W + Base_L);
        if (SumBase_M == 0) { // если значения 0 - автокалибруем 
//            calibration_1();
            delay(200);
            calibration();
            manualcorr ();
            }   
        else  manualcorr ();  
            
        menu = LOW;  //выход в меню 
        return Base_H, Base_W, Base_L; //Возвращаем базовые размеры по осям
        }

int manualcorr ()  //ручная коррекция базовых размеров
      {
      int focuspos = 0;
      sizing(); //запускаю первичный обмер
      display_manualcorr (0);
           do
              {
              sizing(); //запускаю обмер
              //display_manualcorr (0);
              key_read ();  Serial.println (adc_key_in = analogRead(0)); // чтение кнопок
              // движение фокуса по значку осей
              if (lcd_key == 2) {focuspos = focuspos + 1 ; display_manualcorr (focuspos);}
              if (lcd_key == 1) {focuspos = focuspos - 1 ; display_manualcorr (focuspos);}
              if (focuspos < 0) {focuspos = 3; display_manualcorr (focuspos);}
              if (focuspos > 3) {focuspos = 0; display_manualcorr (focuspos);}
              //конец блока обработки фокуса
              //корекция значений
              if ((lcd_key == 0)&&(focuspos == 0)) {++Base_L ; display_manualcorr (focuspos);}
              if ((lcd_key == 3)&&(focuspos == 0)) {--Base_L ; display_manualcorr (focuspos);}
              if ((lcd_key == 0)&&(focuspos == 1)) {++Base_W ; display_manualcorr (focuspos);}
              if ((lcd_key == 3)&&(focuspos == 1)) {--Base_W ; display_manualcorr (focuspos);}
              if ((lcd_key == 0)&&(focuspos == 2)) {++Base_H ; display_manualcorr (focuspos);}
              if ((lcd_key == 3)&&(focuspos == 2)) {--Base_H ; display_manualcorr (focuspos);}
              if ((lcd_key == 0)&&(focuspos == 3)); // weight_manualcorr ();
              }
              while (lcd_key != 4); //Сохранение данных по Центральной кнопке ДЖОЙСТИКА
              if (lcd_key == 4) {
                EEPROM.write(10, Base_H); EEPROM.write(11, Base_H >> 8);
                EEPROM.write(12, Base_W); EEPROM.write(13, Base_W >> 8);
                EEPROM.write(14, Base_L); EEPROM.write(15, Base_L >> 8);
               }
                return Base_H, Base_W, Base_L;           
      }

void display_result_setup_1(long delay_time) { //вывод экрана ручной корректировки
                  sizing(); //запускаю обмер
                  lcd.clear(); // вывод на экран всех параметров
                  lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_I); lcd.createChar(3, bukva_D); lcd.createChar(4, bukva_g); lcd.createChar(5,bukva_RR);
                  lcd.createChar(6, bukva_SH);
                  lcd.setCursor(0, 0); if (lang) lcd.print("  Length: ");
                  if (!lang) {lcd.print("  "); lcd.write(3);lcd.write(1);lcd.write(2);lcd.print("HA  : "); }
                  delay (delay_time); lcd.print(Size_L);           
                  lcd.setCursor(0, 1); if (lang) lcd.print("  Width : "); 
                  if (!lang) {lcd.print("  ");lcd.write(6); lcd.write(2); lcd.print("P"); lcd.write(2);lcd.print("HA : ");}
                  delay (delay_time);lcd.print(Size_W);
                  lcd.setCursor(0, 2); if (lang) lcd.print("  High  : ");
                  if (!lang) {lcd.print("  B");lcd.write(5); lcd.print("COTA : ");}
                  delay (delay_time);  lcd.print(Size_H);
} 
void display_result_setup(long delay_time) { //вывод экрана для автоматической калибровки
                  sizing(); //запускаю обмер
                  lcd.clear(); // вывод на экран всех параметров
                  lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_I); lcd.createChar(3, bukva_D); lcd.createChar(4, bukva_g); lcd.createChar(5,bukva_RR);
                  lcd.createChar(6, bukva_SH);
                  lcd.setCursor(0, 0); if (lang) lcd.print("  Length: ");
                  if (!lang) {lcd.print("  "); lcd.write(3);lcd.write(1);lcd.write(2);lcd.print("HA  : "); }
                  
                  delay (delay_time); lcd.print(Base_L);           
                  lcd.setCursor(0, 1); if (lang) lcd.print("  Width : "); 
                  if (!lang) {lcd.print("  ");lcd.write(6); lcd.write(2); lcd.print("P"); lcd.write(2);lcd.print("HA : ");}
                  delay (delay_time);lcd.print(Base_W);
                  lcd.setCursor(0, 2); if (lang) lcd.print("  High  : ");
                  if (!lang) {lcd.print("  B");lcd.write(5); lcd.print("COTA : ");}
                  delay (delay_time);  lcd.print(Base_H);
} 
int display_manualcorr (int focuspos) {

               display_result_setup_1(0);    //Вывод на экран ручной корректировки строки с весом
                  if (lang) {lcd.setCursor(0, 3); lcd.print("  Weight calibr");}
                  if (!lang) {lcd.setCursor(0, 3); lcd.print("  BEC    >");} 
                  lcd.setCursor(0, focuspos); lcd.print(">");
                  }
/*
int weight_manualcorr ()  //настройка тензодатчика
      {
      taring();
      int f_pos = 0; //коэффициент для настройки веса
         do {
              key_read ();  Serial.println (adc_key_in = analogRead(0)); // чтение кнопок
              lcd.setCursor(9, 2);  weighting(); lcd.print(weight, prec); 
              display_weight_colibr(f_pos, weight_key);
              // движение фокуса по значку осей
              if (lcd_key == 0) {f_pos = f_pos + 1 ; display_weight_colibr(f_pos, weight_key);}
              if (lcd_key == 3) {f_pos = f_pos - 1 ; display_weight_colibr(f_pos, weight_key);}
              if (f_pos < -2) {f_pos = 2; display_weight_colibr(f_pos, weight_key);}
              if (f_pos > 2) {f_pos = -2; display_weight_colibr(f_pos, weight_key);}
              //конец блока обработки фокуса

              //корекция значений
              if (lcd_key == 2){weight_key = weight_key + pow (10, f_pos); display_weight_colibr(f_pos, weight_key);}
              if (lcd_key == 1){weight_key = weight_key - pow (10, f_pos); display_weight_colibr(f_pos, weight_key);}
              }
          while (lcd_key != 4); //Сохранение данных по Центральной кнопке ДЖОЙСТИКА
              if (lcd_key == 4) {
               EEPROM.put(6, weight_key); 
//               scale.set_scale(weight_key);
               }
               menu = LOW;  //выход в меню
               display_ready();
               return weight_key;           
      }
*/
void display_weight_colibr(int f_pos, float weight_key ){
  lcd.clear();
            if (lang){
              lcd.setCursor(0, 0);  lcd.print("Weight calibration");
                lcd.setCursor(0, 1); lcd.print("K="); lcd.print(weight_key);  lcd.setCursor(9, 1);  lcd.print("  *<"); lcd.print(pow(10, f_pos)); lcd.print(">");
                lcd.setCursor(0, 3); lcd.print("Weight : ");  
                lcd.setCursor(9, 3); 
//                scale.set_scale(weight_key); weighting(); 
                lcd.print(round(weight)); //lcd.print((long(weight)/10)*10); 
              }
            if (!lang){
             lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
             lcd.setCursor(0, 0);  lcd.print("KA"); lcd.write(1); lcd.write(3); lcd.write(2); lcd.print("POBKA BECA");
                lcd.setCursor(0, 1); lcd.print("K="); lcd.print(weight_key);  lcd.setCursor(9, 1);  lcd.print("  *<"); lcd.print(pow(10, f_pos)); lcd.print(">");
                lcd.setCursor(0, 3); lcd.print("BEC : "); 
                lcd.setCursor(9, 3); 
//                scale.set_scale(weight_key); weighting(); 
                lcd.print(round(weight));//lcd.print((long(weight)/10)*10); 
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
          
int menuposition (int lcd_key)  // навигация по меню
          {
          if (lcd_key == 4) menupos = 0 ; //главное меню
          if (lcd_key == 1) --menupos ; //реакция на стрелку вверх
          if (lcd_key == 2) ++menupos ; // на стрелку вниз
              if (menupos < 0) menupos = menupos_end; //ограничение по пунктам меню - сверху
              if (menupos > menupos_end) menupos = 0; // - снизу
          
          if (menupos == 0)  {
            lcd.clear();
            if (lang){
              lcd.setCursor(0, 0);  lcd.print("> Auto   Calibr");
              lcd.setCursor(0, 1);  lcd.print("  Manual Calibr");
              lcd.setCursor(0, 2);  lcd.print("  Tare Correction");
              lcd.setCursor(0, 3);  lcd.print("  Output type");
            }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);
              lcd.setCursor(0, 0);  lcd.print("> ABTO KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.setCursor(0, 1);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print(" KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("  TAP"); lcd.write(3);lcd.print("POBKA");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 3);  lcd.print("  T"); lcd.write(3);lcd.write(8); lcd.print("  B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
           }  
          menu = HIGH;
          }
          if (menupos == 1)  {
            lcd.clear();  
            if (lang){
              lcd.setCursor(0, 0);  lcd.print("  Auto   Calibr");
              lcd.setCursor(0, 1);  lcd.print("> Manual Calibr");
              lcd.setCursor(0, 2);  lcd.print("  Tare Correction");
              lcd.setCursor(0, 3);  lcd.print("  Output type");
            }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);
              lcd.setCursor(0, 0);  lcd.print("  ABTO KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.setCursor(0, 1);  lcd.print("> P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print(" KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("  TAP"); lcd.write(3);lcd.print("POBKA");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 3);  lcd.print("  T"); lcd.write(3);lcd.write(8); lcd.print("  B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
           }
          menu = HIGH;  
          }
           if (menupos == 2)  {
            lcd.clear();
           if (lang){
              lcd.setCursor(0, 0);  lcd.print("  Auto   Calibr");
              lcd.setCursor(0, 1);  lcd.print("  Manual Calibr");
              lcd.setCursor(0, 2);  lcd.print("> Tare Correction");
              lcd.setCursor(0, 3);  lcd.print("  Output type");
            }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);
              lcd.setCursor(0, 0);  lcd.print("  ABTO KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.setCursor(0, 1);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print(" KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("> TAP"); lcd.write(3);lcd.print("POBKA");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 3);  lcd.print("  T"); lcd.write(3);lcd.write(8); lcd.print("  B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
          }
          menu = HIGH;  
          }
          if (menupos == 3)  {
            lcd.clear();
           if (lang){
              lcd.setCursor(0, 0);  lcd.print("  Auto   Calibr");
              lcd.setCursor(0, 1);  lcd.print("  Manual Calibr");
              lcd.setCursor(0, 2);  lcd.print("  Tare Correction");
              lcd.setCursor(0, 3);  lcd.print("> Output type");
              }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);
              lcd.setCursor(0, 0);  lcd.print("  ABTO KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.setCursor(0, 1);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print(" KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("  TAP"); lcd.write(3);lcd.print("POBKA");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 3);  lcd.print("> T"); lcd.write(3);lcd.write(8); lcd.print("  B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
          }
          menu = HIGH;  
          }
                  
          if (menupos == menupos_end)  {              
          lcd.clear();
          if (lang){
              lcd.setCursor(0, 0);  lcd.print("  Manual Calibr");
              lcd.setCursor(0, 1);  lcd.print("  Tare Correction");
              lcd.setCursor(0, 2);  lcd.print("  Output type");
              lcd.setCursor(0, 3);  lcd.print("> Exit");
            }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);
              lcd.setCursor(0, 0);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print(" KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 1);  lcd.print("  TAP"); lcd.write(3);lcd.print("POBKA");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("  T"); lcd.write(3);lcd.write(8); lcd.print("  B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
              lcd.setCursor(0, 3);  lcd.print("> B"); lcd.write(7); lcd.print("XO"); lcd.write(6);
            }  
          }
           return menupos;
}  

void printing ()
          {
       int s1, s2, s3, s4, s5, s6;

Keyboard.write(KEY_RIGHT_ARROW);delay (50); Keyboard.write(KEY_UP_ARROW); delay (50);
              
       long w_prn = (long(weight));
            s6 = ((w_prn/100000)%10);
            s5 = ((w_prn/10000)%10);
            s4 = ((w_prn/1000)%10);
            s3 = ((w_prn/100)%10);
            s2 = ((w_prn/10)%10);
            s1 = ((w_prn)%10);  
        if (s6 != 0) {Keyboard.print(s6);}
         if ((s6 != 0)||(s5 != 0)) {Keyboard.print(s5);}  
         Keyboard.print(s4);
           Keyboard.print(",");
            Keyboard.print(s3);
             Keyboard.print(s2);
             // Keyboard.print(s1);
                delay (100);
         Keyboard.write(KEY_RIGHT_ARROW);
                delay (100);       
      
         Keyboard.print(round((float(Size_L))/1));
             Keyboard.write(KEY_RIGHT_ARROW);
                delay (100);
         Keyboard.print(round((float(Size_W))/1));
             Keyboard.write(KEY_RIGHT_ARROW);
                delay (100);
         Keyboard.print(round((float(Size_H))/1));
              Keyboard.write(KEY_RIGHT_ARROW);
                delay (100);
                
                Keyboard.write(KEY_DOWN_ARROW); delay (50); Keyboard.write(KEY_LEFT_ARROW); delay (50);Keyboard.write(KEY_LEFT_ARROW); delay (50); Keyboard.write(KEY_LEFT_ARROW); delay (50); Keyboard.write(KEY_LEFT_ARROW); delay (50); Keyboard.write(KEY_LEFT_ARROW); delay (50);
             

          display_sent ();
          }

void printing_com ()  //вывод результатов в COM-порт  ---------------------- сделать 
{
  Serial.print("PC,GET,");
  Serial.print("45,"+ Size_H);
  Serial.print("46,"+ Size_W);
  Serial.print("47,"+ Size_L);
  Serial.print("48,"+ Size_L);
  Serial.print(",");
  Serial.write(0x0D);Serial.write(0x0A);
}

void display_sent ()    //экран отправленных данных в РС   
        {
        lcd.clear(); lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_P); 
            lcd.setCursor(0, 1);
            if (lang) lcd.print("   Data sent: Ok!");
            if (!lang){
               lcd.print("    OT"); lcd.write(2); lcd.print("PAB"); lcd.write(1); lcd.print("EHO !");
             }  
         }
//Функция калибровки, измеряем 3 раза, находим среднее арифметическое.     

long calibration()   {
        int x=0;
        Size=0; Base_H = 0; Base_W = 0; Base_L = 0;
        //округление по 5 измерениям
          lcd.clear();
          if (lang)  {
            lcd.setCursor(0, 0); 
            lcd.print("  CALIBRATION!  "); 
            lcd.setCursor(0, 3);  
            lcd.print("Weight = "); 
            weighting(); 
            lcd.print(weight, prec); 
            lcd.print(" g");
          }
          if (!lang) {
            lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_I); lcd.createChar(3, bukva_B); lcd.createChar(4, bukva_g); lcd.createChar(5,bukva_RR);
            lcd.setCursor(0, 0); lcd.print("    KA"); lcd.write(1); lcd.write(2); lcd.write(3); lcd.print("POBKA  ");
            lcd.setCursor(0, 3); lcd.print("BEC TAP"); lcd.write(5); lcd.print(": ");  weighting();lcd.print(weight, prec); lcd.print(" "); lcd.write(4);
          }
          measuring (TR_PIN_H, EC_PIN_H);  delay (30); // Прогрев
          measuring (TR_PIN_W, EC_PIN_W);  delay (30);
          measuring (TR_PIN_L, EC_PIN_L);  delay (30);
         for (int i=0; i < 5; i++) {
          lcd.setCursor(0, 1);
          if (i == 0) lcd.print(">>>");
          if (i == 1) lcd.print(">>>>>>");
          if (i == 2) lcd.print(">>>>>>>>>");
          if (i == 3) lcd.print(">>>>>>>>>>>>");
          if (i == 4) lcd.print(">>>>>>>>>>>>>>>");
          delay (500);
          measuring (TR_PIN_H, EC_PIN_H); Base_H = Base_H + Size; delay (30);
          measuring (TR_PIN_W, EC_PIN_W); Base_W = Base_W + Size; delay (30);
          measuring (TR_PIN_L, EC_PIN_L); Base_L = Base_L + Size; delay (30);
          //Serial.print("H-"); Serial.print(Base_H)  ;Serial.print(" W-"); Serial.print(Base_W)  ;Serial.print(" L-"); Serial.println(Base_L)  ;
        }
        Base_H = Base_H/5;
        Base_W = Base_W/5;
        Base_L = Base_L/5;
                 display_result_setup(300);    //Вывод на экран
//        scale.tare();      //тарировка, сброс веса тары
         if (lang) {lcd.setCursor(0, 3);  lcd.print("  Weight: "); weighting(); lcd.print(weight, prec); lcd.print(" g");}
         if (!lang) {lcd.setCursor(0, 3);  lcd.print("  BEC    : "); weighting(); lcd.print(weight,prec); /*lcd.print(" ");lcd.write(4);*/}
        delay (1500);
        menu = LOW;  //выход в меню
        display_ready();
        //Serial.print("H-"); Serial.print(Base_H)  ;Serial.print(" W-"); Serial.print(Base_W)  ;Serial.print(" L-"); Serial.println(Base_L)  ;
        return Base_H, Base_W, Base_L; //Возвращаем базовые размеры по осям
        }
void display_result(long delay_time_1) {
 lcd.clear(); // вывод на экран всех параметров
                  lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_I); lcd.createChar(3, bukva_D); lcd.createChar(4, bukva_g); lcd.createChar(5,bukva_RR);
                  lcd.createChar(6, bukva_SH);
                  lcd.setCursor(0, 0); if (lang) lcd.print("  Length: ");
                  if (!lang) {lcd.print("  "); lcd.write(3);lcd.write(1);lcd.write(2);lcd.print("HA  : "); }
                  delay (delay_time_1); lcd.print(Size_L);  
                  lcd.setCursor(0, 1); if (lang) lcd.print("  Width : "); 
                  if (!lang) {lcd.print("  ");lcd.write(6); lcd.write(2); lcd.print("P"); lcd.write(2);lcd.print("HA : ");}
                  delay (delay_time_1);lcd.print(Size_W);
                  lcd.setCursor(0, 2); if (lang) lcd.print("  High  : ");
                  if (!lang) {lcd.print("  B");lcd.write(5); lcd.print("COTA : ");}
                  delay (delay_time_1);  lcd.print(Size_H);          
}

long measuring(int TP, int EP) { 
  digitalWrite(TP, LOW);  delayMicroseconds(2); digitalWrite(TP, HIGH);      // pull the Trig pin to high level for more than 10us impulse 
  delayMicroseconds(10);  digitalWrite(TP, LOW);
    Size = (pulseIn(EP,HIGH))*0.365/2 ;  //расстояние в mm (0,34 расчетное)
    //Serial.print("//Size-"); Serial.print(Size)  ;Serial.println("//");
  return Size;                    // return Измеренное расстояние
}

long sizing () {  // обмер по трем осям
        delay (10);
          measuring (TR_PIN_H, EC_PIN_H); Size_H = Base_H - Size; delay (30);
          measuring (TR_PIN_W, EC_PIN_W); Size_W = Base_W - Size; delay (30);
          measuring (TR_PIN_L, EC_PIN_L); Size_L = Base_L - Size; delay (30);
         menu = LOW;
         //Serial.print("H-"); Serial.print(Base_H)  ;Serial.print("W-"); Serial.print(Base_W)  ;Serial.print("L-"); Serial.println(Base_L)  ;
        return Size_H, Size_W, Size_L; //Измеряем про осям по осям
        }

void display_ok ()   //экран режима измерения   
        {
       // int s1,s2,s3,s4,s5;
        lcd.clear();
        lcd.createChar(4, bukva_g); 
            display_result(0);
            if (lang) {lcd.setCursor(0, 3);  lcd.print("  Weight: "); lcd.print(weight,prec); lcd.print(" g");}
            if (!lang) {lcd.setCursor(0, 3);  lcd.print("  BEC    : "); lcd.print(weight,prec); lcd.print(" "); lcd.write(4); } // без преобразования типа
            //делю на 10 округляю и умножаю на 10
           }

void display_welcome() // заставка при включении
         {
         lcd.clear();
         if (!lang) {
            lcd.createChar(1, bukva_I); lcd.createChar(2, bukva_G); lcd.createChar(3, bukva_Ya); lcd.createChar(4, bukva_D); lcd.createChar(5, bukva_L); // создаем символы и записываем их в память LCD
            lcd.setCursor(0, 0);  lcd.print("    MACTEP K");lcd.write(1); lcd.print("T");
            lcd.setCursor(0, 2);  lcd.print("CKAHEP ");lcd.write(4);lcd.write(5);lcd.write(3); lcd.print(" ");lcd.write(5);lcd.print("O");lcd.write(2);lcd.write(1);lcd.print("CT");lcd.write(1);lcd.print("K");lcd.write(1);
            lcd.setCursor(0, 3);  lcd.print("      v.3.0");
            delay (1000);
         }  
         if (lang) {
            lcd.setCursor(0, 0);  lcd.print("     Master Kit");
            lcd.setCursor(0, 2);  lcd.print("   Logistic Scaner");
            lcd.setCursor(0, 3);  lcd.print("       v.3.0");
            delay (1000);  
         }
}

int read_LCD_buttons() //считываем нажатие джойстика Моя плата
          {  
          Serial.println (adc_key_in = analogRead(0)); 
        /*  if (adc_key_in > 900)  return btnNONE; 
          if (adc_key_in < 50)   return btnUP; 
          if (adc_key_in < 350)  return btnRIGHT; 
          if (adc_key_in < 500)  return btnDOWN;
          if (adc_key_in < 630)  return btnSELECT;        
          if (adc_key_in < 750)  return btnLEFT;   
          return btnNONE;  // when all others fail, return this... */
          //для платы v 2.2  //// v.2.1 перепутаны кнопки лево и право
          if (adc_key_in > 950)  return btnNONE; 
          if (adc_key_in < 50)   return btnUP; 
          if (adc_key_in < 600)  return btnRIGHT; 
          if (adc_key_in < 720)  return btnDOWN;
          if (adc_key_in < 800)  return btnSELECT;        
          if (adc_key_in < 900)  return btnLEFT;   
          return btnNONE;  
          }           

