/**[txh]********************************************************************

  Copyright (c) 2004-2005 by Salvador E. Tropea.
  Covered by the GPL license.
  Description:
  MS lib have a really useless command line. This wrapper avoids all the
complexities.
  Also: I failed to create a macro that expands to something that creates
a response file. Looks like it only works outside macros. This program
just collects the files from the specified directories and creates the
library with all of them.
  
***************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE 256

int main(int argc, char *argv[])
{
 FILE *f;
 int i,ret;
 WIN32_FIND_DATA de;
 char dirMask[MAX_PATH+8];
 HANDLE d;

 if (argc<3)
   {
    printf("MS lib wrapper\n");
    printf("Copyright (c) 2004-2005 by Salvador E. Tropea. GPL.\n");
    printf("Usage: mlink [switches] [libraries] directories\n");
    return 1;
   }
 f=fopen("mslink.lnk","wt");
 if (!f)
   {
    printf("Can't create response file\n");
    return 2;
   }
 for (i=1; i<argc; i++)
    {
     if (argv[i][0]=='/' || strstr(argv[i],".lib") || strstr(argv[i],".obj"))
       {
        fprintf(f,"%s ",argv[i]);
        continue;
       }
     sprintf(dirMask,"%s\\*.obj",argv[i]);
     d=FindFirstFile(dirMask,&de);
     if (d==INVALID_HANDLE_VALUE)
       {
        printf("Can't open directory %s\n",argv[i]);
        return 4;
       }
     do
       {
        //printf("%s\n",de.cFileName);
        fprintf(f,"%s\\%s ",argv[i],de.cFileName);
       }
     while (FindNextFile(d,&de));
     FindClose(d);
    }
 fprintf(f,"\n");
 fclose(f);

 ret=system("link @mslink.lnk");
 if (!ret)
    unlink("mslink.lnk");
 return ret;
}

