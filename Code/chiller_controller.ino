/*
 * Autor : Eng.Masoud Babaabasi
 * Last date modified : 31/12/2019
 * designed Eng.ALi Shamandi
 * Air conditioner troubleshooter
 * ATMEGA2560
 * 5 temprature sensors : DS18b20 one wire databus
 * 2 pressure sensors : 4-20mA output - 0-40Bar
 * 1 current sensor : SCT013 Current transformer - 0-1V output - 0-70A
 * sim800c module
 * sd card
 */
#include <Arduino.h>
#include <Keypad.h>
#include <U8g2lib.h>
#include <DS3231.h>
#include <SPI.h>
#include <SD.h>
#include <BareBoneSim800.h>
#include "calender_converter.h"
#include "pinMap.h"
#include "chiller.h"

#define BorderW 0
#define Spaceing 0

#define OK 'A'
#define BACK 'B'
#define CANCEL 'C'
//Hold state register bit assignment
#define holdStateBIT(i) (( holdState & ( 1 << i ) ) >> i) 
#define CANCEL_bit 0
#define Menue_bit 1
#define Pressure1_bit 2
#define Pressure2_bit 3
uint8_t holdState = 0;

uint8_t lastFaults;
uint8_t timeFlag;
uint8_t lastStatus;
Time lastTime;
uint32_t faultTime =0;
uint32_t HTDCTime =0;
uint32_t LTshTime =0;
const char limits[22][26] PROGMEM = {
  "1.Max superheat(X1):",
  "2.Min superheat(X2):",
  "3.Max Tdc-W(X3):",
  "4.Max Tdc-A(X4):",
  "5.Max Pc-W(X5):",
  "6.Max Pc-A(X6):",
  "7.Min Pc-W(X7):",
  "8.Min Pc-A(X8):",
  "9.Max liquid diff(X9):",
  "10.Min Toil(X10):",
  "11.Max subcold(X11):",
  "12.Min subcold(X12):",
  "13.Max Pe-W(X13):",
  "14.Max Pe-A(X14):",
  "15.Min Pe-W(X15):",
  "16.Min Pe-A(X16):",
  "17.Min Comp.Current(EA):",
  "18.Min (Pc-Pe)(EP):",
  "19.Stable time:",
  "20.Record Log time:",
  "21.Max Fault time:",
  "22.Critical time:"
};
const char uinits[22][4] PROGMEM = {
  "C",
  "C",
  "C",
  "C",
  "Psi",
  "Psi",
  "Psi",
  "Psi",
  "C",
  "C",
  "C",
  "C",
  "Psi",
  "Psi",
  "Psi",
  "Psi",
  "A",
  "Psi",
  "sec",
  "sec",
  "min",
  "min"
};
/***********************************************************************************************************************/
const uint32_t UID PROGMEM = 5555;
/***********************************************************************************************************************/
Chiller chiller(temp_Pin); // (WATER_COOLED or AIR_COOLED ) and (EXPANSION_VALVE or CAPILLARY_TUBE) ans gas type
/***********************************************************************************************************************/
//U8G2_T6963_240X128_F_8080(rotation, d0, d1, d2, d3, d4, d5, d6, d7, enable, cs, dc , [reset])
// 3840 bytes of ram is needed
//for 2560 8kB of ram is enough
U8G2_T6963_240X128_F_8080 GLCD(U8G2_R0, LCD_DB1, LCD_DB2, LCD_DB3, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7, LCD_DB8
                               , LCD_WR, LCD_CE, LCD_CD, LCD_RST); // Connect RD with +5V, FS0 and FS1 with GND
/***********************************************************************************************************************/
#define ROWS 4 //four rows
#define COLS 4 //four columns
const char keys[ROWS][COLS] PROGMEM = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
const char rowPins[ROWS] PROGMEM =  {key1, key2, key3, key4};//connect to the row pinouts of the keypad
const char colPins[COLS] PROGMEM = {key5, key6, key7, key8}; //connect to the column pinouts of the keypad
Keypad keypad( (char*)(keys), rowPins, colPins, ROWS, COLS );
/***********************************************************************************************************************/
DS3231 rtc(RTC_SDA, RTC_SCL);
/***********************************************************************************************************************/

/***********************************************************************************************************************/
void showMainWindow(uint8_t mod = 1);
void showSettingWindow();
float readnumber(uint8_t x , uint8_t y, uint8_t drawColor,const PROGMEM char  *unit,uint8_t unitmode = 1);
void recordLog();
void sendSMSlog();
void I2CClearBus(uint8_t sclPin, uint8_t sdaPin);
void getPresure(uint8_t PressurePin);
void addTempSeneor();
void deleteTempSensor();
void parameterSet();
void typeSettings();
void thresholds();
void setDateTime();
void setPhoneNumber();
/***********************************************************************************************************************/
void setup() {
  LED_setup;
  Serial_debug.begin(115200);
  relays_setup;
  I2CClearBus(RTC_SCL,RTC_SDA);// check to see if I2C bus is busy
  rtc.begin();
  lastTime = MiladiToShamsi(rtc.getTime());
  if (!SD.begin(SD_CSpin));
  // GLCD setup
  pinMode(LCD_RD, OUTPUT);
  digitalWrite(LCD_RD, HIGH);
  pinMode(LCD_BL, OUTPUT);
  GLCD.begin();
  //******
  GSM_setup;
  GSM_ON;
  keypad.addEventListener(keypadEvent);
  keypad.setHoldTime(1000);
  faultTime = millis();
  lastTime = MiladiToShamsi(rtc.getTime());
  if(digitalRead(GSM_status) == HIGH){
  GSM_OFF;
  delay(1500);
  GSM_ON;
  }
}

