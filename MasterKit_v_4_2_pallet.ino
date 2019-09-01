/////////////////////////////////////////////////////////////
//   LogisticSonar 3d v.4.03 + весы  , протокол            //
//   Leonardo + моя плата v.2.1 + BAT sensors  + COM       //
//   DIY-pragmatiс konakovskiy@gmail.com                   //
//   30.03.2019                                            //
//    Установлено в Чехове, 25.06.19                       //
/////////////////////////////////////////////////////////////
//Версия для измерения паллет, 4 датчика (2 ширины),
//подключение весов через COM-порт1 
//Провод от весов прямой, вилка-розетка
//На модуле RS-232 Tx ---> D1   RX---> D0
//Выход на исходную позицию для сканирования
//Размещаем коробку с известными размерами и настраиваем отображение размеров наиболее точно.
//Максимальные размеры 1200x1200x2700 mm
//макс. вес:  2000 кг (выводятся в формате весов)
//вывод:      размеры в см (3 разряда), 1 знак после запятой
//Передача данных в COM порт компьютера по запросу с РС:EC,GET,45,46,47,48,<CR><LF> или по кнопке
//Строка вывода: РС:EC,GET,45,H,46,W,47,L,48,Wt<CR><LF>

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
#include <EEPROM.h>
#include <Keyboard.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  //LCD standart
float sof ;//= 0.346;  //скорость звука
int ko = 10; // коэффициент округления при выводе
int t = 0;   //количество измерений перед автоматической передачей данных
boolean lang =  0; //0- русский язык, true - английский
int print_btn = 13; // Кнопка передачи данных
int print_btn_1 = 3; // Вторая кнопка передачи данных для брелка
boolean print_stat = HIGH ; //текущее состояние кнопки передачи данных/при нажатии переходит в 0
boolean print_stat_1 = LOW ; //текущее состояние кнопки передачи данных/при нажатии переходит в 1
boolean print_stat_prew = LOW; // предыдущее состояние кнопки
boolean menu = LOW; //признак активированного меню
long Size = 0; //текущее измерение расстояни до цели
long SumSize = 0; //сумма измерений сторон
long SumBase_M = 0; //сумма ручного ввода базовых расстояний
//описание меню меню - EXIT, 0 - главное меню
int menupos = 0;//код пункта меню
int menupos_end = 4; //крайний пункт меню - EXIT, 0 - главное меню
boolean outputtype = 0;  //тип вывода данных в буфер 0 - ручн, 1 - автомат
boolean outputway = 0;  // канал вывода   0 - клавиатура, 1 - компорт по команде РС
boolean incomerequest = 0; //1 поступление входящего запроса данных
//
long SumSizeMin = 600; //порог чувствительности к дребезгу измерения по сумме осей 
//long SizeGate = 500;  //чувствительность к дребезгу измерения по сумме осей 
float Size_WK = 0;// переменная для калибровки скорости звука
//Канал ШИРИНА
int TR_PIN_W = 21; //A3;      //trigger pin on the ultrasonic sensor
int EC_PIN_W = 20; //A2;      //echo pin on the ultrasonic sensor
long Base_W_M = 0; //базовое расстояние, введенное руками
long Base_W = 0;
long Size_W = 0;
//Канал ШИРИНА_1
int TR_PIN_W1 = 12;     //trigger pin on the ultrasonic sensor
int EC_PIN_W1 = 11;     //echo pin on the ultrasonic sensor
long Size_W1 = 0;
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
//Взвешивание
float weight = 0; //полученный вес
float weight_gate = 3; //минимально взвешиваемый вес
int prec = 0; //точность вывода веса 0 - без знаков после запятой


void setup()
{
  //EEPROM.put(30, 0.346);
  Serial.begin (9600); Serial1.begin (9600); 
  lcd.begin(20, 4);  Keyboard.begin(); 
  pinMode(TR_PIN_H,OUTPUT); pinMode(EC_PIN_H,INPUT_PULLUP);    // set TP output for trigger, EP input for echo
  pinMode(TR_PIN_W,OUTPUT); pinMode(EC_PIN_W,INPUT_PULLUP);    
  pinMode(TR_PIN_L,OUTPUT); pinMode(EC_PIN_L,INPUT_PULLUP);
  pinMode(TR_PIN_W1,OUTPUT); pinMode(EC_PIN_W1,INPUT_PULLUP);   
  pinMode (print_btn, INPUT_PULLUP); pinMode (print_btn_1, INPUT_PULLUP);
  pinMode(10,OUTPUT); digitalWrite(10, 1); //Включаем Подсветка LCD
  display_welcome();    // приглашение
  delay (4000);
  outputtype = EEPROM.read(4); outputway = EEPROM.read(6);
  Base_H = EEPROM.read(10) | (EEPROM.read(11) << 8);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
  Base_W = EEPROM.read(12) | (EEPROM.read(13) << 8);
  Base_L = EEPROM.read(14) | (EEPROM.read(15) << 8);
  EEPROM.get(30, sof); //память для скрости звука
  if (sof < 0.3) EEPROM.put(30, 0.3); // минимальная скорость звука
  SumBase_M = (Base_H + Base_W + Base_L);
     if (SumBase_M == 0) { // если значения 0 - автокалибруем 
     manual_calibration(); //основная калибровка если память чистая
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
        delay (1000);
        }
}

