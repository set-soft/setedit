/* MPEG/WAVE Sound library

   (C) 1997 by Jung woo-jae
   (C) 2000 by Salvador E. Tropea */

// Mpegtoraw.cc
// Server which get mpeg format and put raw format.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "mpegsound.h"
#include "mpg_locals.h"

#define MY_PI 3.14159265358979323846

Mpegtoraw::Mpegtoraw(Soundinputstream *loader,Soundplayer *player)
{
  __errorcode=SOUND_ERROR_OK;
  frameoffsets=frametimes=NULL;

  forcetomonoflag=false;
  downfrequency=0;
  playing=false;

  this->loader=loader;
  this->player=player;
}

Mpegtoraw::~Mpegtoraw()
{
  if(frameoffsets)delete [] frameoffsets;
  if(frametimes)delete [] frametimes;
}

#ifndef WORDS_BIGENDIAN
#define _KEY 0
#else
#define _KEY 3
#endif

int Mpegtoraw::getbits(int bits)
{
  union
  {
    char store[4];
    int current;
  }u;
  int bi;

  if(!bits)return 0;

  u.current=0;
  bi=(bitindex&7);
  u.store[_KEY]=buffer[bitindex>>3]<<bi;
  bi=8-bi;
  bitindex+=bi;

  while(bits)
  {
    if(!bi)
    {
      u.store[_KEY]=buffer[bitindex>>3];
      bitindex+=8;
      bi=8;
    }

    if(bits>=bi)
    {
      u.current<<=bi;
      bits-=bi;
      bi=0;
    }
    else
    {
      u.current<<=bits;
      bi-=bits;
      bits=0;
    }
  }
  bitindex-=bi;

  return (u.current>>8);
}

void Mpegtoraw::setforcetomono(bool flag)
{
  forcetomonoflag=flag;
}

void Mpegtoraw::setdownfrequency(int value)
{
  downfrequency=0;
  if(value)downfrequency=1;
}

bool Mpegtoraw::getforcetomono(void)
{
  return forcetomonoflag;
}

int Mpegtoraw::getdownfrequency(void)
{
  return downfrequency;
}

int  Mpegtoraw::getpcmperframe(void)
{
  int s;

  s=32;
  if(layer==3)
  {
    s*=18;
    if(version==0)s*=2;
  }
  else
  {
    s*=SCALEBLOCK;
    if(layer==2)s*=3;
  }

  return s;
}

inline void Mpegtoraw::flushrawdata(void)
#ifdef PTHREADEDMPEG
{
  if(threadflags.thread)
  {
    if(((threadqueue.tail+1)%threadqueue.framenumber)==threadqueue.head)
    {
      while(((threadqueue.tail+1)%threadqueue.framenumber)==threadqueue.head)
	usleep(200);
    }
    memcpy(threadqueue.buffer+(threadqueue.tail*RAWDATASIZE),rawdata,
	   RAWDATASIZE*sizeof(short int));
    threadqueue.sizes[threadqueue.tail]=(rawdataoffset<<1);
    
    if(threadqueue.tail>=threadqueue.frametail)
      threadqueue.tail=0;
    else threadqueue.tail++;
  }
  else
  {
    player->putblock((char *)rawdata,rawdataoffset<<1);
    currentframe++;
  }
}
#else
{
  player->putblock((char *)rawdata,rawdataoffset<<1);
  currentframe++;
};
#endif

typedef struct
{
  char *songname;
  char *artist;
  char *album;
  char *year;
  char *comment;
}ID3;

static void strman(char *str,int max)
{
  int i;

  str[max]=0;

  for(i=max-1;i>=0;i--)
    if(((unsigned char)str[i])<26 || str[i]==' ')str[i]=0; else break;
}

