/**[txh]********************************************************************

  Copyright (c) 2002 by Salvador Eduardo Tropea
  Covered by the GPL license.
  
  Description:
  This is a very simple and limited converter that creates an SFT file from
a raw font. The raw font *must* be 8 pixels wide and *must* be encoded in
code page CP 437. It also *must* contain 256 symbols.
  Only for little endian!!
  
***************************************************************************/

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
 if (argc!=4)
   {
    printf("Usage: %s raw_font new_sft_file fantasy_name\n",argv[0]);
    return 1;
   }
 FILE *f=fopen(argv[1],"rb");
 if (!f)
   {
    printf("Can't open %s\n",argv[1]);
    return 2;
   }
 FILE *d=fopen(argv[2],"wb");
 if (!d)
   {
    printf("Can't create %s\n",argv[2]);
    return 3;
   }
 fprintf(d,"SET's editor font%c",26);

 fseek(f,0,SEEK_END);
 int length=ftell(f);
 fseek(f,0,SEEK_SET);

 int aux=1;
 fwrite(&aux,4,1,d);
 aux=1;
 fwrite(&aux,4,1,d);
 aux=strlen(argv[3])+1;
 fwrite(&aux,2,1,d);
 fwrite(argv[3],aux-1,1,d);
 aux=0;
 fwrite(&aux,4,1,d);
 aux=255;
 fwrite(&aux,4,1,d);
 aux=length/256;
 fwrite(&aux,4,1,d);

 char b[length];
 fread(b,length,1,f);
 fwrite(b,length,1,d);

 return 0;
}
