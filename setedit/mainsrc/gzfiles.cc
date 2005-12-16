/* Copyright (C) 1996-2005 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#define Uses_stdio
#define Uses_stdlib
#define Uses_unistd
#define Uses_fcntl
#define Uses_sys_stat
#define Uses_string
#define Uses_ctype
#define Uses_snprintf
#define Uses_AllocLocal
#define Uses_MsgBox

// EasyDiag requests
#define Uses_TSButton
#define Uses_TSStaticText
#define Uses_TSInputLine
#define Uses_TSLabel

#define Uses_TCEditor_External

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
//#include <tv.h>
// Second request the headers
#include <easydiag.h>
#include <editcoma.h>
#include <diaghelp.h>
#include <rhutils.h>
#include <advice.h>

#ifdef SUP_GZ
#include <zlib.h>
#endif
#include <gzfiles.h>
#ifdef HAVE_GPG
#include <sys/wait.h>
#endif

static int CheckGPGInstalled();

static char *gzError=NULL;

void GZFiles_ResetError()
{
 delete[] gzError;
 gzError=NULL;
}

const char *GZFiles_GetError()
{
 return gzError;
}

void GZFiles_SetError(const char *error)
{
 delete[] gzError;
 gzError=newStr(error);
}

const int eachRead=16300;

#ifdef SUP_GZ
static
int GZFiles_ExpandGZ(char *dest, char *orig)
{
 // First I alloc a 16Kb buffer
 char *buf=new char[eachRead];
 if (!buf)
    return 1;

 gzFile file;
 file=gzopen(orig,"rb");
 if (file==NULL)
   {
    delete buf;
    return 2;
   }

 FILE *f;
 f=fopen(dest,"wb");
 if (!f)
   {
    gzclose(file);
    delete buf;
    return 3;
   }

 int read;
 do
   {
    read=gzread(file,buf,eachRead);
    fwrite(buf,read,1,f);
   }
 while(read==eachRead);

 fclose(f);
 gzclose(file);
 DeleteArray(buf);

 return 0;
}

static
int GZFiles_SimpleCopy(char *dest, char *orig)
{
 // First I alloc a 16Kb buffer
 char *buf=new char[eachRead];
 if (!buf)
    return 1;

 FILE *file;
 file=fopen(orig,"rb");
 if (file==NULL)
   {
    delete buf;
    return 2;
   }

 FILE *f;
 f=fopen(dest,"wb");
 if (!f)
   {
    fclose(file);
    delete buf;
    return 3;
   }

 int read;
 do
   {
    read=fread(buf,1,eachRead,file);
    fwrite(buf,read,1,f);
   }
 while(read==eachRead);

 fclose(f);
 fclose(file);
 delete buf;

 return 0;
}
#endif // SUP_GZ

#ifdef HAVE_BZIP2
#ifdef HAVE_BZIP2PRE1
#define BZ2_bzReadOpen bzReadOpen
#define BZ2_bzReadClose bzReadClose
#define BZ2_bzRead bzRead
#define BZ2_bzWriteOpen bzWriteOpen
#define BZ2_bzWriteClose bzWriteClose
#define BZ2_bzWrite bzWrite
#endif
static
int GZFiles_ExpandBZ2(char *dest, char *orig)
{
 // First I alloc a 16Kb buffer
 char *buf=new char[eachRead];
 if (!buf)
    return 1;

 FILE *fi;
 fi=fopen(orig,"rb");
 if (fi==NULL)
   {
    delete buf;
    return 2;
   }

 BZFILE *file;
 int bzError;
 // Small=1 => slow, silence
 file=BZ2_bzReadOpen(&bzError,fi,1,0,NULL,0);
 if (bzError!=BZ_OK)
   {
    fclose(fi);
    delete buf;
    return 2;
   }

 FILE *f;
 f=fopen(dest,"wb");
 if (!f)
   {
    BZ2_bzReadClose(&bzError,file);
    fclose(fi);
    delete[] buf;
    return 3;
   }

 int read;
 do
   {
    read=BZ2_bzRead(&bzError,file,buf,eachRead);
    fwrite(buf,read,1,f);
   }
 while(bzError==BZ_OK);

 fclose(f);
 BZ2_bzReadClose(&bzError,file);
 fclose(fi);
 delete[] buf;

 return 0;
}
#endif // HAVE_BZIP2


int GZFiles_ExpandHL(char *dest, char *orig)
{
 int ret=-1;

 FILE *f=fopen(orig,"rb");
 int mode=GZFiles_IsGZ(f);
 fclose(f);

 switch (mode)
   {
    case gzNoCompressed:
         ret=GZFiles_SimpleCopy(dest,orig);
         break;

    #ifdef SUP_GZ
    case gzGZIP:
         // If libz is linked use this routine
         ret=GZFiles_ExpandGZ(dest,orig);
         break;
    #endif

    #ifdef HAVE_BZIP2
    case gzBZIP2:
         ret=GZFiles_ExpandBZ2(dest,orig);
         break;
    #endif

    #ifdef HAVE_GPG
    case gzGPG:
         ret=GZFiles_DecryptGPG(dest,orig);
         break;
    #endif
   }

 return ret;
}

int GZFiles_IsGZ(FILE *f)
{
 unsigned val=0;
 long pos=ftell(f);

 #ifdef SUP_GZ
 // Try with gzip
 fread(&val,2,1,f);
 fseek(f,pos,SEEK_SET);
 if (val==0x8B1F)
    return gzGZIP;
 #endif

 #ifdef HAVE_BZIP2
 // Try Bzip2
 char b[4];
 fread(b,4,1,f);
 fseek(f,pos,SEEK_SET);
 if (strncmp(b,"BZh",3)==0 && ucisdigit(b[3]))
    return gzBZIP2;
 #endif

 #ifdef HAVE_GPG
 // GPG encrypted
 char bgpg[27];
 fread(bgpg,27,1,f);
 fseek(f,pos,SEEK_SET);
 if (strncmp(bgpg,"-----BEGIN PGP MESSAGE-----",27)==0)
    return CheckGPGInstalled() ? gzGPG : gzNoCompressed;
 #endif

 return gzNoCompressed;
}

TGZFileWrite::TGZFileWrite(char *fileName, int comp)
{
 switch (comp)
   {
    case gzNoCompressed:
         f=fopen(fileName,"wb");
         ok=f!=NULL;
         break;

    #ifdef SUP_GZ
    case gzGZIP:
         fc=gzopen(fileName,"wb9");
         ok=fc!=NULL;
         break;
    #endif

    #ifdef HAVE_BZIP2
    case gzBZIP2:
         f=fopen(fileName,"wb");
         ok=f!=NULL && ferror(f)==0;
         if (ok)
           {
            int bzError;
            // Block: 900Kb (no memory conservative), silent, default workFactor
            // I don't know if that's correct but is the default of the
            // standalone tool.
            fc2=BZ2_bzWriteOpen(&bzError,f,9,0,0);
            ok=fc2!=NULL && bzError==BZ_OK;
           }
         break;
    #endif

    #ifdef HAVE_GPG
    case gzGPG:
         ok=GZFiles_CreateGPG(fileName,hi,ho,he,child);
         break;
    #endif
   }
 compressed=comp;
}

void TGZFileWrite::close()
{
 switch (compressed)
   {
    case gzNoCompressed:
         if (f)
           {
            fclose(f);
            f=NULL;
           }
         break;

    #ifdef SUP_GZ
    case gzGZIP:
         if (fc)
           {
            gzclose(fc);
            fc=NULL;
           }
         break;
    #endif

    #ifdef HAVE_BZIP2
    case gzBZIP2:
         if (ok)
           {
            int bzError;
            BZ2_bzWriteClose(&bzError,fc2,0,0,0);
           }
         if (f)
           {
            fclose(f);
            f=NULL;
           }
         break;
    #endif

    #ifdef HAVE_GPG
    case gzGPG:
         if (hi!=-1)
           {
            ok=!GZFiles_CloseGPG(hi,ho,he,child);
            hi=-1;
           }
         break;
    #endif
   }
}

TGZFileWrite::~TGZFileWrite()
{
}

size_t TGZFileWrite::write(void *buffer, size_t len)
{
 if (!ok || len==0) return 0;

 size_t ret;
 switch (compressed)
   {
    case gzNoCompressed:
         ret=fwrite(buffer,len,1,f);
         if (ret<1) ok=0;
         break;

    #ifdef SUP_GZ
    case gzGZIP:
         ret=gzwrite(fc,buffer,len);
         if (ret==0)
           {
            ok=0; ret=(size_t)-1;
           }
         break;
    #endif

    #ifdef HAVE_BZIP2
    case gzBZIP2:
         {
          int bzError;
          BZ2_bzWrite(&bzError,fc2,buffer,len);
          if (bzError!=BZ_OK)
            {
             ok=0; ret=(size_t)-1;
            }
          else
             ret=len;
         }
         break;
    #endif

    #ifdef HAVE_GPG
    case gzGPG:
         ret=::write(hi,buffer,len);
         //printf("len=%d , ret=%d\n",len,ret);
         break;
    #endif

    default:
         ret=0;
   }
 return ret;
}

/*****************************************************************************
  GPG stuff. Experimental and not secure at all, use at your own risk.
  Only for Linux.
*****************************************************************************/