void loop() {
  if(outputway) {  //если вывод в COM порт - слушаем его
                serial_listen();  
                if (incomerequest) {printing_com(); incomerequest = 0; } // Если услышали - печатаем в СОМ. Это для Курьерки, чтобы не висло
                }
   key_read ();   //чтение кнопок с защитой от дребезга;
        if ((lcd_key == 5)&&(menu==LOW)) 
        {  
                sizing(); SumSize = (Size_H+Size_W+Size_L); //запуск измерениe, запоминаем сумму размеров  

                //проверяем не нажата ли кнопка
                if ((digitalRead(print_btn)==0)||(digitalRead(print_btn_1)==1))  print_stat = LOW; //читаем 2 кнопки, если одна из них нажата - то передача
                else print_stat = HIGH; //если не нажата ни одна - восстанавливаем исходное состояние, иначе после передачи останется в нажатом состоянии
                              
        if ((SumSize < SumSizeMin)&&(!print_stat_prew && print_stat)) {display_ready(); weight = 0; Size_L = 0; Size_W = 0; Size_H = 0;} //Размеры стали меньше порога, ГОТОВ К РАБОТЕ
        
        if ((SumSize >= SumSizeMin)&&(!print_stat_prew && print_stat))  {weighting(); display_ok();}
                   // Размеры больше порога, дисплей ИЗМЕРЕНИЕ, слушаем вес
                  
// ---------вывод данных при автоматическом типе
         if ((((Size_H+Size_W+Size_L) >= SumSizeMin)&&(outputtype && !print_stat_prew))&&(weight>weight_gate)){
                     t=t+1; delay(350); //Serial.println (t);
                      if ((!outputway && outputtype && !print_stat_prew) && (t == 4)) {// автоматическая передача данных после 4-х замеров
                       printing (); print_stat_prew = HIGH; t = 0;    //вывод данных и запрет на передачу данных, пока не сброшены размеры
                       weight = 0; //сбрасываем вес
                      }
                    } 
//
//----------выдача в ручном режиме        
       
         if ((SumSize >= SumSizeMin)&&(!outputway && !print_stat_prew && !print_stat)) 
             {  
             printing(); 
             //weight = 0; 
             print_stat_prew = HIGH; //запрет на передачу данных, пока не сброшены размеры
             }  
         else if (((SumSize) < SumSizeMin)&&(!outputway && print_stat_prew && print_stat)) print_stat_prew = LOW; //перезапуск в режим измерений из печати по кнопке 

//----------передача по запросу РС:    
 
         if ((((Size_H+Size_W+Size_L) >= SumSizeMin)&&(outputway && !print_stat_prew))&&(weight>weight_gate)) {
           serial_listen();
            if (incomerequest) {printing_com(); /*weight = 0;*/ print_stat_prew = HIGH; incomerequest = 0; } 
         } 
         else if (((Size_H+Size_W+Size_L) < SumSizeMin)&&(outputway && print_stat_prew)) {print_stat_prew = LOW;} //перезапуск в режим измерений из передача в РС       
         }

//-----------обработка нажатия кнопок в режиме меню 

     else if ((lcd_key == 4)&&(menu == LOW))  menuposition (lcd_key);//главное меню          
     else if ((lcd_key == 2)&&(menu == HIGH)) menuposition (lcd_key); //меню вниз
     else if ((lcd_key == 1)&&(menu == HIGH)) menuposition (lcd_key); //меню вверх
     else if ((lcd_key == 0)&&(menu == HIGH)) {
      
          if (menupos == 0) {t_compensation();} //температурная компенсация
             //for (int i = 10; i < 20; i++) EEPROM.write(i, 0);} // очистка памяти начиная с 10-й ячейки, не трогаем 4 - тип вывода 
          
          if (menupos == 1) manual_calibration();  //Ручная Калибровка 
          if (menupos == 2) set_outputway();  //Выбор канала вывода
          if (menupos == 3) set_outputtype();  //Выбор типа вывода
          if (menupos == menupos_end) {menu = LOW; display_ready();} //Exit
        }
 
}  //конец цикла loop

//вызываемые функции