/**************** Genre by Salvador E. Tropea (SET) *********************/
static
const char *GenreStr[]={
"Blues","Classic Rock","Country","Dance","Disco","Funk","Grunge","Hip-Hop", // 0-7
"Jazz","Metal","New Age","Oldies","Other","Pop","R&B","Rap","Reggae","Rock",// 8-17
"Techno","Industrial","Alternative","Ska","Death Metal","Pranks",           // 18-23
"Soundtrack","Euro-Techno","Ambient","Trip-Hop","Vocal","Jazz+Funk",        // 24-29
"Fusion","Trance","Classical","Instrumental","Acid","House","Game",         // 30-36
"Sound Clip","Gospel","Noise","Alt. Rock","Bass","Soul","Punk","Space",     // 37-44
"Meditative","Instrumental Pop","Instrumental Rock","Ethnic","Gothic",      // 45-49
"Darkwave","Techno-Industrial","Electronic","Pop-Folk","Eurodance","Dream", // 50-55
"Southern Rock","Comedy","Cult","Gangsta Rap","Top 40","Christian Rap",     // 56-61
"Pop/Funk","Jungle","Native American","Cabaret","New Wave","Psychedelic",   // 62-67
"Rave","Showtunes","Trailer","Lo-Fi","Tribal","Acid Punk","Acid Jazz",      // 63-74
"Polka","Retro","Musical","Rock & Roll","Hard Rock","Folk","Folk/Rock",     // 75-81
"National Folk","Swing","Fast-Fusion","Bebob","Latin","Revival","Celtic",   // 82-88
"Bluegrass","Avantgarde","Gothic Rock","Progressive Rock",                  // 89-92
"Psychedelic Rock","Symphonic Rock","Slow Rock","Big Band","Chorus",        // 93-97
"Easy Listening","Acoustic","Humour","Speech","Chanson","Opera",            // 98-103
"Chamber Music","Sonata","Symphony","Booty Bass","Primus","Porn Groove",    // 104-109
"Satire","Slow Jam","Club","Tango","Samba","Folklore","Ballad",             // 110-116
"Power Ballad","Rhythmic Soul","Freestyle","Duet","Punk Rock","Drum Solo",  // 117-122
"Acapella","Euro-House","Dance Hall","Goa","Drum & Bass","Club-House",      // 123-128
"Hardcore","Terror","Indie","BritPop","Negerpunk","Polsk punk","Beat",      // 129-135
"Christian Gangsta Rap","Heavy Metal","Black Metal","Crossover",            // 136-139
"Contemporary Christian","Christian Rock"                                   // 140-141
};

const int maxKnownGenre=sizeof(GenreStr)/sizeof(char *);
/********** End of Genre by Salvador E. Tropea (SET) *********************/

inline void parseID3(Soundinputstream *fp,ID3 *data,const char *&genre,
                     bool avoidID3)
{
  int tryflag=0;

  data->songname[0]=0;
  data->artist  [0]=0;
  data->album   [0]=0;
  data->year    [0]=0;
  data->comment [0]=0;
  genre="Unknown";
  if(avoidID3 || !fp->getcanseek())return;

  int oldPos=fp->getposition();
  fp->setposition(fp->getsize()-128);

  for(;;)
  {
    if(fp->getbytedirect()==0x54)
      if(fp->getbytedirect()==0x41)
        if(fp->getbytedirect()==0x47)
	{
	  fp->_readbuffer(data->songname,30);strman(data->songname,30);
	  fp->_readbuffer(data->artist  ,30);strman(data->artist,  30);
	  fp->_readbuffer(data->album   ,30);strman(data->album,   30);
	  fp->_readbuffer(data->year    , 4);strman(data->year,     4);
	  fp->_readbuffer(data->comment ,30);strman(data->comment, 30);
          int g=fp->getbytedirect();
          if(g<maxKnownGenre && g>=0)genre=GenreStr[g];
          break;
        }

    tryflag++;
    if(tryflag==1)fp->setposition(fp->getsize()-125); // for mpd 0.1, Sorry..
    else break;
  }

  fp->setposition(oldPos);
}

inline void stripfilename(char *dtr,char *str,int max)
{
  char *ss;
  int p=0,s=0;

  for(;str[p];p++)
    if(str[p]=='/')
    {
      p++;
      s=p;
    }

  ss=str+s;
  for(p=0;p<max && ss[p];p++)dtr[p]=ss[p];
  dtr[p]=0;
}

/*****************************************************************************

 SET:
 The following member checks if the file is a Variable Bit Rate one (VBR).
 I took the information of VBR files from LAME source code.

*****************************************************************************/

