/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class ipstream;
class TView;
class TDskWinMP3;
extern TView *MP3ReadInfo(ipstream &is, int &zorder, TDskWinMP3 *manager);

#ifdef SUP_MP3

class opstream;
class TView;

// From intermp3.cc
extern int  MP3ProcessCommand(int command);
extern void MP3PollPlayer();
extern void MP3InitStuff();
extern void MP3DeInitStuff();
extern void MP3PausePlayer();
extern void MP3ResumePlayer();
extern void MP3SetPlayerStrategy(int strategy);
extern void MP3WriteInfo(opstream &os, TView *view);
extern void MP3SetUpPlayListFunctions();

#define ProcessMP3Commands if (MP3ProcessCommand(event.message.command)) break
#define ProcessMP3Idle   MP3PollPlayer()
#define MP3Initialize    MP3InitStuff()
#define MP3DeInitialize  MP3DeInitStuff()
#define MP3Suspend       MP3PausePlayer()
#define MP3Resume        MP3ResumePlayer()
#define MP3Buffered      MP3SetPlayerStrategy(1)
#define MP3UnBuffered    MP3SetPlayerStrategy(0)
#define MP3SetUpPlayList MP3SetUpPlayListFunctions()

#ifdef Uses_PrivateMP3Info
// From ampdiag.cc
extern void MP3ShowTime();
extern void MP3CreateShowDialog();
// From mp3list.cc
extern void MP3EditPlayList();
extern char *MP3ListGetNext();
extern void MP3ListResetCount();
extern int  MP3ListHavePrev();
extern int  MP3ListHaveNext();
extern void MP3ListGoBack();
extern void MP3ListSaveState(opstream &os);
extern void MP3ListLoadState(ipstream &is);

extern void InitMP3Engine();

#endif // Uses_PrivateMP3Info

#else

extern int  MP3ProcessCommand(int command);
extern void MP3InitStuff();
extern void MP3ListLoadState(ipstream &is);

#define ProcessMP3Commands if (MP3ProcessCommand(event.message.command)) break
#define ProcessMP3Idle
#define MP3Initialize      MP3InitStuff()
#define MP3DeInitialize
#define MP3Suspend
#define MP3Resume
#define MP3Buffered
#define MP3UnBuffered
#define MP3SetUpPlayList

#endif

#ifdef Uses_PrivateMP3Info
const int cmaBase=0x22A0;
#define h(a,b) const int cma##a=cmaBase+b,hc##a=cmaBase+b;
h(UpdateTime,0)
h(IsAmpDiagThere,1)
h(UpdateFile,2)
h(MP3Play,3)
h(MP3Stop,4)
h(MP3Pause,5)
h(MP3Ffw,6)
h(MP3Rew,7)
h(ReflectStatus,8)
h(AddMP3,9)
h(InsertMP3,10)
h(DeleteMP3,11)
h(SaveMP3List,12)
h(LoadMP3List,13)
h(MP3Prev,14)
h(MP3Next,15)
#undef h

// These values are coordinated with the application to add a new
// command you must look the available number in setapp.h
const int cmeBase=0x2500;
const int
  cmeMP3Open        = cmeBase+42,
  cmeMP3Convert     = cmeBase+43,
  cmeMP3EditPlayList= cmeBase+44,
  cmeMP3Start       = cmeBase+45,
  cmeMP3Stop        = cmeBase+46,
  cmeMP3PlayList    = cmeBase+47,
  cmeMP3StopList    = cmeBase+48;
#endif
