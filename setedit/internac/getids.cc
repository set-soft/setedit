/**[txh]********************************************************************

  Copyright (c) 2003 by Salvador E. Tropea
  Part of setedit package.
  Covered by the GPL license.

  Description:
  Program to extract the msgids from a file.
  Used to create tvision.cc.
  
***************************************************************************/

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
 FILE *f=fopen(argv[1],"rt");
 char b[200],*s;
 while (fgets(b,199,f))
   {
    if (strncmp(b,"msgid",5)==0)
      {
       for (s=b+6; *s!='\n' && *s!='\r'; s++); *s=0;
       printf("__(%s",b+6);
       while (fgets(b,199,f))
         {
          if (strncmp(b,"msgstr",6)==0)
            {
             printf(")\n");
             break;
            }
          else
            {
             for (s=b+6; *s!='\n' && *s!='\r'; s++); *s=0;
             printf("\n%s",b);
            }
         }
      }
   }
 return 0;
}
