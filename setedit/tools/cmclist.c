/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*
 Este programa lo use para generar la lista ordenada alfabeticamente de los
 comandos del editor.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
 char *name;
 int val;
} str;

int compara(const void *e1, const void *e2)
{
 return strcmp(((str *)e1)->name,((str *)e2)->name);
}

int main(void)
{
 FILE *f;
 int cant=0,i;
 char *s;
 char buf[80];
 str Lista[255];

 f=fopen("menubind.h","rt");
 fgets(buf,80,f);
 while (!feof(f))
   {
    s=strtok(buf," ");
    Lista[cant].name=strdup(s+3);
    s=strtok(NULL," ");
    Lista[cant].val=atoi(s);
    cant++;
    fgets(buf,80,f);
   }
 fclose(f);
 qsort(Lista,cant,sizeof(str),compara);
 for (i=0; i<cant; i++)
     printf("{ \"%s\", %d },\n",Lista[i].name,Lista[i].val-600);
}