void loop() {
  //if ( holdState & ( 1 << Menu_bit ) >> Menu_bit ) showSettingWindow();
  //else showMainWindow();
  char key = keypad.getKey();
  //uint32_t u;
  switch(key){
    case '3':
    //chiller.faults =0xff;
     sendSMSlog();

    default:
    showMainWindow();
  }
  /*shamsi = MiladiToShamsi(rtc.getTime());
  GLCD.clearBuffer();
    GLCD.setFont(u8g2_font_ncenB14_tr);
    sprintf_P(str2,"%d:%d:%d",shamsi.hour,shamsi.min,shamsi.sec);
    GLCD.drawStr(20,20,str2);
    sprintf_P(str2,"%d/%d/%d",shamsi.year,shamsi.mon,shamsi.date);
    GLCD.drawStr(20,40,str2);
     GLCD.sendBuffer();*/
}
/***********************************************************************************************************************/
/***********************************************************************************************************************/
void showMainWindow(uint8_t mod){
    Time shamsi;
    char *str="xxxxxxxxxxxxxxxxxxx";
    char str1[32];
    char str2[5];
    uint8_t y,check,nl;
    int dt=0;

    if( mod ){
      lastFaults = chiller.faults;
      if(chiller.checkStatus(HTDC)) lastStatus |= 1 << 0 ;
      else lastStatus &= ~(1 << 0) ;
      if(chiller.checkStatus(LTsh)) lastStatus |= 1 << 1 ;
      else lastStatus &= ~(1 << 1) ;
      
      chiller.temprature();
      chiller.current();
      if (! holdStateBIT(Pressure1_bit) ) chiller.Pc = chiller.presure(presure_pin1);
      if (! holdStateBIT(Pressure2_bit) ) chiller.Pe = chiller.presure(presure_pin2);
      chiller.superheat();
      chiller.subcold();
      chiller.refreshStatus();
      chiller.diagnose();
    }
    
    if(chiller.checkStatus(HTDC) && !((lastStatus & ( 1 << 0 ) ) >> 0) ){
      timeFlag |= 1 << 1 ;
      HTDCTime = millis();
    }
    else if(!chiller.checkStatus(HTDC)){
      timeFlag &= ~(1 << 1) ;
    }
    if( ((timeFlag & ( 1 << 1 ) ) >> 1) && ((millis() - HTDCTime) / 60000.0) >= chiller.T_critical){
      chiller.faults |= 1 << 6 ;
    }
    else{
       chiller.faults &= ~(1 << 6) ;
    }

    if(chiller.checkStatus(LTsh) && !((lastStatus & ( 1 << 1 ) ) >> 1) ){
      timeFlag |= 1 << 2 ;
      LTshTime = millis();
    }
    else if(!chiller.checkStatus(LTsh)){
      timeFlag &= ~(1 << 2) ;
    }
    if( ((timeFlag & ( 1 << 2 ) ) >> 2) && ((millis() - LTshTime) / 60000.0) >= chiller.T_critical){
      chiller.faults |= 1 << 7 ;
    }
    else{
       chiller.faults &= ~(1 << 7) ;
    }
    
    if( chiller.faults != lastFaults  && chiller.faults != 0 && !( (timeFlag & ( 1 << 0 ) ) >> 0)) {
      timeFlag |= 1 << 0 ;
      faultTime = millis();
    }
    if(( (timeFlag & ( 1 << 0 ) ) >> 0) && ((millis() - faultTime) / 60000.0) >= chiller.T_fault) {
      sendSMSlog();
      faultTime = millis();
    }
    else if(chiller.faults == 0){
      timeFlag &=  ~( 1 << 0 );
    }

    if(chiller.faults) LED_ON;
    else LED_OFF;
    
    shamsi = MiladiToShamsi(rtc.getTime());
    if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
    if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
    dt = (lastTime.hour - shamsi.hour) * 3600 + (lastTime.min - shamsi.min) * 60 + lastTime.sec - shamsi.sec;
    if( dt >= chiller.T_sample || dt < 0  && mod) {
      lastTime = shamsi;
      recordLog(); 
    }
    //chiller.faults = ( 1 << RU ) | ( 1 << RO ) |( 1 << DC ) |( 1 << REF ) |( 1 << RDF ) |( 1 << CF );
    //chiller.faults = ( 1 << 6 ) | ( 1 << 7 ) ;
    //chiller.status = ( 1 << HPc ) | ( 1 << HPe ) |( 1 << HTDC ) |( 1 << RLL ) |( 1 << LToil ) |( 1 << HTsh ) |( 1 << HTsc );
    GLCD.setFont( u8g2_font_crox1c_mf);
    if(mod){
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.drawBox(0,26,240,13);
      GLCD.drawBox(0,52,168,13);
      GLCD.drawBox(0,78,168,13);
      
      GLCD.drawFrame(168,39,72,15);
      GLCD.drawFrame(168,39,72,66);
      GLCD.drawFrame(169,39,70,65);
      GLCD.drawFrame(0,104,240,24);
      GLCD.drawFrame(1,105,238,22);
      }
      //GLCD.setFont( u8g2_font_7x14_mf);
      //GLCD.setFont( u8g2_font_unifont_t_arabic);
      //GLCD.setFont( u8g2_font_unifont_tf);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,str);
  
      
      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);
  
      if(mod){
        GLCD.setDrawColor(1);
      switch(chiller.getFluid()){
        case WATER_COOLED: 
                  sprintf_P(str1,PSTR("%S"),F("Water Cooled"));
                  break;
        case AIR_COOLED: 
                  sprintf_P(str1,PSTR("%S"),F("Air Cooled"));
                  break;      
      }
      GLCD.drawStr(2,24,str1);
      
          switch(chiller.getGas()){
        case R22: 
                  sprintf_P(str1,PSTR("%S"),F("R22"));
                  break;
        case R123: 
                  sprintf_P(str1,PSTR("%S"),F("R123"));
                  break;
        case R134A: 
                  sprintf_P(str1,PSTR("%S"),F("R134A2"));
                  break;
        case R404A: 
                  sprintf_P(str1,PSTR("%S"),F("R404A"));
                  break;
        case R410A: 
                  sprintf_P(str1,PSTR("%S"),F("R410A"));
                  break;
        case R507: 
                  sprintf_P(str1,PSTR("%S"),F("R507"));
                  break;
        
      }
      GLCD.drawStr(114,24,str1);
  
      switch(chiller.getType()){
        case EXPANSION_VALVE: 
                  sprintf_P(str1,PSTR("%S"),F("EXV"));
                  break;
        case CAPILLARY_TUBE: 
                  sprintf_P(str1,PSTR("%S"),F("Cap.Tube"));
                  break;      
      }
      GLCD.drawStr(168,24,str1);
  
      GLCD.setDrawColor(0);
      switch(chiller.checkStatus(STABLE)){
        case 0: 
                  sprintf_P(str1,PSTR("%S"),F("State:Unstable"));
                  break;
        case 1: 
                  sprintf_P(str1,PSTR("%S"),F("State:Stable"));
                  break;      
      }
      GLCD.drawStr(0,37,str1);
    }
    /*switch(chiller.checkStatus(ON)){
      case 0: 
                sprintf_P(str1,PSTR("%S"),F("OFF"));
                break;
      case 1: 
                sprintf_P(str1,PSTR("%S"),F("ON"));
                break;      
    }
    GLCD.drawStr(80,37,str1);
    */
    if(mod){
      dtostrf(chiller.Icomp, 3, 1, str2);
      sprintf_P(str1,PSTR("%S%s%S"),F("Current:"), str2,F("A"));
      GLCD.drawStr(120,37,str1);
    }
    
    GLCD.setFont( u8g2_font_crox1c_mf);
    GLCD.setDrawColor(1);
    sprintf_P(str1,PSTR("%S"),F("           "));
    GLCD.drawStr(0,50,str1);
    if( chiller.Pc >= 0 ) {
     if( chiller.Pc >= 100 || (chiller.Pc - floor(chiller.Pc) == 0) ) dtostrf(chiller.Pc, 1, 0, str2);
     else dtostrf(chiller.Pc, 2, 1, str2);
    }
    else sprintf_P(str2,PSTR("%S"),F("--"));
    if( holdStateBIT(Pressure1_bit)) sprintf_P(str1,PSTR("%S%s%S"),F("Pc="), str2,F("Psi"));
    else sprintf_P(str1,PSTR("%S%s%S"),F("Pc:"), str2,F("Psi"));
    GLCD.drawStr(0,50,str1);
    
    if(mod){
      if( chiller.tempSenPre(Tdc_sen) ){
      if( chiller.Tdc >= 100 ) dtostrf(chiller.Tdc, 3, 0, str2);
      else dtostrf(chiller.Tdc, 4, 1, str2);
    }
    else sprintf_P(str2,PSTR("%S"),F("--"));
    sprintf_P(str1,PSTR("%S%s\xb0%S"),F("Tdc:"), str2,F("C"));
    GLCD.drawStr(88, 50,str1);
    }

    GLCD.setDrawColor(0);
    sprintf_P(str1,PSTR("%S"),F("           "));
    GLCD.drawStr(0,63,str1);
    if( chiller.Pe >= 0 ) {
     if( chiller.Pe >= 100 || (chiller.Pe - floor(chiller.Pe) == 0) ) dtostrf(chiller.Pe, 1, 0, str2);
     else dtostrf(chiller.Pe, 2, 1, str2);
    }
    else sprintf_P(str2,PSTR("%S"),F("--"));
    if( holdStateBIT(Pressure2_bit)) sprintf_P(str1,PSTR("%S%s%S"),F("Pe="), str2,F("Psi"));
    else sprintf_P(str1,PSTR("%S%s%S"),F("Pe:"), str2,F("Psi"));
    GLCD.drawStr(0,63,str1);
    if(mod){
    if( chiller.tempSenPre(Tsuc_sen) ) {
      if( chiller.Tsuc >= 100 ) dtostrf(chiller.Tsuc, 3, 0, str2);
      else dtostrf(chiller.Tsuc, 3, 1, str2);
    }
    else sprintf_P(str2,PSTR("%S"),F("--"));
    sprintf_P(str1,PSTR("%S%s\xb0%S"),F("Tsu:"), str2,F("C"));
    GLCD.drawStr(88, 63,str1);

    GLCD.setDrawColor(1);
    if( chiller.tempSenPre(TLi_sen) ) {
      if( chiller.TLi >= 100 ) dtostrf(chiller.TLi, 3, 0, str2);
      else dtostrf(chiller.TLi, 4, 1, str2);
    }
    else sprintf_P(str2,PSTR("%S"),F("--"));
    sprintf_P(str1,PSTR("%S%s\xb0%S"),F("Lin:"), str2,F("C"));
    GLCD.drawStr(0,76,str1);

    if( chiller.tempSenPre(TLo_sen) ){
      if( chiller.TLo >= 100 ) dtostrf(chiller.TLo, 3, 0, str2);
      else dtostrf(chiller.TLo, 4, 1, str2);
    }
    else sprintf_P(str2,PSTR("%S"),F("--"));
    sprintf_P(str1,PSTR("%S%s\xb0%S"),F("Lo:"), str2,F("C"));
    GLCD.drawStr(88,76,str1);

    GLCD.setDrawColor(0);
    if( chiller.tempSenPre(Tsuc_sen) &&  chiller.Pe >= 0 ) dtostrf(chiller.Tsh, 3, 1, str2);
    else sprintf_P(str2,PSTR("%S"),F("--"));
    sprintf_P(str1,PSTR("%S%s\xb0%S"),F("Tsh:"), str2,F("C"));
    GLCD.drawStr(0,89,str1);

    if( chiller.tempSenPre(Tdc_sen) &&  chiller.Pc >= 0) dtostrf(chiller.Tsc, 3, 1, str2);
    else sprintf_P(str2,PSTR("%S"),F("--"));
    sprintf_P(str1,PSTR("%S%s\xb0%S"),F("Tsc:"), str2,F("C"));
    GLCD.drawStr(88,89,str1);

    GLCD.setDrawColor(1);
    if( chiller.tempSenPre(Toil_sen) ) dtostrf(chiller.Toil, 3, 1, str2);
    else sprintf_P(str2,PSTR("%S"),F("--"));
    sprintf_P(str1,PSTR("%S%s\xb0%S"),F("Oil Temp:"), str2,F("C"));
    GLCD.drawStr(0,102,str1);
    
    GLCD.setDrawColor(1);
    sprintf_P(str1,PSTR("%S"),F("Faults:"));
    GLCD.drawStr(176,51,str1);
    y = 57;
    check = 0;
    if(chiller.checkFault(RU)) {
      sprintf_P(str1,PSTR("%S"),F("RU"));
      y += 13;
      GLCD.drawStr(176,y,str1);
      check = 1;
    }
    if(chiller.checkFault(RO)) {
      sprintf_P(str1,PSTR("%S"),F("RO"));
      y += 13;
      GLCD.drawStr(176,y,str1);
      check = 1;
    }
    if(chiller.checkFault(DC)) {
      sprintf_P(str1,PSTR("%S"),F("DC"));
      y += 13;
      GLCD.drawStr(176,y,str1);
      check = 1;
    }
    if(chiller.checkFault(REF)) {
      sprintf_P(str1,PSTR("%S"),F("REF"));
      y += 13;
      if( y > 96) GLCD.drawStr(212,y - 39,str1);
      else GLCD.drawStr(176,y,str1);
      check = 1;
    }
    if(chiller.checkFault(RDF)) {
      sprintf_P(str1,PSTR("%S"),F("RDF"));
      y += 13;
      if( y > 96) GLCD.drawStr(212,y - 39,str1);
      else GLCD.drawStr(176,y,str1);
      check = 1;
    }
    if(chiller.checkFault(CF)) {
      sprintf_P(str1,PSTR("%S"),F("CF"));
      y += 13;
      if( y > 96) GLCD.drawStr(212,y - 39,str1);
      else GLCD.drawStr(176,y,str1);
      check = 1;
    }
    if(chiller.checkFault(6)) {
      sprintf_P(str1,PSTR("%S"),F("HTDC"));
      y += 13;
      if( y > 96) GLCD.drawStr(212,y - 39,str1);
      else GLCD.drawStr(176,y,str1);
      check = 1;
    }
    if(chiller.checkFault(7)) {
      sprintf_P(str1,PSTR("%S"),F("LTsh"));
      y += 13;
      if( y > 96) GLCD.drawStr(212,y - 39,str1);
      else GLCD.drawStr(176,y,str1);
      check = 1;
    }
    if( !check ) {
      if(chiller.checkStatus(ON) && chiller.tempSenPre(Tdc_sen) && chiller.tempSenPre(Tsuc_sen) && chiller.tempSenPre(Toil_sen) && chiller.tempSenPre(TLi_sen) && chiller.tempSenPre(TLo_sen)&&  chiller.Pc >= 0 &&  chiller.Pe >= 0) 
        sprintf_P(str1,PSTR("%S"),F("No Fault"));
      else if(chiller.checkStatus(ON)) sprintf_P(str1,PSTR("%S"),F("No Data"));
      else if(!chiller.checkStatus(ON)) sprintf_P(str1,PSTR("%S"),F("OFF"));
      GLCD.drawStr(172,70,str1);
    }

    GLCD.setDrawColor(1);
    GLCD.setFont( u8g2_font_profont11_mf);
    check = nl = 0;
    sprintf_P(str1,PSTR("%S"),F("Errors:"));
    GLCD.drawStr(4,118,str1);
    y = 46;
    
    if(chiller.checkStatus(HPc)){
      sprintf_P(str1,PSTR("%S"),F("HPc"));
      GLCD.drawStr(y,118,str1);
      check = 1;
      y += 18;
    }
    if(chiller.checkStatus(HPe)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",HPe"));
      else sprintf_P(str1,PSTR("%S"),F("HPE"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 24;
      else y += 18;
      check = 1;
    }
    if(chiller.checkStatus(LPc)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",LPc"));
      else sprintf_P(str1,PSTR("%S"),F("LPc"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 24;
      else y += 18;
      check = 1;
    }
    if(chiller.checkStatus(LPe)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",LPe"));
      else sprintf_P(str1,PSTR("%S"),F("LPe"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 24;
      else y += 18;
      check = 1;
    }
    if(chiller.checkStatus(HTDC)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",HTDC"));
      else sprintf_P(str1,PSTR("%S"),F("HTDC"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 30;
      else y += 24;
      check = 1;
    }
    if(chiller.checkStatus(RLL)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",RLL"));
      else sprintf_P(str1,PSTR("%S"),F("RLL"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 24;
      else y += 18;
      check = 1;
    }
    if(chiller.checkStatus(LToil)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",LToil"));
      else sprintf_P(str1,PSTR("%S"),F("LToil"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 36;
      else y += 30;
      check = 1;
    }
    if(chiller.checkStatus(HTsh)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",HTsh"));
      else sprintf_P(str1,PSTR("%S"),F("HTsh"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 30;
      else y += 24;
      check = 1;
    }
    if(chiller.checkStatus(LTsh)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",LTsh"));
      else sprintf_P(str1,PSTR("%S"),F("LTsh"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 30;
      else y += 24;
      check = 1;
    }
    if(chiller.checkStatus(HTsc)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",HTsc"));
      else sprintf_P(str1,PSTR("%S"),F("HTsc"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 30;
      else y += 24;
      check = 1;
    }
    if(chiller.checkStatus(LTsc)){
      if(check) sprintf_P(str1,PSTR("%S"),F(",LTsc"));
      else sprintf_P(str1,PSTR("%S"),F("LTsc"));
      GLCD.drawStr(y,118,str1);
      if(check) y += 30;
      else y += 24;
      check = 1;
    }
    if( !check ) {
      if(chiller.checkStatus(ON) && chiller.tempSenPre(Tdc_sen) && chiller.tempSenPre(Tsuc_sen) && chiller.tempSenPre(Toil_sen) && chiller.tempSenPre(TLi_sen) && chiller.tempSenPre(TLo_sen)&&  chiller.Pc >= 0 &&  chiller.Pe >= 0) 
        sprintf_P(str1,PSTR("%S"),F("No Errors"));
      else sprintf_P(str1,PSTR("%S"),F("No Data available"));
      GLCD.drawStr(y,118,str1);
    }
    }
    GLCD.sendBuffer();
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void showSettingWindow(){
  uint8_t line=1;
  keypad.deleteEventListener();
  Time shamsi;
  while(1){
    char *str="xxxxxxxxxxxxxxxxxxx";
      char key = keypad.getKey();
      if(key == BACK) {
        break;
      }
      if( key == OK ) {
        switch(line){
          case 1:
          addTempSeneor();
          break;
          case 2:
          deleteTempSensor();
          break;
          case 3:
          parameterSet();
          break;
          case 4:
          setDateTime();
          break;
          case 5:
          setPhoneNumber();
          break;
        }
        continue;
      }
      if(key >= '1' && key <= '5') {
        line = key - '0';
        switch(line){
          case 1:
          addTempSeneor();
          break;
          case 2:
          deleteTempSensor();
          break;
          case 3:
          parameterSet();
          break;
          case 4:
          setDateTime();
          break;
          case 5:
          setPhoneNumber();
          break;
        }
      }
      if( key == 'C' ) line--;
      if( key == 'D' ) line++;
      if( line > 5 ) line = 1;
      if( line < 1 ) line = 5;
      shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,str);

      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%S"),F("Setting Menue:"));
      GLCD.drawStr(2,24,str);

      GLCD.drawBox(2,(line-1)*13+30,236,13);
      
      GLCD.setDrawColor(line != 1);
      sprintf_P(str,PSTR("%S"),F("1.Add a new Temprature sensor"));
      GLCD.drawStr(2,41,str);

      GLCD.setDrawColor(line != 2);
      sprintf_P(str,PSTR("%S"),F("2.Delete a Temprature sensor"));
      GLCD.drawStr(2,54,str);

      GLCD.setDrawColor(line != 3);
      sprintf_P(str,PSTR("%S"),F("3.Device parameters"));
      GLCD.drawStr(2,67,str);

      GLCD.setDrawColor(line != 4);
      sprintf_P(str,PSTR("%S"),F("4.Date and time"));
      GLCD.drawStr(2,80,str);

      GLCD.setDrawColor(line != 5);
      sprintf_P(str,PSTR("%S"),F("5.Enter phone number"));
      GLCD.drawStr(2,93,str);

      GLCD.setDrawColor(1);
      GLCD.drawFrame(0,28,240,95);
      
      GLCD.sendBuffer();
  }
  keypad.addEventListener(keypadEvent);
  
 }
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void addTempSeneor(){
  Time shamsi;
  uint8_t line=1;
  while(1){
    char *str="xxxxxxxxxxxxxxxxxxx";
      char key = keypad.getKey();
      if(key == BACK) {
        break;
      }
      if( key == OK ) {
        uint8_t r = chiller.addTempSen(line-1);
        GLCD.clearBuffer();
        GLCD.setDrawColor(1);
        if( r == 0x28 ) sprintf_P(str,PSTR("%S"),F("Sensor added successfully"));
        else if( r == 0xff ) sprintf_P(str,PSTR("%S"),F("NO sensor Found!"));
        else sprintf_P(str,PSTR("%S"),F("Sensor Already Exists!"));
        GLCD.drawStr(2,11,str);
        GLCD.sendBuffer();
        delay(2000);
        continue;
      }
      if(key >= '1' && key <= '5') {
        line = key - '0';
      }
      if( key == 'C' ) line--;
      if( key == 'D' ) line++;
      if( line > 5 ) line = 1;
      if( line < 1 ) line = 5;
      shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,str);

      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%S"),F("Add a new Temprature sensor:"));
      GLCD.drawStr(2,24,str);

      GLCD.drawBox(2,(line-1)*13+30,236,13);
      
      GLCD.setDrawColor(line != 1);
      sprintf_P(str,PSTR("%S"),F("1.Discharge sensor(Tdc)"));
      GLCD.drawStr(2,41,str);

      GLCD.setDrawColor(line != 2);
      sprintf_P(str,PSTR("%S"),F("2.Suction sensor(Tsu)"));
      GLCD.drawStr(2,54,str);

      GLCD.setDrawColor(line != 3);
      sprintf_P(str,PSTR("%S"),F("3.Oil sensor(Toil)"));
      GLCD.drawStr(2,67,str);

      GLCD.setDrawColor(line != 4);
      sprintf_P(str,PSTR("%S"),F("4.Liquid Line input(TLin)"));
      GLCD.drawStr(2,80,str);

      GLCD.setDrawColor(line != 5);
      sprintf_P(str,PSTR("%S"),F("5.Liquid Line output(TLo)"));
      GLCD.drawStr(2,93,str);

      GLCD.setDrawColor(1);
      GLCD.drawFrame(0,28,240,95);
      
      GLCD.sendBuffer();
  }
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void deleteTempSensor(){
  Time shamsi;
  uint8_t line=1;
  while(1){
    char *str="xxxxxxxxxxxxxxxxxxx";
      char key = keypad.getKey();
      if(key == BACK) {
        break;
      }
      if( key == OK ) {
        chiller.deleteTempSen(line-1);
        GLCD.clearBuffer();
        GLCD.setDrawColor(1); 
        sprintf_P(str,PSTR("%S"),F("Sensor deleted."));
        GLCD.drawStr(2,11,str);
        GLCD.sendBuffer();
        delay(1000);
        continue;
      }
      if( key == 'C' ) line--;
      if( key == 'D' ) line++;
      if(key >= '1' && key <= '5') {
        line = key - '0';
      }
      if( line > 5 ) line = 1;
      if( line < 1 ) line = 5;
      shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,str);

      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%S"),F("Delete a Temprature sensor:"));
      GLCD.drawStr(2,24,str);

      GLCD.drawBox(2,(line-1)*13+30,236,13);
      
      GLCD.setDrawColor(line != 1);
      sprintf_P(str,PSTR("%S"),F("1.Discharge sensor(Tdc)"));
      GLCD.drawStr(2,41,str);

      GLCD.setDrawColor(line != 2);
      sprintf_P(str,PSTR("%S"),F("2.Suction sensor(Tsu)"));
      GLCD.drawStr(2,54,str);

      GLCD.setDrawColor(line != 3);
      sprintf_P(str,PSTR("%S"),F("3.Oil sensor(Toil)"));
      GLCD.drawStr(2,67,str);

      GLCD.setDrawColor(line != 4);
      sprintf_P(str,PSTR("%S"),F("4.Liquid Line input(TLin)"));
      GLCD.drawStr(2,80,str);

      GLCD.setDrawColor(line != 5);
      sprintf_P(str,PSTR("%S"),F("5.Liquid Line output(TLo)"));
      GLCD.drawStr(2,93,str);

      GLCD.setDrawColor(1);
      GLCD.drawFrame(0,28,240,95);
      
      GLCD.sendBuffer();
  }
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void parameterSet(){
  Time shamsi;
  uint8_t line=1;
  while(1){
    char *str="xxxxxxxxxxxxxxxxxxx";
      char key = keypad.getKey();
      if(key == BACK) {
        break;
      }
      if( key == OK ) {
        if(line==1) typeSettings();
        else if(line == 2 ) thresholds();
        else if(line == 3 ) factoryReset();
        continue;
      }
      if( key == 'C' ) line--;
      if( key == 'D' ) line++;
      if( line > 3 ) line = 1;
      if( line < 1 ) line = 3;
      shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,str);

      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%S"),F("Device parameters:"));
      GLCD.drawStr(2,24,str);

      GLCD.drawBox(2,(line-1)*13+30,236,13);
      
      GLCD.setDrawColor(line != 1);
      sprintf_P(str,PSTR("%S"),F("1.Device Type settings"));
      GLCD.drawStr(2,41,str);

      GLCD.setDrawColor(line != 2);
      sprintf_P(str,PSTR("%S"),F("2.Sensor value thresholds"));
      GLCD.drawStr(2,54,str);

      GLCD.setDrawColor(line != 3);
      sprintf_P(str,PSTR("%S"),F("3.Reset to default values"));
      GLCD.drawStr(2,67,str);

      GLCD.setDrawColor(1);
      GLCD.drawFrame(0,28,240,95);
      
      GLCD.sendBuffer();
  }
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void typeSettings(){
  Time shamsi;
  char col;
  char row[3];
  col=0;
  row[0] = chiller.getFluid();
  row[1] = chiller.getGas();
  row[2] = chiller.getType();
  while(1){
    char *str="xxxxxxxxxxxxxxxxxxx";
      char key = keypad.getKey();
      if(key == BACK) {
        break;
      }
      if( key == OK ) {
        col++;
        if( col > 2 ) col=0;
      }
      if( key == 'C' ) {
        row[col]--;
        if( row[0] < 0 ) row[0] = 1;
        if( row[1] < 0 ) row[1] = 5;
        if( row[2] < 0 ) row[2] = 1;
        switch(col){
          case 0: chiller.setFluid(row[0]);
          break;
          case 1:chiller.setGas(row[1]);
          break;
          case 2:chiller.setType(row[2]);
          break;
        }
      }
      if( key == 'D' ) {
        row[col]++;
        if( row[0] > 1 ) row[0] = 0;
        if( row[1] > 5 ) row[1] = 0;
        if( row[2] > 1 ) row[2] = 0;
        switch(col){
          case 0: chiller.setFluid(row[0]);
          break;
          case 1:chiller.setGas(row[1]);
          break;
          case 2:chiller.setType(row[2]);
          break;
        }
      }      
      shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,str);

      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%S"),F("Device Type settings:"));
      GLCD.drawStr(2,24,str);

      switch(col){
        case 0: GLCD.drawFrame(0,26,106,102);  
        break;
        case 1: GLCD.drawFrame(106,26,54,102); 
        break;
        case 2: GLCD.drawFrame(160,26,80,102); 
        break;
      }
      GLCD.drawBox(2,(row[0])*13+28,102,13);
      GLCD.drawBox(108,(row[1])*13+28,50,13);
      GLCD.drawBox(162,(row[2])*13+28,76,13);
      
      GLCD.setDrawColor(row[0] != 0);
      sprintf_P(str,PSTR("%S"),F("Water Cooled"));
      GLCD.drawStr(2,39,str);

      GLCD.setDrawColor(row[0] != 1);
      sprintf_P(str,PSTR("%S"),F("Air Cooled"));
      GLCD.drawStr(2,52,str);


      GLCD.setDrawColor(row[1] != 0);
      sprintf_P(str,PSTR("%S"),F("R22"));
      GLCD.drawStr(108,39,str);

      GLCD.setDrawColor(row[1] != 1);
      sprintf_P(str,PSTR("%S"),F("R123"));
      GLCD.drawStr(108,52,str);

      GLCD.setDrawColor(row[1] != 2);
      sprintf_P(str,PSTR("%S"),F("R134A2"));
      GLCD.drawStr(108,65,str);

      GLCD.setDrawColor(row[1] != 3);
      sprintf_P(str,PSTR("%S"),F("R404A"));
      GLCD.drawStr(108,78,str);

      GLCD.setDrawColor(row[1] != 4);
      sprintf_P(str,PSTR("%S"),F("R410A"));
      GLCD.drawStr(108,91,str);

      GLCD.setDrawColor(row[1] != 5);
      sprintf_P(str,PSTR("%S"),F("R507"));
      GLCD.drawStr(108,104,str);

      GLCD.setDrawColor(row[2] != 0);
      sprintf_P(str,PSTR("%S"),F("EXV"));
      GLCD.drawStr(162,39,str);

      GLCD.setDrawColor(row[2] != 1);
      sprintf_P(str,PSTR("%S"),F("Cap.Tube"));
      GLCD.drawStr(162,52,str);
      
      GLCD.sendBuffer();
  }
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void thresholds(){
  Time shamsi;
  char line = 0;
  char row = 0;
  while(1){
    char *str="xxxxxxxxxxxxxxxxxxx";
      char key = keypad.getKey();
      if(key == BACK) {
        break;
      }
      if( key == OK ) {
        GLCD.setDrawColor(0);
        sprintf_P(str,PSTR("%S"),limits[line]);
        char ww = GLCD.getStrWidth(str);
        sprintf_P(str,PSTR("%S"),F("      "));
        GLCD.drawStr(2+ww ,41+(line - row)*13,str);
        chiller.limitsVar[line] = readnumber(2+ ww,41+(line - row)*13,0,uinits[line]);
        EEPROM.put(tempADDEEP + sizeof(chiller.tempADD) + line*sizeof(int16_t), chiller.limitsVar[line]);
      }
      if( key == 'C' ) {
       line--;
       if(line < 0 ) {
        line = 21;
        row = line - 6;
       }
       if(line < row) row = line;
      }
      if( key == 'D' ) {
        line++;
        if(line > 21) {
          line = 0 ;
          row = 0;
        }
        if(line > (row + 6)) row = line - 6;
      }      
      shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,str);

      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%S"),F("Parameters limit values:"));
      GLCD.drawStr(2,24,str);

      for(int i=0; i<7 ; i++){
        GLCD.setDrawColor(line - row == i);
        GLCD.drawBox(2,i*13+30,236,13);
        GLCD.setDrawColor(line - row != i);
        sprintf_P(str,PSTR("%S%d%S"),limits[i+row],chiller.limitsVar[i+row],uinits[i+row]);
        GLCD.drawStr(2,41+i*13,str);
      }

      GLCD.setDrawColor(1);
      GLCD.drawFrame(0,28,240,95); 
      /*GLCD.drawTriangle(237,24 , 235, 27 , 239 , 27);
      GLCD.drawTriangle(237,127 , 235, 124 , 239 , 124);
      GLCD.drawRBox(237,25 , 5, 7 , 2);  */  
      GLCD.sendBuffer();
  }
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void factoryReset(){
   Time shamsi;
  uint8_t line=1;
  while(1){
    char *str="xxxxxxxxxxxxxxxxxxx";
      char key = keypad.getKey();
      if(key == BACK) {
        break;
      }
      if( key == OK ) {
        if(line== 1) {
          chiller.resetDefaults();
          GLCD.clearBuffer();
          GLCD.setDrawColor(1);
          sprintf_P(str,PSTR("%S"),F("Values reset to defaults."));
          GLCD.drawStr(2,11,str);
          GLCD.sendBuffer();
          delay(2500);
        }
        break;
      }
      if( key == 'C' ) line--;
      if( key == 'D' ) line++;
      if( line > 2 ) line = 1;
      if( line < 1 ) line = 2;
      shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,str);

      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%S"),F("Reset to defaults values?"));
      GLCD.drawStr(2,24,str);

      GLCD.drawBox(2,(line-1)*13+30,236,13);
      
      GLCD.setDrawColor(line != 1);
      sprintf_P(str,PSTR("%S"),F("1.Yes"));
      GLCD.drawStr(2,41,str);

      GLCD.setDrawColor(line != 2);
      sprintf_P(str,PSTR("%S"),F("2.No"));
      GLCD.drawStr(2,54,str);

      GLCD.setDrawColor(1);
      GLCD.drawFrame(0,28,240,95);
      
      GLCD.sendBuffer();
  }
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void setDateTime(){
   Time shamsi;
   uint8_t code=0;
  while(1){
    char *str="xxxxxxxxxxxxxxxxxxx";
      char key = keypad.getKey();
      if(key == BACK) {
        break;
      }
      if( key == OK ) {
        code++;
        if(code == 7) code =0;
      }
      if( key == 'C' ) {
        Time milady;
        switch(code){
          case 0:
          shamsi.year++;
          milady = ShamsiToMiladi(shamsi);
          rtc.setDate(milady.date, milady.mon, milady.year);
          break;
          case 1:
          shamsi.mon++;
          if(shamsi.mon > 12 ) shamsi.mon = 1;
          milady = ShamsiToMiladi(shamsi);
          rtc.setDate(milady.date, milady.mon, milady.year);
          break;
          case 2:
          shamsi.date++;
          if(shamsi.date > 31 ) shamsi.date = 1;
          milady = ShamsiToMiladi(shamsi);
          rtc.setDate(milady.date, milady.mon, milady.year);
          break;
          case 3:
          shamsi.dow++;
          if(shamsi.dow > 7 ) shamsi.dow = 1;
          rtc.setDOW(shamsi.dow);
          break;
          case 4:
          shamsi.hour++;
          if(shamsi.hour > 23 ) shamsi.hour = 0;
          rtc.setTime(shamsi.hour, shamsi.min,shamsi.sec);
          break;
          case 5:
          shamsi.min++;
          if(shamsi.min > 59 ) shamsi.min = 0;
          rtc.setTime(shamsi.hour, shamsi.min,shamsi.sec);
          break;
          case 6:
          shamsi.sec++;
          if(shamsi.sec > 59 ) shamsi.sec = 0;
          rtc.setTime(shamsi.hour, shamsi.min,shamsi.sec);
          break;
        }
      }
      if( key == 'D' ) {
        Time milady;
        switch(code){
          case 0:
          shamsi.year--;
          milady = ShamsiToMiladi(shamsi);
          rtc.setDate(milady.date, milady.mon, milady.year);
          break;
          case 1:
          shamsi.mon--;
          if(shamsi.mon < 1 ) shamsi.mon = 12;
          milady = ShamsiToMiladi(shamsi);
          rtc.setDate(milady.date, milady.mon, milady.year);
          break;
          case 2:
          shamsi.date--;
          if(shamsi.date < 1 ) shamsi.date = 31;
          milady = ShamsiToMiladi(shamsi);
          rtc.setDate(milady.date, milady.mon, milady.year);
          break;
          case 3:
          shamsi.dow--;
          if(shamsi.dow < 1 ) shamsi.dow = 7;
          rtc.setDOW(shamsi.dow);
          break;
          case 4:
          shamsi.hour--;
          if(shamsi.hour > 23 ) shamsi.hour = 23;
          rtc.setTime(shamsi.hour, shamsi.min,shamsi.sec);
          break;
          case 5:
          shamsi.min--;
          if(shamsi.min > 59 ) shamsi.min = 59;
          rtc.setTime(shamsi.hour, shamsi.min,shamsi.sec);
          break;
          case 6:
          shamsi.sec--;
          if(shamsi.sec > 59 ) shamsi.sec = 59;
          rtc.setTime(shamsi.hour, shamsi.min,shamsi.sec);
          break;
        }
      }      
      shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,str);

      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%S"),F("Date and time settings:"));
      GLCD.drawStr(2,24,str);

      GLCD.setDrawColor(code!=0);
      sprintf_P(str,PSTR("%04d"),shamsi.year);
      GLCD.drawStr(2,37,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("/"));
      GLCD.drawStr(34,37,str);

      GLCD.setDrawColor(code!=1);
      sprintf_P(str,PSTR("%02d"),shamsi.mon);
      GLCD.drawStr(42,37,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("/"));
      GLCD.drawStr(58,37,str);

      GLCD.setDrawColor(code!=2);
      sprintf_P(str,PSTR("%02d"),shamsi.date);
      GLCD.drawStr(66,37,str);

      GLCD.setDrawColor(code!=3);
      sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(2,50,str);

      GLCD.setDrawColor(code!=4);
      sprintf_P(str,PSTR("%02d"),shamsi.hour);
      GLCD.drawStr(2,63,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR(":"));
      GLCD.drawStr(18,63,str);

      GLCD.setDrawColor(code!=5);
      sprintf_P(str,PSTR("%02d"),shamsi.min);
      GLCD.drawStr(26,63,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR(":"));
      GLCD.drawStr(42,63,str);

      GLCD.setDrawColor(code!=6);
      sprintf_P(str,PSTR("%02d"),shamsi.sec);
      GLCD.drawStr(50,63,str);
      
      
      GLCD.setDrawColor(1);
      GLCD.drawFrame(0,25,240,93);
      GLCD.sendBuffer();
  }
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void setPhoneNumber(){
    Time shamsi;
    char len=0;
    uint32_t num = 0;
    keypad.addEventListener(cancelHoldEvent);
    holdState &= ~(1<<CANCEL_bit);
  while(1){
    char *str="xxxxxxxxxxxxxxxxxxx";
      char key = keypad.getKey();
      if( key >= '0' && key <= '9'){
        num *= 10 ;
        num += key - '0'; 
        len++;
      }
      else if(key == BACK){
        num /= 10;
        len--;
        if(len < 0 ) len = 0;
      }
      else if( key == OK ) {
        if(len == 9){
          chiller.setPhoneNum(num);
          GLCD.setDrawColor(1);
          sprintf_P(str,PSTR("%S"),F("Phone number added successfully."));
          GLCD.drawStr(2,50,str);
          delay(1500);
          break;
        }
      }
      if(holdStateBIT(CANCEL_bit)){
        holdState &= ~(1<<CANCEL_bit);
        break;
      }
      shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.clearBuffer();
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(str,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,str);
      
      //sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,rtc.getDOWStr());

      sprintf_P(str,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%S"),F("Phone number:09"));
      GLCD.drawStr(2,24,str);

      GLCD.setDrawColor(1);
      sprintf_P(str,PSTR("%09lu"),chiller.phone);
      GLCD.drawStr(122,24,str);

      sprintf_P(str,PSTR("%S"),F("New phone number:09"));
      GLCD.drawStr(2,37,str);

      GLCD.setDrawColor(1);
      //sprintf_P(str,PSTR("   "),num);
      switch(len){
        case 0:
        sprintf_P(str,PSTR("---------"),num);
        break;
        case 1:
        sprintf_P(str,PSTR("%01lu--------"),num);
        break;
        case 2:
        sprintf_P(str,PSTR("%02lu-------"),num);
        break;
        case 3:
        sprintf_P(str,PSTR("%03lu------"),num);
        break;
        case 4:
        sprintf_P(str,PSTR("%04lu-----"),num);
        break;
        case 5:
        sprintf_P(str,PSTR("%05lu----"),num);
        break;
        case 6:
        sprintf_P(str,PSTR("%06lu---"),num);
        break;
        case 7:
        sprintf_P(str,PSTR("%07lu--"),num);
        break;
        case 8:
        sprintf_P(str,PSTR("%08lu-"),num);
        break;
        case 9:
        sprintf_P(str,PSTR("%09lu"),num);
      }
      GLCD.drawStr(154,37,str);
      
      GLCD.drawFrame(0,25,240,93); 
      
      GLCD.sendBuffer();
  } 
  keypad.deleteEventListener();
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
float readnumber(uint8_t x , uint8_t y, uint8_t drawColor,const PROGMEM char  *unit,uint8_t unitmode){
  Time shamsi;
  float num = 0;
  uint8_t i,len,f,u,strW;
  uint32_t msec = millis();
  strW = u=f=len=0;
  char strnum[10];
  keypad.addEventListener(cancelHoldEvent);
  while (1) {
    //wdt_reset();
   char key = keypad.getKey();  
   if ( key ){
    GLCD.setDrawColor(drawColor);
    if (key>='0' && key<= '9') {
      //wdt_reset();
       if(f) len++;
       dtostrf(num, 1, len, strnum);
       num *= (10/pow(10,f));
       num +=(float( key - '0')/pow(10,len));

       if(num) {
        dtostrf(num, 1, len, strnum);
       GLCD.drawStr(x ,y ,strnum);
       strW = GLCD.getStrWidth(strnum);
       if(f==1 && len ==0){
        GLCD.drawGlyph(x + strW ,y ,'.');
        strW += 8;
       }  
       }
       if(unitmode){
       sprintf_P(strnum,PSTR("%S"),unit);
       GLCD.drawStr(x + strW + 8,y ,strnum);
       GLCD.sendBuffer();
       }
    } else if (key == '*') {
      if(f==0){
        f=1;
        dtostrf(num, 1, len, strnum);
        GLCD.drawStr(x ,y ,strnum);
        strW = GLCD.getStrWidth(strnum);
        if(f==1 && len ==0){
        GLCD.drawGlyph(x + strW,y ,'.');
        strW += 8;
       }
       if(unitmode){
       sprintf_P(strnum,PSTR("%S"),unit);
       GLCD.drawStr(x + strW + 8 ,y ,strnum);
       }
      }
    }    
    else if (key == BACK) {
      //wdt_reset();
      dtostrf(num, 1, len, strnum);
      
      if(f){
        if(len){
        num *= pow(10,len);
        num = int(num/10);
        len--;
        num /= pow(10,len);
        }
        else f=0;
      }
      else num = int(num/10);
      if(unitmode) {
        sprintf_P(strnum,PSTR("%S"),unit);
        for(int i=0 ; i < (GLCD.getStrWidth(strnum) / 8)+1; i++ ){
        GLCD.drawGlyph(x + strW + 8*i,y ,' ');
      }
      }
       dtostrf(num, 1, len, strnum);
       GLCD.drawStr(x ,y ,strnum);
       strW = GLCD.getStrWidth(strnum);
       
       if(f==1 && len ==0){
        GLCD.drawGlyph(x + strW,y ,'.');
        strW += 8;
       }
       if(unitmode) {
       sprintf_P(strnum,PSTR("%S"),unit);
       GLCD.drawStr(x + strW + 8,y ,strnum);     
       }
       GLCD.sendBuffer();
    } 
    else if (key == OK) {
      //wdt_reset();
      keypad.deleteEventListener();
       return num;
    } 
   }
    else if(holdStateBIT(CANCEL_bit)){
        holdState &= ~(1<<CANCEL_bit);
        keypad.deleteEventListener();
        return CANCEL;
      }
     else if( (millis() - msec) > 300){
      GLCD.setDrawColor(drawColor);
      if(u) GLCD.drawGlyph(x + strW  ,y ,'_');
      else GLCD.drawGlyph(x + strW  ,y ,' ');
      GLCD.sendBuffer();
      msec=millis();
      u = ~u;
     }
     shamsi = MiladiToShamsi(rtc.getTime());
      if(shamsi.mon == 6 && shamsi.date == 31 && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59 ) rtc.setTime(22, 59, 59); 
      if(shamsi.mon == 12 && shamsi.date == 29  && shamsi.hour == 23 && shamsi.min == 59 && shamsi.sec == 59) rtc.setTime(0, 59, 59);
      GLCD.setFont( u8g2_font_crox1c_mf);
      GLCD.setDrawColor(1);
      GLCD.drawBox(0,0,240,13);
      GLCD.setDrawColor(0);
      sprintf_P(strnum,PSTR("%04d/%02d/%02d"),shamsi.year,shamsi.mon,shamsi.date);
      GLCD.drawStr(2,11,strnum);
      
      //sprintf_P(str,PSTR("%s"),rtc.getDOWStr());
      GLCD.drawStr(96,11,rtc.getDOWStr());

      sprintf_P(strnum,PSTR("%02d:%02d:%02d"),shamsi.hour,shamsi.min,shamsi.sec);
      GLCD.drawStr(170,11,strnum);
  }
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void recordLog(){
  File loger;
  char str[13];
  char str2[6];
  sprintf_P(str,PSTR("%04d%02d%02d.csv"),lastTime.year,lastTime.mon,lastTime.date);
  if( !SD.exists(str) ){
  loger = SD.open(str, FILE_WRITE);
  loger.print(F("Milady date,"));
  loger.print(F("Shamsi date,"));
  loger.print(F("Day of the week,"));
  loger.print(F("Time,"));
  loger.print(F("System fluide,"));
  loger.print(F("System Gas,"));
  loger.print(F("System type,"));
  loger.print(F("Tdc,"));
  loger.print(F("Tsu,"));
  loger.print(F("Toil,"));
  loger.print(F("TLi,"));
  loger.print(F("TLo,"));
  loger.print(F("Pc,"));
  loger.print(F("Pe,"));
  loger.print(F("Current,"));
  loger.print(F("Tsh,"));
  loger.print(F("Tsc,"));
  loger.print(F("status,"));
  loger.print(F("Faults"));
  loger.print(F("\n"));
  loger.close();
  }
  loger = SD.open(str, FILE_WRITE);
  if(loger){
  loger.print(rtc.getDateStr());
  loger.print(F(","));
  loger.print(lastTime.year);
  loger.print(F("/"));
  loger.print(lastTime.mon);
  loger.print(F("/"));
  loger.print(lastTime.date);
  loger.print(F(","));
              
  loger.print(rtc.getDOWStr());
  loger.print(F(","));

  loger.print(rtc.getTimeStr());
  loger.print(F(","));

  switch(chiller.getFluid()){
      case WATER_COOLED: 
                loger.print(F("Water Cooled,"));
                break;
      case AIR_COOLED: 
                loger.print(F("Air Cooled,"));
                break;      
    }
    
    
        switch(chiller.getGas()){
      case R22: 
                loger.print(F("R22,"));
                break;
      case R123: 
                loger.print(F("R123,"));
                break;
      case R134A: 
                loger.print(F("R134A2,"));
                break;
      case R404A: 
                loger.print(F("R404A,"));
                break;
      case R410A: 
                loger.print(F("R410A,"));
                break;
      case R507: 
                loger.print(F("R507,"));
                break;
      
    }

    switch(chiller.getType()){
      case EXPANSION_VALVE: 
                loger.print(F("EXV,"));
                break;
      case CAPILLARY_TUBE: 
                loger.print(F("Cap.Tube,"));
                break;      
    } 
  
  if( chiller.tempSenPre(Tdc_sen) ) dtostrf(chiller.Tdc, 6, 2, str2);
  else sprintf_P(str2,PSTR("%S"),F("--"));
  sprintf_P(str,PSTR("%s\xb0%S"), str2,F("C,"));
  loger.print(str);

  if( chiller.tempSenPre(Tsuc_sen) ) dtostrf(chiller.Tsuc, 6, 2, str2);
  else sprintf_P(str2,PSTR("%S"),F("--"));
  sprintf_P(str,PSTR("%s\xb0%S"), str2,F("C,"));
  loger.print(str);

  if( chiller.tempSenPre(Toil_sen) ) dtostrf(chiller.Toil, 6, 2, str2);
  else sprintf_P(str2,PSTR("%S"),F("--"));
  sprintf_P(str,PSTR("%s\xb0%S"), str2,F("C,"));
  loger.print(str);

  if( chiller.tempSenPre(TLi_sen) ) dtostrf(chiller.TLi, 6, 2, str2);
  else sprintf_P(str2,PSTR("%S"),F("--"));
  sprintf_P(str,PSTR("%s\xb0%S"), str2,F("C,"));
  loger.print(str);

  if( chiller.tempSenPre(TLo_sen) ) dtostrf(chiller.TLo, 6, 2, str2);
  else sprintf_P(str2,PSTR("%S"),F("--"));
  sprintf_P(str,PSTR("%s\xb0%S"), str2,F("C,"));
  loger.print(str);

  if( chiller.Pc >= 0 ) dtostrf(chiller.Pc, 6, 2, str2);
  else sprintf_P(str2,PSTR("%S"),F("--"));
  sprintf_P(str,PSTR("%s%S"), str2,F("Psi,"));
  loger.print(str);

  if( chiller.Pe >= 0  ) dtostrf(chiller.Pe, 6, 2, str2);
  else sprintf_P(str2,PSTR("%S"),F("--"));
  sprintf_P(str,PSTR("%s%S"), str2,F("Psi,"));
  loger.print(str);

  dtostrf(chiller.Icomp, 6, 2, str2);
  sprintf_P(str,PSTR("%s%S"), str2,F("A,"));
  loger.print(str);

  if( chiller.tempSenPre(Tsuc_sen) &&  chiller.Pe >= 0 ) dtostrf(chiller.Tsh, 6, 2, str2);
  else sprintf_P(str2,PSTR("%S"),F("--"));
  sprintf_P(str,PSTR("%s\xb0%S"), str2,F("C,"));
  loger.print(str);
  
  if( chiller.tempSenPre(Tdc_sen) &&  chiller.Pc >= 0) dtostrf(chiller.Tsc, 6, 2, str2);
  else sprintf_P(str2,PSTR("%S"),F("--"));
  sprintf_P(str,PSTR("%s\xb0%S"), str2,F("C,"));
  loger.print(str);
 
  for(uint8_t i = ON ; i<= STABLE ; i++){
    if(chiller.checkStatus(i)){
      sprintf_P(str,PSTR("%S & "),ERRORS[i]);
      loger.print(str);
    }
  }
  loger.print(F(","));

  for(uint8_t i = RU ; i<= 7 ; i++){
    if(chiller.checkFault(i)){
      sprintf_P(str,PSTR("%S & "),FAULTS[i]);
      loger.print(str);
    }
  }
  loger.print(F("\n"));
  loger.close();
  }
  
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void sendSMSlog(){
  char number[14];
  char str[6];
  char message[70];
  if(digitalRead(GSM_status) == HIGH){
  GSM_OFF;
  delay(1500);
  GSM_ON;
  }
  if(digitalRead(GSM_status) == LOW){
  delay(1000);
  GSM_OFF;
  delay(1500);
  GSM_ON;
  }
  BareBoneSim800 sim800;
  delay(8000);
  if(sim800.isAttached()){
   Time shamsi = MiladiToShamsi(rtc.getTime());
  sprintf_P(message,PSTR("%04d/%02d/%02d,%02d:%02d:%02d,"),shamsi.year,shamsi.mon,shamsi.date,shamsi.hour,shamsi.min,shamsi.sec);

  sprintf_P(number,PSTR("+989%09lu"),chiller.phone);
  for(uint8_t i = RU ; i<= 7 ; i++){
    if(chiller.checkFault(i)){
      sprintf_P(str,PSTR("%S&"),FAULTS[i]);
      strcat(message,str);
    }
  }
  
  sim800.sendSMS(number, message);
  
  /*if(sim800.checkNewSMS()){
     if(sim800.readSMS(sim800.currentMessageIndex)=="R1ON") relay1_ON;
  }*/
  }
  GSM_OFF;
  delay(1500);
  GSM_ON;
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void I2CClearBus(uint8_t sclPin, uint8_t sdaPin){
  if(digitalRead(sdaPin) == 0) {
    uint32_t thisTime,thatTime;
    uint8_t pin;
    pinMode(sclPin,INPUT_PULLUP);
    pinMode(sclPin,OUTPUT);
    thatTime = millis();
    while( (millis() - thatTime) < 500){
    digitalWrite(sclPin,LOW);
    delayMicroseconds(5);
    digitalWrite(sclPin,HIGH);
    pin = digitalRead(sdaPin);
    thisTime = micros();
    while( (micros - thisTime ) > 5){
      if( (digitalRead(sdaPin) == 1) && (pin == 0) ) {
        pinMode(sclPin,INPUT);
        return 1;
      }
      pin = digitalRead(sdaPin);
    }
    }
  }
  pinMode(sclPin,INPUT);
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void getPresure(uint8_t PressurePin){
  char str[10];
  GLCD.setFont( u8g2_font_crox1c_mf);
  sprintf_P(str,PSTR("%S"),F("=     "));
    if(PressurePin ==1) {
      GLCD.setDrawColor(1);
      GLCD.drawStr(16,50,str);
      chiller.Pc = readnumber(24 , 50, 1,uinits[4]);
    }
    if(PressurePin ==2) {
      GLCD.setDrawColor(0);
      GLCD.drawStr(16,63,str);
      chiller.Pe = readnumber(24 , 63, 0,uinits[4]);
    }
    keypad.addEventListener(keypadEvent);
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void keypadEvent(KeypadEvent key){
    switch (keypad.getState()){
    case PRESSED:
        break;

    case RELEASED:
        break;

    case HOLD:
        if (key == BACK) {
            holdState |= ( 1 << CANCEL_bit );
        }
        else if (key == OK) {
            //holdState |= ( 1 << Menue_bit );
            showSettingWindow();
        }
        else if (key == '1') {
            holdState ^= ( 1 << Pressure1_bit );
            if (holdStateBIT(Pressure1_bit) ){
              chiller.Pc = 0;
              getPresure(1);
            }
        }
        else if (key == '2') {
            holdState ^= ( 1 << Pressure2_bit );
            if ( holdStateBIT(Pressure2_bit) ){
              chiller.Pe = 0;
              getPresure(2);
            }
        }
        break;
    }
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
void cancelHoldEvent(KeypadEvent key){
switch (keypad.getState()){
    case PRESSED:
        break;

    case RELEASED:
        break;

    case HOLD:
        if (key == BACK) {
            holdState |= ( 1 << CANCEL_bit );
        }
        break;
        
    }
}