#ifdef HAVE_GPG
const unsigned maxPhraseLen=256;
// Cached passphrase
static char gpgPassPhrase[maxPhraseLen];
// Do we have a phrase to try?
static int  gpgValidPhrase=0;
// User ID to encrypt
static ulong gpgUserID;

const int sysError=-1;
const int gpgEachRead=32768;
const char *gpgCommand="/usr/bin/gpg";
const char *gpgCLine=" --passphrase-fd 0 --no-verbose --batch --output - ";
const char *gpgWCLine="%s --batch --quiet --no-verbose --output %s --encrypt --textmode --armor --always-trust -r 0x%lX";

static
TDialog *createGetPassPhrase(int len)
{
 TSViewCol *col=new TSViewCol(__("Validation"));

 TSInputLine *il=new TSInputLine(len,GetDeskTopCols()-8);
 il->setHide(True);
 TSLabel *label=new TSLabel(__("Passphrase"),il);
 col->insert(xTSCenter,yTSUp,label);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(hcPassphrase);
 delete col;
 return d;
}

static
char *gpgGetPhrase()
{
 if (gpgValidPhrase)
    return gpgPassPhrase;
 if (execDialog(createGetPassPhrase(maxPhraseLen),gpgPassPhrase)==cmOK)
    return gpgPassPhrase;

 return NULL;
}


