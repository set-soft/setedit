/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#ifdef SUP_MP3

#define Uses_limits
#define Uses_stdio
#define Uses_string
#include <tv.h>

#ifdef HAVE_ALLEGRO
#include <allegro.h>
#endif
#include <mp3play.h>

MP3Player mp3;

char MP3Player::Name[PATH_MAX]="";

char MP3Player::Stoped=1;
char MP3Player::Selected=0;
char MP3Player::SelectedIsLoaded=0;
char MP3Player::Paused=0;
char MP3Player::PlayingList=0;
char MP3Player::Converting=0;

char MP3Player::Title[31];
char MP3Player::Author[31];
char MP3Player::Album[35];
char MP3Player::Comment[31];
const char *MP3Player::Genre="Unknown";
int  MP3Player::SampleRate=0;
int  MP3Player::BitRate=0;
const char *MP3Player::Mode="";
int  MP3Player::MPEGVer=0;
int  MP3Player::Layer=0;
int  MP3Player::TotalLen=0;
int  MP3Player::PausedPos=0;
char *(*MP3Player::GetNext)()=0;
void  (*MP3Player::EndOfList)()=0;
char MP3Player::FileToPlay[PATH_MAX]="";
int  MP3Player::Strategy=mp3UnBuffered;
char MP3Player::butRew[]="\x11\x11";
char MP3Player::butStop[]="\xFE";
char MP3Player::butPlay[]="\x10";
char MP3Player::butPause[]="\x13";
char MP3Player::butFfw[]="\x10\x10";
char MP3Player::obutRew[]="\x11\x11";
char MP3Player::obutStop[]="\xFE";
char MP3Player::obutPlay[]="\x10";
char MP3Player::obutPause[]="\x13";
char MP3Player::obutFfw[]="\x10\x10";

void MP3Player::SelectNewFile(char *fileName)
{
 strcpy(Name,fileName);
 Selected=1;
 SelectedIsLoaded=0;
}

int MP3Player::ConvertSelectedFile(char *name)
{
 if (!Selected)
    return 1;

 Converting=1;
 PlayingList=0;
 strcpy(FileToPlay,Name);
 if (!PlayFileToPlay(name))
    Converting=0;

 return 2;
}

void MP3Player::StartPlayList(char *(*aGetNext)(), void (*aEndOfList)())
{
 SetupPlayList(aGetNext,aEndOfList);
 PlayingList=1;
 PlayNext();
}

void MP3Player::SetupPlayList(char *(*aGetNext)(), void (*aEndOfList)())
{
 GetNext=aGetNext;
 EndOfList=aEndOfList;
}

void MP3Player::PlayNext()
{
 if (!PlayingList)
    return;
 char *next=GetNext();
 if (!next)
   {
    Stoped=1;
    PlayingList=0;
    EndOfList();
   }
 else
   {
    strcpy(FileToPlay,next);
    PlayFileToPlay();
   }
}

void MP3Player::StopPlayList()
{
 Stop();
 PlayingList=0;
}

#ifdef HAVE_AMP
/*****************************************************************************

I'm using a patched libamp because:

1) formats.c: wav_end ends with exit(0) Ugh!
2) audiolib.c: undefined TRACK... to save CPU and to get track of the
   position when writing to a file.

Old stuff:
Q: Why the sound quality is so poor? the same data sent to a file and played
with GRepVoc is FAR better.
A: Seems that Allegro streaming routines are crap (22KHz 8bits).
--> So then I contributed 16 bits mixing code to Allegro project ;-)))

*****************************************************************************/


#include <libamp.h>

extern "C" {
#include <getbits.h>
#include <audio.h>
#include <formats.h>
}


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

extern struct AUDIO_HEADER amp_cur_header;

// Returns 1 if no file were selected, 2 if the file was loaded, 3 if the file
// was already loaded and just did a play
int MP3Player::PlaySelectedFile()
{
 if (!Selected)
    return 1;

 int ret;
 PlayingList=0;

 if (SelectedIsLoaded)
   {
    replay_amp();
    Stoped=0;
    ret=3;
   }
 else
   {
    strcpy(FileToPlay,Name);
    if (PlayFileToPlay())
       SelectedIsLoaded=1;
    ret=2;
   }
 return ret;
}

