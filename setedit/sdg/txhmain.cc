/*****************************************************************************

  Copyright (c) 1997-2003 by Salvador Eduardo Tropea.

  That's an example of interface with SDG.
  In this case the program is a command line one.

  Important:
  SDGLib uses SET TV Utils and bufun.cc.
  This example uses a special bufun.cc to avoid linking half of TVision to
the program.

*****************************************************************************/
#include <ceditint.h>

#ifdef TVCompf_djgpp
#include <io.h>
#endif
#include <stdio.h>
#define Uses_string
#define Uses_filelength
#include <stdlib.h>
#include <txhgen.h>
#define Uses_TMLISDGDefs
#include <mli.h>
#include <tv.h>

static int CountFiles;
static int CantFiles;
static char **Files;

#ifdef SECompf_djgpp
#include <dpmi.h>
#include <go32.h>
#include <fcntl.h>
const int maxSFNSize=68;

/**[txh]********************************************************************

  Description:
  Returns the short file name of the passed file. The shortName pointer must
point to a buffer with at least maxSFNSize bytes.

  Return:
  A pointer to the converted name if succesful or the original name if it
fails.

***************************************************************************/

char *GetShortNameOf(char *longName, char *shortName)
{
 __dpmi_regs r;
 unsigned long tbuf=__tb;

 r.x.ax=0x7100;
 if (_USE_LFN)
   {
     dosmemput (longName,strlen(longName)+1,tbuf+maxSFNSize);
     r.x.ax=0x7160;
     r.x.es=r.x.ds=tbuf >> 4;
     r.x.di=0;
     r.x.si=maxSFNSize;
     r.x.cx=0x8001;
     __dpmi_int(0x21, &r);
   }
 if ((r.x.flags & 1)==0 && r.x.ax!=0x7100)
   {
    dosmemget(tbuf,maxSFNSize,shortName);
    return shortName;
   }
 return longName;
}
#else
char *GetShortNameOf(char *longName, char *)
{
 return longName;
}
#endif


/**[txh]**********************************************************************

  Description:
  This function must be provided by the program. Your purpose is return the
next buffer to process by SDG.
@p
  The function must fill l with the length of the buffer, MustBeDeleted with
1 or 0 according to the kind of buffer and FileName with the name of the file.
The name must be provided as you want it appears in the generated docs.

  Return:
  A pointer to the buffer with the C source or NULL if no more files are
provided.

*****************************************************************************/

static char *GetNextFile(int &l, int &MustBeDeleted, char *FileName)
{
 FILE *f;
 char *buffer,*pos;

 if (CountFiles<CantFiles)
   {
    CountFiles++;
    // Read the test file
    f=fopen(Files[CountFiles],"rt");
    if (!f)
      {
       printf("Failed to open the file %s\n",Files[CountFiles]);
       return NULL;
      }
   
    l=filelength(fileno(f))+1;
    buffer=new char[l];
    if (!buffer)
       return NULL;
    fread(buffer,l,1,f);
    buffer[l-1]=0;
    fclose(f);

    MustBeDeleted=1;
    // Let just the filename
    pos=strrchr(Files[CountFiles],'/');
    if (pos)
       pos++;
    else
       pos=Files[CountFiles];
    strcpy(FileName,pos);
    return buffer;
   }
 return NULL;
}

void PrintMessage(char *s)
{
 puts(s);
}

int main(int argc, char *argv[])
{
 // MUST BE ON or some frt files will fail
 #ifdef SECompf_djgpp
 __system_flags|=__system_allow_multiple_cmds;
 #endif

 // These variables controls SDG
 TXHGetNextFile=GetNextFile;
 TXHKeepTemporal=1;
 TXHFormatFile="txhgen.txt";
 TXHTempGenFile="Generated.txh";
 TXHOutBaseName="out";
 TXHPrintMessage=PrintMessage;

 printf("SET's Documentation Generator (SDG)\nCopyright (c) 1997,2003 by Salvador E. Tropea\n");
 if (argc==1)
   {
    printf("\nUse: sdg list_of_sources\n\n");
    return 1;
   }
 CountFiles=0;
 Files=argv;
 CantFiles=argc-1;

 // That's an example on how to decode the an error, isn't so easy
 int error;
 if ((error=TXHGenerateAll())!=0)
   {
    printf("Error while %s:\n",TXHGetErrorSection(error));
    switch (error)
      {
       case 1:
            printf("(%d) %s in line %d\n",TXHError,TXHGetErrorMessage(),TXHLine);
            break;
       case 0:
       case 2:
            printf("(%d) %s\n",TXHError,TXHGetErrorMessage());
            break;
       case 3:
            printf("In section %s\n",TXHGetGenSect());
            printf("Error of type: %s\n",MLISDGTypeError);
            printf("%s\n",MLISDGErrorName);
            printf("Code: %s\n",MLISDGErrorCode);
            break;
      }
   }
 return 0;
}
