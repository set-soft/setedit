/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
   
#define Uses_sys_stat
#define Uses_string
#define Uses_fnmatch
#define Uses_stdlib
#define Uses_unistd
#define Uses_limits
#define Uses_fcntl
#define Uses_mkstemp
#define Uses_dirent
#define Uses_stdio
#ifdef SEOSf_djgpp
#include <io.h>    // _chmod
#include <dpmi.h>  // GetShortNameOf: dpmi_regs, dpmi_int
#include <go32.h>  // GetShortNameOf: transfer buffer
#endif
#ifdef __TURBOC__
#include <io.h>
#endif
#include <rhutils.h>
#define Uses_TScreen
#define Uses_TStringCollectionW
#include <settvuti.h>

#ifdef SEOS_Win32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

extern const char *GetVariable(const char *variable);
static char PathOrig[PATH_MAX];
static char FileNameToLoad[PATH_MAX];
static char *PathOrigPos;

int IsADirectory(const char *name);

int edTestForFile(const char *name)
{
 struct stat st;

 if (stat(name,&st)==0)
    return S_ISREG(st.st_mode);
 return 0;
}

char *GetHome(void)
{
 char *s=getenv("HOME");
 if (!s)
   {
    s=getenv("HOMEDIR");
    if (!s)
       s=".";
   }
 return s;
}

static
char *ExpandFileNameToUserHomeNoDot(const char *s)
{
 strcpy(FileNameToLoad,GetHome());
 strcat(FileNameToLoad,"/");
 strcat(FileNameToLoad,s);
 return FileNameToLoad;
}

#ifdef NoHomeOrientedOS
char *ExpandHomeSave(const char *s)
{
 return ExpandHome(s);
}
#else
static
char *ExpandHomeWDir(const char *s)
{
 static int HiddenDirFailed=0;
 char *h=GetHome();
 if (!HiddenDirFailed)
   {// Check ~/.setedit/
    strcpy(FileNameToLoad,h);
    strcat(FileNameToLoad,"/.setedit");
    if (!IsADirectory(FileNameToLoad))
      {// No? try to create it. 0750 mode
       if (mkdir(FileNameToLoad,S_IRWXU | S_IRGRP | S_IXGRP)==-1)
         {// Oops! something went wrong
          HiddenDirFailed=1;
         }
      }
    if (!HiddenDirFailed)
      {// Ok see if we can write
       if (access(FileNameToLoad,W_OK)==0)
         {
          strcat(FileNameToLoad,"/");
          strcat(FileNameToLoad,s);
          return FileNameToLoad;
         }
       // No write access?!
       HiddenDirFailed=1;
      }
   }
 return 0;
}

static
char *ExpandHomeCommon(const char *s)
{
 strcpy(FileNameToLoad,GetHome());
 strcat(FileNameToLoad,"/.");
 strcat(FileNameToLoad,s);
 return FileNameToLoad;
}

/**[txh]********************************************************************

  Description:
  This funtion returns a suitable place to write configuration files. Note
it doesn't have to be the same point as where we read it. In UNIX we will
read default values from /usr/share/setedit and store them in
~/.setedit/.file.
  
  Return: A pointer to a static buffer containing the path+file name.
  
***************************************************************************/

char *ExpandHomeSave(const char *s)
{
 char *r=ExpandHomeWDir(s);
 return r ? r : ExpandHomeCommon(s);
}
#endif

#ifdef STANDALONE
/**[txh]********************************************************************

  Description:
  This routine searchs the desired file in various directories and returns
the full qualified path for the location or the default location if the
file doesn't exist.@*
  Linux behavior: (a) The user's home (first in ~/.setedit/?, then ~/.? and
finally ~/?) (b) The SET_FILES directory.@*
  DOS behavior: (a) The user's home (b) The SET_FILES directory
(c) The directory where the editor was loaded. Note that (c) shoul not
happend.@*

  Return:
  The full path for the file found or the place where the file should be.

***************************************************************************/

