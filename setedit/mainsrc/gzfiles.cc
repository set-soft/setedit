/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#define Uses_stdio
#define Uses_stdlib
#define Uses_unistd
#define Uses_fcntl
#define Uses_sys_stat
#define Uses_string
#define Uses_ctype
#define Uses_MsgBox
#include <tv.h>

#ifdef SUP_GZ
#include <zlib.h>
#endif
#include <gzfiles.h>

#if !defined(TVOSf_Linux) && !defined(SUP_GZ)
/*
  This routine checks if gzip is there. If we can't find it we must put a warning
*/
int GZFiles_CheckForGZIP(void)
{
 static isGZIPInstalled=0;

 if (!isGZIPInstalled)
   {
    int h_err=open("/dev/null",O_WRONLY | O_CREAT | O_TRUNC,S_IREAD | S_IWRITE);
    int h_errbak=dup(2);
    fflush(stderr);  /* so any buffered chars will be written out */
    dup2(h_err,2);
    int ret=TV_System("gzip");
    dup2(h_errbak,2);
    close(h_err);
    close(h_errbak);

    if (ret>0)
       isGZIPInstalled=1;
    else
       messageBox(__("You must install gzip to browse compressed files!"), mfError | mfOKButton);
   }

 return isGZIPInstalled;
}
#endif

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
    delete buf;
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
 delete buf;

 return 0;
}
#endif // HAVE_BZIP2
#endif // SUP_GZ


int GZFiles_ExpandHL(char *dest, char *orig)
{
 int ret=0;

#ifdef SUP_GZ

 FILE *f=fopen(orig,"rb");
 int mode=GZFiles_IsGZ(f);
 fclose(f);

 switch (mode)
   {
    case gzNoCompressed:
         ret=GZFiles_SimpleCopy(dest,orig);
         break;

    case gzGZIP:
         // If libz is linked use this routine
         ret=GZFiles_ExpandGZ(dest,orig);
         break;

    #ifdef HAVE_BZIP2
    case gzBZIP2:
         ret=GZFiles_ExpandBZ2(dest,orig);
         break;
    #endif
   }


#else // SUP_GZ

 //---- NO libz call gzip

 char Buf2[PATH_MAX*3];
# ifndef TVOSf_Linux

 // Just run the gzip and get your output in __infc__
 sprintf(Buf2,"gzip -d -c %s > %s",orig,dest);
 if (GZFiles_CheckForGZIP())
    TV_System(Buf2);

# else // TVOSf_Linux
 // Not so easy, here we create another process, extract your output and copy
 // it to __infc__, the DOS technique doesn't work well.
 FILE *f,*d;

 strcpy(Buf2,"gzip -d -c ");
 strcat(Buf2,orig);
 f=popen(Buf2,"r");
 #ifdef DEBSTD
 fprintf(stderr,"Piping: %s returns %s\n",Buf2,f==NULL ? "fail" : "ok");
 #endif
 if (!f)
    return 1;
 if ((d=fopen(dest,"wb"))==NULL) return 0;
 do
  {
   fputc(fgetc(f),d);
  }
 while(!feof(f));
 fclose(d);
 pclose(f);
 strcpy(orig,dest);

# endif // !TVOSf_Linux
#endif // !SUP_GZ

 return ret;
}

int GZFiles_IsGZ(FILE *f)
{
 unsigned val=0;
 long pos=ftell(f);

 // Try with gzip
 fread(&val,2,1,f);
 fseek(f,pos,SEEK_SET);
 if (val==0x8B1F)
    return gzGZIP;

 #ifdef HAVE_BZIP2
 // Try Bzip2
 char b[4];
 fread(b,4,1,f);
 fseek(f,pos,SEEK_SET);
 if (strncmp(b,"BZh",3)==0 && ucisdigit(b[3]))
    return gzBZIP2;
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

    case gzGZIP:
         fc=gzopen(fileName,"wb9");
         ok=fc!=NULL;
         break;

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
   }
 compressed=comp;
}

TGZFileWrite::~TGZFileWrite()
{
 switch (compressed)
   {
    case gzNoCompressed:
         if (f)
            fclose(f);
         break;

    case gzGZIP:
         if (fc)
            gzclose(fc);
         break;

    #ifdef HAVE_BZIP2
    case gzBZIP2:
         if (ok)
           {
            int bzError;
            BZ2_bzWriteClose(&bzError,fc2,0,0,0);
           }
         if (f)
            fclose(f);
         break;
    #endif
   }
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

    case gzGZIP:
         ret=gzwrite(fc,buffer,len);
         if (ret==0)
           {
            ok=0; ret=(size_t)-1;
           }
         break;

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
    default:
         ret=0;
   }
 return ret;
}

