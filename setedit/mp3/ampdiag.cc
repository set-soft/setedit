/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#ifdef SUP_MP3
#define Uses_stdio
#define Uses_string
#define Uses_limits

#define Uses_TApplication
#define Uses_TDialog
#define Uses_TEvent
#define Uses_TStaticText
#define Uses_TDeskTop
#define Uses_fpstream
#define Uses_MsgBox

#define Uses_TSNoStaticText
#define Uses_TSVeGroup
#define Uses_TSHzGroup
#define Uses_TSButton
#define Uses_TSHScrollBar

#include <easydia1.h>
#include <settvuti.h>
#include <easydiag.h>

#define Uses_PrivateMP3Info
#include <intermp3.h>
#include <mp3play.h>

#define Uses_SETAppHelper
#include <setapp.h>

#include <dskwin.h>
#include <dskmp3.h>

static TDskWinMP3 *MP3Manager=0;

void MP3ShowTime()
{
 static int lastTime;
 int thisTime=mp3.GetTime();

 if (thisTime!=lastTime)
   {
    lastTime=thisTime;
    message(TProgram::deskTop,evBroadcast,cmaUpdateTime,&thisTime);
   }
}

class TAmpDiag : public TDialog
{
public:
 TAmpDiag(const TRect& bounds, const char *aTitle) :
   TWindowInit(&TAmpDiag::initFrame),
   TDialog(bounds,aTitle) {lockSBTime=0;};
 virtual void handleEvent(TEvent &event);
 virtual void close();
 virtual Boolean valid(ushort command);

 void SetHeaderValues();
 void ReflectStatus();
 TNoStaticText *time,*status,*total;
 TNoStaticText *SR_BR;
 TNoStaticText *Mode_Type;
 TNoStaticText *info1,*info2,*info3,*info4,*info5;
 TScrollBar *timeBar;
 char lockSBTime;
};

void TAmpDiag::close()
{
 hide();
 if (MP3Manager)
    MP3Manager->CanBeSaved=0;
}

Boolean TAmpDiag::valid(ushort command)
{
 if (mp3.Converting)
   {
    unsigned ret=messageBox(__("You are expanding a MP3 file, do you really want to quit?"),
                            mfConfirmation | mfYesButton | mfNoButton);
    if (ret==cmYes)
      {
       mp3.Stop();
       return True;
      }
    return False;
   }
 return TDialog::valid(command);
}

void TAmpDiag::SetHeaderValues()
{
 char buffer[80];

 TProgram::deskTop->lock();
 TVIntl::snprintf(buffer,80,__("Sample Rate: %5d Hz   Bitrate: %4d Kbits/s"),
                  mp3.SampleRate,mp3.BitRate);
 SR_BR->setText(buffer);
 TVIntl::snprintf(buffer,80,__("Mode: %6s            File: MPEG %1d.0 layer %1d"),
                  mp3.Mode,mp3.MPEGVer,mp3.Layer);
 Mode_Type->setText(buffer);
 TVIntl::snprintf(buffer,80,__("Total: %3d:%02d"),mp3.TotalLen/60,mp3.TotalLen%60);
 total->setText(buffer);

 TVIntl::snprintf(buffer,80,__("Title:  %s"),mp3.Title);
 info1->setText(buffer);
 TVIntl::snprintf(buffer,80,__("Author: %s"),mp3.Author);
 info2->setText(buffer);
 TVIntl::snprintf(buffer,80,__("Album:  %s"),mp3.Album);
 info3->setText(buffer);
 TVIntl::snprintf(buffer,80,__("Comme.: %s"),mp3.Comment);
 info4->setText(buffer);
 TVIntl::snprintf(buffer,80,__("Genre:  %s"),mp3.Genre);
 info5->setText(buffer);

 timeBar->setParams(0,0,mp3.TotalLen,(int)(mp3.TotalLen/40.0+0.5),2);

 ReflectStatus();
 TProgram::deskTop->unlock();
}


void TAmpDiag::ReflectStatus()
{
 if (mp3.Paused)
    status->setText(__("[ PAUSE  ]"));
 else
 if (mp3.Stoped)
    status->setText(__("[  STOP  ]"));
 else
    status->setText(__("[  PLAY  ]"));
 if (TView::commandEnabled(cmaMP3Prev))
   {
    if (!mp3.PlayingList || !MP3ListHavePrev())
       TView::disableCommand(cmaMP3Prev);
   }
 else
   {
    if (mp3.PlayingList && MP3ListHavePrev())
       TView::enableCommand(cmaMP3Prev);
   }
 if (TView::commandEnabled(cmaMP3Next))
   {
    if (!mp3.PlayingList || !MP3ListHaveNext())
       TView::disableCommand(cmaMP3Next);
   }
 else
   {
    if (mp3.PlayingList && MP3ListHaveNext())
       TView::enableCommand(cmaMP3Next);
   }
}


