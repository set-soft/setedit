/**[txh]********************************************************************

  Copyright (c) 1992-2003 by Salvador E. Tropea
  Covered by the GPL license, see the see copyrigh file for details.

  Module: Date Tools
  Description:
  This module contains functions used to compute dates. They can convert a
date into a unique number and the reverse, find which day of the week
corresponds to a date expressed as a number and find the easter dates for
any year.@*
  All of them are designed to support the discontinuity introduced in
october 1582 when the rules to compute leap years changed. But I don't know
if it works ok.@*
  I designed some portions of this code for Commodore 128 Basic, then
recoded for Clipper 5 and now for C.@*
  Most comments are in spanish, I just translated the descriptions and the
names of the most relevant arguments. If you need more information and
can't translate the spanish text just ask me.@*
  The easter computation is something really interesting, is based in the
moon and equinoxes, I'm using the original methode used by priest to
compute these dates and that's why the code refers to archaic terms as
"Aureo número" (golden day). But it works perfectly.@*
  Also note this code doesn't use any library function.@p
  
***************************************************************************/

static
int DaysTo[2][13]=
{
 {0,31,59,90,120,151,181,212,243,273,304,334,365},
 {0,31,60,91,121,152,182,213,244,274,305,335,366}
};

static
int IsLeap(int a)
{
 return ((a%4==0) && (a%100!=0)) || (a%400==0);
}

/**[txh]********************************************************************

  Description:
  Converts a date (day, month, year) to a number. Note this number takes
care about the discontinuity introduced in october 1582.
  
  Return: A number representing the date, suitable for additions and
substractions.
  
***************************************************************************/

int Day2Number(int d, int m, int a)
{
 int nDay,nAux;

 // Ver si es anterior al 5/10/1582
 if (a<1582 || (a==1582 && (m<10 || (m==10 && d<5))))
   {// Anterior al 5/10/1582 => Juliano
    nDay=(int)(a*365.25+0.99)+DaysTo[a%4==0 ? 1 : 0][m-1]+d;
   }
 else
   {// Posterior => Gregoriano
    nDay=(int)((a-1580)*365.25+0.99)+DaysTo[a%4==0 ? 1 : 0][m-1]+d-1462;
    // Cuantas centurias, para restar el hecho de que los terminados en 00
    // no son bisiestos
    nAux=(nDay-5903)/36525;
    // Pero ojo que los múltiplo de 400 si
    nDay-=nAux-nAux/4;
    nDay+=578557;
   }
 return nDay;
}

/**[txh]********************************************************************

  Description:
  Returns the number of day of the week corresponding for a specified date
(day, month, year). Supports the discontinuity introduced in october 1582.
  
  Return: The number of day in the week, 1 is sunday and 7 is satursday.
  
***************************************************************************/

int WeekDay(int d, int m, int a)
{
 int nDay,nAux;

 // Ver si es anterior al 5/10/1582
 if (a<1582 || (a==1582 && (m<10 || (m==10 && d<5))))
    // Anterior al 5/10/1582 => Juliano
    nDay=(int)(a*365.25+0.99)+DaysTo[a%4==0 ? 1 : 0][m-1]+d+3;
 else
   {
    // Posterior => Gregoriano
    nDay=(int)((a-1580)*365.25+0.99)+DaysTo[a%4==0 ? 1 : 0][m-1]+d-1462;
    // Cuantas centurias, para restar el hecho de que los terminados en 00
    // no son bisiestos
    nAux=(nDay-5903)/36525;
    // Pero ojo que los múltiplo de 400 si
    nDay-=nAux-nAux/4;
   }

 return (nDay%7)+1;
}

/**[txh]********************************************************************

  Description:
  Finds which day (day, month and year) corresponds to a specified number.
This number is the one you get from @x{Day2Number}. Supports the
discontinuity introduced in october 1582.
  
  Return: Nothing, the values are returned through the pointers.
  
***************************************************************************/

void Number2Day(int nNumber, int *day, int *month, int *year)
{
 int nRest,nBis,nAux1,nAux2,nYear,nMonth,nDay;

 if (nNumber>578103)
   {// Desajustar la fecha gregoriana para que sea de la forma de la juliana,
    // es decir sumar los años 1700,1800,1900,... que no son bisiestos, como
    // si lo fueran
    nNumber-=578557;
    nAux1=(nNumber-5903)/36525;    // Centurias después de 1600
    nAux2=nAux1-nAux1/4;           // Menos las que si son bisiestas
    nNumber+=731+nAux2;            // Reajustar con referencia de año 0 = 1582
    nYear=nNumber/365.25;
    // Cuantos días sobran redondeando hacia arriba
    nRest=nNumber-(int)(nYear*365.25+0.3);
    // Reajustar el año
    nYear+=1582;
    nBis=IsLeap(nYear) ? 1 : 0;
   }
 else
   {
    nYear=nNumber/365.25;
    nBis=nYear%4==0 ? 1 : 0;
    // Cuantos días sobran redondeando hacia arriba
    nRest=nNumber-(int)(nYear*365.25+0.9);
   }
 // A cual mes corresponde
 nMonth=2;
 while (DaysTo[nBis][nMonth-1]<nRest)
   nMonth++;
 // Cuantos días sobran
 nMonth--;
 nDay=nRest-DaysTo[nBis][nMonth-1];
 *day=nDay;
 *month=nMonth;
 *year=nYear;
}

/**[txh]********************************************************************

  Description:
  Finds the number of the days corresponding to the Thursday and Friday
before the Easter Sunday for a desired year.
  
  Return: Nothing, the values are returned through the pointers.
  
***************************************************************************/

