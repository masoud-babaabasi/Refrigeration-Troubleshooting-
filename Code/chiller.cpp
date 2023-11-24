#include "chiller.h"

Chiller::Chiller(uint8_t Tpin):temp(Tpin){
	//resetDefaults();
	getParamEEP();
	systemType = EEPROM.read(systemType_ADD);
	fluid = EEPROM.read(fluidType_ADD);
	gas = EEPROM.read(gasType_ADD);
	EEPROM.get( tempADDEEP, tempADD );
	EEPROM.get(phoneADD, phone);
}

void Chiller::getParamEEP(){
  if(EEPROM.read(EEPROM.end()) != 0xaa){
    EEPROM.write(EEPROM.end() ,0xaa);
    resetDefaults();
  }
  else{
	EEPROM.get( T_stable_ADD, T_stable );
	EEPROM.get( T_sample_ADD, T_sample );
	EEPROM.get( T_fault_ADD, T_fault );
	EEPROM.get( T_critical_ADD, T_critical );
	EEPROM.get( EA_ADD, EA );
	EEPROM.get( EP_ADD, EP );
	EEPROM.get( X1_ADD , X1 );
	EEPROM.get( X2_ADD , X2 );
	EEPROM.get( X3_ADD , X3 );
	EEPROM.get( X4_ADD , X4 );
	EEPROM.get( X5_ADD , X5 );
	EEPROM.get( X6_ADD , X6 );
	EEPROM.get( X7_ADD , X7 );
	EEPROM.get( X8_ADD , X8 );
	EEPROM.get( X9_ADD , X9 );
	EEPROM.get( X10_ADD, X10 );
	EEPROM.get( X11_ADD, X11 );
	EEPROM.get( X12_ADD, X12 );
	EEPROM.get( X13_ADD, X13 );
	EEPROM.get( X14_ADD, X14 );
	EEPROM.get( X15_ADD, X15 );
	EEPROM.get( X16_ADD, X16 );
  }
}

void Chiller::setPhoneNum(uint32_t number){
	phone = number;
	EEPROM.put(phoneADD, number);
}

void Chiller::setType(uint8_t t){
	if( t == EXPANSION_VALVE || t == CAPILLARY_TUBE ){
		systemType = t;
		EEPROM.write(systemType_ADD, t);
	}
}
uint8_t Chiller::getType(){
	return systemType;
}

void Chiller::setFluid(uint8_t f){
	if( f == WATER_COOLED || f == AIR_COOLED ){
		fluid = f;
		EEPROM.write(fluidType_ADD, f);
	}
}

uint8_t Chiller::getFluid(){
	return fluid;
}


void Chiller::setGas(uint8_t g){
	if( g == R22 || g == R123 || g == R134A || g == R404A || g == R410A || g == R507 ) {
		gas = g;
		EEPROM.write(gasType_ADD, g);
	}
}

uint8_t Chiller::getGas(){
	return gas;
}

void Chiller::setStatus(uint8_t i,char set){ //set or reset the i-th bit of status reg
	if( set == 'S' || set == 's') status = status | ( uint32_t(1) << i );
	else if( set == 'R' || set == 'r') status = status & ~( uint32_t(1) << i );
}
uint8_t Chiller::checkStatus(uint8_t i){
	return (status & ( uint32_t(1) << i )) >> i ;
}

uint8_t Chiller::checkFault(uint8_t i){
	return (faults & ( 1 << i )) >> i ;
}

uint8_t Chiller::tempSenPre(uint8_t tempSen){
	return (temp_present & ( 1 << tempSen )) >> tempSen ;
}

uint8_t Chiller::addTempSen(uint8_t tempSen){
	uint8_t deviceAddress[8];
	int i,j;
	uint8_t eq;
	eq = 0x1f;
	temp.reset_search();
	while (temp.search(deviceAddress)) {

		if (temp.crc8(deviceAddress, 7) == deviceAddress[7]) {
			if (deviceAddress[0] == 0x28) {
				for(j = 0 ; j < 5 ; j++){
					for(i = 0 ; i < 8 ; i++){
					if(tempADD[j][i] != deviceAddress[i]) {
						eq &= ~(1<<j) ;
						break;
						}
					}
					if( (eq & ( 1 << j )) >> j == 1 ) return j;//returns the number of sensor that already exits
				}
				if(eq==0){
					for(i = 0 ; i < 8 ; i++) tempADD[tempSen][i] = deviceAddress[i];
					EEPROM.put( tempADDEEP, tempADD );
					return 0x28;//sensor saved succesfully
				}
			}
		}
	}
	return 0xff;//no sensor found
}

