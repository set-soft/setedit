/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*
  Este programa lo use para convertir los cmbXXXX en keybind.h a cmcXXXXX
  en menubind.h
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(void)
{
 FILE *f,*d;
 int val;
 char buffer[200],*def;

 f=fopen("keybind.h","rt");
 d=fopen("keybind.hv","wt");
 while (!feof(f))
   {
    fgets(buffer,200,f);
    if (!ucisspace(buffer[0]) && !feof(f))
      {
       strtok(buffer," ");
       def=strtok(NULL," ");
       def[2]='c';
       val=atoi(strtok(NULL," "))+600;
       fprintf(d,"#define %s %d\n",def,val);
      }
   }

 return 0;
}
