#include <stdio.h>
#include <fcntl.h>

#include "mpegsound.h"

mode_t Mpegsound_DefaultCreateMode=0644;

int Mpegsound_creat(const char *filename)
{
 #ifdef __DJGPP__
 int oldmode=_fmode;
 _fmode=O_BINARY;
 #endif
 int ret=creat(filename,Mpegsound_DefaultCreateMode);
 #ifdef __DJGPP__
 _fmode=oldmode;
 #endif
 return ret;
}

FILE *Mpegsound_fopenR(const char *filename)
{
 return fopen(filename,"rb");
}