int manual_calibration()  
{ //ручная коррекция базовых размеров
      int focuspos = 0;
      calibration();
      delay (500);
      display_manualcorr (0);
           do
              {
              key_read ();  //Serial.println (adc_key_in = analogRead(0)); // чтение кнопок
              // движение фокуса по значку осей
              if (lcd_key == 2) {focuspos = focuspos + 1 ; display_manualcorr (focuspos);}
              if (lcd_key == 1) {focuspos = focuspos - 1 ; display_manualcorr (focuspos);}
              if (focuspos < 0) {focuspos = 2; display_manualcorr (focuspos);}
              if (focuspos > 2) {focuspos = 0; display_manualcorr (focuspos);}
              //конец блока обработки фокуса
              //корекция значений
              if ((lcd_key == 0)&&(focuspos == 0)) {++Base_L ; display_manualcorr (focuspos);}
              if ((lcd_key == 3)&&(focuspos == 0)) {--Base_L ; display_manualcorr (focuspos);}
              if ((lcd_key == 0)&&(focuspos == 1)) {++Base_W ; display_manualcorr (focuspos);}
              if ((lcd_key == 3)&&(focuspos == 1)) {--Base_W ; display_manualcorr (focuspos);}
              if ((lcd_key == 0)&&(focuspos == 2)) {++Base_H ; display_manualcorr (focuspos);}
              if ((lcd_key == 3)&&(focuspos == 2)) {--Base_H ; display_manualcorr (focuspos);}
              //if ((lcd_key == 0)&&(focuspos == 3)); 
              }
              while (lcd_key != 4); //Сохранение данных по Центральной кнопке ДЖОЙСТИКА
              if (lcd_key == 4) {
                EEPROM.write(10, Base_H); EEPROM.write(11, Base_H >> 8);
                EEPROM.write(12, Base_W); EEPROM.write(13, Base_W >> 8);
                EEPROM.write(14, Base_L); EEPROM.write(15, Base_L >> 8);
               }
                menu = LOW;  //выход в меню 
                return Base_H, Base_W, Base_L;           
}

int display_manualcorr (int focuspos) 
{            //Вывод на экран результатов ручной корректировки
                  //sizing(); //запускаю обмер
                  lcd.clear(); // вывод на экран всех параметров
                 
                  lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_I); lcd.createChar(3, bukva_D); lcd.createChar(4, bukva_g); lcd.createChar(5,bukva_RR);
                  lcd.createChar(6, bukva_SH);
                  lcd.setCursor(0, 0); if (lang) lcd.print("  Length: ");
                  if (!lang) {lcd.print("  "); lcd.write(3);lcd.write(1);lcd.write(2);lcd.print("HA  : "); }
                  //delay (delay_time); 
                  lcd.print(Base_L);           
                  lcd.setCursor(0, 1); if (lang) lcd.print("   Width: "); 
                  if (!lang) {lcd.print("  ");lcd.write(6); lcd.write(2); lcd.print("P"); lcd.write(2);lcd.print("HA : ");}
                  //delay (delay_time);
                  lcd.print(Base_W);
                  lcd.setCursor(0, 2); if (lang) lcd.print("  Heigth: ");
                  if (!lang) {lcd.print("  B");lcd.write(5); lcd.print("COTA : ");}
                  //delay (delay_time);  
                  lcd.print(Base_H);
                  lcd.setCursor(0, focuspos); lcd.print(">"); // положение курсора
}     
long calibration()   
{ //Функция калибровки, измеряем 5 раз , находим среднее арифметическое. 
        int x=0;
        Size=0; Base_H = 0; Base_W = 0; Base_L = 0;
        //округление по 5 измерениям
          lcd.clear();
          if (lang)  {
            lcd.setCursor(0, 0); 
            lcd.print("  CALIBRATION!  "); 
            }
          if (!lang) {
            lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_I); lcd.createChar(3, bukva_B); lcd.createChar(4, bukva_g); lcd.createChar(5,bukva_RR);
            lcd.setCursor(0, 0); lcd.print("    KA"); lcd.write(1); lcd.write(2); lcd.write(3); lcd.print("POBKA  ");
            }
          measuring_H (TR_PIN_H, EC_PIN_H);  delay (30); // Прогрев
          //measuring (TR_PIN_H, EC_PIN_H);  delay (30); // Прогрев
          measuring (TR_PIN_W, EC_PIN_W);  delay (30);
          measuring (TR_PIN_L, EC_PIN_L);  delay (30);
         for (int i=0; i < 5; i++) {
          lcd.setCursor(0, 1);
          if (i == 0) lcd.print("....");
          if (i == 1) lcd.print("........");
          if (i == 2) lcd.print("............");
          if (i == 3) lcd.print("................");
          if (i == 4) lcd.print("....................");
          delay (100);
          
         measuring_H (TR_PIN_H, EC_PIN_H); Base_H = Base_H + Size; delay (130); 
         //measuring (TR_PIN_H, EC_PIN_H); Base_H = Base_H + Size; delay (130);
         measuring (TR_PIN_W, EC_PIN_W); Base_W = Base_W + Size; delay (130);
         measuring (TR_PIN_L, EC_PIN_L); Base_L = Base_L + Size; delay (130);
         //Serial.print(" Base_H=");Serial.print(Base_H);Serial.print(" Base_W=");Serial.print(Base_W);Serial.print(" Base_L=");Serial.println(Base_L);
         }         
          Base_H = Base_H/5; //Base_H = (round(float((Base_H/5)/10)))*10;
          Base_W = Base_W/5; //Base_W = (round(float((Base_W/5)/10)))*10;
          Base_L = Base_L/5; //Base_L = (round(float((Base_L/5)/10)))*10;
            
            display_result_setup(300);    //Вывод на экран
        
        delay (500);
        //menu = LOW;  //выход в меню
        //display_ready();
        //Serial.print(" Base_H=");Serial.print(Base_H);Serial.print(" Base_W=");Serial.print(Base_W);Serial.print(" Base_L=");Serial.println(Base_L);
        return Base_H, Base_W, Base_L; //Возвращаем базовые размеры по осям
        
}