/**[txh]********************************************************************

  Description:
  A popen replacement to redirect stdin, stdout and stderr.
  
  Return: The stdin handle on success, -1 on error.
  Example: 
  
***************************************************************************/

static
int gpgPopen(const char *command, pid_t &pid, int &wrh, int &rde)
{
 int rc, wr, er;
 int pipefd[2], pipewr[2], pipeer[2];

 rc=pipe(pipefd);
 if (rc==sysError)
    return sysError;
 wr=pipe(pipewr);
 if (wr==sysError)
   {
    close(pipefd[0]);
    close(pipefd[1]);
    return sysError;
   }
 er=pipe(pipeer);
 if (er==sysError)
   {
    close(pipewr[0]);
    close(pipewr[1]);
    close(pipefd[0]);
    close(pipefd[1]);
    return sysError;
   }

 pid=fork();
 switch (pid)
   {
    case sysError:
         close(pipeer[0]);
         close(pipeer[1]);
         close(pipewr[0]);
         close(pipewr[1]);
         close(pipefd[0]);
         close(pipefd[1]);
         return sysError;
      
    case 0: /* Child */
         close(pipefd[0]);
         dup2(pipefd[1],STDOUT_FILENO);
         close(pipefd[1]);

         close(pipeer[0]);
         dup2(pipeer[1],STDERR_FILENO);
         close(pipeer[1]);

         close(pipewr[1]);
         dup2(pipewr[0],STDIN_FILENO);
         close(pipewr[0]);
         /*
          * The System() call assumes that /bin/sh is
          * always available, and so will we.
          */
         execl("/bin/sh", "/bin/sh", "-c", command, NULL);
         _exit(EXIT_FAILURE);
         break;
      
    default: /* Parent */
         close(pipefd[1]);
         rc=pipefd[0];
         close(pipeer[1]);
         rde=pipeer[0];
         close(pipewr[0]);
         wrh=pipewr[1];
         break;
   } /* switch */
 return rc;
}