// Amount of free bytes after the header
int SizeOfEmptyFrame[2][2]={{32,17},{17,9}};
// Options flags
const int fFrames=1,fBytes=2,fTOC=4,fVBRScale=8;
// Data is stored in big endian format, this is a converter
static
unsigned ExtractI4(unsigned char *buf)
{
 unsigned x;
 #if WORDS_BIGENDIAN
 x=*((unsigned *)buf);
 #else
 x =buf[0]; x<<=8;
 x|=buf[1]; x<<=8;
 x|=buf[2]; x<<=8;
 x|=buf[3];
 #endif
 return x;
}

bool Mpegtoraw::checkforvbr(void)
{
 // Seek to the VBR tag position, 4 is the size of the MPG header
 loader->setposition(SizeOfEmptyFrame[version][mode==single ? 1 : 0]+4);
 // Check the tag: Xing
 char buf[4];
 loader->getblock(buf,4);
 if (buf[0]!='X' || buf[1]!='i' || buf[2]!='n' || buf[3]!='g') return false;
 // Get the flags
 unsigned flags;
 loader->getbytedirect();
 loader->getbytedirect();
 loader->getbytedirect();
 flags=loader->getbytedirect();
 // Total number of frames
 if (!(flags & fFrames)) return false;
 loader->getblock(buf,4);
 totalframe=ExtractI4((unsigned char *)buf);
 // Total file size
 if (flags & fBytes)
    loader->getblock(buf,4);
 // Table of contents
 if (flags & fTOC)
   {
    haveTOC=true;
    loader->getblock((char *)TOC,100);
   }
 // Then comes the VBR scale but we don't need it.
 // Now go to the first real frame
 int startPosition=framesize;
 loader->setposition(startPosition);
 // Read information of a real frame
 loadheader();
 // Let the position where the real frame is.
 loader->setposition(startPosition);
 return true;
}

// Convert mpeg to raw
// Mpeg headder class
void Mpegtoraw::initialize(char *filename, bool avoidID3)
{
  static bool initialized=false;

  register int i;
  register REAL *s1,*s2;
  REAL *s3,*s4;

  scalefactor=SCALE;
  calcbufferoffset=15;
  currentcalcbuffer=0;

  s1=calcbufferL[0];s2=calcbufferR[0];
  s3=calcbufferL[1];s4=calcbufferR[1];
  for(i=CALCBUFFERSIZE-1;i>=0;i--)
    calcbufferL[0][i]=calcbufferL[1][i]=
    calcbufferR[0][i]=calcbufferR[1][i]=0.0;

  if(!initialized)
  {
    for(i=0;i<16;i++)hcos_64[i]=1.0/(2.0*cos(MY_PI*double(i*2+1)/64.0));
    for(i=0;i< 8;i++)hcos_32[i]=1.0/(2.0*cos(MY_PI*double(i*2+1)/32.0));
    for(i=0;i< 4;i++)hcos_16[i]=1.0/(2.0*cos(MY_PI*double(i*2+1)/16.0));
    for(i=0;i< 2;i++)hcos_8 [i]=1.0/(2.0*cos(MY_PI*double(i*2+1)/ 8.0));
    hcos_4=1.0/(2.0*cos(MY_PI*1.0/4.0));
    initialized=true;
  }

  layer3initialize();

  currentframe=decodeframe=0;
  haveTOC=false;
  if(loader->getcanseek() && loadheader())
  {
    if(!checkforvbr())
    {
      totalframe=(loader->getsize()+framesize-1)/framesize;
      loader->setposition(0);
    }
   totaltime=8*getframesize()/getbitrate()*totalframe;
  }
  else totalframe=0;


  if(frameoffsets)delete [] frameoffsets;
  if(frametimes)delete [] frametimes;

  songinfo.name[0]=0;
  if(totalframe>0)
  {
    frameoffsets=new int[totalframe];
    frametimes=new int[totalframe];
    for(i=totalframe-1;i>=0;i--)
      frameoffsets[i]=0, frametimes[i]=0;

    {
      ID3 data;

      data.songname=songinfo.name;
      data.artist  =songinfo.artist;
      data.album   =songinfo.album;
      data.year    =songinfo.year;
      data.comment =songinfo.comment;
      parseID3(loader,&data,songinfo.genre,avoidID3);
    }
    frameoffsets[0]=loader->getposition();
  }
  else frameoffsets=frametimes=NULL;

  if(songinfo.name[0]==0)
    if(filename!=NULL)
      stripfilename(songinfo.name,filename,30);

#ifdef PTHREADEDMPEG
  threadflags.thread=false;
  threadqueue.buffer=NULL;
  threadqueue.sizes=NULL;
#endif
};