int MP3Player::PlayFileToPlay(char *out)
{
 long fileLen=GetMP3Info();

 if (fileLen==-1)
   {
    Stoped=1;
    return 0;
   }

 if (out)
   {
    out_file=fopen(out,"wb");
    A_AUDIO_PLAY=FALSE;
    A_WRITE_TO_FILE=TRUE;
    A_FORMAT_WAVE=TRUE;
   }
 else
   {
    A_AUDIO_PLAY=TRUE;
    A_WRITE_TO_FILE=FALSE;
   }
 if (!load_amp(FileToPlay,0))
   {
    Stoped=1;
    return 0;
   }

 // Fill the information fields with the data from the header
 SampleRate=t_sampling_frequency[amp_cur_header.ID][amp_cur_header.sampling_frequency];
 BitRate=t_bitrate[amp_cur_header.ID][3-amp_cur_header.layer][amp_cur_header.bitrate_index];
 Mode=amp_cur_header.mode==3 ? "mono" : "stereo";
 MPEGVer=amp_cur_header.ID==1 ? 1 : 2;
 Layer=4-amp_cur_header.layer;
 TotalLen=(int)(fileLen/(BitRate*1000.0)*8.0);

 Stoped=0;
 return 1;
}

long MP3Player::GetMP3Info(void)
{
 FILE *f;
 char buffer[128];
 long ret;

 f=fopen(FileToPlay,"rb");
 if (!f)
   {
    *Title=0;
    *Author=0;
    *Album=0;
    *Comment=0;
    Genre="";
    return -1;
   }
 fseek(f,-128,SEEK_END);
 fread(buffer,128,1,f);
 ret=ftell(f);
 if (buffer[0]=='T' && buffer[1]=='A' && buffer[2]=='G')
   {
    memcpy(Title,buffer+3,30);
    Title[30]=0;
    memcpy(Author,buffer+33,30);
    Author[30]=0;
    memcpy(Album,buffer+63,34); // Plus year
    Album[34]=0;
    memcpy(Comment,buffer+97,30);
    Comment[30]=0;
    if (((unsigned char)buffer[127])>=maxKnownGenre)
       Genre="Unknown";
    else
       Genre=GenreStr[(unsigned char)buffer[127]];
    ret-=128;
   }
 else
   {
    int l=strlen(FileToPlay);
    if (l>30)
      {
       memcpy(Title,FileToPlay+(l-30),30);
       Title[0]=Title[1]=Title[2]='.';
       Title[30]=0;
      }
    else
      {
       memcpy(Title,FileToPlay,l);
       Title[l]=0;
      }
    *Author=0;
    *Album=0;
    *Comment=0;
   }
 fclose(f);

 return ret;
}

void MP3Player::Stop()
{
 stop_amp();
 if (A_WRITE_TO_FILE)
   {
    A_AUDIO_PLAY=TRUE;
    A_WRITE_TO_FILE=FALSE;
    wav_end(&amp_cur_header);
    fclose(out_file);
   }
 Stoped=1;
 Paused=0;
 Converting=0;
}

int MP3Player::Poll()
{
 if (!Stoped)
   {
    int ret=Strategy==mp3UnBuffered ? poll_amp() : run_amp();
    if (ret<0)
      {
       if (PlayingList)
         {
          PlayNext();
          if (Stoped)
             return 4;
          return 3;
         }
       Stoped=1;
       Converting=0;
       return 2;
      }
    return 1;
   }
 return 0;
}

int MP3Player::Init()
{
 allegro_init();
 if (install_sound(DIGI_AUTODETECT,MIDI_NONE,NULL))
   {
    printf("Unable to install Allegro sound driver\n");
    return 1;
   }
 install_amp();

 set_volume(255,-1);
 amp_reverse_phase=1;
 return 0;
}