float t_compensation() 
{ //настройка скорости звука, подгонокой измеренной ширины под реальную
    EEPROM.get(30, sof); //получаем из памяти скорость звука
   //измерение по высоте для температурной калибровки скорости звука
          sof = sof - 0.001; //пока не знаю отчего само прибавляется  0.001
          do  {key_read ();  // чтение кнопок, корекция значений
              sizing_WK(); //запускаю обмер Ширины
              if (lcd_key == 0) {sof = sof + 0.001 ; display_setspeed();}
              if (lcd_key == 3) {sof = sof - 0.001 ; display_setspeed();}
              }
              while (lcd_key != 4); //Сохранение данных по Центральной кнопке ДЖОЙСТИКА
              if (lcd_key == 4) {EEPROM.put(30, sof);} //запись в память по центральной кнопке
              //Serial.println(sof,3);
              return sof;           
}
void display_setspeed()
{
      lcd.clear(); lcd.setCursor(0, 0); lcd.print("Sound speed = "); lcd.print(sof,3); 
      lcd.setCursor(0, 1); lcd.print("      Width = "); lcd.print(Size_WK,0); 
      lcd.setCursor(0, 3); lcd.print("< set >  Enter - o");
}  
//измерение W для калибровки скорости звука       
float sizing_WK() 
{
        Size_WK = 0; int k = 3; int n = 10;//n - коэффициент для устранения погрешности измерения высоты, K - коэф усреднения
        for (int i=0; i < k; i++) {measuring (TR_PIN_W, EC_PIN_W); Size_WK = Size_WK + Size; delay (30);}
        Size_WK = Size_WK/k;
        menu = LOW;
        return Size_WK; //Возвращаем ширину
}
        
long sizing () 
{  // обмер по трем осям с усреднением за 3 разa 4 датчика
 Size_H = 0; Size_W = 0; Size_L = 0; Size = 0; Size_W1 = 0; int k = 3; int n = 10;//n - коэффициент для устранения погрешности измерения высоты, K - коэф усреднения
 for (int i=0; i < k; i++) {
          measuring_H (TR_PIN_H, EC_PIN_H); Size_H = Size_H + Size; delay (50);
          measuring (TR_PIN_W, EC_PIN_W); Size_W = Size_W + Size; delay (50);
          measuring (TR_PIN_W1, EC_PIN_W1); Size_W1 = Size_W1 + Size; delay (50);
          measuring (TR_PIN_L, EC_PIN_L); Size_L = Size_L + Size; delay (50);
          }
        Size_H = Base_H - Size_H/k;
        if (Size_H > (Base_H - n)) Size_L = 0; // когда паллета убрана, высота равна базе +-n
        else Size_L = (Base_L - Size_L/k); // паллета установлена
        Size_W1 = Size_W1/k;
        Size_W = Size_W/k;
        //Serial.print(Size_W);Serial.print("-----");Serial.println(Size_W1);
        if (Size_H > (Base_H - n)) Size_W = 0; // когда паллета убрана, высота равна базе +-n
        else Size_W = Base_W - Size_W - Size_W1;// паллета установлена
        menu = LOW;
        //return Size_H = ((round(float(Size_H)/10))*10), Size_W = ((round(float(Size_W)/10))*10), Size_L = ((round(float(Size_L)/10))*10);//Измеряем про осям по осям
        
        return Size_H, Size_W, Size_L;//Измеряем про осям по осям
}

//Чтение команды РС
boolean serial_listen() 
{
  if (Serial.available()>0) {
    delay (4); incomerequest = Serial.find("EC,GET,45,46,47,48");
    if (!incomerequest) {Serial.flush(); incomerequest = 0;}
    }
  return incomerequest; 
}