// This function computes how many miliseconds of sound contains this frame
// and adds it to the previous one.
// Added by SET to compute Variable Bit Rate encoded files.
int Mpegtoraw::ComputeTimeForFrame(int i)
{
 int prev=i==0 ? 0 : frametimes[i-1];
 int br=getbitrate();
 return br ? prev+(8*getframesize()/br) : prev;
}

// This function returns an estimation for the total time of the MP3 based
// on the amount of sound played.
// Time is in miliseconds.
int Mpegtoraw::gettotaltime(void)
{
 return totaltime;
 /*if(!frameoffsets)return 0;
 if(frameoffsets[currentframe]==0)return 8*loader->getsize()/getbitrate();
 return (int)((double)(loader->getsize())/frameoffsets[currentframe]*
         getcurtime()+0.5);*/
}

void Mpegtoraw::setframe(int framenumber)
{
  player->abort();
  int pos=0;

  if(frameoffsets==NULL)return;
  if(framenumber<=0)pos=frameoffsets[0];
  else
  {
    if(framenumber>=totalframe)framenumber=totalframe-1;
    pos=frameoffsets[framenumber];
    if(pos==0)
    {
      int i;

      for(i=framenumber-1;i>0;i--)
	if(frameoffsets[i]!=0)break;

      loader->setposition(frameoffsets[i]);

      while(i<framenumber)
      {
	loadheader();
	i++;
	frameoffsets[i]=loader->getposition();
        frametimes[i]=ComputeTimeForFrame(i);
      }
      pos=frameoffsets[framenumber];
    }
  }

  clearbuffer();
  loader->setposition(pos);
  decodeframe=currentframe=framenumber;
}


void Mpegtoraw::settimepos(int time)
{
  player->abort();
  int pos,i=0;

  if(frameoffsets==NULL)return;
  if(time<=0)pos=frameoffsets[0];
  else
  {
    i=0; pos=frametimes[0];
    while (i<totalframe && pos && pos<time) pos=frametimes[++i];
    if(!pos)
    {
      loader->setposition(frameoffsets[i-1]);
      while (i<totalframe)
      {
        loadheader();
        if(geterrorcode()==SOUND_ERROR_BAD)
        {
          seterrorcode(SOUND_ERROR_OK);
          continue; // Just skip this frame
        }
        if(geterrorcode()!=SOUND_ERROR_OK)break;
        frameoffsets[i]=loader->getposition();
        pos=frametimes[i]=ComputeTimeForFrame(i);
        if(pos>=time)break;
        i++;
      }
    }
    i--;
    pos=frameoffsets[i];
  }

  clearbuffer();
  loader->setposition(pos);
  decodeframe=currentframe=i;
}

void Mpegtoraw::clearbuffer(void)
{
#ifdef PTHREADEDMPEG
  if(threadflags.thread)
  {
    threadflags.criticalflag=false;
    threadflags.criticallock=true;
    while(!threadflags.criticalflag)usleep(1);
    threadqueue.head=threadqueue.tail=0;
    threadflags.criticallock=false;
  }
#endif
  player->abort();
  player->resetsoundtype();
  playing=true;
}

