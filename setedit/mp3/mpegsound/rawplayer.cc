/* MPEG/WAVE Sound library

   (C) 1997 by Jung woo-jae
   Allegro routines:
   (C) 1998 by Ove Kaaven <ovek@arcticnet.no>
   (C) 2000/2001 by Salvador E. Tropea <set@ieee.org> */

// Rawplayer.cc
// Playing raw data with sound type.
// It's only for Linux

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "mpegsound.h"

/* IOCTL */
#ifdef SOUND_VERSION
#define IOCTL(a,b,c)		ioctl(a,b,&c)
#else
#define IOCTL(a,b,c)		(c = ioctl(a,b,c) )
#endif

char *Rawplayer::defaultdevice="/dev/dsp";

/* Volume */
int Rawplayer::setvolume(int volume)
{
  int handle;
  int r;

  handle=open("/dev/mixer",O_RDWR);

  if(volume>100)volume=100;
  if(volume>=0)
  {
    r=(volume<<8) | volume;

    ioctl(handle,MIXER_WRITE(SOUND_MIXER_VOLUME),&r);
  }
  ioctl(handle,MIXER_READ(SOUND_MIXER_VOLUME),&r);

  close(handle);

  return (r&0xFF);
}

/*******************/
/* Rawplayer class */
/*******************/
// Rawplayer class
Rawplayer::~Rawplayer()
{
  close(audiohandle);
}

bool Rawplayer::initialize(char *filename)
{
  int flag;

  rawbuffersize=0;
  quota=0;

  if((audiohandle=open(filename,O_WRONLY|O_NDELAY,0))==-1)
    return seterrorcode(SOUND_ERROR_DEVOPENFAIL);

  if((flag=fcntl(audiohandle,F_GETFL,0))<0)
    return seterrorcode(SOUND_ERROR_DEVOPENFAIL);
  flag&=~O_NDELAY;
  if(fcntl(audiohandle,F_SETFL,flag)<0)
    return seterrorcode(SOUND_ERROR_DEVOPENFAIL);

  IOCTL(audiohandle,SNDCTL_DSP_GETBLKSIZE,audiobuffersize);
  if(audiobuffersize<4 || audiobuffersize>65536)
    return seterrorcode(SOUND_ERROR_DEVBADBUFFERSIZE);

  return true;
}

void Rawplayer::abort(void)
{
  int a;

  IOCTL(audiohandle,SNDCTL_DSP_RESET,a);
}

int Rawplayer::getprocessed(void)
{
  audio_buf_info info;
  int r;

  IOCTL(audiohandle,SNDCTL_DSP_GETOSPACE,info);

  r=(info.fragstotal-info.fragments)*info.fragsize;

  return r;
}

bool Rawplayer::roomformore(unsigned size)
{
  audio_buf_info info;

  IOCTL(audiohandle,SNDCTL_DSP_GETOSPACE,info);
  return info.bytes>=size ? true : false;
}

bool Rawplayer::setsoundtype(int stereo,int samplesize,int speed)
{
  rawstereo=stereo;
  rawsamplesize=samplesize;
  rawspeed=speed;
  forcetomono=forceto8=false;

  return resetsoundtype();
}

bool Rawplayer::resetsoundtype(void)
{
  int tmp;

  if(ioctl(audiohandle,SNDCTL_DSP_SYNC,NULL)<0)
    return seterrorcode(SOUND_ERROR_DEVCTRLERROR);

#ifdef SOUND_VERSION
  if(ioctl(audiohandle,SNDCTL_DSP_STEREO,&rawstereo)<0)
#else
  if(rawstereo!=ioctl(audiohandle,SNDCTL_DSP_STEREO,rawstereo))
#endif
  {
    rawstereo=MODE_MONO;
    forcetomono=true;
  }

  tmp=rawsamplesize;
  IOCTL(audiohandle,SNDCTL_DSP_SAMPLESIZE,tmp);
  if(tmp!=rawsamplesize)
    if(rawsamplesize==16)
    {
      rawsamplesize=8;
      IOCTL(audiohandle,SNDCTL_DSP_SAMPLESIZE,rawsamplesize);
      if(rawsamplesize!=8)
	return seterrorcode(SOUND_ERROR_DEVCTRLERROR);

      forceto8=true;
    }

  if(IOCTL(audiohandle,SNDCTL_DSP_SPEED,rawspeed)<0)
    return seterrorcode(SOUND_ERROR_DEVCTRLERROR);

  return true;
}

bool Rawplayer::putblock(void *buffer,int size)
{
  int modifiedsize=size;

  if(forcetomono || forceto8)
  {
    register unsigned char modify=0;
    register unsigned char *source,*dest;
    int increment=0,c;

    source=dest=(unsigned char *)buffer;

    if(forcetomono)increment++;
    if(forceto8)increment++,source++;

    c=modifiedsize=size>>increment;
    increment<<=1;

    while(c--)
    {
      *(dest++)=(*source)+modify;
      source+=increment;
    }
  }

  if(quota)
    while(getprocessed()>quota)usleep(3);
  write(audiohandle,buffer,modifiedsize);

  return true;
}