/**[txh]********************************************************************

  Description:
  Gets a line of text from the child.
  By breese.
  
  Return: 
  The ammount of bytes read.
  
***************************************************************************/
static
int gpgGetline(int fd, char *buffer, int max)
{
 char c;
 int i=0;
 
 do
   {
    if (read(fd,&c,1)<1)
       return 0;
    if (i<max)
       buffer[i++]=c;
   }
 while (c!='\n');
 buffer[i]=(char)0;
 return i;
}

/**[txh]********************************************************************

  Description:
  Closes the pipe and ensures the child dies.
  By SET.
  
***************************************************************************/
static
int gpgPclose(int fd, int wp, int we, int pid)
{
 int wstatus;
 /* Close the pipe, it should kill the child with SIGPIPE */
 close(fd);
 close(wp);
 close(we);
 waitpid(pid,&wstatus,0);
 return wstatus>256 ? (wstatus>>8) : wstatus;
}

// TODO: Send the error to the message window. Tell the user, a callback?
int GZFiles_DecryptGPG(char *dest, char *orig)
{
 // Get the passphrase
 char *pass=gpgGetPhrase();
 if (!pass)
   {
    GZFiles_SetError(__("No passphrase available"));
    return 1;
   }
 gpgValidPhrase=0;

 // Alloc a 32Kb buffer
 char *buf=new char[gpgEachRead];
 if (!buf)
   {
    GZFiles_SetError(__("Out of memory"));
    return 2;
   }

 // Create destination
 FILE *f;
 f=fopen(dest,"wb");
 if (!f)
   {
    delete[] buf;
    GZFiles_SetError(__("Can't create temporal file"));
    return 3;
   }

 // Pipe to GPG
 pid_t child;
 int wr, er;
 char *aux=NULL;
 string_cat(aux,gpgCommand,gpgCLine,orig,NULL);
 int h=gpgPopen(aux,child,wr,er);
 string_free(aux);
 if (h<0)
   {
    fclose(f);
    delete[] buf;
    GZFiles_SetError(__("Failed to invoke gpg"));
    return 4;
   }

 // Send the passphrase
 write(wr,pass,strlen(pass));
 write(wr,"\n",1);

 // Copy the output from GPG
 int l;
 while ((l=gpgGetline(h,buf,gpgEachRead))!=0)
    fwrite(buf,l,1,f);

 // Analyze error messages
 int foundId=0;
 while (gpgGetline(er,buf,gpgEachRead))
   {
    printf("%s",buf);
    if (strncmp(buf,"gpg: encrypted with",19)==0)
      {
       char *name=strstr(buf,", ID ");
       if (name)
         {
          char *end;
          gpgUserID=strtoul(name+5,&end,16);
          foundId=1;
         }
      }
   }

 // Close the pipe
 int gpgStatus=gpgPclose(h,wr,er,child);
 // Check if we have a valid passphrase
 if (gpgStatus)
    GZFiles_SetError(__("Error while decrypting"));
 else
    if (!foundId)
      {
       GZFiles_SetError(__("Failed to determine user ID"));
       gpgStatus=-1;
      }
    else
       gpgValidPhrase=1;
 fclose(f);
 delete[] buf;

 return gpgStatus;
}

int GZFiles_CreateGPG(const char *file, int &hi, int &ho, int &he, pid_t &child)
{
 int len=CLY_snprintf(NULL,0,gpgWCLine,gpgCommand,file,gpgUserID);
 AllocLocalStr(buf,len+1);
 CLY_snprintf(buf,len+1,gpgWCLine,file,gpgUserID);

 unlink(file);
 // Pipe to GPG
 ho=gpgPopen(buf,child,hi,he);
 if (ho<0)
   {
    GZFiles_SetError(__("Failed to invoke gpg"));
    return 0;
   }

 return 1;
}


int GZFiles_CloseGPG(int hi, int ho, int he, pid_t child)
{
 /*char b[80];
 while (gpgGetline(he,b,80))
   {
    printf("%s",b);
   }*/
 return gpgPclose(hi,ho,he,child);
}


static
int CheckGPGInstalled()
{
 struct stat st;
 static int informed=0;

 if (stat(gpgCommand,&st)==0)
    return 1;
 if (!informed)
   {
    informed=1;
    GiveAdvice(gadvGPG);
   }
 return 0;
}
#endif