void Chiller::deleteTempSen(uint8_t tempSen){
	int i;
	for(i=0;i<8;i++) {
		tempADD[tempSen][i] = 0;
		EEPROM.write(tempADDEEP+ ( tempSen * 8 ) + i , 0);
	}
}

void Chiller::resetDefaults(){
	T_stable = T_stable_def;
    EEPROM.put(T_stable_ADD, T_stable);
	T_sample = T_sample_def;
    EEPROM.put(T_sample_ADD, T_sample);
	T_fault = T_fault_def;
    EEPROM.put(T_fault_ADD, T_fault);
	T_critical = T_critical_def;
    EEPROM.put(T_critical_ADD, T_critical);
	EA = EA_def;  
    EEPROM.put(EA_ADD, EA);
    EP = EP_def;  
    EEPROM.put(EP_ADD, EP);
    X1 = X1_def; 
    EEPROM.put(X1_ADD, X1);
    X2 = X2_def;    
    EEPROM.put(X2_ADD, X2);
    X3 = X3_def; 
    EEPROM.put(X3_ADD, X3);
    X4 = X4_def;     
    EEPROM.put(X4_ADD, X4);
    X5 = X5_def; 
    EEPROM.put(X5_ADD, X5);
    X6 = X6_def;    
    EEPROM.put(X6_ADD, X6);
    X7 = X7_def; 
    EEPROM.put(X7_ADD, X7);
    X8 = X8_def;     
    EEPROM.put(X8_ADD, X8);
    X9 = X9_def; 
    EEPROM.put(X9_ADD, X9);
    X10 = X10_def;    
    EEPROM.put(X10_ADD, X10);
    X11 = X11_def;
    EEPROM.put(X11_ADD, X11);
    X12 = X12_def;    
    EEPROM.put(X12_ADD, X12);
    X13 = X13_def;
    EEPROM.put(X13_ADD, X13);
    X14 = X14_def;    
    EEPROM.put(X14_ADD, X14);
    X15 = X15_def;
    EEPROM.put(X15_ADD, X15);
    X16 = X16_def;
    EEPROM.put(X16_ADD, X16);
}

void Chiller::refreshStatus(){
	if( (Icomp >= EA  || (abs( Pc - Pe ) >= EP && Pc >= 0&& Pe >= 0)) && !checkStatus(ON)) {
		setStatus(ON,'S');
		stCapture = millis();
	}
	else if(!(Icomp >= EA  || (abs( Pc - Pe ) >= EP && Pc >= 0&& Pe >= 0)))setStatus(ON,'R');
	
	if( ((millis() - stCapture )/ 1000) >= T_stable && checkStatus(ON)) setStatus(STABLE,'S');
	else setStatus(STABLE,'R');
	
	if(fluid == WATER_COOLED ){
		if( checkStatus(ON) && Pc >= X5 && checkStatus(STABLE) ) setStatus(HPc,'S');
		else setStatus(HPc,'R');
		
		if( checkStatus(ON) && Pc <= X7  && Pc >= 0 && checkStatus(STABLE) ) setStatus(LPc,'S');
		else setStatus(LPc,'R');
		
		if( checkStatus(ON) && Pe >= X13 && checkStatus(STABLE) ) setStatus(HPe,'S');
		else setStatus(HPe,'R');
		
		if( checkStatus(ON) && Pe <= X15 && Pe >= 0 && checkStatus(STABLE) ) setStatus(LPe,'S');
		else setStatus(LPe,'R');
		
		if( Tdc >= X3 && checkStatus(STABLE) ) setStatus(HTDC,'S');
		else setStatus(HTDC,'R');

	}	
	else if(fluid == AIR_COOLED){
		if( checkStatus(ON) && Pc >= X6 && checkStatus(STABLE)) setStatus(HPc,'S');
		else setStatus(HPc,'R');
		
		if( checkStatus(ON) && Pc <= X8 && Pc >= 0 && checkStatus(STABLE)) setStatus(LPc,'S');
		else setStatus(LPc,'R');
		
		if( checkStatus(ON) && Pe >= X14 && checkStatus(STABLE)) setStatus(HPe,'S');
		else setStatus(HPe,'R');
		
		if( checkStatus(ON) && Pe <= X16 && Pe >= 0 && checkStatus(STABLE)) setStatus(LPe,'S');
		else setStatus(LPe,'R');
		
		if( Tdc >= X4 && checkStatus(STABLE)) setStatus(HTDC,'S');
		else setStatus(HTDC,'R');
	}
	
	if( checkStatus(ON) && abs(TLi - TLo) >= X9 && checkStatus(STABLE) && tempSenPre(TLo_sen) && tempSenPre(TLi_sen) ) setStatus(RLL,'S');
	else setStatus(RLL,'R');
	
	if( checkStatus(ON) && Toil <= X10 && checkStatus(STABLE) && tempSenPre(Toil_sen)) setStatus(LToil,'S');
	else setStatus(LToil,'R');
	
	if( checkStatus(ON) && Tsh >= X1 && checkStatus(STABLE) && tempSenPre(Tsuc_sen) && Pe >= 0) setStatus(HTsh,'S');
	else setStatus(HTsh,'R');
	
	if( checkStatus(ON)&& Tsh <= X2 && checkStatus(STABLE) && tempSenPre(Tsuc_sen) && Pe >= 0) setStatus(LTsh,'S');
	else setStatus(LTsh,'R');
	
	if( checkStatus(ON) && Tsc >= X11 && checkStatus(STABLE) && tempSenPre(TLi_sen) && Pc >= 0) setStatus(HTsc,'S');
	else setStatus(HTsc,'R');
	
	if( checkStatus(ON) && Tsc <= X12 && checkStatus(STABLE) && tempSenPre(TLi_sen) && Pc >= 0) setStatus(LTsc,'S');
	else setStatus(LTsc,'R');
}