char *ExpandFileNameToThePointWhereTheProgramWasLoaded(const char *s)
{
 const char *v;

 #ifndef NoHomeOrientedOS
 char *r;
 // That's only for UNIX, try a dot-file (hidden)
 r=ExpandHomeWDir(s);
 if (r && edTestForFile(r))
    return r;
 r=ExpandHomeCommon(s);
 if (edTestForFile(r))
    return r;
 #endif

 // Try it in DOS too, so the user can define HOME or HOMEDIR
 ExpandFileNameToUserHomeNoDot(s);
 if (edTestForFile(FileNameToLoad))
    return FileNameToLoad;
    
 // The directory where the original files are installed
 if ((v=GetVariable("SET_FILES"))!=NULL)
   {
    strcpy(FileNameToLoad,v);
    strcat(FileNameToLoad,"/");
    strcat(FileNameToLoad,s);
    return FileNameToLoad;
   }

 #ifndef SEOS_UNIX
 // The point where the editor was loaded doesn't have any sense in UNIX
 if (PathOrigPos!=NULL)
   {
    *PathOrigPos=0;
    strcat(PathOrig,s);
   }
 else
    strcpy(PathOrig,s);
 if (edTestForFile(PathOrig))
    return PathOrig;
 #endif

 return PathOrig;
}

int FindFile(const char * name,char * & fullName)
{
 char *base;

 if (strlen(name)>=PATH_MAX)
    return 0;
 fullName=new char[PATH_MAX];
 if (!fullName)
    return 0;
 strcpy(fullName,name);
 if (!edTestForFile(fullName))
   {
    base=getenv("DJDIR");
    if (!base)
      {
       delete fullName;
       return 0;
      }
    strcpy(fullName,base);
    strcat(fullName,"/include/");
    strcat(fullName,name);
    if (!edTestForFile(fullName))
      {
       delete fullName;
       return 0;
      }
   }
 CLY_fexpand(fullName);
 return 1;
}
#endif // STANDALONE

/**[txh]********************************************************************

  Description:
  Replaces old extension of name by the provided extension.

  Return:
  name

***************************************************************************/

char *ReplaceExtension(char *name, char *ext, char *old)
{
 char *pos;
 pos=strrchr(name,'/');
 if (!pos)
    pos=name;
 pos=strstr(pos,old);
 if (pos)
    strcpy(pos,ext);
 else
    strcat(name,ext);
 return name;
}

/**[txh]********************************************************************

  Description:
  Replaces the extension of name by the provided extension. If none is found
the extension is added and if the file name starts with dot the extension
is added too.

  Return:
  name.

***************************************************************************/

char *ReplaceExtension(char *name, char *ext)
{
 char *dot,*slash;
 int flen=strlen(name);

 dot=strrchr(name,'.');
 slash=strrchr(name,'/');
 if (dot<slash) // dot in directories, not in file
    dot=0;
 if (!dot || dot==slash+1) // .files should be .files.bkp and not .bkp ;-)
    dot=name+flen;
 strcpy(dot,ext);

 return name;
}

/**[txh]********************************************************************

  Description:
  That's used to create back-up file names with xxxx~ stile.

  Return:
  fname.

***************************************************************************/

char *AddToNameOfFile(char *fname, char *add)
{
 #ifdef SEOS_DOS
 //if (_use_lfn(fname))
 //   return strcat(fname,add);
 // Not so easy boy, 8+3 limitations:
 char *dir,*name,*ext;
 int lext;
 split_fname(fname,dir,name,ext);
 lext=strlen(ext);
 if (lext==0)
    string_cat(ext,".",add,0);
 else
 if (lext<4)
    string_cat(ext,add);
 else
   {
    for (lext=3; lext>1 && ext[lext]==*add; lext--);
    ext[lext]=*add;
   }
 strcpy(fname,dir);
 strcat(fname,name);
 strcat(fname,ext);
 string_free(dir);
 string_free(name);
 string_free(ext);
 return fname;
 #else
 // In UNIX that's much more easy
 return strcat(fname,add);
 #endif
}

int DeleteWildcard(char *mask)
{
 int deleted=0;
 DIR *d;
 d=opendir(".");
 
 if (d)
   {
    struct dirent *de;
    while ((de=readdir(d))!=0)
      {
       if (!fnmatch(mask,de->d_name,0))
         {
          unlink(de->d_name);
          deleted++;
         }
      }
    closedir(d);
   }
 return deleted;
}


char *GetPathRelativeToRunPoint(char *dest, const char *binReplace, char *file)
{
 char *ret;

 if (PathOrigPos!=NULL)
   {
    *PathOrigPos=0;
    strcpy(dest,PathOrig);
    char *s=strstr(dest,"bin");
    if (!s)
       s=strstr(dest,"BIN");
    if (s)
       strcpy(s,binReplace);
    ret=dest+strlen(dest)-1;
   }
 else
   {
    *dest=0;
    ret=dest;
   }

 strcat(dest,file);

 return ret;
}


