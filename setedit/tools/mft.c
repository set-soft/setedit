/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#include <ftw.h>
#include <string.h>

int extra;

int func(const char *path, struct stat *stbuf, int flag)
{
 if (flag==FTW_F)
   {
    printf("%s\n",path+extra+1);
   }
 return 0;
}

int main(int argc, char *argv[])
{
 int i;
 if (argc!=2)
   {
    printf("Use: mft path\n");
    return 1;
   }
 extra=strlen(argv[1]);
 ftw(argv[1],func,1);
 return 0;
}
