/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#include <sys/stat.h>

int LineAlloc[32];

int ProxPow(int a, int poner=0)
{
 int r=1,i=0;
 if (a==0)
    return 0;
 a+=4; // malloc overhead
 while (r<a) { r<<=1; i++; }

 if (poner)
    LineAlloc[i]++;

 return r;
}

void Barra(int val, int max)
{
 int len=(int)(50.0*val/max);
 while (len--)
   {
    printf("X");
   }
 printf("\n");
}

int main(int argc, char *argv[])
{
 char b[30000];

 if (argc<2)
   {
    printf("Medidor de memoria usada por el editor\n");
    printf("Uso: %s archivos\n",argv[0]);
   }
 int i=1,bbig,lines,bsmall,alines,aflags,afulls,afull;
 int bigacu=0,smallacu=0,sm2acu=0,acul,lenl,acult=0,linet=0;
 struct stat s;

 for (i=0; i<32; i++)
     LineAlloc[i]=0;

 i=1;
 while (i<argc)
   {
    if (stat(argv[i],&s)==0)
      {
       printf("%12s %7d",argv[i],s.st_size);
       bbig=ProxPow(s.st_size+4084);
       printf(" %7d",bbig);
       lines=bsmall=0;
       FILE *f=fopen(argv[i],"rt");
       acul=0;
       fgets(b,30000,f);
       while (!feof(f))
         {
          lines++;
          lenl=strlen(b)-1;
          acul+=lenl;
          bsmall+=ProxPow(lenl,1);
          fgets(b,30000,f);
         }
       fclose(f);
       acult+=acul;
       linet+=lines;
       alines=aflags=ProxPow(lines*2);
       printf(" %6d %6d %6d %7d",lines,alines,aflags,bbig+alines+aflags);
       bigacu+=bbig+alines+aflags;
       afulls=ProxPow(lines*8);
       printf(" %6d %7d %7d",afulls,bsmall,afulls+bsmall);
       smallacu+=afulls+bsmall;
       afull=ProxPow(lines*12);
       sm2acu+=afull+bsmall;
       printf(" %7d %3d",afull,(int)((double)acul/lines+.5));
       printf("\n");
      }
    i++;
   }
 printf(
"
Actual: %d Nuevo: %d (%d)
Tama¤o promedio de las l¡neas: %d
Histograma de alocaciones de las l¡neas:
",bigacu,smallacu,sm2acu,(int)((double)acult/linet+.5));
 int max=0;
 for (i=0; i<32; i++)
     if (LineAlloc[i]>max)
        max=LineAlloc[i];
 for (i=0; i<32; i++)
    {
     printf("2^%2d (%10d): %6d ",i,1<<i,LineAlloc[i]);
     Barra(LineAlloc[i],max);
    }
 return 0;
}
