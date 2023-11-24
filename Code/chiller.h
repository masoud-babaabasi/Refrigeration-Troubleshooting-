#include <arduino.h>
#include <EEPROM.h>
#include <OneWire.h>
#include "default_values.h"
#include "tables.h"
#include "pinMap.h"


//fluid type
#define WATER_COOLED 0
#define AIR_COOLED 1
#define fluidType_ADD 1

//system type
#define EXPANSION_VALVE 0
#define CAPILLARY_TUBE 1
#define systemType_ADD 2

//gas type
#define R22 0
#define R123 1
#define R134A 2
#define R404A 3
#define R410A 4
#define R507 5
#define gasType_ADD 3

//temp sesnors eeprom Adresse
#define tempADDEEP 4

//eep address
#define T_stable_ADD tempADDEEP + sizeof(tempADD) + 18*sizeof(int16_t)
#define T_sample_ADD tempADDEEP + sizeof(tempADD) + 19*sizeof(int16_t)
#define T_fault_ADD tempADDEEP + sizeof(tempADD) + 20*sizeof(int16_t)
#define T_critical_ADD tempADDEEP + sizeof(tempADD) + 21*sizeof(int16_t)
#define EA_ADD tempADDEEP + sizeof(tempADD) + 16*sizeof(int16_t)
#define EP_ADD tempADDEEP + sizeof(tempADD) + 17*sizeof(int16_t)
#define X1_ADD tempADDEEP + sizeof(tempADD) + 0*sizeof(int16_t)
#define X2_ADD tempADDEEP + sizeof(tempADD) + 1*sizeof(int16_t)
#define X3_ADD tempADDEEP + sizeof(tempADD) + 2*sizeof(int16_t)
#define X4_ADD tempADDEEP + sizeof(tempADD) + 3*sizeof(int16_t)
#define X5_ADD tempADDEEP + sizeof(tempADD) + 4*sizeof(int16_t)
#define X6_ADD tempADDEEP + sizeof(tempADD) + 5*sizeof(int16_t)
#define X7_ADD tempADDEEP + sizeof(tempADD) + 6*sizeof(int16_t)
#define X8_ADD tempADDEEP + sizeof(tempADD) + 7*sizeof(int16_t)
#define X9_ADD tempADDEEP + sizeof(tempADD) + 8*sizeof(int16_t)
#define X10_ADD tempADDEEP + sizeof(tempADD) + 9*sizeof(int16_t)
#define X11_ADD tempADDEEP + sizeof(tempADD) + 10*sizeof(int16_t)
#define X12_ADD tempADDEEP + sizeof(tempADD) + 11*sizeof(int16_t)
#define X13_ADD tempADDEEP + sizeof(tempADD) + 12*sizeof(int16_t)
#define X14_ADD tempADDEEP + sizeof(tempADD) + 13*sizeof(int16_t)
#define X15_ADD tempADDEEP + sizeof(tempADD) + 14*sizeof(int16_t)
#define X16_ADD tempADDEEP + sizeof(tempADD) + 15*sizeof(int16_t)
#define phoneADD tempADDEEP + sizeof(tempADD) + 22*sizeof(int16_t)

//limit variables
#define EA limitsVar[16]
#define EP limitsVar[17]
#define T_stable limitsVar[18]
#define T_sample limitsVar[19]
#define T_fault limitsVar[20]
#define T_critical limitsVar[21]
#define X1 limitsVar[0]
#define X2 limitsVar[1]
#define X3 limitsVar[2]
#define X4 limitsVar[3]
#define X5 limitsVar[4]
#define X6 limitsVar[5]
#define X7 limitsVar[6]
#define X8 limitsVar[7]
#define X9 limitsVar[8]
#define X10 limitsVar[9]
#define X11 limitsVar[10]
#define X12 limitsVar[11]
#define X13 limitsVar[12]
#define X14 limitsVar[13]
#define X15 limitsVar[14]
#define X16 limitsVar[15]

//sattus register bits
#define ON 0
#define HPc 1
#define LPc 2
#define HPe 3
#define LPe 4
#define HTDC 5
#define RLL 6 
#define LToil 7 
#define HTsh 8
#define LTsh 9
#define HTsc 10
#define LTsc 11
#define STABLE 12

const char ERRORS[13][6] PROGMEM = {
	"ON"
	,"HPc"
	,"LPc"
	,"HPe"
	,"LPe"
	,"HTDC"
	,"RLL" 
	,"LToil"
	,"HTsh"
	,"LTsh"
	,"HTsc"
	,"LTsc"
	,"STABLE"
};

// faults register bits
#define RU 0
#define RO 1
#define DC 2
#define REF 3
#define RDF 4
#define CF 5
const char FAULTS[8][5] PROGMEM = {
	"RU"
	,"RO"
	,"DC"
	,"REF"
	,"RDF"
	,"CF"
	,"HTDC"
	,"LTsh"
};

//temps sensors name
#define Tdc_sen 0
#define Tsuc_sen 1
#define Toil_sen 2
#define TLi_sen 3
#define TLo_sen 4


#define Tdc	Temps[Tdc_sen]
#define Tsuc 	Temps[Tsuc_sen]
#define Toil	Temps[Toil_sen]
#define TLi	Temps[TLi_sen]
#define TLo	Temps[TLo_sen]

#define Pc Presures[0]
#define Pe Presures[1]

class Chiller{
	public:
	Chiller(uint8_t Tpin);
	void setType(uint8_t t);
	void setFluid(uint8_t f);
	void setGas(uint8_t g);
	uint8_t getType();
	uint8_t getFluid();
	uint8_t getGas();
	uint8_t checkStatus(uint8_t i);
	void resetDefaults();
	void getParamEEP();
	void refreshStatus();
	void diagnose();
	uint8_t checkFault(uint8_t i);
	void superheat();
	void subcold();
	float presure(uint8_t presurePin);
	void temprature();
	uint8_t tempSenPre(uint8_t tempSen);
	uint8_t addTempSen(uint8_t tempSen);
	void deleteTempSen(uint8_t tempSen);
	void current();
	void setPhoneNum(uint32_t number);
	
	OneWire  temp;
	
	/*int16_t EA,EP;
	int16_t T_stable;
	uint8_t T_sample;
	uint8_t T_fault;
	int16_t X1,X2,X3,X4,X5,X6,X7,X8;
	int16_t X9,X10,X11,X12,X13,X14,X15,X16;*/
	int16_t limitsVar[22];
	float Temps[5];
	float Presures[2];
	float Icomp,Tsh,Tsc;
	
	uint8_t faults;
	uint16_t status;
	uint32_t phone;
	uint32_t stCapture;
	uint8_t tempADD[5][8];
	
	private:
	void setStatus(uint8_t i,char set);
	
	uint8_t temp_present;
	uint8_t fluid;
	uint8_t systemType;
	uint8_t gas;
	
	
	
};
