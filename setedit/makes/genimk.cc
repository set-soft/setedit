#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

static
void ChangeExt(char *s, const char *newE)
{
 int l=strlen(s);
 int lE=strlen(newE);
 strcpy(s+(l-lE),newE);
}

int main(int argc, char *argv[])
{
 if (argc<3)
   {
    printf("Usage: genimk output main.gpr [other.gprs...]\n");
    return 1;
   }
 const char *dest=argv[1];
 const char *mainGpr=argv[2];
 char *mainMak=strdup(mainGpr);
 ChangeExt(mainMak,".mak");
 printf("\nGenerating %s for %s (%s)\n",dest,mainGpr,mainMak);

 int i;
 unsigned lenTot=20;
 for (i=0; i<argc; i++)
     lenTot+=2+strlen(argv[i]);

 char *b=new char[lenTot];
 for (i=2; i<argc; i++)
    {
     if (strstr(argv[i],".gpr"))
       {
        char *gpr=argv[i];
        char *mak=strdup(gpr);
        char *imk=strdup(gpr);
        ChangeExt(mak,".mak");
        ChangeExt(imk,".imk");

        struct stat stGpr;
        if (stat(gpr,&stGpr)!=0)
          {
           fprintf(stderr,"%s: Can't stat %s\n",argv[0],gpr);
           return 20;
          }

        struct stat stMak;
        if (stat(mak,&stMak)!=0 || stMak.st_mtime<stGpr.st_mtime)
          {
           printf("%s => %s\n",gpr,mak);
           sprintf(b,"gpr2mak %s",gpr);
           system(b);
           if (stat(mak,&stMak)!=0)
             {
              fprintf(stderr,"%s: Can't create .mak file for %s\n",argv[0],gpr);
              return 21;
             }
          }

        struct stat stImk;
        printf("%s => %s\n",mak,imk);
        sprintf(b,"./extrimk.exe %s > %s",mak,imk);
        int ret=system(b);
        if (ret)
           return ret>>8;
        if (stat(mak,&stImk)!=0)
          {
           fprintf(stderr,"%s: Can't create .imk file for %s\n",argv[0],mak);
           return 22;
          }

        free(mak);
        free(imk);
       }
    }

 printf("\n");
 return 0;
}