void MP3Player::deInit()
{
 unload_amp();
 remove_sound();
}

int MP3Player::GetTime()
{
 return amp_time;
}

void MP3Player::FFwd()
{
 if (!Converting && !Stoped)
    seek_amp_rel(4*amp_samprat/amp_pollsize); /* seek 4 seconds */
}

void MP3Player::Rew()
{
 if (!Converting && !Stoped)
    seek_amp_rel(-4*amp_samprat/amp_pollsize); /* seek 4 seconds */
}

void MP3Player::TogglePause()
{
 if (!Converting && (!Stoped || Paused))
   {
    if (Paused)
      {
       Paused=0;
       //PlaySelectedFile();
       if (PlayFileToPlay())
          seek_amp_abs(PausedPos);
      }
    else
      {
       PausedPos=amp_frame;
       Stop();
       Paused=1;
      }
   }
}

void MP3Player::Pause()
{
 if (!Stoped && !Converting)
   {
    Stop();
    Paused=1; // After calling stop because stop resets Paused
    PausedPos=amp_frame;
   }
}

void MP3Player::SeekAbs(int seconds)
{
 seek_amp_abs(seconds*amp_samprat/amp_pollsize);
}

#endif // HAVE_AMP

#ifdef HAVE_MPEGSOUND
/*****************************************************************************

I'm using a patched mpegsound library because:

1) I added support for MP3s inside wavs.
2) I added support for Allegro.
3) I did the library more robust to support damaged files.
4) I added support for VBR encoded files.
5) Fixed many bugs and added various members to make it easier to use.

I tried to contact the author to merge my changes without luck.

*****************************************************************************/

#include <string.h>
#include "mpegsound/mpegsound.h"

static Mpegwavtoraw *MP3Engine=0;
static Soundinputstream *InputStream=0;
static Rawplayer *Player=0;
static Rawtowav *ToFile=0;

// Returns 1 if no file were selected, 2 if the file was loaded, 3 if the file
// was already loaded and just did a play
int MP3Player::PlaySelectedFile()
{
 if (!Selected)
    return 1;

 int ret;
 PlayingList=0;

 if (SelectedIsLoaded)
   {
    MP3Engine->setframe(0);
    MP3Engine->startplay();
    Stoped=0;
    ret=3;
   }
 else
   {
    strcpy(FileToPlay,Name);
    if (PlayFileToPlay())
       SelectedIsLoaded=1;
    ret=2;
   }
 return ret;
}

static
double OneFrameInSeconds()
{
 return (MP3Engine->getframesize()*8.0)/(MP3Engine->getbitrate()*1000);
}

int MP3Player::PlayFileToPlay(char *out)
{
 // Destroy old engine:
 delete MP3Engine;
 delete InputStream;
 delete ToFile;
 MP3Engine=0;
 InputStream=0;
 ToFile=0;
 // Create a new input stream
 int errorcode;
 InputStream=Soundinputstream::hopen(FileToPlay,&errorcode);
 if (!InputStream)
   {
    GetMP3Info(); // Just the name
    Stoped=1;
    return 0;
   }
 // If we are converting to a file create it
 if (out)
   {
    ToFile=new Rawtowav;
    if (!ToFile->initialize(out))
      {
       Stoped=0;
       return 0;
      }
   }
 // Now create the engine
 MP3Engine=new Mpegwavtoraw(InputStream,out ? (Soundplayer *)ToFile :
                            (Soundplayer *)Player);
 MP3Engine->initialize(FileToPlay,false);
 GetMP3Info();
 MP3Engine->startplay();

 // Fill the information fields with the data from the header
 SampleRate=MP3Engine->getfrequency();
 BitRate=MP3Engine->getbitrate();
 Mode=MP3Engine->isstereo() ? "stereo" : "mono";
 MPEGVer=MP3Engine->getversion()+1;
 Layer=MP3Engine->getlayer();
 TotalLen=MP3Engine->gettotaltime()/1000;

 Stoped=0;
 return 1;
}