void Easter(int nY, int *nThursday, int *nFriday)
{// Verified for 1995, 1998, 1999 and 2002.
 int nAureo,nSiglos,nCantBis,nLetraDom,nEpactaJ,nCorrSolar,nCorrLunar;
 int nEpacta,nDireccion,nDiaNum,nDia,nMes;

 // Aúreo Número:
 // Para conocer las lunaciones se descubrió que cada 19 años julianos estas
 // se repetían, a esto se lo llamó período de Metón. Este período abarca 4
 // o 5 bisiestos. El aúreo número es la posición del año dentro de este ciclo
 // La referencia es el año -1 donde el novilunio fue el 1 de Enero.
 nAureo=(nY+1)%19;
 if (nAureo==0)
    nAureo=19;

 // Letra Dominical: Si a los 7 primeros días del año los nombro A,B,...,G
 // la letra dominical es la letra del primer domingo del año.
 // Según el calendario gregoriano el año anterior al 1 empezó en domingo (A)
 // cada año se corre un día ya que un año de 365 días tiene 52 semanas+1 día
 // pero los bisiestos adelantan 2, la idea es saber cuan corrido está el
 // año en cuestión es decir cual es la letra dominical de ese año
 // El 7-x viene del hecho de que las letras retroceden al avanzar los
 // defasajes. La letra dominical es que número de día del año le corresponde
 // al primer domingo del año, en los años bisiestos hay 2: una hasta el
 // 29/2 y otra para el resto, este cálculo busca la que sirve para el resto
 // del año por lo que en los años bisiestos no coincide con el nro. del 1er
 // domingo, pero sirve para cálculos suponiendo que no es un bisiesto.
 nSiglos=nY/100;
 nCantBis=nY/4-nSiglos+nY/400;
 nLetraDom=7-(nY+nCantBis-1)%7;

 // Epacta: Es el número de días que excede el año solar al lunar.
 // el solar son 365, el lunar 29,5*12 = 354 => cada año la epacta se corre
 // 11 días, las epactas van hasta el 30, por lo que es un resto.
 // Se sabe que la epacta de 1582 era 26 => el resto de (26+11(Año-1582))/30
 // Nota: No el año, sino el corrimiento lunar (Aúreo Número)
 // es la epacta de dicho año, con esto obtengo la del Juliano (nEpactaJ).
 // Si se tiene en cuenta que el aúreo número de ese año es 6 puedo poner
 // resto de (26+11(nAureo-6))/30
 nEpactaJ=(26+11*(nAureo-6))%30;
 // Luego las correcciones: hay que restar los no bisiestos de 1700,1800,1900
 // etc a partir de 1582 esto es: (Siglos-16)/4-(Siglos-16)
 nCorrSolar=(nSiglos-16)/4-(nSiglos-16);
 // Ahora la corrección Lunar: Es un corrimiento en el Lunar que equivale a
 // 8 días cada 25 siglos:
 nCorrLunar=(nSiglos-15-((nSiglos-17)/25))/3;
 nEpacta=nEpactaJ+nCorrSolar+nCorrLunar;
 if (nEpacta<1)
    nEpacta+=30;

 /* Pascuas: Lilius autor de la reforma Gregoriana estableció en el concilio
 de Nicea que la fecha de Pascuas de Resurrección debe cumplir:
 1 - Ser Domingo
 2 - Debe ser el que sigue al 14avo día de la luna pascual. Si el 14avo fuera
     Domingo entonces es el que sigue.
 3 - Luna pascual es aquella cuyo 14avo día tiene lugar en o inmediatamente
     después del equinoccio vernal.
 4 - El equinoccio vernal tiene lugar el 21 de Marzo.
  Nota mia: El 14avo día es la luna llena, el 1ro es el novilunio (nueva luna)
  No se refieren a datos astronómicos reales, sino a datos eclesiásticos, ya
 que el equinoccio puede no ser el 21 (20 o 19) lo mismo con la luna, se
 refieren a períodos preestablecidos al crear el calendario gregoriano.
 
 nDiasH15: Es la cantidad de días entre el 21/3 y el 15avo día del ciclo lunar.
 nLetraDom: Letra Dominical (día de la semana del 1/1)
 nDireccion: Número de días entre el 21/3 y pascuas.
 nDia15: Es el número de día de la semana que cae el 15avo lunar.
 Es:
    nDireccion = nDiasH15 + nLetraDom - nDia15
 Luego: La edad de la luna el 1/3 es la misma que el 1/1 que sale de la nEpacta
    y se cumple que nDiasH15 = 24 - nEpacta
                    nDia15 = 27 - nEpacta
 como nDiasH15 debe ser mayor que 0 si nEpacta>23 se le suma 30 (un período
 lunar)
 */
 // Averiguar que día despues del 21 tiene 14 días la luna:
 // Primero cuanto después del 1/3 queda
 if (nEpacta<24)
    nDireccion=44-nEpacta;
 else
    nDireccion=73-nEpacta;
 // Ahora cual día es
 nDiaNum=Day2Number(1,3,nY)+nDireccion;
 // Que día de la semana es
 Number2Day(nDiaNum,&nDia,&nMes,&nY);
 nDireccion=WeekDay(nDia,nMes,nY);
 // Ahora cuantos días faltan para el domingo
 nDireccion=8-nDireccion;
 if (nDireccion==7)
    nDireccion=0;
 // Número de día del Domingo de Pascua
 nDiaNum+=nDireccion;
 *nThursday=nDiaNum-3;
 *nFriday=nDiaNum-2;
}

