/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

 Module: Math 2
 Comments:
 Generates a ramdom table of 65536 elements. The table is in the code. Uses
the time to start the pseudo-ramdom sequence.

***************************************************************************/

#include <stdlib.h>
#include <go32.h>
#include <libc/farptrgs.h>
#include "math2.h"

// 64Kb
unsigned char  RandomTable[RandomTableSize];
// short to wrap in the table
unsigned short RandomTablePos;
static   char  Initialized=0;

// No the library rawclock.
inline unsigned long _rawclock(void)
{
 return _farpeekl(_dos_ds, 0x46c);
}

/**[txh]********************************************************************

  Include: math2.h
  Description:
  Fills the table.

***************************************************************************/

void MA2_InitRTable(void)
{
 unsigned short *p=(unsigned short *)RandomTable;
 int i;

 RandomTablePos=0;
 if (Initialized)
    return;
 Initialized=1;
 // 64K of Ramdom numbers
 srandom(_rawclock());
 for (i=RandomTableSize/2; i; --i)
     p[i-1]=random();
}

