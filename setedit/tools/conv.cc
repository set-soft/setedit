/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*
  Este programa lo use para convertir los viejos archivos .h de teclado al
  nuevo formato (.lay)
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define GetLine() fgets(buf,199,f)

char buf[200];
char Name[11];

typedef char ScanNamesType[128][11];
ScanNamesType ScanNames;

int main(int argc, char *argv[])
{
 char *s;
 int i,pos,unk,k;

 if (argc==1)
   {
    printf("Uso: %s archivos ...\n",argv[0]);
    return 1;
   }
 FILE *f;
 for (k=1; k<argc; k++)
    {
     printf("Procesando: %s\n",argv[k]);
     f=fopen(argv[k],"rt");
     if (!f)
        return 1;
     for (i=0; i<128; i++)
         ScanNames[i][0]=0;
     GetLine();
     while (!feof(f))
       {
        if (strncmp(buf,"#define",7)==0)
          {
           for (s=buf+7; *s && ucisspace(*s); s++);
           if (*s)
             {
              for (i=0;i<10 && *s && ucisalnum(*s); s++) Name[i++]=*s;
              Name[i]=0;
              if (Name[0]=='U' && Name[1]=='N' && Name[2]=='K')
                 *Name=0;
              if (ucisalnum(*s))
                {
                 printf("Mide m s de 10!:\n%s",buf);
                 return 1;
                }
              for (; *s && ucisspace(*s); s++);
              if (*s=='?')
                 pos=127;
              else
                 pos=atoi(s);
              if (pos>127)
                {
                 printf("Cuidado scan %d\n%s",pos,buf);
                 pos&=127;
                }
              strcpy(ScanNames[pos],Name);
             }
          }
        GetLine();
       }
     for (unk=1,i=0; i<128; i++)
         if (ScanNames[i][0]==0)
            sprintf(ScanNames[i],"UNK%d",unk++);
     fclose(f);
     f=fopen(argv[k],"wt");
     for (i=0; i<128; i++)
         fprintf(f,"%s %d\n",ScanNames[i],i);
     fclose(f);
    }
 return 0;
}
