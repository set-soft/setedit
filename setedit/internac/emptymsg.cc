/**[txh]********************************************************************

  Copyright (c) 2001 by Salvador E. Tropea (SET) set@ieee.org
  Covered by the GPL license, you should get a copy of it with this file.

  Description:
  This simple program searchs for empty or fuzzy entries in a .po file and
generates a report in the same style used by GNU tools. Running it from
setedit you'll be able to jump forward and backward in the report using
Alt+F7/F8 keys.
  
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char uchar;

int main(int argc, char *argv[])
{
 if (argc<2)
   {
    printf("Empty message searcher. Copyright (c) 2001 by Salvador E. Tropea <set@ieee.org>\n");
    printf("Covered by the GPL license, you should get a copy of it with this file.");
    printf("\nUse: emptymsg files...\n");
    return 1;
   }
 int i;
 for (i=1; i<argc; i++)
    {
     FILE *f=fopen(argv[i],"rt");
     if (!f)
       {
        fprintf(stderr,"Failed to open %s\n",argv[i]);
        continue;
       }
    
     char b[200],*s,NextMsgStrFuzzy=0;
     int line=0;
     while (!feof(f))
       {
        if (fgets(b,199,f))
          {
           line++;
           if (strncmp(b,"msgstr",6)==0)
             {
              if (NextMsgStrFuzzy)
                {
                 printf("%s:%d: fuzzy\n",argv[i],line);
                 NextMsgStrFuzzy=0;
                }
              else
                {
                 for (s=b+6; *s && isspace((uchar)*s); s++);
                 if (*s && *s=='"' && s[1]=='"')
                   {// Candidate
                    if (fgets(b,199,f))
                      {
                       if (b[0]!='"')
                          printf("%s:%d: empty\n",argv[i],line);
                       line++;
                      }
                   }
                }
             }
           else
             if (strncmp(b,"#, fuzzy",8)==0)
                NextMsgStrFuzzy=1;
          }
       }
     fclose(f);
    }
 return 0;
}
