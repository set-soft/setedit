/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#ifdef SUP_MP3
#define Uses_string
#define Uses_TView
#define Uses_MsgBox
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_FileOpenAid
#include <settvuti.h>

// Commands and functions
#define Uses_PrivateMP3Info
#include <intermp3.h>
// MP3 playing routings interface
#include <mp3play.h>

// ************* Functions to process the commands from the editor *****************
static
void CommandsForStop()
{
 TView::disableCommand(cmeMP3Stop);
 TView::enableCommand(cmeMP3Start);
 TView::enableCommand(cmeMP3Convert);
}

static
void CommandsForPlay()
{
 TView::enableCommand(cmeMP3Stop);
 TView::disableCommand(cmeMP3Start);
 TView::disableCommand(cmeMP3Convert);
}

void InitMP3Engine()
{
 static int mp3Initialized=0;
 if (!mp3Initialized)
   {
    if (mp3.Init())
       return;
    else
       mp3Initialized=1;
   }
}

static
void Open()
{
 char fileName[PATH_MAX];
 strcpy(fileName,"*.mp[23]");

 if (GenericFileDialog(__("MP3 Open"),fileName,0,hID_OpenMP3)!=cmCancel)
   {
    InitMP3Engine();
    mp3.SelectNewFile(fileName);
    TView::enableCommand(cmeMP3Start);
    TView::enableCommand(cmeMP3Convert);
   }
}

static
void Convert()
{
 char fileName[PATH_MAX];
 strcpy(fileName,"*.wav");

 if (GenericFileDialog(__("Output WAV file"),fileName,0,hID_SaveMP3,fdDialogForSave)
     ==cmCancel)
    return;

 switch (mp3.ConvertSelectedFile(fileName))
   {
    case 1:
         messageBox(__("First select a file"),mfError | mfOKButton);
         break;
    case 2:
         message(TProgram::deskTop,evBroadcast,cmaUpdateFile,0);
         MP3CreateShowDialog();
         CommandsForStop();
         break;
   }
}

static
void Start()
{
 switch (mp3.PlaySelectedFile())
   {
    case 1:
         messageBox(__("First select a file"),mfError | mfOKButton);
         break;
    case 2:
         message(TProgram::deskTop,evBroadcast,cmaUpdateFile,0);
    case 3:
         MP3CreateShowDialog();
         CommandsForStop();
         break;
   }
}

static
void Stop()
{
 mp3.Stop();
 message(TProgram::deskTop,evBroadcast,cmaReflectStatus,0);
 CommandsForPlay();
}

static
void CommandsForStopList()
{
 TView::disableCommand(cmeMP3StopList);
 TView::enableCommand(cmeMP3PlayList);
 TView::enableCommand(cmeMP3EditPlayList);
}

static
void CommandsForPlayList()
{
 TView::enableCommand(cmeMP3StopList);
 TView::disableCommand(cmeMP3PlayList);
 TView::disableCommand(cmeMP3EditPlayList);
}

static
void MP3EndOfList()
{
 MP3ListResetCount();
 CommandsForStopList();
}

static
void PlayList()
{
 InitMP3Engine();
 mp3.StartPlayList(MP3ListGetNext,MP3EndOfList);
 if (!mp3.Stoped)
   {
    message(TProgram::deskTop,evBroadcast,cmaUpdateFile,0);
    MP3CreateShowDialog();
    CommandsForPlayList();
   }
}

void MP3SetUpPlayListFunctions()
{
 mp3.SetupPlayList(MP3ListGetNext,MP3EndOfList);
}

static
void StopList()
{
 mp3.StopPlayList();
 MP3ListResetCount();
 CommandsForStopList();
}



// ************* Checks if the command is an MP3 one *****************
#define m(a) case cmeMP3##a: a(); break;
#define k(a) case cmeMP3##a: MP3##a(); break;
int MP3ProcessCommand(int command)
{
 switch (command)
  {
   m(Open)
   m(Convert)
   k(EditPlayList)
   m(Start)
   m(Stop)
   m(PlayList)
   m(StopList)
   default:
        return 0;
  }
 return 1;
}
#undef k
#undef m

void MP3PollPlayer()
{
 switch (mp3.Poll())
   {
    case 1:
         MP3ShowTime();
         break;
    case 2:
         TView::disableCommand(cmeMP3Stop);
         TView::enableCommand(cmeMP3Start);
    case 4:
         message(TProgram::deskTop,evBroadcast,cmaReflectStatus,0);
         break;
    case 3:
         message(TProgram::deskTop,evBroadcast,cmaUpdateFile,0);
         break;
   }
}

void MP3DeInitStuff()
{
 mp3.deInit();
}

// The following routines are used to temporally stop the engine and then resume
// for example when calling the shell.

static int playerWasPaused=0;

void MP3PausePlayer()
{
 if (!playerWasPaused && !mp3.Stoped)
   {
    mp3.Pause();
    playerWasPaused=1;
   }
}

void MP3ResumePlayer()
{
 if (playerWasPaused)
   {
    mp3.TogglePause();
    playerWasPaused=0;
   }
}

void MP3SetPlayerStrategy(int strategy)
{
 mp3.Strategy=strategy ? mp3Buffered : mp3UnBuffered;
}

#else

#define Uses_MsgBox
#define Uses_TView
#include <tv.h>
// Commands and functions
#define Uses_PrivateMP3Info
#include <intermp3.h>

#define m(a) case cmeMP3##a:
int MP3ProcessCommand(int command)
{
 switch (command)
  {
   m(Open)
   m(Convert)
   m(EditPlayList)
   m(Start)
   m(Stop)
   m(PlayList)
   m(StopList)
     messageBox(__("The MP3 routines aren't linked in the editor"),mfError | mfOKButton);
     break;
   default:
        return 0;
  }
 return 1;
}
#undef m

#endif

void MP3InitStuff()
{
 #define d(a) TView::disableCommand(cmeMP3##a)
 d(Start);
 d(Stop);
 d(PlayList);
 d(StopList);
 d(Convert);
 #undef d
}