void display_sent ()    //экран отправленных данных в РС   
{
        int pos=14;
        lcd.clear(); lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_P); 
            lcd.setCursor(0, 0);
            if (lang) lcd.print("Data sent:");
            if (!lang){
               lcd.print("OT"); lcd.write(2); lcd.print("PAB"); lcd.write(1); lcd.print("EHO:");
             }
         
         lcd.setCursor(9,3); lcd.print("BEC ");
                  if (weight<10000){pos = 12;}
                  if (weight<1000) {pos = 13;}    
                  if (weight<100)  {pos = 14;} 
                  if (weight<10)   {pos = 15;} 
                  lcd.setCursor(pos,3); lcd.print(weight);
                                    
         lcd.setCursor(11,0); lcd.print("L ");
                  if (Size_L<10000) {pos = 15;}    
                  if (Size_L<1000)  {pos = 16;} 
                  if (Size_L<100)   {pos = 17;} 
                  if (Size_L<10)    {pos = 18;}
                  lcd.setCursor(pos,0);lcd.print((round((float(Size_L))/10))*10);
                        
         lcd.setCursor(11,1); lcd.print("W "); 
                  if (Size_W<10000) {pos = 15;}     
                  if (Size_W<1000)  {pos = 16;} 
                  if (Size_W<100)   {pos = 17;} 
                  if (Size_W<10)    {pos = 18;} 
                  lcd.setCursor(pos,1);lcd.print((round((float(Size_W))/10))*10);
                        
         lcd.setCursor(11,2); lcd.print("H "); 
                  if (Size_H<10000) {pos = 15;}     
                  if (Size_H<1000)  {pos = 16;} 
                  if (Size_H<100)   {pos = 17;}
                  if (Size_H<10)    {pos = 18;}
                  lcd.setCursor(pos,2);lcd.print((round((float(Size_H))/10))*10);       
}
void printing ()
{
       
         Keyboard.print((long)weight); Keyboard.print(","); Keyboard.print((long)(weight * 100) % 100);
         delay (50); Keyboard.write(KEY_TAB); delay(50);
         Keyboard.print((round((float(Size_L))/10))*10); Keyboard.write(KEY_TAB); delay (50); 
         Keyboard.print((round((float(Size_W))/10))*10); Keyboard.write(KEY_TAB); delay (50);
         Keyboard.print((round((float(Size_H))/10))*10); Keyboard.write(KEY_RETURN); delay (50);
         display_sent();
}

void printing_com ()  //вывод результатов в COM-порт  ---------------------- сделать 
{
  Serial.print("PC,GET,45,");
  Serial.print(round((float(Size_H))/10));
  Serial.print(",46,");
  Serial.print(round((float(Size_W))/10));
  Serial.print(",47,");
  Serial.print(round((float(Size_L))/10));
  Serial.print(",48,");
  Serial.print((long)weight); Serial.print(","); Serial.print((long)(weight * 100) % 100);
  //Serial.print(weight);
  Serial.print(",");
  Serial.write(0x0D);Serial.write(0x0A);
if (weight>0) display_sent();
}


/* получение веса (весы МЕРА протокол печати этикеток) 
float weighting (){
  
    if (Serial1.available()>10) {delay(2); int tmp = Serial1.parseInt(); weight = Serial1.parseFloat(); delay(5); Serial1.write("!"); display_ok();}
        if (weight>0) {Serial1.flush(); }
     
        //Serial.println (weight);
        return weight;
}
//Serial.print(Stream.read());
*/

// получение веса (весы МП ВЕ(Д)ЖА Циклоп-12 протокол передачи после стабилизации веса) 
float weighting ()
{
  
    if (Serial1.available()>11) {delay(20);
    
        //Serial.println (Serial1.readString()); 
        weight = Serial1.parseFloat(); 
        delay(50); 
        //display_ok();
        }
        if (weight>0) {Serial1.flush(); return weight;}
        //return weight;
}

boolean set_outputway() //выбор канала вывода
{
        outputway = EEPROM.read(6); display_outputway ();
        do { 
              key_read ();  //Serial.println (adc_key_in = analogRead(0)); // чтение кнопок
              if (lcd_key == 0) {outputway = true  ; display_outputway();}
              if (lcd_key == 3) {outputway = false ; display_outputway();}
        }
        while (lcd_key != 4); //Сохранение данных по Центральной кнопке ДЖОЙСТИКА
              if (lcd_key == 4) {EEPROM.write(6, outputway);}
           menu = LOW; display_ready(); Serial.flush();
        return outputway;
}

void display_outputway()    //дисплей выбора типа вывода
{
        lcd.clear(); 
            lcd.setCursor(0, 1);
            if (lang){
              lcd.setCursor(0, 0);  lcd.print("  Output channel");
              if (outputway)  {lcd.setCursor(0, 2);  lcd.print("Keyboard   >> Serial");}
              if (!outputway) {lcd.setCursor(0, 2);  lcd.print("Keyboard <<   Serial");}
            }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_I); lcd.createChar(3, bukva_D);lcd.createChar(7, bukva_RR);lcd.createChar(4, bukva_Y); 
              lcd.setCursor(0, 0);  lcd.print("   KAHA"); lcd.write(1); lcd.print(" B"); lcd.write(7);lcd.print("BO");lcd.write(3); lcd.print("A");
              if (outputway)  {lcd.setCursor(0, 2); lcd.print("K"); lcd.write(1); lcd.write("AB");lcd.write(2);lcd.write("AT"); lcd.write(4); lcd.write("PA"); lcd.print("  >> "); lcd.write("COM");}
              if (!outputway) {lcd.setCursor(0, 2); lcd.print("K"); lcd.write(1); lcd.write("AB");lcd.write(2);lcd.write("AT"); lcd.write(4); lcd.write("PA"); lcd.print(" <<  "); lcd.write("COM");}
            }  
}

