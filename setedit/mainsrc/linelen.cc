/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdlib.h>
#include <stdio.h>
#define Uses_string

#define Uses_LineLengthArray
#include <ceditor.h>

/****************************************************************************

    Class for a transparent and dynamic array used for the length of the
  lines and for the syntax flags.

****************************************************************************/

const unsigned incElements=1020;

LineLengthArray::LineLengthArray()
{
 elArray    =(uint16 *)malloc(incElements*2);
 elArrayAttr=(uint32 *)malloc(incElements*4);
 if (!elArray || !elArrayAttr)
    MaxPos=0;
 else
    MaxPos=incElements;
 Length=0;
}

LineLengthArray::~LineLengthArray()
{
 if (elArray)
    free(elArray);
 if (elArrayAttr)
    free(elArrayAttr);
 elArrayAttr=0;
 elArray=0;
}

void LineLengthArray::Resize(unsigned size)
{
 if ((elArray=elArray=(uint16 *)realloc(elArray,size*2))==0
     || (elArrayAttr=(uint32 *)realloc(elArrayAttr,size*4))==0)
    abort();
 MaxPos=size;
}

void LineLengthArray::set(unsigned pos, uint16 val)
{
 if (pos<MaxPos)
   {
    elArray[pos]=val;
   }
 else
   {
    Resize((pos/incElements+1)*incElements);
    elArray[pos]=val;
   }
 if (++pos>Length)
    Length=pos;
 return;
}

void LineLengthArray::setAttr(unsigned pos, uint32 attr)
{
 if (pos>=MaxPos)
    set(pos,0);
 elArrayAttr[pos]=attr;
 return;
}

void LineLengthArray::setAll(unsigned pos, uint16 length, uint32 attr)
{
 set(pos,length);
 elArrayAttr[pos]=attr;
 return;
}

void LineLengthArray::insert(unsigned pos, uint16 val)
{
 if (Length>=MaxPos)
    Resize(MaxPos+incElements);

 memmove(&elArray[pos+1],&elArray[pos],(Length-pos)<<1);
 memmove(&elArrayAttr[pos+1],&elArrayAttr[pos],(Length-pos)<<2);
 Length++;
 elArray[pos]=val;
}

void LineLengthArray::del(unsigned pos)
{
 memcpy(&elArray[pos],&elArray[pos+1],(Length-pos-1)<<1);
 memcpy(&elArrayAttr[pos],&elArrayAttr[pos+1],(Length-pos-1)<<2);
 Length--;
}

void LineLengthArray::deleteRange(unsigned from,unsigned to)
{
 unsigned count=to-from+1;
 CLY_memcpy(&elArray[from],&elArray[to+1],(Length-from-count)<<1);
 CLY_memcpy(&elArrayAttr[from],&elArrayAttr[to+1],(Length-from-count)<<2);
 Length-=count;
}

