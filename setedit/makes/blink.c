/**[txh]********************************************************************

  Copyright (c) 2003 by Salvador E. Tropea.
  Covered by the GPL license.
  Description:
  tlink have a really useless command line. This wrapper avoids all the
complexities.
  
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE 256

int main(int argc, char *argv[])
{
 FILE *f;
 int i,ret;
 char *s;
 char b[MAX_FILE];

 if (argc<3)
   {
    printf("tlink wrapper\n");
    printf("Copyright (c) 2003 by Salvador E. Tropea. GPL.\n");
    printf("Usage: blink lib list_obj_files\n");
    return 1;
   }
 f=fopen("bcclink.lnk","wt");
 if (!f)
   {
    printf("Can't create response file\n");
    return 2;
   }
 for (i=2; i<argc; i++)
    {
     s=argv[i];
     if (!strstr(s,".obj"))
        continue;
     ret=strlen(s);
     if (ret>MAX_FILE-1)
       {
        printf("Name too long: %s\n",s);
        return 3;
       }
     strcpy(b,s);
     for (s=b; *s; s++)
         if (*s=='/') *s='\\';
     fprintf(f,"+%s &\n",b);
    }
 fprintf(f,"+");
 fclose(f);

 unlink(argv[1]);
 s=(char *)malloc(strlen(argv[1])+32);
 sprintf(s,"tlib %s /E @bcclink.lnk",argv[1]);
 ret=system(s);
 if (!ret)
    unlink("bcclink.lnk");
 return ret;
}
