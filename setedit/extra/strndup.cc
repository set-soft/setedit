/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <string.h>

/**[txh]********************************************************************

  Description:
  Creates a copy of an string copying at most len characters. The allocated
string have len+1 bytes and is ever ASCIIZ.

***************************************************************************/

char *strndup(char *source, int len)
{
 char *s=new char[len+1];
 strncpy(s,source,len);
 s[len]=0;
 return s;
}
