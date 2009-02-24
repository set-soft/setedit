/**[txh]********************************************************************

  Copyright (c) 2009 by Salvador E. Tropea
  Covered by the GPL license, see the see copyrigh file for details.

  Description:
  Simple test program to test any of the holidays modules. I.e.
  gcc -o test test.c datetools.c deutschland.c
  
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "datetools.h"

extern struct dayMonth *GetListOfHolidays(int year, int *holidays);

int main(int argc, char *argv[])
{
 int i, c, y;
 struct dayMonth *d;

 if (argc!=2)
   {
    printf("Usage: test year\n");
    return 1;
   }
 y=atoi(argv[1]);
 printf("Computing holidays for %d:\n",y);
 d=GetListOfHolidays(y,&c);
 for (i=0; i<c; i++)
     printf("%2d) %02d/%02d %s\n",i+1,d[i].day,d[i].month,d[i].description);
 return 0;
}