void Chiller::diagnose(){
	if(systemType == CAPILLARY_TUBE){
		if(checkStatus(ON) && !checkStatus(HPc) && checkStatus(LPc) && !checkStatus(HPe) && checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && checkStatus(HTsh) && !checkStatus(LTsh) && !checkStatus(HTsc) && checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << RU );
		else faults = faults & ~( 1 << RU );
		
		if(checkStatus(ON) && checkStatus(HPc) && !checkStatus(LPc) && checkStatus(HPe) && !checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && !checkStatus(HTsh) && !checkStatus(LTsh) && checkStatus(HTsc) && !checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << RO );
		else faults = faults & ~( 1 << RO );
		
		if(checkStatus(ON) && checkStatus(HPc) && !checkStatus(LPc) && checkStatus(HPe) && !checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && !checkStatus(HTsh) && checkStatus(LTsh) && !checkStatus(HTsc) && checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << DC );
		else faults = faults & ~( 1 << DC );
		
		if(checkStatus(ON) && !checkStatus(HPc) && !checkStatus(LPc) && !checkStatus(HPe) && !checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && !checkStatus(HTsh) && checkStatus(LTsh) && !checkStatus(HTsc) && checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << REF );
		else faults = faults & ~( 1 << REF );
		
		if(checkStatus(ON) && !checkStatus(HPc) && !checkStatus(LPc) && !checkStatus(HPe) && !checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && checkStatus(HTsh) && !checkStatus(LTsh) && checkStatus(HTsc) && !checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << RDF );
		else faults = faults & ~( 1 << RDF );
		
		if(checkStatus(ON) && !checkStatus(HPc) && checkStatus(LPc) && checkStatus(HPe) && !checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && !checkStatus(HTsh) && !checkStatus(LTsh) && !checkStatus(HTsc) && checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << CF );
		else faults = faults & ~( 1 << CF );
	}
	else if(systemType == EXPANSION_VALVE){
		if(checkStatus(ON) && !checkStatus(HPc) && checkStatus(LPc) && !checkStatus(HPe) && checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && checkStatus(HTsh) && !checkStatus(LTsh) && !checkStatus(HTsc) && checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << RU );
		else faults = faults & ~( 1 << RU );
		
		if(checkStatus(ON) && checkStatus(HPc) && !checkStatus(LPc) && checkStatus(HPe) && !checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && !checkStatus(HTsh) && checkStatus(LTsh) && checkStatus(HTsc) && !checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << RO );
		else faults = faults & ~( 1 << RO );
		
		if(checkStatus(ON) && checkStatus(HPc) && !checkStatus(LPc) && !checkStatus(HPe) && !checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && checkStatus(HTsh) && !checkStatus(LTsh) && checkStatus(HTsc) && !checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << DC );
		else faults = faults & ~( 1 << DC );
		
		if(checkStatus(ON) && !checkStatus(HPc) && checkStatus(LPc) && !checkStatus(HPe) && checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && !checkStatus(HTsh) && checkStatus(LTsh) && !checkStatus(HTsc) && checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << REF );
		else faults = faults & ~( 1 << REF );
		
		if(checkStatus(ON) && !checkStatus(HPc) && !checkStatus(LPc) && !checkStatus(HPe) && !checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && checkStatus(HTsh) && !checkStatus(LTsh) && checkStatus(HTsc) && !checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << RDF );
		else faults = faults & ~( 1 << RDF );
		
		if(checkStatus(ON) && !checkStatus(HPc) && checkStatus(LPc) && checkStatus(HPe) && !checkStatus(LPe) && !checkStatus(HTDC) && !checkStatus(RLL) && !checkStatus(LToil) && !checkStatus(HTsh) && !checkStatus(LTsh) && !checkStatus(HTsc) && checkStatus(LTsc) && checkStatus(STABLE) ) faults = faults | ( 1 << CF );
		else faults = faults & ~( 1 << CF );
	}	
}
void Chiller::superheat(){
	int i;
	float te;
 if(tempSenPre(Tsuc_sen) && Pe >= 0){
  for( i = 0 ; i <= 94 ; i++ ){
    if((Pe - pgm_read_float(&SatCon[gas][i])) >=0 && (Pe - pgm_read_float(&SatCon[gas][i])) <= (pgm_read_float(&SatCon[gas][i+1]) - pgm_read_float(&SatCon[gas][i])) ) 
    {
    te = (Pe - pgm_read_float(&SatCon[gas][i]))/(pgm_read_float(&SatCon[gas][i+1]) - pgm_read_float(&SatCon[gas][i])) + i;
    break;
    }
  }
  te = 2 * te / 1.8 - 40;
  Tsh = Tsuc - te;
 }
}

