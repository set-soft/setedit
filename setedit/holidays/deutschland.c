/**[txh]********************************************************************

  Copyright (c) 1992-2009 by Salvador E. Tropea
  Covered by the GPL license, see the see copyrigh file for details.

  Description:
  Module to find the holidays for Deutschland (Germany).@*
  
***************************************************************************/

#include <stdlib.h>
#include "datetools.h"

#define FIXED          10
#define ONLY_NATIONAL   0
#define TOTAL          16
#define EASTER_REL      5
static char FixedHolidays[FIXED][2]=
  {{1,1},{6,1},{1,5},{8,8},{15,8},{3,10},{31,10},{1,11},{25,12},{26,12}};
static char FixedNational[FIXED]=
  {    1,    0,    1,    0,     0,     1,     0,      0,      1,     1};
static const char *FixedDescriptions[FIXED]=
{
 "Neujahrstag",
 "Heilige Drei Könige",
 "Tag der Arbeit",
 "Friedensfest",
 "Mariä Himmelfahrt",
 "Tag der Deutschen Einheit",
 "Reformationstag",
 "Allerheiligen",
 "1. Weihnachtstag",
 "2. Weihnachtstag"
};
static char EasterRelHolidays[EASTER_REL]={-2,1,39,50,60};
static char EasterRelNational[EASTER_REL]={ 1,1, 1, 1, 0};
static const char *EasterRelDescriptions[EASTER_REL]=
{
 "Karfreitag",
 "Ostermontag",
 "Christi Himmelfahrt",
 "Pfingstmontag",
 "Fronleichnam"
};

struct dayMonth *GetListOfHolidays(int year, int *holidays)
{
 int j,v,easter_sunday,a;
 struct dayMonth *ret=(struct dayMonth *)malloc(sizeof(struct dayMonth)*TOTAL));
 int i,k;

 /* Fixed dates */
 for (i=0,k=0; i<FIXED; i++)
    {
     if (FixedNational[i] || !ONLY_NATIONAL)
       {
        ret[k].day=FixedHolidays[i][0];
        ret[k].month=FixedHolidays[i][1];
        ret[k].description=FixedDescriptions[i];
        k++;
       }
    }
 /* Relative to easter */
 Easter(year,&j,&v);
 easter_sunday=v+2;
 for (i=0; i<EASTER_REL; i++)
    {
     if (EasterRelNational[i] || !ONLY_NATIONAL)
       {
        Number2Day(easter_sunday+EasterRelHolidays[i],
                   &ret[k].day,&ret[k].month,&a);
        ret[k++].description=EasterRelDescriptions[i];
       }
    }
 /* A special one */
 if (year<1995 || !ONLY_NATIONAL)
   {
    int d, wd;
    d=Day2Number(23,11,year);
    wd=WeekDay(23,11,year);
    if (wd>=5) /* Move to wednesday */
       d-=wd-4;
    else
       d-=wd+3;
    Number2Day(d,&ret[k].day,&ret[k].month,&a);
    ret[k++].description="Buß- und Bettag";
   }

 *holidays=k;
 return ret;
}