boolean set_outputtype()
{
        outputtype = EEPROM.read(4); display_outputtype ();
        do { 
              key_read ();  //Serial.println (adc_key_in = analogRead(0)); // чтение кнопок
              // движение фокуса по значку осей
              if (lcd_key == 0) {outputtype = true  ; display_outputtype();}
              if (lcd_key == 3) {outputtype = false ; display_outputtype();}
        }
        while (lcd_key != 4); //Сохранение данных по Центральной кнопке ДЖОЙСТИКА
              if (lcd_key == 4) {EEPROM.write(4, outputtype);}
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



void display_ready() //экран готовности к измерению
{
        lcd.clear();  
        if (lang) {
            lcd.setCursor(0, 1);  lcd.print("     Ready for");
            lcd.setCursor(0, 2);  lcd.print("    measurement");
            }         
        if (!lang) { // на русском
            lcd.createChar(1, bukva_G); lcd.createChar(2, bukva_B); 
            lcd.setCursor(0, 1);  lcd.print("   "); lcd.write(1); lcd.print("OTOB K PA"); lcd.write(2); lcd.print("OTE");
            } 
}        

//Функция MANUAL калибровки, измеряем 3 раза, находим среднее арифметическое, оставляем 
// на экране, делаем редактирование.     

void display_result_setup(long delay_time) 
{ //вывод экрана для автоматической калибровки
                  //sizing(); //запускаю обмер
                  lcd.clear(); // вывод на экран всех параметров
                  lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_I); lcd.createChar(3, bukva_D); lcd.createChar(4, bukva_g); lcd.createChar(5,bukva_RR);
                  lcd.createChar(6, bukva_SH);
                  lcd.setCursor(0, 0); if (lang) lcd.print("  Length: ");
                  if (!lang) {lcd.print("  "); lcd.write(3);lcd.write(1);lcd.write(2);lcd.print("HA  : "); }
                  
                  delay (delay_time); lcd.print(Base_L);           
                  lcd.setCursor(0, 1); if (lang) lcd.print("   Width: "); 
                  if (!lang) {lcd.print("  ");lcd.write(6); lcd.write(2); lcd.print("P"); lcd.write(2);lcd.print("HA : ");}
                  delay (delay_time);lcd.print(Base_W);
                  lcd.setCursor(0, 2); if (lang) lcd.print("  Heigth: ");
                  if (!lang) {lcd.print("  B");lcd.write(5); lcd.print("COTA : ");}
                  delay (delay_time);  lcd.print(Base_H);
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
              lcd.setCursor(0, 0);  lcd.print("> Temperature Calibr");
              lcd.setCursor(0, 1);  lcd.print("  Manual Calibr");
              lcd.setCursor(0, 2);  lcd.print("  Output channel");
              lcd.setCursor(0, 3);  lcd.print("  Output type");
            }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);lcd.createChar(8, bukva_P);
              lcd.setCursor(0, 0);  lcd.print("> TEM"); lcd.write(8); lcd.print("  KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.setCursor(0, 1);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print("  KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("  KAHA"); lcd.write(1);lcd.print(" B"); lcd.write(7);lcd.print("BO"); lcd.write(6); lcd.print("A");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 3);  lcd.print("  T"); lcd.write(3);lcd.write(8); lcd.print("   B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
           }  
          menu = HIGH;
          }
          if (menupos == 1)  {
            lcd.clear();  
            if (lang){
              lcd.setCursor(0, 0);  lcd.print("  Temperature Calibr");
              lcd.setCursor(0, 1);  lcd.print("> Manual Calibr");
              lcd.setCursor(0, 2);  lcd.print("  Output channel");
              lcd.setCursor(0, 3);  lcd.print("  Output type");
            }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);lcd.createChar(8, bukva_P);
              lcd.setCursor(0, 0);  lcd.print("  TEM"); lcd.write(8); lcd.print("  KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.setCursor(0, 1);  lcd.print("> P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print("  KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("  KAHA"); lcd.write(1);lcd.print(" B"); lcd.write(7);lcd.print("BO"); lcd.write(6); lcd.print("A");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 3);  lcd.print("  T"); lcd.write(3);lcd.write(8); lcd.print("   B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
           }
          menu = HIGH;  
          }
           if (menupos == 2)  {
            lcd.clear();
           if (lang){
              lcd.setCursor(0, 0);  lcd.print("  Temperature Calibr");
              lcd.setCursor(0, 1);  lcd.print("  Manual Calibr");
              lcd.setCursor(0, 2);  lcd.print("> Output channel");
              lcd.setCursor(0, 3);  lcd.print("  Output type");
            }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);lcd.createChar(8, bukva_P);
              lcd.setCursor(0, 0);  lcd.print("  TEM"); lcd.write(8); lcd.print("  KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.setCursor(0, 1);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print("  KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("> KAHA"); lcd.write(1);lcd.print(" B"); lcd.write(7);lcd.print("BO"); lcd.write(6); lcd.print("A");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 3);  lcd.print("  T"); lcd.write(3);lcd.write(8); lcd.print("   B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
          }
          menu = HIGH;  
          }
          if (menupos == 3)  {
            lcd.clear();
           if (lang){
              lcd.setCursor(0, 0);  lcd.print("  Temperature Calibr");
              lcd.setCursor(0, 1);  lcd.print("  Manual Calibr");
              lcd.setCursor(0, 2);  lcd.print("  Output channel");
              lcd.setCursor(0, 3);  lcd.print("> Output type");
              }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);lcd.createChar(8, bukva_P);
              lcd.setCursor(0, 0);  lcd.print("  TEM"); lcd.write(8); lcd.print("  KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.setCursor(0, 1);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print("  KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("  KAHA"); lcd.write(1);lcd.print(" B"); lcd.write(7);lcd.print("BO"); lcd.write(6); lcd.print("A");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 3);  lcd.print("> T"); lcd.write(3);lcd.write(8); lcd.print("   B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
          }
          menu = HIGH;  
          }
                  
          if (menupos == menupos_end)  {              
          lcd.clear();
          if (lang){
              lcd.setCursor(0, 0);  lcd.print("  Manual Calibr");
              lcd.setCursor(0, 1);  lcd.print("  Output channel");
              lcd.setCursor(0, 2);  lcd.print("  Output type");
              lcd.setCursor(0, 3);  lcd.print("> Exit");
            }
            if (!lang){
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_Ya);
              lcd.setCursor(0, 0);  lcd.print("  P"); lcd.write(4); lcd.write(5); lcd.print("H"); lcd.print("  KA"); lcd.write(1);lcd.write(3);lcd.write(2); lcd.print("P");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(8, bukva_D);
              lcd.setCursor(0, 1);  lcd.print("  KAHA"); lcd.write(1);lcd.print(" B"); lcd.write(7);lcd.print("BO"); lcd.write(6); lcd.print("A");
              lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_B); lcd.createChar(3, bukva_I);lcd.createChar(8, bukva_P);
              lcd.createChar(4, bukva_Y); lcd.createChar(5, bukva_CH); lcd.createChar(7, bukva_RR); lcd.createChar(6, bukva_D);
              lcd.setCursor(0, 2);  lcd.print("  T"); lcd.write(3);lcd.write(8); lcd.print("   B"); lcd.write(7);lcd.print("BO");lcd.write(6); lcd.print("A");
              lcd.setCursor(0, 3);  lcd.print("> B"); lcd.write(7); lcd.print("XO"); lcd.write(6);
            }  
          }
           return menupos;
}  