void SetReferencePath(char *orig)
{
 PathOrigPos=strrchr(orig,'/');
 if (PathOrigPos)
   {
    PathOrigPos++;
    char oldVal=*PathOrigPos;
    *PathOrigPos=0;
    strcpy(PathOrig,orig);
    *PathOrigPos=oldVal;
    PathOrigPos=PathOrig+(int)(PathOrigPos-orig);
   }
 else
    PathOrig[0]=0;
}

char *RedirectStdErrToATemp(int &StdErrOri,int &StdErrNew)
{
 char aux[PATH_MAX];
 char *s=ExpandHomeSave("");
 char *ret=0;

 /*#ifdef __TURBOC__
 sprintf(aux,"%s/",s);
 StdErrNew=creattemp(aux, 0);
 #elif defined(__MINGW32__) || defined(_MSC_VER)
 sprintf(aux,"%s/erXXXXXX",s);
 char *tempName=mktemp(aux);
 StdErrNew=open(tempName,O_RDWR | O_CREAT,S_IRUSR | S_IWUSR);
 #else
 #endif*/
 sprintf(aux,"%serXXXXXX",s);
 StdErrNew=mkstemp(aux);
 if (StdErrNew>0)
   {
    ret=strdup(aux);
    StdErrOri=dup(fileno(stderr));
    fflush(stderr);
    dup2(StdErrNew,fileno(stderr));
   }
 return ret;
}

const int eachRead=16300;

int FileCopy(const char *orig, const char *dest)
{
 // First I alloc a 16Kb buffer
 char *buf=new char[eachRead];
 if (!buf)
    return 0;

 // Create destination
 FILE *d=fopen(dest,"wb");
 if (!d)
   {
    delete buf;
    return 0;
   }

 // Open source
 FILE *o=fopen(orig,"rb");
 if (!o)
   {
    fclose(d);
    delete buf;
    return 0;
   }

 int read;
 do
   {
    read=fread(buf,1,eachRead,o);
    fwrite(buf,read,1,d);
   }
 while (read==eachRead);

 int ret=!(ferror(o) || ferror(d));
 fclose(o);
 fclose(d);
 delete buf;

 return ret;
}

/**[txh]********************************************************************

  Description:
  That's a direct replacement for access(name,D_OK) which Linux lacks.

  Return:
  0 not a directory or no access allowed.

***************************************************************************/

int IsADirectory(const char *name)
{
 #ifdef SEOS_UNIX
 struct stat s;
 return stat(name,&s)==0 && S_ISDIR(s.st_mode) && !access(name,X_OK);
 #endif
 #ifdef SEOSf_djgpp
 return !access(name,D_OK);
 #endif
 #ifdef SEOS_Win32
 return CLY_IsDir(name)==True;
 #endif
}

/**[txh]********************************************************************

  Description:
  Checks if a file is a symbolic link.

  Return:
  0 not a link.

***************************************************************************/

int IsASoftLink(const char *name)
{
 #ifdef S_ISLNK
 struct stat s;
 return lstat(name,&s)==0 && S_ISLNK(s.st_mode);
 #else
 return 0;
 #endif
}

/**[txh]********************************************************************

  Description:
  TMPDIR variable could be not defined or pointing to an invalid drive. As
the librhutils assumes TMPDIR is working I must ensure it or the
stderr/stdout could misserably fail.

***************************************************************************/

void CheckForValidTMPDIR()
{
 char *tmp=getenv("TMPDIR");

 // If the variable is defined && is a directory all is ok
 if (!(tmp && IsADirectory(tmp)))
   {
    // Ok, this system lacks it or worst it point to nowhere
    tmp=getenv("TEMP");
    if (!tmp || !IsADirectory(tmp))
      { // No luck with TEMP
       tmp=getenv("TMP");
       if (!tmp || !IsADirectory(tmp))
         { // No luck with TMP
          tmp="/tmp"; // It should work in UNIX
          if (!IsADirectory(tmp))
            {
             tmp="c:/"; // It should work in DOS
             if (!IsADirectory(tmp))
               { // Hey! nothing is good!
                tmp="."; // Well, give up and try in current directory
               }
            }
         }
      }
   }

 char *b=new char[8+strlen(tmp)];
 sprintf(b,"TMPDIR=%s",tmp);
 #if defined(SEOSf_djgpp) || defined(__TURBOC__)
 // Mixing forward and backslashes could produce problems (path\/file is invalid).
 char *s=b;
 for (; *s; s++)
     if (*s=='\\')
        *s='/';
 #endif
 putenv(b);
 //fprintf(stderr,"Using: %s\n",tmp);
}