void TAmpDiag::handleEvent(TEvent &event)
{
 char timestr[32];
 int thisTime;

 TDialog::handleEvent( event );
 if (event.what==evCommand)
   {
    switch( event.message.command )
      {
       case cmaMP3Play:
            if (mp3.Paused)
               mp3.TogglePause();
            else
               if (mp3.Stoped)
                 {
                  mp3.PlaySelectedFile();
                  SetHeaderValues();
                 }
            break;
       case cmaMP3Stop:
            mp3.Stop();
            break;
       case cmaMP3Pause:
            mp3.TogglePause();
            break;
       case cmaMP3Ffw:
            mp3.FFwd();
            break;
       case cmaMP3Rew:
            mp3.Rew();
            break;
       case cmaMP3Prev:
            MP3ListGoBack();
            mp3.PlayNext();
            SetHeaderValues();
            break;
       case cmaMP3Next:
            mp3.PlayNext();
            SetHeaderValues();
            break;
       default:
            return;
      }
    ReflectStatus();
   }
 else
  if (event.what==evBroadcast)
    {
     switch( event.message.command )
        {
         case cmaIsAmpDiagThere:
              break;
         case cmaUpdateTime:
              thisTime=*((int *)event.message.infoPtr);
              TVIntl::snprintf(timestr,32,__("Time: %3d:%02d"),thisTime/60,thisTime%60);
              time->setText(timestr);
              lockSBTime=1;
              timeBar->setValue(thisTime);
              lockSBTime=0;
              break;
         case cmaUpdateFile:
              SetHeaderValues();
              break;
         case cmaReflectStatus:
              ReflectStatus();
              break;
         case cmScrollBarChanged:
              if (!lockSBTime)
                 mp3.SeekAbs(timeBar->value);
              break;
         default:
              return;
        }
    }
  else
    return;
 clearEvent(event);
}

TAmpDiag *ampDia;

static
TAmpDiag *CreateShowDialog()
{
 ampDia=new TAmpDiag(TRect(1,1,1,1),__("Player status"));

 char buffer[80];
 int i;
 for (i=0; i<46; i++) buffer[i]=' ';
 buffer[i]=0;

 TSViewCol *col=new TSViewCol(ampDia);

 TSNoStaticText *info1=new TSNoStaticText(buffer);
 ampDia->info1=(TNoStaticText *)info1->view;
 TSNoStaticText *info2=new TSNoStaticText(buffer);
 ampDia->info2=(TNoStaticText *)info2->view;
 TSNoStaticText *info3=new TSNoStaticText(buffer);
 ampDia->info3=(TNoStaticText *)info3->view;
 TSNoStaticText *info4=new TSNoStaticText(buffer);
 ampDia->info4=(TNoStaticText *)info4->view;
 TSNoStaticText *info5=new TSNoStaticText(buffer);
 ampDia->info5=(TNoStaticText *)info5->view;
 TSVeGroup *Info=MakeVeGroup(0,info1,info2,info3,info4,info5,0);

 TSNoStaticText *SR_BR=new TSNoStaticText(buffer);
 ampDia->SR_BR=(TNoStaticText *)SR_BR->view;
 TSNoStaticText *Mode_Type=new TSNoStaticText(buffer);
 ampDia->Mode_Type=(TNoStaticText *)Mode_Type->view;
 TSVeGroup *Rest=new TSVeGroup(SR_BR,Mode_Type,0);

 TSNoStaticText *time=new TSNoStaticText(__("Time: 000:00"));
 ampDia->time=(TNoStaticText *)time->view;
 TSNoStaticText *status=new TSNoStaticText(__("[  STOP  ]"));
 ampDia->status=(TNoStaticText *)status->view;
 TSNoStaticText *total=new TSNoStaticText(__("Total: 000:00"));
 ampDia->total=(TNoStaticText *)total->view;
 TSHzGroup *Time=new TSHzGroup(time,new TSHzGroup(status,total,2),2);

 TSHScrollBar *timeBar=new TSHScrollBar(40);
 ampDia->timeBar=timeBar->sb;

 TSHzGroup *buttons=MakeHzGroup(
                    new TSButton(__("Prev."),cmaMP3Prev),
                    new TSButton(MP3Player::butRew,cmaMP3Rew),
                    new TSButton(MP3Player::butStop,cmaMP3Stop,bfDefault),
                    new TSButton(MP3Player::butPlay,cmaMP3Play),
                    new TSButton(MP3Player::butPause,cmaMP3Pause),
                    new TSButton(MP3Player::butFfw,cmaMP3Ffw),
                    new TSButton(__("Next"),cmaMP3Next),
                    0);

 ampDia->SetHeaderValues();
 col->insert(2,1,Info);
 col->insert(2,yTSUnder,Rest,0,Info);
 col->insert(xTSCenter,yTSUnder,Time,0,Rest);
 col->insert(xTSCenter,yTSUnder,timeBar,0,Time);
 col->insert(xTSCenter,yTSDown,buttons);

 ampDia->options|=ofCentered;
 col->doIt();

 delete col;
 return ampDia;
}