long MP3Player::GetMP3Info(void)
{
 if (MP3Engine)
   {
    memcpy(Title,MP3Engine->getname(),31);
    memcpy(Author,MP3Engine->getartist(),31);
    memcpy(Album,MP3Engine->getalbum(),31);
    int i;
    for (i=0; Album[i]; i++);
    for (; i<30; i++) Album[i]=' ';
    Album[i]=0;
    strcat(Album,MP3Engine->getyear());
    memcpy(Comment,MP3Engine->getcomment(),31);
    Genre=MP3Engine->getgenre();
   }
 else
   {
    *Title=0;
    *Author=0;
    *Album=0;
    *Comment=0;
    Genre="Unknown";
   }
   
 if (!*Title)
   {
    int l=strlen(FileToPlay);
    if (l>30)
      {
       memcpy(Title,FileToPlay+(l-30),30);
       Title[0]=Title[1]=Title[2]='.';
       Title[30]=0;
      }
    else
      {
       memcpy(Title,FileToPlay,l);
       Title[l]=0;
      }
   }
 return 0;
}

void MP3Player::Stop()
{
 if (MP3Engine) MP3Engine->stopplay();
 Stoped=1;
 Paused=0;
 Converting=0;
}

static
int DoAPoll(int strategy)
{
 if (!MP3Engine) return -1;
 if (strategy==mp3UnBuffered)
   {
    if (MP3Engine->callformore())
       return MP3Engine->run(1) ? 0 : -1;
   }
 else
   {
    while (MP3Engine->callformore())
      if (!MP3Engine->run(1)) return -1;
   }
 return 0;
}

int MP3Player::Poll()
{
 if (!Stoped && !Paused)
   {
    #ifdef TVOS_DOS
    int ret=DoAPoll(Strategy);
    #else
    int ret=DoAPoll(mp3Buffered);
    #endif
    if (ret<0)
      {
       if (PlayingList)
         {
          PlayNext();
          if (Stoped)
             return 4;
          return 3;
         }
       Stoped=1;
       Converting=0;
       return 2;
      }
    return 1;
   }
 return 0;
}

int MP3Player::Init()
{
 Player=new Rawplayer;
 Player->initialize(Rawplayer::defaultdevice);
 if (Player->geterrorcode()!=SOUND_ERROR_OK) return 1;
 return 0;
}

void MP3Player::deInit()
{
 delete MP3Engine;
 delete InputStream;
 delete Player;
}

int MP3Player::GetTime()
{
 return MP3Engine ? MP3Engine->getcurtime()/1000 : 0;
}

void MP3Player::FFwd()
{
 if (!MP3Engine) return;
 if (!Converting && !Stoped)
   { // Compute 4 seconds in frames:
    int offset=(int)(4.0/OneFrameInSeconds());
    MP3Engine->setframe(MP3Engine->getcurrentframe()+offset);
   }
}

void MP3Player::Rew()
{
 if (!MP3Engine) return;
 if (!Converting && !Stoped)
   { // Compute 4 seconds in frames:
    int offset=(int)(4.0/OneFrameInSeconds());
    MP3Engine->setframe(MP3Engine->getcurrentframe()-offset);
   }
}

void MP3Player::TogglePause()
{
 if (!Converting && (!Stoped || Paused))
   {
    if (Paused)
      {
       if (!MP3Engine) PlayFileToPlay();
       if (!MP3Engine) return;
       MP3Engine->setframe(PausedPos);
       Paused=0;
      }
    else
      {
       if (!MP3Engine) return;
       PausedPos=MP3Engine->getcurrentframe();
       MP3Engine->stopplay();
       Paused=1;
      }
   }
}

void MP3Player::Pause()
{
 if (!Stoped && !Converting)
   {
    PausedPos=MP3Engine->getcurrentframe();
    MP3Engine->stopplay();
    Paused=1;
   }
}

void MP3Player::SeekAbs(int seconds)
{
 if (!MP3Engine) return;
 MP3Engine->settimepos(seconds*1000);
}

#endif // HAVE_MPEGSOUND

#endif // SUP_MP3