void Chiller::subcold(){///Tli
	int i;
	float tli;
 if(tempSenPre(TLi_sen) &&  Pc >= 0){
  for( i = 0 ; i <= 94 ; i++ ){
    if((Pc - pgm_read_float(&SatCon[gas][i])) >=0 && (Pc - pgm_read_float(&SatCon[gas][i])) <= (pgm_read_float(&SatCon[gas][i+1]) - pgm_read_float(&SatCon[gas][i])) ) 
    {
    tli = (Pc - pgm_read_float(&SatCon[gas][i]))/(pgm_read_float(&SatCon[gas][i+1]) - pgm_read_float(&SatCon[gas][i])) + i;
    break;
    }
  }
  tli = 2 * tli / 1.8 - 40;
  Tsc = tli - TLi;

 }
}

float Chiller::presure(uint8_t presurePin){
	int adc;
	float Isen;
	adc = analogRead(presurePin);
	// I = adc *vref / 1024 * R3 /(R3+R2) * 1000 / R1 mA
	// p[bar] = 2.5 * I - 10;
	// Vref =5v R1=47 R2=5.1 R3=1.2 
	Isen = adc * 0.0197885 ;
	return (2.5*Isen-10)*14.7;
}

void Chiller::temprature(){
	uint8_t data[9];
	temp.reset();
	temp.write(0xCC);
	temp.write(0x4E); 
	temp.write(0x00);
	temp.write(0x00);
	temp.write(0x5f);//11 bit configuration
	
	temp.reset();
	temp.write(0xCC);//all temp sensors
	temp.write(0x44);// start conversion
	delay(385); //11bit resolotion 375ms delay time
	
	for(int tempNum = Tdc_sen ; tempNum <= TLo_sen ; tempNum++){
    if(tempADD[tempNum][0] != 0x28) {
		temp_present = temp_present & ~( 1 << tempNum );
		continue;
	}
		temp.reset();
		temp.select(tempADD[tempNum]);
		
			temp.write(0xBE);
			for (int  i = 0; i < 9; i++) {           // we need 9 bytes
				data[i] = temp.read();
			  }	
			if(data[7] == 0x10){
				temp_present = temp_present | ( 1 << tempNum );
				int16_t raw = (data[1] << 8) | data[0];
				raw = raw & ~1;// 11 bit res, 375 ms
				Temps[tempNum] = (float)raw / 16.0;
			}
			else temp_present = temp_present & ~( 1 << tempNum );
		}
}

void Chiller::current(){
	int adc;
	adc = analogRead(CT_pin); //20ms maximum calculation
	//I = adc * Vref / 1024 * R1 / R2 * 70;
	// Vref = 5 , R1 = 1.2 , R2 = 5.1
	Icomp = (Icomp + (adc * 0.0804)) / 2 ;
}
