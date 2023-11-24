#include "calender_converter.h"
int MiladiIsLeap(int miladiYear)
{
if(((miladiYear % 100)!= 0 && (miladiYear % 4) == 0) || ((miladiYear % 100)== 0 && (miladiYear % 400) == 0))
  return 1;
else
  return 0;
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
int DoomsDay(int year)
{
  int a=0;
  if(year>=1800 && year<=1899) a=5;
  else if (year>=1900 && year<=1999) a=3;
  else if (year>=2000 && year<=2099) a=2;
  else if (year>=2100 && year<=2199) a=0;
  else return -1;
  int  y=year %100;
  int z= (y+floor(y/4)+a);
  z=z % 7;
  return (z);
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
int GetDayOfWeek(int y,int m,int d)
{
  int doom;
  doom=DoomsDay(y);
  int AllDoomsy[13] PROGMEM ={-1,3,7,7,4,2,6,4,1,5,3,7,5};
  int c=pgm_read_word(&AllDoomsy[m]);
  if((m==1 || m==2) && MiladiIsLeap(y)==1) c++;
  int r=d-c+doom;
  if(r<0) r=r+7;
  return (r%7);
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
Time MiladiToShamsi(Time miladi)
{
  Time shamsi = miladi;
   int  dayCount,deyDayDiff ;
	uint8_t farvardinDayDiff =79;

if (MiladiIsLeap(miladi.year))
 {
    dayCount = pgm_read_word(&sumDayMiladiMonthLeap[miladi.mon-1]) + miladi.date;
 }
else
 {
    dayCount = pgm_read_word(&sumDayMiladiMonth[miladi.mon-1]) + miladi.date;
 }
if((MiladiIsLeap(miladi.year - 1)))
 {
    deyDayDiff = 11;
 }
else
 {
    deyDayDiff = 10;
 }
if (dayCount > farvardinDayDiff)
{
    dayCount = dayCount - farvardinDayDiff;
    if (dayCount <= 186)
     {
      switch (dayCount%31)
       {
    case 0:
     shamsi.mon = dayCount / 31;
     shamsi.date = 31;
    break;
    default:
     shamsi.mon = (dayCount / 31) + 1;
     shamsi.date = (dayCount%31);
    break;
      }
      shamsi.year = miladi.year - 621;
     }
   else
     {
    dayCount = dayCount - 186;
    switch (dayCount%30)
      {
       case 0:
    shamsi.mon = (dayCount / 30) + 6;
    shamsi.date = 30;
       break;
     default:
       shamsi.mon = (dayCount / 30) + 7;
       shamsi.date = (dayCount%30);
       break;
     }
      shamsi.year = miladi.year - 621;
    }
  }
else
  {
    dayCount = dayCount + deyDayDiff;

    switch (dayCount%30)
    {
    case 0 :
      shamsi.mon = (dayCount / 30) + 9;
      shamsi.date = 30;
     break;
    default:
      shamsi.mon = (dayCount / 30) + 10;
      shamsi.date = (dayCount%30);
     break;
    }
    shamsi.year = miladi.year - 622;

  }
  return shamsi;
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/
Time ShamsiToMiladi(Time shamsi)
{
  int miladidays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        int yy, mm, dd,daycount;
        Time miladi;
        daycount = shamsi.date;
        if (shamsi.mon > 1) daycount =daycount+ pgm_read_word(&sumshamsi[shamsi.mon - 2]);
        yy = shamsi.year + 621;
        daycount = daycount + 79;
        if (MiladiIsLeap(yy))
        {
        if (daycount > 366)
        {
            daycount -= 366;
            yy++;
        }

        }
        else if (daycount > 365)
        {
        daycount -= 365;
        yy++;
        }
        if (MiladiIsLeap(yy)) miladidays[1] = 29;
        mm=0;
        while (daycount > miladidays[mm])
        {
        daycount = daycount - miladidays[mm];
        mm++;
        }
    
        miladi.year= yy;
        miladi.mon=(mm + 1);
        miladi.date=daycount;
        return miladi;
}
