/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]**********************************************************************

  Module: Plasma 1
  Comments:
  This module provides some routines for trigonometric plasmas.
  The functions uses the sin and cos look up tables from @x{Math 1}.
  Some routines are originally by Nutty, I added new routines and enhanced
the rest using C instead of assembler. I saw the same plasma idea in other
sources so I think Nutty just enhanced some code.
  This plasma is calculated in real time, that's the surface is calculated
for each frame. That's limited but very good.
  The module can be optimized using assembler because GCC isn't perfect.

*****************************************************************************/

#include <stdlib.h>
#include <math.h>
#include "math1.h"
#include "plasa1.h"
#include "palette.h"

//
// Some macros to make the code more clear
//
#define IncWithWrap(a)   if (++a==TrigTableSize) a=0
#define CutExesValue(a)  if (a>=TrigTableSize-1) a=0
#define WrapValue(a)     if (a>TrigTableSize-1) a-=TrigTableSize

static int dsI1=0;

/**[txh]**********************************************************************

  Include: plasa1.h
  Description:
  Plasma with the equation: [cos(step)+cos(Y)+sin(Y)+cos(X)]*128. A temporal
copy of step is incremented with Y and the value is incremented each pass.
@p
  The values are in half of degrees. That's is X=320 is 160 degrees.
@p
  Original code by Nutty. Optimized in C by SET.

*****************************************************************************/

void PLA1_Step320x200_1(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsI1l=dsI1;

 for (Y=200; Y; --Y)
    {
     int Yv=cos_table[dsI1l]+cos_table[Y]+sin_table[Y];
     for (X=320; X; s++,--X)
         *s=(Yv+cos_table[X])>>1;
     IncWithWrap(dsI1l);
    }
 IncWithWrap(dsI1);
}

/**[txh]**********************************************************************

  Description:
  Same as PLA1_Step320x200_1 but with less zoom. @x{PLA1_Step320x200_1}.

*****************************************************************************/

void PLA1_Step320x200_1b(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsI1l=dsI1;

 for (Y=400; Y; Y-=2)
    {
     int Yv=cos_table[dsI1l]+cos_table[Y]+sin_table[Y];
     for (X=640; X; s++,X-=2)
         *s=(Yv+cos_table[X])>>1;
     IncWithWrap(dsI1l);
    }
 IncWithWrap(dsI1);
}

static int dsI2=0;

/**[txh]**********************************************************************

  Description:
  Equation: [cos(256*sin(I)+X)+cos(Itemp)+sin(Y)+cos(Y)]*256. I is incremented
with each pass, Itemp starts with I and is incremented with Y. The argument
of the first cos is truncated to 0 if the values is negative.
@p
  Original code by Nutty. Optimized in C by SET.

*****************************************************************************/

void PLA1_Step320x200_2(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=dsI2;
 int temp=sin_table[dsI2]; // Can be negative!

 for (Y=200; Y; --Y)
    {
     for (X=320; X; s++,--X) // --X at the end to use the flag saves 1 instruction
        {
         register unsigned t2 asm ("%eax"); // Force the addition in 32 bits ;-) saves 1 instruction
         t2=temp+X;
         CutExesValue(t2);
         t2=cos_table[t2]+cos_table[dsIl]+cos_table[Y]+sin_table[Y];
         *s=t2;
        }
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI2);
}

/**[txh]**********************************************************************

  Description:
  Same as PLA1_Step320x200_2 but with less zoom. @x{PLA1_Step320x200_2}.

*****************************************************************************/

void PLA1_Step320x200_2b(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=dsI2;
 int temp=sin_table[dsI2]; // Can be negative!

 for (Y=400; Y; Y-=2)
    {
     for (X=640; X; s++,X-=2) // --X at the end to use the flag saves 1 instruction
        {
         register unsigned t2 asm ("%eax"); // Force the addition in 32 bits ;-) saves 1 instruction
         t2=temp+X;
         CutExesValue(t2);
         t2=cos_table[t2]+cos_table[dsIl]+cos_table[Y]+sin_table[Y];
         *s=t2;
        }
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI2);
}

static int dsI3=0;

/**[txh]**********************************************************************

  Description:
  Equation: {cos(step)+[cos(X)+cos(Y)]/2}*256. See the
@x{PLA1_Step320x200_1,plasma 1}.
@p
  Original code by Nutty. Optimized in C by SET.

*****************************************************************************/

void PLA1_Step320x200_3(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=dsI3;

 for (Y=200; Y; --Y)
    {
     int temp=(cos_table[dsIl]<<1)+sin_table[Y];
     for (X=320; X; s++,--X)
         *s=(temp+cos_table[X])>>1;
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI3);
}

/**[txh]**********************************************************************

  Description:
  Same as PLA1_Step320x200_3 but with less zoom. @x{PLA1_Step320x200_3}.

*****************************************************************************/

void PLA1_Step320x200_3b(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=dsI3;

 for (Y=400; Y; Y-=2)
    {
     int temp=(cos_table[dsIl]<<1)+sin_table[Y];
     for (X=640; X; s++,X-=2)
         *s=(temp+cos_table[X])>>1;
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI3);
}

static int dsI4=0;

/**[txh]**********************************************************************

  Description:
  Equation: {sin(step)+[cos(X)+cos(Y)]/2}*256. See the
@x{PLA1_Step320x200_1,plasma 1}.
@p
  by SET.

*****************************************************************************/

