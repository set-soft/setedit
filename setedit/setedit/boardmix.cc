/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <configed.h>
#include <mixer.h>

static int JoinChannels=1,SaveToDisk=0;
static const BOARD_MIXER *elements;
static int cantElements;
static char Disabled=0;

#ifdef HAVE_MIXER

#define Uses_TSSlider
#define Uses_TSStaticText
#define Uses_TSButton
#define Uses_TSCheckBoxes
#include <easydia1.h>
#define Uses_MsgBox
#define Uses_TEvent
#define Uses_fpstream
#include <tv.h>
#include <easydiag.h>
#define Uses_SETAppDialogs
#define Uses_SETAppConst
#include <setapp.h>

const int Height=15,chLeft=0,chRight=1;

static inline
void *CreateID(int index, int channel)
{
 return (void *)(long)(index | (channel<<16));
}

static inline
int DecomposeID(void *value, int &index)
{
 int val=(long)value;
 index=(val) & 0x7FFF;
 return (val)>>16;
}

static
void CallBack(int value, TScrollBarCB *, void *data)
{
 int index;
 if (DecomposeID(data,index)==chLeft)
    SetMixerValue(elements[index].id,MaxMixerVol-value,elements[index].right);
 else
    SetMixerValue(elements[index].id,elements[index].left,MaxMixerVol-value);
}

class TMixDiag : public TDialog
{
public:
 TMixDiag(const char *aTitle) :
   TWindowInit(&TMixDiag::initFrame),
   TDialog(TRect(1,1,1,1),aTitle) {};
 virtual void handleEvent(TEvent &event);
};

void TMixDiag::handleEvent(TEvent &event)
{
 TDialog::handleEvent(event);
 if (event.what==evBroadcast && event.message.command==cmClusterPress)
   {
    TCluster *cl=(TCluster *)event.message.infoPtr;
    uint32 val;
    cl->getData(&val);
    JoinChannels=val & 1;
    SaveToDisk=val & 2;
   }
}

void BoardMixerDialog()
{
 if (Disabled)
   {
    messageBox(__("Disabled by configuration"),mfError | mfOKButton);
    return;
   }
 elements=0;
 if (MixerInit())
    elements=GetElements(&cantElements);
 if (!elements || !cantElements)
   {
    messageBox(__("Failed to initialize mixer. Sorry"),mfError | mfOKButton);
    return;
   }

 // gcc 2.95.2 doesn't like the _() call in the inlined constructor, I wonder why
 // because 2.7.2.3 and 2.8.1 allows it as spected.
 TSViewCol *col=new TSViewCol(new TMixDiag(__("Hardward Mixer Settings")));

 TSStaticText *text=new TSStaticText(__("Mixer model:"));
 TSStaticText *name=new TSStaticText(GetMixerName());
 col->insert(xTSLeft,yTSUp,text);
 col->insert(xTSRightOf,yTSUp,name,text);
 TSSlider *ant=0,*act;
 int i,oldJoin=JoinChannels;
 JoinChannels=0; // Setup the values without propagation to the othe channel
 ant=new TSSlider(Height,IS_BOARD_MIXER_ELEM_STEREO(elements[0]) ? True : False,
                  elements[0].s_name,CallBack,CreateID(0,chLeft),
                  CallBack,CreateID(0,chRight),&JoinChannels,MaxMixerVol);
 ant->Set(MaxMixerVol-elements[0].left,MaxMixerVol-elements[0].right);
 col->insert(xTSLeft,yTSUnder,ant,0,text);
 for (i=1; i<cantElements; i++)
    {
     act=new TSSlider(Height,IS_BOARD_MIXER_ELEM_STEREO(elements[i]) ? True :
                      False,elements[i].s_name,CallBack,CreateID(i,chLeft),
                      CallBack,CreateID(i,chRight),&JoinChannels,MaxMixerVol);
     act->Set(MaxMixerVol-elements[i].left,MaxMixerVol-elements[i].right);
     col->insert(xTSRightOf,yTSUnder,act,ant,text);
     ant=act;
    }
 JoinChannels=oldJoin;
 TSCheckBoxes *lock=new TSCheckBoxes(new TSItem(__("~L~ock channels"),new TSItem(__("~S~ave to disk"),0)));
 col->insert(xTSLeft,yTSUnder,lock,0,ant);
 TSButton *ok=new TSButton(__("O~K~"),cmOK,bfDefault);
 ok->Flags=wSpan;
 col->insert(xTSRightOf,yTSUnder,ok,lock,ant);

 TDialog *d=col->doItCenter(cmeBoardMixer);
 uint32 ops=JoinChannels | SaveToDisk;
 execDialog(d,&ops);

 delete col;
 MixerDeInit();
}

void BoardMixerInit()
{
 if (Disabled) return;
 MixerInit();
 MixerDeInit();
}

void BoardMixerDisable()
{
 Disabled=1;
}

#else  // HAVE_MIXER

#define Uses_MsgBox
#define Uses_fpstream
#include <tv.h>

void BoardMixerDialog()
{
 messageBox(__("No mixer support linked in during configuration, sorry"),mfError | mfOKButton);
}

void BoardMixerInit()
{
}

void BoardMixerDisable()
{
}
#endif // HAVE_MIXER

const char Version=1;

void BoardMixerSave(fpstream& s)
{
 s << Version << (JoinChannels | SaveToDisk);
 elements=0; cantElements=0;
 if (SaveToDisk)
   {
    if (!Disabled && MixerInit())
       elements=GetElements(&cantElements);
   }
 else
    cantElements=0;
 s << cantElements;
 int i;
 for (i=0; i<cantElements; i++)
     s << elements[i].id << elements[i].right << elements[i].left;
 if (!Disabled && SaveToDisk) MixerDeInit();
}

void BoardMixerLoad(fpstream& s)
{
 char version;
 int flags,id,left,right;
 s >> version >> flags;
 JoinChannels=flags & 1;
 SaveToDisk=flags & 2;

 if (!Disabled && SaveToDisk) MixerInit();
 s >> cantElements;
 int i;
 for (i=0; i<cantElements; i++)
    {
     s >> id >> right >> left;
     if (SaveToDisk)
        SetMixerValue(id,left,right);
    }
 if (!Disabled && SaveToDisk) MixerDeInit();
}