int Rawplayer::getblocksize(void)
{
  return audiobuffersize;
}
#endif

#ifdef __sun__
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/audioio.h>

#include "mpegsound.h"

char *Rawplayer::defaultdevice="/dev/audio";
static int sizeSamp,bufSize;

/* Volume */
int Rawplayer::setvolume(int volume)
{
 return volume;
}

/*******************/
/* Rawplayer class */
/*******************/
// Rawplayer class
Rawplayer::~Rawplayer()
{
 if (audiohandle!=-1)
    close(audiohandle);
}

bool Rawplayer::initialize(char *filename)
{
 int flag;

 rawbuffersize=0;
 quota=0;

 if ((audiohandle=open(filename,O_WRONLY | O_NONBLOCK,0))==-1)
    return seterrorcode(SOUND_ERROR_DEVOPENFAIL);

 audio_info_t info;
 if (ioctl(audiohandle,AUDIO_GETINFO,&info)==-1)
    return seterrorcode(SOUND_ERROR_DEVCTRLERROR);
 audiobuffersize=info.play.buffer_size;
 if (audiobuffersize<4 || audiobuffersize>65536)
    return seterrorcode(SOUND_ERROR_DEVBADBUFFERSIZE);

 return true;
}

void Rawplayer::abort(void)
{
 //int a;
 //IOCTL(audiohandle,SNDCTL_DSP_RESET,a);
}

int Rawplayer::getprocessed(void)
{
 audio_info_t info;
 ioctl(audiohandle,AUDIO_GETINFO,&info);

 return bufSize/sizeSamp-info.play.samples;
}

bool Rawplayer::roomformore(unsigned size)
{
 audio_info_t info;
 ioctl(audiohandle,AUDIO_GETINFO,&info);
 
 return bufSize-info.play.samples*sizeSamp>=size ? true : false;
}

bool Rawplayer::setsoundtype(int stereo,int samplesize,int speed)
{
 rawstereo=stereo;
 rawsamplesize=samplesize;
 rawspeed=speed;
 forcetomono=forceto8=false;
 sizeSamp=1;
 if (stereo) sizeSamp*=2;
 if (samplesize==16) sizeSamp*=2;

 return resetsoundtype();
}

bool Rawplayer::resetsoundtype(void)
{
 audio_info_t info;
 
 ioctl(audiohandle,AUDIO_GETINFO,&info);
 info.play.encoding=AUDIO_ENCODING_LINEAR;
 info.play.precision=rawsamplesize;
 info.play.channels=rawstereo ? 2 : 1;
 info.play.sample_rate=rawspeed;
 ioctl(audiohandle,AUDIO_SETINFO,&info);
 ioctl(audiohandle,AUDIO_GETINFO,&info);
 
 return info.play.sample_rate==rawspeed && info.play.precision==rawsamplesize ? true : false;
}

bool Rawplayer::putblock(void *buffer,int size)
{
 if (quota)
    while (getprocessed()>quota) usleep(3);
 bufSize=write(audiohandle,buffer,size);

 return true;
}

int Rawplayer::getblocksize(void)
{
  return audiobuffersize;
}
#endif

#ifdef __DJGPP__

#include <string.h>
#include "mpegsound.h"
#include <allegro.h>

char *Rawplayer::defaultdevice="/dev/allegro";

// Dirty hack, no get_volume!
extern int _digi_volume;
static AUDIOSTREAM *audio_left=0,*audio_right=0;
// 16 bits, 16*576 (9*1024)
const unsigned StreamSize=2*9*1024,BufferSize=2*StreamSize;
static unsigned short*buffer_left,*buffer_right;
static unsigned short*stream_left,*stream_right;
static unsigned buffer_head,buffer_tail,buffer_used;

/* Volume */
int Rawplayer::setvolume(int volume)
{
 //fprintf(stderr,"Volumen %d\n",volume);
 set_volume(volume*2,-1);
 // Is that correct? volumen is 0-100?
 if(volume>100)volume=100;
 if(volume>=0)
   set_volume(volume*255/100,-1);
 return _digi_volume;
}

inline
void Stop()
{
 if (audio_left)  stop_audio_stream(audio_left);
 if (audio_right) stop_audio_stream(audio_right);
 audio_left=0;
 audio_right=0;
}

Rawplayer::~Rawplayer()
{
 Stop();
 free(buffer_left);
 free(buffer_right);
 remove_sound();
}