bool Mpegtoraw::loadheader(void)
{
  register int c;
  bool flag;

  sync();

// Synchronize
  flag=false;
  do
  {

    if((c=loader->getbytedirect())<0)break;

    if(c==0xff)
      while(!flag)
      {
	if((c=loader->getbytedirect())<0)
	{
	  flag=true;
	  break;
	}
	if((c&0xf0)==0xf0)
	{
	  flag=true;
	  break;
	}
	else if(c!=0xff)break;
      }
  }while(!flag);

  if(c<0)return seterrorcode(SOUND_ERROR_FINISH);



// Analyzing
  c&=0xf;
  protection=c&1;
  layer=4-((c>>1)&3);
  version=(_mpegversion)((c>>3)^1);

  c=((loader->getbytedirect()))>>1;
  padding=(c&1);             c>>=1;
  frequency=(_frequency)(c&2); c>>=2;
  bitrateindex=(int)c;
  if(bitrateindex==15)return seterrorcode(SOUND_ERROR_BAD);

  c=((unsigned int)(loader->getbytedirect()))>>4;
  extendedmode=c&3;
  mode=(_mode)(c>>2);


// Making information
  inputstereo= (mode==single)?0:1;
  if(forcetomonoflag)outputstereo=0; else outputstereo=inputstereo;

  /*  if(layer==2)
    if((bitrateindex>=1 && bitrateindex<=3) || (bitrateindex==5)) {
      if(inputstereo)return seterrorcode(SOUND_ERROR_BAD); }
    else if(bitrateindex==11 && mode==single)
    return seterrorcode(SOUND_ERROR_BAD); */

  channelbitrate=bitrateindex;
  if(inputstereo)
    if(channelbitrate==4)channelbitrate=1;
    else channelbitrate-=4;

  if(channelbitrate==1 || channelbitrate==2)tableindex=0; else tableindex=1;

  if(layer==1)subbandnumber=MAXSUBBAND;
  else
  {
    if(!tableindex)
      if(frequency==frequency32000)subbandnumber=12; else subbandnumber=8;
    else if(frequency==frequency48000||
	    (channelbitrate>=3 && channelbitrate<=5))
      subbandnumber=27;
    else subbandnumber=30;
  }

  if(mode==single)stereobound=0;
  else if(mode==joint)stereobound=(extendedmode+1)<<2;
  else stereobound=subbandnumber;

  if(stereobound>subbandnumber)stereobound=subbandnumber;

  // framesize & slots
  if(layer==1)
  {
    framesize=(12000*bitrate[version][0][bitrateindex])/
              frequencies[version][frequency];
    if(frequency==frequency44100 && padding)framesize++;
    framesize<<=2;
  }
  else
  {
    framesize=(144000*bitrate[version][layer-1][bitrateindex])/
      (frequencies[version][frequency]<<version);
    if(padding)framesize++;
    if(layer==3)
    {
      if(version)
	layer3slots=framesize-((mode==single)?9:17)
	                     -(protection?0:2)
	                     -4;
      else
	layer3slots=framesize-((mode==single)?17:32)
	                     -(protection?0:2)
	                     -4;
    }
  }

  if(framesize-4>RAWDATASIZE)
  {// SET: Avoid corrupting data if we got a wrong framesize.
    seterrorcode(SOUND_ERROR_BAD);
    return false;
  }
  if(!fillbuffer(framesize-4))seterrorcode(SOUND_ERROR_FILEREADFAIL);

  if(!protection)
  {
    getbyte();                      // CRC, Not check!!
    getbyte();
  }


  if(loader->eof())return seterrorcode(SOUND_ERROR_FINISH);

  return true;
}

/***************************/
/* Playing in multi-thread */
/***************************/
#ifdef PTHREADEDMPEG
/* Player routine */
void Mpegtoraw::threadedplayer(void)
{
  while(!threadflags.quit)
  {
    while(threadflags.pause || threadflags.criticallock)
    {
      threadflags.criticalflag=true;
      usleep(200);
    }

    if(threadqueue.head!=threadqueue.tail)
    {
      player->putblock(threadqueue.buffer+threadqueue.head*RAWDATASIZE,
      		       threadqueue.sizes[threadqueue.head]);
      currentframe++;
      if(threadqueue.head==threadqueue.frametail)
	threadqueue.head=0;
      else threadqueue.head++;
    }
    else
    {
      if(threadflags.done)break;  // Terminate when done
      usleep(200);
    }
  }
  threadflags.thread=false;
}

static void *threadlinker(void *arg)
{
  ((Mpegtoraw *)arg)->threadedplayer();

  return NULL;
}