void display_result(long delay_time_1) 
{
 lcd.clear(); // вывод на экран всех параметров? ko - коэф. округления
                  lcd.createChar(1, bukva_L); lcd.createChar(2, bukva_I); lcd.createChar(3, bukva_D); lcd.createChar(4, bukva_g); lcd.createChar(5,bukva_RR);
                  lcd.createChar(6, bukva_SH);
                  lcd.setCursor(0, 0); if (lang) lcd.print("  Length: ");
                  if (!lang) {lcd.print("  "); lcd.write(3);lcd.write(1);lcd.write(2);lcd.print("HA  : "); }
                  delay (delay_time_1); lcd.print(Size_L = (round(float(Size_L/ko))*ko));
                  lcd.setCursor(0, 1); if (lang) lcd.print("   Width: "); 
                  if (!lang) {lcd.print("  ");lcd.write(6); lcd.write(2); lcd.print("P"); lcd.write(2);lcd.print("HA : ");}
                  delay (delay_time_1);lcd.print(Size_W = (round(float(Size_W/ko))*ko));
                  lcd.setCursor(0, 2); if (lang) lcd.print("  Heigth: ");
                  if (!lang) {lcd.print("  B");lcd.write(5); lcd.print("COTA : ");}
                  delay (delay_time_1);  lcd.print(Size_H = (round(float(Size_H/ko))*ko));         
}

long measuring(int TP, int EP) //запуск датчика для измерения
{ 
  digitalWrite(TP, LOW);  delayMicroseconds(30); digitalWrite(TP, HIGH);      // pull the Trig pin to high level for more than 10us impulse 
  delayMicroseconds(30);  digitalWrite(TP, LOW);
    Size = (pulseIn(EP,HIGH))*sof/2 ; //0.362/2 ;  //расстояние в mm (0,34 расчетное)
    return Size;                    // return Измеренное расстояние
}

