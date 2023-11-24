#include <DS3231.h>
#include <avr/pgmspace.h>

static const int AllDoomsy[13] PROGMEM ={-1,3,7,7,4,2,6,4,1,5,3,7,5};
static const int  sumDayMiladiMonth[12] PROGMEM = {0,31,59,90,120,151,181,212,243,273,304,334};
static const int  sumDayMiladiMonthLeap[12] PROGMEM = {0,31,60,91,121,152,182,213,244,274,305,335};
static const int sumshamsi[12] PROGMEM = { 31, 62, 93, 124, 155, 186, 216, 246, 276, 306, 336, 365 };

int MiladiIsLeap(int miladiYear);
int DoomsDay(int year);
int GetDayOfWeek(int y,int m,int d);
Time MiladiToShamsi(Time miladi);
Time ShamsiToMiladi(Time shamsi);
