/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <configed.h>
#include <string.h>
#include <dyncat.h>
#include <stdlib.h>

/**[txh]********************************************************************

  Description:
  This function is used to concatenate strings that we don't know the final
size. To make it faster the routine uses an structure to hold the end of
the string, in this way the routine doesn't need to scan the string all the
time.@p
  Struct is a pointer to a local DynStrCatStruct, str is the string and len
is the len of the string to concatenate. The len is provided by the calling
routine, if the parameter is skipped the routine calls to strlen to find it.@p
  For a simple init call it like this:@*
DynStrCatInit(&stru,0,0);

***************************************************************************/

void DynStrCatInit(DynStrCatStruct *Struct, const char *str, int len)
{
 if (len<0)
    len=str ? strlen(str) : 0;
 Struct->str=(char *)malloc(len+1);
 if (str)
    strncpy(Struct->str,str,len);
 Struct->str[len]=0;
 Struct->len=len;
}

/**[txh]********************************************************************

  Description:
  That's called after DynStrCatInit to add new strings. @x{DynStrCatInit}.

***************************************************************************/

void DynStrCat(DynStrCatStruct *Struct, const char *str, int len)
{
 int oldLen=Struct->len;

 if (len<0)
    len=strlen(str);
 Struct->len+=len;
 Struct->str=(char *)realloc(Struct->str,Struct->len+1);
 memcpy(Struct->str+oldLen,str,len);
 Struct->str[Struct->len]=0;
}

char *newStrL(const char *start, int len)
{
 char *ret=new char[len+1];
 memcpy(ret,start,len);
 ret[len]=0;
 return ret;
}

