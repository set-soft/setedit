/* MPEG/WAVE Sound library

   (C) 2000 by Salvador E. Tropea */

// Rawtowav.cc
// Output raw data to a wav file.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <unistd.h>

#include "mpegsound.h"

// Rawtowav class
Rawtowav::~Rawtowav()
{
  unsigned l;
  lseek(filehandle,4,SEEK_SET);
  l=length-8;
  write(filehandle,&l,4);
  lseek(filehandle,sizeof(WAVEHEADER)-4,SEEK_SET);
  l=length-sizeof(WAVEHEADER);
  write(filehandle,&l,4);
  close(filehandle);
}

bool Rawtowav::initialize(char *filename)
{
  if(filename==NULL)filehandle=1;
  else if((filehandle=Mpegsound_creat(filename))==-1)
    return seterrorcode(SOUND_ERROR_DEVOPENFAIL);
  length=sizeof(WAVEHEADER);

  return true;
}

bool Rawtowav::setsoundtype(int stereo,int samplesize,int speed)
{
  WAVEHEADER header;

  header.main_chunk=RIFF;
  header.length=0; // Will be computed when closing.
  header.chunk_type=WAVE;
  header.sub_chunk=FMT;
  header.sc_len=16;
  header.format=PCM_CODE;
  header.modus=stereo==MODE_STEREO ? WAVE_STEREO : WAVE_MONO;
  header.sample_fq=speed;
  header.byte_p_spl=samplesize==8 ? 1 : 2;
  header.byte_p_sec=header.byte_p_spl*speed;
  header.bit_p_spl=samplesize;
  header.data_chunk=DATA;
  header.data_length=0; // Will be computed when closing.

  return write(filehandle,&header,sizeof(WAVEHEADER))==sizeof(WAVEHEADER);
}

bool Rawtowav::putblock(void *buffer,int size)
{
  length+=size;
  return (write(filehandle,buffer,size)==size);
}