bool Rawplayer::initialize(char *)
{
 //fprintf(stderr,"Inicializaci¢n\n");
 audiobuffersize=BufferSize;
 buffer_head=buffer_tail=buffer_used=0;
 buffer_left=(unsigned short *)malloc(BufferSize*sizeof(short));
 if (!buffer_left) return false;
 buffer_right=(unsigned short *)malloc(BufferSize*sizeof(short));
 if (!buffer_right) return false;

 allegro_init();
 bool ret=install_sound(DIGI_AUTODETECT,MIDI_NONE,0) ? false : true;
 set_volume(255,-1);
 return ret;
}

void Rawplayer::abort(void)
{
 Stop();
}

int Rawplayer::getprocessed(void)
{
 return buffer_used;
}

bool Rawplayer::setsoundtype(int stereo,int samplesize,int speed)
{
 rawstereo=stereo;
 rawsamplesize=samplesize;
 rawspeed=speed;
 forcetomono=forceto8=false;

 return resetsoundtype();
}

bool Rawplayer::resetsoundtype(void)
{
 //fprintf(stderr,"Reset sound\n");
 Stop();
 int size=StreamSize;
 if (rawsamplesize==8) size/=2;
 audio_left=play_audio_stream(size,rawsamplesize,0,rawspeed,255,rawstereo?0:128);
 if (!audio_left) return false;
 //free_audio_stream_buffer(audio_left);
 if (rawstereo)
   {
    audio_right=play_audio_stream(size,rawsamplesize,0,rawspeed,255,255);
    if (!audio_right) return false;
    //free_audio_stream_buffer(audio_right);
   }
 stream_left=stream_right=0;
 buffer_head=buffer_tail=buffer_used=0;
 return true;
}

static
int audioReady(int rawstereo)
{
 if (!stream_left)
    stream_left=(unsigned short *)get_audio_stream_buffer(audio_left);
 if (!stream_left) return 0;
 if (rawstereo)
   {
    if (!stream_right)
       stream_right=(unsigned short *)get_audio_stream_buffer(audio_right);
    if (!stream_right) return 0;
   }
 return 1;
}

static
void audioBufferTransfer(int instereo)
{
 while (audioReady(instereo) && (buffer_used>=StreamSize))
   {
    memcpy(stream_left,buffer_left+buffer_tail,StreamSize*sizeof(short));
    if (instereo)
       memcpy(stream_right,buffer_right+buffer_tail,StreamSize*sizeof(short));
    free_audio_stream_buffer(audio_left);
    if (instereo)
       free_audio_stream_buffer(audio_right);
    stream_left=stream_right=0;
    buffer_tail=(buffer_tail+StreamSize)%BufferSize;
    buffer_used-=StreamSize;
   }
}

bool Rawplayer::putblock(void *buffer,int size)
{
 //fprintf(stderr,"Put block\n");
 unsigned asmp,smp,cnt;

 asmp=size/sizeof(short);
 smp=rawstereo ? asmp/2 : asmp;
 while (smp)
   {
    cnt=MIN(smp,BufferSize-buffer_head);
    /* block until free buffer space, if necessary */
    while ((buffer_used+cnt)>BufferSize)
      {
       while (!audioReady(rawstereo)) __dpmi_yield();
       audioBufferTransfer(rawstereo);
      }
    /* split channels */
    if (rawstereo)
      {
       int d1,d2,d3,d4;
       asm volatile (
       "0:                   \n"
       "lodsw                \n"
       "addw $0x8000,%%ax    \n"
       "movw %%ax,(%%edi)    \n"
       "lodsw                \n"
       "addw $0x8000,%%ax    \n"
       "movw %%ax,(%%ebx)    \n"
       "addl $2,%%edi        \n"
       "addl $2,%%ebx        \n"
       "decl %%ecx           \n"
       "jnz 0b               \n"
         : "=b" (d1), "=c" (d2), "=S" (d3), "=D" (d4)
         : "S" (buffer), "D" (buffer_left+buffer_head), "b" (buffer_right+buffer_head), "c" (cnt)
         : "ax", "memory" );
      }
    else
      {
       int d2,d3,d4;
       asm volatile (
       "0:                    \n"
       "lodsw                 \n"
       "addw $0x8000,%%ax     \n"
       "stosw                 \n"
       "decl %%ecx            \n"
       "jnz 0b                \n"
         : "=c" (d2), "=S" (d3), "=D" (d4)
         : "S" (buffer), "D" (buffer_left+buffer_head), "c" (cnt)
         : "ax", "memory" );
      }
    buffer_head=(buffer_head+cnt)%BufferSize;
    buffer_used+=cnt; smp-=cnt;
   }

 audioBufferTransfer(rawstereo);
 return true;
}

int Rawplayer::getblocksize(void)
{
 return audiobuffersize;
}

bool Rawplayer::roomformore(unsigned size)
{
 unsigned asmp,smp,remaining;

 asmp=size/sizeof(short);
 smp=rawstereo ? asmp/2 : asmp;
 remaining=BufferSize-buffer_head;
 if (remaining>=size) return true;
 size-=remaining;
 return audioReady(rawstereo) ? true : false;
}
#endif