void PLA1_Step320x200_4(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=dsI4;

 for (Y=200; Y; --Y)
    {
     int temp=(sin_table[dsIl]<<1)+sin_table[Y];
     for (X=320; X; s++,--X)
         *s=(temp+cos_table[X])>>1;
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI4);
}

static int dsI5=0;

/**[txh]**********************************************************************

  Description:
  Equation: [cos(step)+cos(X)+cos(Y)]*256. See the
@x{PLA1_Step320x200_1,plasma 1}.
@p
  by SET.

*****************************************************************************/

void PLA1_Step320x200_5(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=dsI5;

 for (Y=600; Y; Y-=3)
    {
     int Yv=sin_table[Y]+sin_table[dsIl];
     for (X=640; X; s++,X-=2)
         *s=Yv+cos_table[X];
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI5);
}

static int dsI6=0;

/**[txh]**********************************************************************

  Description:
  Equation: [cos(step)+cos(X)+cos(Y)]*256. See the
@x{PLA1_Step320x200_1,plasma 1}.
@p
  by SET.

*****************************************************************************/

void PLA1_Step320x200_6(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=sin_table[dsI6]+256;

 for (Y=600; Y; Y-=3)
    {
     int temp=dsIl;
     for (X=640; X; s++,X-=2)
        {
         IncWithWrap(temp);
         *s=cos_table[X]+sin_table[Y]+sin_table[temp];
        }
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI6);
}


static int dsI7=0,dsX7=0,dsY7=0,direction7=1;

/**[txh]**********************************************************************

  Description:
  Equation: [sin(step)+cos(X+Xoff)+cos(Y+Yoff)]*256. That's complex, Xoff is
a sin, Yoff is a ping-pong (linear) and step is the most complex. The static
value of step is incremented with each pass, a temporal copy is used for step
first the value is loaded with (sin(static step)+1)*256 then the value is
incremented with each Y, finally the step used is a second temporal
incremented with each X. See the @x{PLA1_Step320x200_1,plasma 1}.
@p
  by SET.

*****************************************************************************/

void PLA1_Step320x200_7(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=sin_table[dsI7]+256;

 for (Y=200; Y; Y--)
    {
     int temp=dsIl;
     for (X=320; X; s++,X--)
        {
         IncWithWrap(temp);
         *s=cos_table[X+dsX7]+sin_table[Y+dsY7]+sin_table[temp];
        }
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI7);
 if (dsY7==(TrigTableSize-200))
    direction7=-1;
 else
   if (dsY7==0 && direction7==-1)
      direction7=1;
 dsY7+=direction7;
 dsX7=(cos_table[dsI7]+256)>>1;
}

/**[txh]********************************************************************

  Description:
  A generic version of @x{PLA1_Step320x200_1}.

***************************************************************************/

void PLA1_Step_1(int w, int h, unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsI1l=dsI1;
 int Yv;

 for (Y=h; Y; --Y)
    {
     // Nutty didn't saw it, C code faster than ASM ;-)
     Yv=cos_table[dsI1l]+cos_table[Y]+sin_table[Y];
     for (X=w; X; s++,--X)
         *s=(Yv+cos_table[X])>>1;
     IncWithWrap(dsI1l);
    }
 IncWithWrap(dsI1);
}

/**[txh]********************************************************************

  Description:
  A generic version of @x{PLA1_Step320x200_2}.

***************************************************************************/

void PLA1_Step_2(int w, int h, unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=dsI2;
 int temp=sin_table[dsI2];

 for (Y=h; Y; --Y)
    {
     int Yv=cos_table[Y]+sin_table[Y]+cos_table[dsIl];
     for (X=w; X; s++,--X)
        {
         unsigned t2;
         t2=temp+X;
         CutExesValue(t2);
         *s=cos_table[t2]+Yv;
        }
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI2);
}

/**[txh]********************************************************************

  Description:
  A generic version of @x{PLA1_Step320x200_3}.

***************************************************************************/

void PLA1_Step_3(int w, int h, unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=dsI3;

 for (Y=h; Y; --Y)
    {
     int temp=(cos_table[dsIl]<<1)+sin_table[Y];
     for (X=w; X; s++,--X)
         *s=(temp+cos_table[X])>>1;
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI3);
}

/**[txh]********************************************************************

  Description:
  A generic version of @x{PLA1_Step320x200_4}.

***************************************************************************/

void PLA1_Step_4(int w, int h, unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=dsI4;

 for (Y=h; Y; --Y)
    {
     int temp=(sin_table[dsIl]<<1)+sin_table[Y];
     for (X=w; X; s++,--X)
         *s=(temp+cos_table[X])>>1;
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI4);
}

/**[txh]********************************************************************

  Description:
  A generic version of @x{PLA1_Step320x200_7}.

***************************************************************************/

void PLA1_Step_7(int w, int h, unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=sin_table[dsI7]+256;

 for (Y=h; Y; Y--)
    {
     int temp=dsIl;
     int temp2=sin_table[Y+dsY7];
     for (X=w; X; s++,X--)
        {
         IncWithWrap(temp);
         *s=temp2+cos_table[X+dsX7]+sin_table[temp];
        }
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI7);
 if (dsY7==(TrigTableSize*2-h))
    direction7=-1;
 else
   if (dsY7==0 && direction7==-1)
      direction7=1;
 dsY7+=direction7;
 dsX7=(cos_table[dsI7]+256)>>1;
}


