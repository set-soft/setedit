/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]**********************************************************************

  Module: Math 1
  Description:
  This module provides a co/sine table of 4*PI in length, 0.5 degrees of
resolution and in the range of [-255,255].
  The tables are merged to save space so the real size is 9/2*PI. That
reduce the size from 11520 bytes to 6480 bytes.
  The arrays are called sin_table and cos_table (the last is fake).
  2*PI is equivalent to TrigTableSize, use it for convertions.
  If you need to normalize the amplitud to [-1,1] use the tsinConst constant.
  The math 4 module is faster but with less resolution. @x{Math 4}.

*****************************************************************************/

#include <math.h>
#include "math1.h"

#define TamReal (TrigTableSize*2+TrigTableSize/4) // 2*360+90 degrees
int sin_table[TamReal];

/**[txh]********************************************************************

 Include: math1.h
 Description:
 Initializes the look up tables for the module.

***************************************************************************/

void MA1_CreateLookUp(void)
{
 unsigned int i;
 double temp;

 for (i=0; i<TamReal; i++)
    {
     temp = sin((double)(i*PI/(180*(TrigTableSize/360)) ));
     temp *= 255;
     sin_table[i] = (int)temp;
    }
}


