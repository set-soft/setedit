#include <stdio.h>

#include "mpegsound.h"

static bool CheckForMP3InsideWavDontMove(Soundinputstream *f);
static bool CheckForMP3InsideWav(Soundinputstream *f);

void Mpegwavtoraw::initialize(char *filename, bool avoidID3)
{
 if (loader->getcanseek())
    // Check if that's an MP3 inside a RIFF/WAVe.
    // In this case avoid looking for the ID3 info.
    avoidID3=CheckForMP3InsideWavDontMove(loader);
 Mpegtoraw::initialize(filename,avoidID3);
}


#define getVal() f->getblock((char *)&val,4)
#define seekCur(off) f->setposition(f->getposition()+off)
// That's the internal function
static
bool CheckForMP3InsideWav(Soundinputstream *f)
{
 // Check if that's a RIFF and contains WAVE data
 unsigned int val=0;

 getVal();
 if (val!=RIFF) return false;
 getVal(); // Length of file
 getVal();
 if (val!=WAVE) return false;

 // Ok, now look for the format of the WAVE
 while(1)
   {
    if (getVal()==0) return false;
    if (val==FMT) break;
    // Nope, it isn't a format tag, skip it
    getVal();
    seekCur(val);
    if (f->eof()) return false;
   }

 getVal(); // Size of fmt chunk
 if (val<16) return false; // Should be at least 16 bytes

 // Read the fmt values we know all WAVE files have
 #pragma pack(1)
 struct {
  short wFormatTag;
  short nChannels;
  long  nSamplesPerSec;
  long  nAvgBytePerSec;
  short nBlockAlign;
  short wBitsPerSample;
 } pcm;
 #pragma pack()
 f->getblock((char *)&pcm,16);
 // Skip the rest because we don't know what it means
 if (val!=16) seekCur(val-16);

 // Check if we have an MPEG stream
 if (pcm.wFormatTag!=MPEG_CODE) return false;

 // Now look for a data tag
 while(1)
   {
    if (getVal()==0) return false;
    if (val==DATA) break;
    // Nope, it isn't a data tag, skip it
    getVal();
    seekCur(val);
    if (f->eof()) return false;
   }
 getVal();
 // Ok, now the file pointer is in a data and that's an MPEG stream.
 // It isn't completly right because we could have more fmt,data,info,etc.
 // tags, but this library wasn't designed for such a flexibility.
 return true;
}


// A wrapper to avoid moving the file position
static
bool CheckForMP3InsideWavDontMove(Soundinputstream *f)
{
 long pos=f->getposition();
 bool ret=CheckForMP3InsideWav(f);
 if (!ret)
    f->setposition(pos);
 return ret;
}

// Checks if a file is an MPEG stream inside a WAVE file.
bool Mpegsound_CheckMP3WAV(char *filename)
{
 if (!filename) return false;
 int error;
 Soundinputstream *in=Soundinputstream::hopen(filename,&error);
 if (!in) return false;
 bool ret=false;
 if (in->getcanseek()) ret=CheckForMP3InsideWav(in);
 delete in;
 return ret;
}
