/**[txh]********************************************************************

  Copyright (c) 2003 by Salvador E. Tropea
  Covered by the GPL license, see the see copyrigh file for details.

  Description:
  Module with the default holidays.@*
  That's to show how to write a holidays plug-in. It returns a list of
dates that are more or less important worldwide.@*
  
***************************************************************************/

#include <stdlib.h>
#include "datetools.h"

/**[txh]********************************************************************

  Description:
  The implementation of this function is mandatory.
  
  Return: A pointer to a newly malloced block of memory containing the list
of holidays (struct dayMonth). The number of entries is returned thru the
@var{cant} argument.
  
***************************************************************************/

struct dayMonth *GetListOfHolidays(int year, int *cant)
{
 // We have four holidays
 struct dayMonth *ret=(struct dayMonth *)malloc(sizeof(struct dayMonth)*4);
 int t,f,aux;

 // january 1st, may 1st and december 25th are fixed:
 ret[0].day=1;
 ret[0].month=1;
 ret[0].description="First day of the year";
 ret[1].day=1;
 ret[1].month=5;
 ret[1].description="Worker's day";
 ret[2].day=25;
 ret[2].month=12;
 ret[2].description="Christmas";

 // Easter sunday must be calculated, but this is provided by the datetools.so
 // module.
 Easter(year,&t,&f);
 // What we get is the thursday and friday, we will just indicate the sunday
 f+=2; // Sunday=Friday+2
 Number2Day(f,&ret[3].day,&ret[3].month,&aux);
 ret[3].description="Easter sunday";

 *cant=3;
 return ret;
}