void MP3CreateShowDialog()
{
 // First check if already in desktop:
 if (!message(TProgram::deskTop,evBroadcast,cmaIsAmpDiagThere,0))
   {
    CreateShowDialog();
    TProgram::deskTop->insert(ampDia);
    MP3Manager=new TDskWinMP3(ampDia);
    AddNonEditorToHelper(MP3Manager);
   }
}


void MP3WriteInfo(opstream &os, TView *view)
{
 TAmpDiag *edw=(TAmpDiag *)view;
 os << (int)1 << edw->origin << (int)(TProgram::deskTop->indexOf(edw));

 // Stoped means 1 => no song is playing 0 => a song is playing so that's a very
 // important variable
 os << mp3.Stoped <<
 // When the engine is in Pause is at the same time stoped, but we have information
 // to resume the playing
       mp3.Paused << mp3.PausedPos;
 // That's the name of the file we are playing
 os.writeString(mp3.FileToPlay);
 // Now the information for a single sound stuff
 os << mp3.Selected;
 if (mp3.Selected)
   {
    os.writeString(mp3.Name);
   }
 // Now stop playing
 mp3.Pause();
 // Remember the position
 os << mp3.PausedPos;
 // And finally the list stuff
 os << mp3.PlayingList;
 MP3ListSaveState(os);
}

TView *MP3ReadInfo(ipstream &is, int &zorder, TDskWinMP3 *manager)
{
 TPoint pos;
 int version;
 int from;

 MP3Manager=manager;

 is >> version >> pos >> zorder;
 is >> mp3.Stoped >> mp3.Paused >> mp3.PausedPos;
 is.readString(mp3.FileToPlay,PATH_MAX);
 is >> mp3.Selected;
 if (mp3.Selected)
   {
    is.readString(mp3.Name,PATH_MAX);
   }
 is >> from >> mp3.PlayingList;
 MP3ListLoadState(is);

 InitMP3Engine();
 if (mp3.Selected && (mp3.Stoped && !mp3.Paused))
   {
    TView::enableCommand(cmeMP3Start);
    TView::enableCommand(cmeMP3Convert);
   }
 if (mp3.PlayingList)
   {// Setup the playing list stuff first!!
    MP3SetUpPlayList;
    TView::enableCommand(cmeMP3StopList);
    TView::disableCommand(cmeMP3PlayList);
    TView::disableCommand(cmeMP3EditPlayList);
    if (!mp3.Paused)
      {
       mp3.Paused=1;
       mp3.PausedPos=from;
       mp3.TogglePause();
      }
    else
      {
       mp3.TogglePause();
       mp3.TogglePause();
      }
   }
 else
   {
    if (!mp3.Stoped || mp3.Paused)
       TView::enableCommand(cmeMP3Stop);
    if (!mp3.Stoped && !mp3.Paused)
      {
       mp3.Paused=1;
       mp3.PausedPos=from;
       mp3.TogglePause();
      }
    else
      if (mp3.Paused)
        {
         mp3.TogglePause();
         mp3.TogglePause();
        }
      else
        {
         mp3.PlayFileToPlay();
         mp3.Stop();
        }
   }
 CreateShowDialog();
 ampDia->moveTo(pos.x,pos.y);
 return ampDia;
}

#else

#define Uses_TDialog
#define Uses_TRect
#define Uses_fpstream
#define Uses_TStaticText
#define Uses_TView
#define Uses_limits
#include <tv.h>

#include <dskwin.h>
#include <dskmp3.h>

#include <intermp3.h>

class TDialogX : public TDialog
{
public:
 TDialogX(const TRect& bounds, const char *aTitle) :
   TWindowInit(&TDialogX::initFrame),
   TDialog(bounds,aTitle) {};
 virtual void close() { hide(); };
};

TView *MP3ReadInfo(ipstream &is, int &zorder, TDskWinMP3 * /*manager*/)
{
 TPoint pos;
 int iAux;
 char aux;
 char buf[PATH_MAX];

 is >> iAux >> pos >> zorder;
 is >> aux >> aux >> iAux;
 is.readString(buf,PATH_MAX);
 is >> aux;
 if (aux)
   {
    is.readString(buf,PATH_MAX);
   }
 is >> iAux >> aux;
 MP3ListLoadState(is);

 TDialogX *d=new TDialogX(TRect(0,0,30,6),__("MP3 Player"));
 d->insert(new TStaticText(TRect(2,2,28,3),__("MP3 support not linked")));
 d->options|=ofCentered;
 return d;
}
#endif
