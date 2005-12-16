/* Copyright (C) 1996-2005 by Salvador E. Tropea (SET),
   see copyrigh file for details */
int  GZFiles_CheckForGZIP(void);
int  GZFiles_IsGZ(FILE *f);
int  GZFiles_Expand(char *dest, char *orig);
int  GZFiles_ExpandHL(FILE *dest, char *orig);
int  GZFiles_DecryptGPG(FILE *dest, char *orig);
int  GZFiles_CreateGPG(const char *file, int &hi, int &ho, int &he, pid_t &child);
int  GZFiles_CloseGPG(int hi, int ho, int he, pid_t child);
void GZFiles_ResetError();
const char *GZFiles_GetError();
void GZFiles_SetError(const char *error);
typedef void (*GZFiles_CB)(const char *);
void GZFiles_SetMessageCallback(GZFiles_CB cb);

#ifndef SUP_GZ
#define gzFile FILE *
#define gzopen fopen
#define gzwrite(s,b,l) fwrite(b,l,1,s)
#define gzclose(f) fclose(f)
#else
#include <zlib.h>
#endif

#ifndef HAVE_BZIP2
 #define BZFILE void
#else
 #if defined(Uses_GZInterfaceOnly) && (defined(TVComp_BCPP) || defined(TVComp_MSC))
  // Don't ask me why but BC++ dies if bzlib.h includes windows.h again.
  // Collisions appears if we include windows.h in MSVC from ceditor.cc.
  #define BZFILE void
 #else
  #include <bzlib.h>
 #endif
#endif

#define HAVE_GPG
const int gzNoCompressed=0, gzGZIP=1, gzBZIP2=2, gzGPG=3,
          gzGZIPMagic=0x8B1F;

class TGZFileWrite
{
public:
 TGZFileWrite(char *fileName, int compressed=gzNoCompressed);
 ~TGZFileWrite();
 size_t write(void *buffer, size_t len);
 void close();
 int ok;

private:
 int compressed;
 FILE   *f;
 gzFile  fc;
 BZFILE *fc2;
 int hi, ho, he;
 pid_t child;
};