#ifdef SEOS_UNIX
static
char *MakeItHiddenName(char *file)
{
 // Look for the real name:
 char *name=strrchr(file,'/');
 if (name)
    name++;
 else
    name=file;
 // Already hidden?
 if (*name=='.')
    return 0;
 // Ugh! here UNIX is pretty bad:
 int l=strlen(file)+2;
 char *s=new char[l];
 strcpy(s,file);
 l=name-file;
 strcpy(s+l+1,name);
 s[l]='.';

 return s;
}
#endif

/**[txh]********************************************************************

  Description:
  Changes the status of the file to hidden. IMPORTANT! if the OS needs to
change the name of the file to make it hidden (UNIX case) the name provided
will be altered so it should have enough space for it.

  Return:
  0 succes, 1 fail

***************************************************************************/

int MakeFileHidden(char *file)
{
 if (!file) return 0;
 int ret=0;

 #ifdef SEOS_Win32
 // At least BC++ 5.5 says _chmod is obsolet and looking in the implementation
 // (oh no! reverse eng.! just saw the .obj ;-) it calls Get/SetFileAttributes
 // from Win32 API
 // Get current status
 DWORD mode=GetFileAttributes(file);
 if (mode==0xFFFFFFFF) return 1;
 // Already hidden?
 if (mode & FILE_ATTRIBUTE_HIDDEN) return 0;
 // Set the attribute
 SetFileAttributes(file,mode | FILE_ATTRIBUTE_HIDDEN);
 #endif
 #ifdef SEOS_DOS
 // Get current status
 int mode=_chmod(file,0,0);
 if (mode==-1) return 1;
 // Already hidden?
 if (mode & 2) return 0;
 // Set the attribute
 _chmod(file,1,mode | 2);
 #endif
 #ifdef SEOS_UNIX
 char *s=MakeItHiddenName(file);
 if (!s) return 0;
 ret=rename(file,s);
 strcpy(file,s);
 delete[] s;
 #endif
 return ret;
}

int RemoveFileHidden(char *file)
{
 #ifndef SEOS_UNIX
 // Hidden files are the same as not hidden files, just an attribute.
 return 1;
 #else
 if (!file) return 0;
 int ret=0;

 char *s=MakeItHiddenName(file);
 if (!s) return 0;
 ret=unlink(s);
 delete[] s;
 return ret;
 #endif
}


static TStringCollectionW *filesToKill=0;

void AddToFilesToKill(char *name)
{
 if (!filesToKill)
    filesToKill=new TStringCollectionW(6,4);
    
 ccIndex i;
 if (filesToKill->search(filesToKill->keyOf(name),i)==0)
    // A simple insert here will leak memory because the dupped name won't
    // be released.
    filesToKill->atInsert(i,strdup(name));
}

static
void killIt(void *name, void *)
{
 unlink((char *)name);
}

void KillFilesToKill()
{
 if (!filesToKill)
    return;
 filesToKill->forEach(killIt,0);
}

void ReleaseFilesToKill()
{
 destroy0(filesToKill);
}

TStringCollectionW *GetFilesToKill()
{
 return filesToKill;
}

void SetFilesToKill(TStringCollectionW *files)
{
 ReleaseFilesToKill();
 filesToKill=files;
}

/**[txh]********************************************************************

  Description:
  Determines if 2 file names belongs to the same file. The function uses the
inode/device to find it.

  Return:
  !=0 if the files are the same.

***************************************************************************/

int CompareFileNames(char *origFile, char *destFile)
{
 struct stat st1,st2;

 if (stat(origFile,&st1) || stat(destFile,&st2))
    return 0;

 return st1.st_dev==st2.st_dev && st1.st_ino==st2.st_ino;
}

/**[txh]********************************************************************

  Description:
  If the current directory is invalid aborts the execution.

***************************************************************************/

void CheckIfCurDirValid(void)
{
 long *aux=(long *)PathOrig;
 *aux=0xFFFEFDFC;
 getcwd(PathOrig,PATH_MAX);
 if (*aux==(long)0xFFFEFDFC)
   {
    TScreen::suspend();
    fprintf(stderr,_("\nError! please run the editor from a valid directory\n\n"));
    fflush(stderr);
    exit(1);
   }
}

const int maxSFNSize=68;

#ifdef SEOSf_djgpp
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

int CheckIfPathAbsolute(const char *s)
{
 //$todo: implement it for Win32 (SAA)
 if (!s) return 0;
 #ifndef SEOS_UNIX
 return *s=='/' || *s=='\\' || s[1]==':';
 #else
 return *s=='/';
 #endif
}