long measuring_H(int TP, int EP) //запуск датчика MAXBOTIX для измерения
{ 
  digitalWrite(TP, LOW);  delayMicroseconds(30); digitalWrite(TP, HIGH);      // pull the Trig pin to high level for more than 10us impulse 
  delayMicroseconds(30);  digitalWrite(TP, LOW);
    Size = ((pulseIn(EP,HIGH))/147)*25.4; //*sof/2 ; //0.362/2 ;  //расстояние в mm (0,34 расчетное)
    return Size;                    // return Измеренное расстояние
}

/*
long sizing () {  // обмер по трем осям с усреднением за 3 разa
 Size_H = 0; Size_W = 0; Size_L = 0; Size = 0;
 for (int i=0; i < 3; i++) {
          measuring (TR_PIN_H, EC_PIN_H); Size_H = Size_H + Size; delay (30);
          measuring (TR_PIN_W, EC_PIN_W); Size_W = Size_W + Size; delay (30);
          measuring (TR_PIN_L, EC_PIN_L); Size_L = Size_L + Size; delay (30);
          //delay(500);Serial.print("H-"); Serial.print(Size_H)  ;Serial.print(" W-"); Serial.print(Size_W)  ;Serial.print(" L-"); Serial.println(Size_L)  ;
        }
        Size_H = Base_H - Size_H/3;
        Size_W = Base_W - Size_W/3;
        Size_L = Base_L - Size_L/3;
        menu = LOW;
        return Size_H, Size_W, Size_L;//Измеряем про осям по осям
}
*/
/*
long sizing () {  // обмер по трем осям, без усреднения
        delay (10);
          measuring (TR_PIN_H, EC_PIN_H); Size_H = Base_H - Size; delay (30);
          measuring (TR_PIN_W, EC_PIN_W); Size_W = Base_W - Size; delay (30);
          measuring (TR_PIN_L, EC_PIN_L); Size_L = Base_L - Size; delay (30);
         menu = LOW;
        //weighting(); //Serial.print("fast weight"); Serial.println(weight);      // вылавливал дрейф тары
        // Serial.print(" H-"); Serial.print(Base_H)  ;Serial.print(" W-"); Serial.print(Base_W)  ;Serial.print(" L-"); Serial.println(Base_L)  ;
        return Size_H, Size_W, Size_L;//Измеряем про осям по осям
        }
*/
void display_ok ()   //экран режима измерения   
        {
             lcd.clear(); lcd.createChar(4, bukva_g); 
            display_result(0);
            if (lang)  {lcd.setCursor(0, 3);  lcd.print("  Weight : "); lcd.print(weight); }; //lcd.print(" kg");}
            if (!lang) {lcd.setCursor(0, 3);  lcd.print("  BEC    : "); lcd.print(weight); }; //lcd.print(" K"); lcd.write(4); } // без преобразования типа
            }

void display_welcome() // заставка при включении
{
         lcd.clear();
         if (!lang) {
            lcd.createChar(1, bukva_I); lcd.createChar(2, bukva_G); lcd.createChar(3, bukva_Ya); lcd.createChar(4, bukva_D); lcd.createChar(5, bukva_L); // создаем символы и записываем их в память LCD
            lcd.setCursor(0, 0);  lcd.print("     Master Kit");//lcd.write(1); lcd.print("T");
            lcd.setCursor(0, 1);  lcd.print("CKAHEP ");lcd.write(4);lcd.write(5);lcd.write(3); lcd.print(" ");lcd.write(5);lcd.print("O");lcd.write(2);lcd.write(1);lcd.print("CT");lcd.write(1);lcd.print("K");lcd.write(1);
            //lcd.setCursor(0, 2);  lcd.print("      v.4.03");
            lcd.setCursor(0, 3);  lcd.print("pallet vers. s/n 001");
            //delay (1000);
         }  
         if (lang) {
            lcd.setCursor(0, 0);  lcd.print("     Master Kit");
            lcd.setCursor(0, 1);  lcd.print("   Logistic Scaner");
            //lcd.setCursor(0, 2);  lcd.print("       v.4.03");
            lcd.setCursor(0, 3);  lcd.print("pallet version S/N 001");
            //delay (1000);  
         }
}

int read_LCD_buttons() //считываем нажатие джойстика Моя плата
{  
         // Serial.println (adc_key_in = analogRead(0)); 
          adc_key_in = analogRead(0); 
          if (adc_key_in > 950)  return btnNONE; 
          if (adc_key_in < 50)   return btnUP; 
          if (adc_key_in < 600)  return btnRIGHT; 
          if (adc_key_in < 720)  return btnDOWN;
          if (adc_key_in < 800)  return btnSELECT;        
          if (adc_key_in < 900)  return btnLEFT;   
          return btnNONE;  
}          
