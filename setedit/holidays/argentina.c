/**[txh]********************************************************************

  Copyright (c) 1992-2003 by Salvador E. Tropea
  Covered by the GPL license, see the see copyrigh file for details.

  Description:
  Module to find the holidays for Argentina.@*
  Note that some holidays are moved to make "long weekends", this is a
little bit tricky.@*
  Supports the changes introduced in 1996 and 2002.
  
***************************************************************************/

#include <stdlib.h>
#include "datetools.h"

/************************** Feriados *********************************/
// Domingo,Sábado y Lunes no se corren, Martes y Miércoles pasan al Lunes de
// esa semana y Jueves y Viernes al Lunes de la siguiente
#define nFeriados 11
static int Corrimientos[]={0,0,-1,-2,+4,+3,0};
static char Feriados[nFeriados][2]=
  {{1,1},{2,4},{1,5},{25,5},{10,6},{20,6},{9,7},{17,8},{12,10},{8,12},{25,12}};
static char SeCorre[nFeriados]=
  {    0,    1,    0,     0,     1,     1,    0,     1,      1,     0,     0};
static char ADonde[nFeriados]=
  {    0,    1,    0,     0,     1,     2,    0,     2,      1,     0,     0};
static const char *Descripciones[nFeriados]=
{
 "Primer día del año",
 "Día de las Malvinas",
 "Día del trabajador",
 "Revolución de Mayo",
 "Día de la Soberanía",
 "Día de la bandera",
 "Día de la Independencia",
 "Aniversario de San Martín",
 "Día de la Raza",
 "Día de la Virgen",
 "Navidad"
};

static
int CalculaFeriado(int nFeriado, int nAno, int *nDiaNum, int *nDiaSem,
                   const char **desc)
{
 int nDiaSem1ro,nDia,nMes;

 nDia=Feriados[nFeriado][0];
 nMes=Feriados[nFeriado][1];
 // El 10/6 se movió al 2/4, parece que fue a partir del 2002.
 if (nDia==10 && nMes==6 && nAno>=2002)
    return 0;
 if (nDia==2 && nMes==4 && nAno<2002)
    return 0;
 *nDiaNum=Day2Number(nDia,nMes,nAno);
 *nDiaSem=WeekDay(nDia,nMes,nAno);

 if (SeCorre[nFeriado])
   {
    if (nAno<1996 || ADonde[nFeriado]==1)
      {
       *nDiaNum+=Corrimientos[*nDiaSem-1];
       // Si se lo corrió se lo hizo a lunes
       if (Corrimientos[*nDiaSem-1]!=0)
          *nDiaSem=2;
      }
    else
      {// A partir de 1996 y para feriados tipo 2
       // Buscar el primer día del mes con el nombre indicado
       nDiaSem1ro=2-WeekDay(1,nMes,nAno);
       if (nDiaSem1ro<0)
          nDiaSem1ro+=7;
       // Buscar el número de día
       *nDiaNum=Day2Number(1,nMes,nAno)+nDiaSem1ro+14;
       // Si o si a lunes
       *nDiaSem=2;
      }
   }
 *desc=Descripciones[nFeriado];
 return 1;
}

struct dayMonth *GetListOfHolidays(int anio, int *feriados)
{
 int a,nD,nS,j,v;
 struct dayMonth *ret=(struct dayMonth *)malloc(sizeof(struct dayMonth)*(nFeriados+2));
 int i,k;

 for (i=0,k=0; i<nFeriados; i++)
    {
     if (CalculaFeriado(i,anio,&nD,&nS,&ret[k].description))
       {
        Number2Day(nD,&ret[k].day,&ret[k].month,&a);
        k++;
       }
    }

 Easter(anio,&j,&v);
 Number2Day(j,&ret[k].day,&ret[k].month,&a); ret[k++].description="Jueves Santo";
 Number2Day(v,&ret[k].day,&ret[k].month,&a); ret[k++].description="Viernes Santo";

 *feriados=k;
 return ret;
}