bool Mpegtoraw::makethreadedplayer(int framenumbers)
{
  threadqueue.buffer=
    (short int *)malloc(sizeof(short int)*RAWDATASIZE*framenumbers);
  if(threadqueue.buffer==NULL)
    seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
  threadqueue.sizes=(int *)malloc(sizeof(int)*framenumbers);
  if(threadqueue.sizes==NULL)
    seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
  threadqueue.framenumber=framenumbers;
  threadqueue.frametail=framenumbers-1;
  threadqueue.head=threadqueue.tail=0;


  threadflags.quit=threadflags.done=
  threadflags.pause=threadflags.criticallock=false;

  threadflags.thread=true;
  if(pthread_create(&thread,0,threadlinker,this))
    seterrorcode(SOUND_ERROR_THREADFAIL);

  return true;
}

void Mpegtoraw::freethreadedplayer(void)
{
  threadflags.criticallock=
  threadflags.pause=false;
  threadflags.done=true;               // Terminate thread player
  while(threadflags.thread)usleep(10); // Wait for done...
  if(threadqueue.buffer)free(threadqueue.buffer);
  if(threadqueue.sizes)free(threadqueue.sizes);
}




void Mpegtoraw::stopthreadedplayer(void)
{
  threadflags.quit=true;
};

void Mpegtoraw::pausethreadedplayer(void)
{
  threadflags.pause=true;
};

void Mpegtoraw::unpausethreadedplayer(void)
{
  threadflags.pause=false;
};


bool Mpegtoraw::existthread(void)
{
  return threadflags.thread;
}

int  Mpegtoraw::getframesaved(void)
{
  if(threadqueue.framenumber)
    return
      ((threadqueue.tail+threadqueue.framenumber-threadqueue.head)
       %threadqueue.framenumber);
  return 0;
}

#endif

void Mpegtoraw::stopplay()
{
  player->abort();
  playing=false;
  loader->setposition(frameoffsets ? frameoffsets[0] : 0);
  currentframe=decodeframe=0;
  loadheader();
}

// Convert mpeg to raw
bool Mpegtoraw::run(int frames)
{
  if (!playing && frames!=-1) return false;
  clearrawdata();
  if(frames<0)lastfrequency=-1; // SET: not 0 because that's 44.1 KHz!

  for(;frames;frames--)
  {
    if(totalframe>0)
    {
      if(decodeframe<totalframe)
      {
        frameoffsets[decodeframe]=loader->getposition();
        frametimes[decodeframe]=ComputeTimeForFrame(decodeframe);
      }
    }

    if(loader->eof())
    {
      seterrorcode(SOUND_ERROR_FINISH);
      break;
    }

    bool headerOK=loadheader();
    if(!headerOK || (frequency!=lastfrequency && lastfrequency!=-1))
    {
      // SET: I added it to make the player more robust. I have a lot a severely
      // damaged files.
      // SOUND_ERROR_BAD is generated when the bit rate is incoherent. Under such
      // a case I force to read more headers until I find one with the same sample
      // rate.
      // It works quite well for my files. I have files with 5 and more errors.
      //printf("Error al leer el header en: frame %d offset: %d\n",decodeframe,loader->getposition());
      if(frequency!=lastfrequency)seterrorcode(SOUND_ERROR_BAD);
      bool giveChance=true;
      while(geterrorcode()==SOUND_ERROR_BAD && !loader->eof() && giveChance)
        giveChance=!(loadheader() && frequency==lastfrequency);
      if(frequency!=lastfrequency){seterrorcode(SOUND_ERROR_BAD); break;}
      if(giveChance)break;
      seterrorcode(SOUND_ERROR_OK);
    }

    if(frequency!=lastfrequency)lastfrequency=frequency;
    if(frames<0)
    {
      frames=-frames;
      player->setsoundtype(outputstereo,16,
			   frequencies[version][frequency]>>downfrequency);
      playing=true;
      seterrorcode(SOUND_ERROR_OK);
    }

    decodeframe++;

    rawdataoffset=0; // SET: Clear it only here so callformore have an
    // estimation of how much data is a an uncompressed frame.
    if     (layer==3)extractlayer3();
    else if(layer==2)extractlayer2();
    else if(layer==1)extractlayer1();

    flushrawdata();
    if(player->geterrorcode())seterrorcode(player->geterrorcode());
  }

  bool ret=(geterrorcode()==SOUND_ERROR_OK);
  if (!ret)
     stopplay();
  return ret;
}

