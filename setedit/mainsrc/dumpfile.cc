/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_stdio
#define Uses_unistd
#define Uses_string
#define Uses_sys_stat
#include <tv.h> // Boolean typedef
#include <edmsg.h>

int DumpFileToMessage(char *file, const char *from, uint32 SMOps,
                      char *(*ParseFun)(char *buf,FileInfo &fI,char *&fileName),
                      int kill)
{
 FILE *f;
 char buf[1024];
 int l;
 FileInfo fInfo;
 char *fileName;
 int count=0;

 f=fopen(file,"rt");
 if (f)
   {
    fgets(buf,1000,f);
    if (!feof(f))
      {
       EdShowMessageI(from);
       do
         {
          l=strlen(buf);
          if (buf[l-1]=='\n')
             buf[l-1]=0;
          if (ParseFun)
            {
             char *s=ParseFun(buf,fInfo,fileName);
             if (s)
               {
                EdShowMessageFile(s,fInfo,fileName,SMOps);
                delete[] s;
                delete[] fileName;
                if (fileName)
                   count++;
               }
            }
          else
             EdShowMessage(buf,SMOps);
         }
       while (fgets(buf,1000,f));
      }
    fclose(f);
    if (kill)
       unlink(file);
   }
 return count;
}

/*****************************************************************************
  Incremental dumper
*****************************************************************************/

static char *incFile;
static char *(*incParseFun)(char *buf,FileInfo &fI,char *&fileName);
static FILE *incf;
static char incFinished;
static uint32 incSMOps;

/**[txh]********************************************************************

  Description:
  Initializes the incremental dumper. It just sets some internal variables
(to avoid passing them all the time) and prints the starting message.
  
***************************************************************************/

void DumpFileToMessageInit(char *file, const char *from, uint32 SMOps,
                           char *(*ParseFun)(char *buf,FileInfo &fI,char *&fileName))
{
 EdShowMessageI(from);
 incParseFun=ParseFun;
 incFile=file;
 incSMOps=SMOps;
 incf=0;
 incFinished=0;
}

/**[txh]********************************************************************

  Description:
  Dumps the next line to the message window.
  
  Return:
  !=0 if we finished parsing or we found an error or there are nothing to
  parse.
  
***************************************************************************/

int  DumpFileToMessageParseMore(int Lines, int *goBack)
{
 if (incFinished)
    return 1;
 if (!incf)
   {// We didn't open the file yet
    struct stat st;
    if (stat(incFile,&st))
      {// If the file doesn't exist something went wrong
       incFinished=1; // Don't try again
       //fprintf(stderr,"Error: Stat sobre %s falló\n",incFile);
       return 1;
      }
    if (st.st_size==0)
      {
       // If the file is empty then we finished or we must
       // wait until it grows
       //fprintf(stderr,"Warning: Largo 0\n");
       return 1;
      }
    // Ok, the file is there and have something
    incf=fopen(incFile,"rt");
    if (!incf)
      {// Ugh!
       incFinished=1; // Don't try again
       //fprintf(stderr,"Error: al abrir %s\n",incFile);
       return 1;
      }
   }

 // Get a line, if nothing is there return
 char buf[1024];
 int lines=0,ret=0;
 while (lines<Lines)
   {
    clearerr(incf);
    fgets(buf,1000,incf);
    if (feof(incf))
      {
       ret=1;
       break;
      }
   
    // Add it
    lines++;
    int l=strlen(buf);
    if (buf[l-1]=='\n')
       buf[l-1]=0;
    if (incParseFun)
      {
       FileInfo fInfo;
       char *fileName;
       char *s=incParseFun(buf,fInfo,fileName);
       if (s)
         {
          EdShowMessageFile(s,fInfo,fileName,edsmUpdateSpLines | edsmDontUpdate);
          delete s;
          delete fileName;
          if (fileName)
             (*goBack)++;
         }
      }
    else
       EdShowMessage(buf,edsmDontUpdate);
   }
 if (lines)
    EdShowMessageUpdate(edsmDontSelect | incSMOps);
 return ret;
}

/**[txh]********************************************************************

  Description:
  Indicates we don't want to continue dumping so the routines can close the
file.
  
***************************************************************************/

void DumpFileToMessageEnd()
{
 if (incf)
    fclose(incf);
 incFinished=1;
}
