/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string

#define Uses_TDialogAID
#define Uses_TKeys
#define Uses_TKeys_Extended
#define Uses_TEvent
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TRadioButtons
#define Uses_TSItem
#define Uses_TLabel
#define Uses_TButton
#define Uses_TRect
#define Uses_TSortedListBox

#define Uses_TSLabel
#define Uses_TSButton
#define Uses_TSHzGroup
#define Uses_TSStringableListBox
#define Uses_TSSortedListBox
#define Uses_TSVeGroup
#define Uses_TSLabelRadio

#include <easydia1.h>
#include <settvuti.h>
#include <easydiag.h>

#include <diaghelp.h>


TDialogAID::TDialogAID(const TRect& bounds, const char *aTitle,
                       TStringableListBox *slb ) :
  TWindowInit(&TDialogAID::initFrame),
  TGrowDialog(bounds,aTitle)
{
 List=slb;
 AddAction=0;
 InsAction=0;
 DelAction=0;
 OkAction=0;
 CancelAction=0;
 InfoAction=0;
}

static
void EnableComms(void)
{
 TView::enableCommand(cmOKApply);
 TView::enableCommand(cmDeleteKey);
 TView::enableCommand(cmInsertKey);
}

static
void DisableComms(unsigned leftOKEnabled)
{
 if (!leftOKEnabled)
    TView::disableCommand(cmOKApply);
 TView::disableCommand(cmDeleteKey);
 TView::disableCommand(cmInsertKey);
}


void TDialogAID::handleEvent(TEvent& event)
{
 if (event.what==evKeyDown)
   {
    switch (event.keyDown.keyCode)
      {
       case kbEsc:
            event.what=evCommand;
            event.message.command=cmCancelApply;
            break;
       case kbInsert:
            event.what=evCommand;
            event.message.command=(List->list())->GetCount() ? cmInsertKey : cmAddKey;
            if (event.message.command==cmInsertKey && InsAction==0)
               event.message.command=cmAddKey;
            break;
       case kbDelete:
            event.what=evCommand;
            event.message.command=cmDeleteKey;
            break;
       default:
            TDialog::handleEvent(event);
      }
   }
 else
    TDialog::handleEvent(event);

 if ( event.what == evCommand || event.what == evBroadcast)
   {
    switch ( event.message.command )
      {
       case cmAddKey:
            if (AddAction)
              {
               if (AddAction())
                  List->Update();
               if ((List->list())->GetCount())
                  EnableComms();
              }
            break;
       case cmInsertKey:
            if (InsAction && (List->list())->GetCount())
              {
               if (InsAction(List->focused))
                  List->Update();
              }
            break;
       case cmDeleteKey:
            if (DelAction && (List->list())->GetCount())
              {
               if (DelAction(List->focused))
                  List->Update();
               if ((List->list())->GetCount()==0)
                  DisableComms(flags & aidOKEnabled);
              }
            break;
       case cmOKApply:
            if (OkAction)
              {
               if (OkAction())
                  endModal(cmOK);
              }
            else
               endModal(cmOK);
            break;
       case cmCancelApply:
            if (CancelAction)
              {
               if (CancelAction())
                  endModal(cmCancel);
              }
            else
               endModal(cmCancel);
            break;
       case cmInfoAID:
            if (InfoAction)
               InfoAction(List->focused);
            break;
       case cmBrowseAID:
            if (BrowseAction)
              {
               if (BrowseAction())
                  List->Update();
               if ((List->list())->GetCount())
                  EnableComms();
              }
            break;
       case cmeZoom:
            event.message.command=cmZoom;
            TDialog::handleEvent(event);
            break;
       default:
           return;
      }
    clearEvent(event);
   }
}

static char *nbotAdd=__("~A~dd");
static char *nbotIns=__("~I~nsert");
static char *nbotDel=__("~D~elete");
static char *nbotOk =__("~O~k");
static char *nbotCan=__("~C~ancel");
static char *nbotInfo=__("~I~nfo.");
static char *nbotBrowse=__("~B~rowse");

const int lSepb=2;

TDialogAID *CreateAddInsDelDialog(int x, int y, const char *name, int h, int w,
                                  int flags)
{
 unsigned options=0;
 if (x<=0)
   {
    x=1;
    options|=ofCenterX;
   }
 if (y<=0)
   {
    y=1;
    options|=ofCenterY;
   }

 TSStringableListBox *slb=new TSStringableListBox(w,h+1,tsslbVertical);
 TSView *upper=slb;
 slb->view->growMode=gfMoveBottomCorner;
 TDialogAID *d=new TDialogAID(TRect(x,y,1,1),name,
                              (TStringableListBox *)slb->view);
 TSViewCol *col=new TSViewCol(d);

 if (flags & aidComMac)
   {
    TSLabel *tl=TSLabelRadio(__("Assignmen~t~"),__("Command~s~"),__("~M~acro"),
                             __("s~L~isp code"),0);
    tl->setGrowMode(gfGrowHiX | gfGrowHiY | gfGrowLoY);
    TSVeGroup *up=new TSVeGroup(slb,tl,0);
    upper=up;
   }
 upper->Flags=wSpan;

 TSButton *exBts[2];
 exBts[0]=exBts[1]=NULL;
 int idxEx=0;
 if (flags & aidInfo)
    exBts[idxEx++]=new TSButton(nbotInfo,cmInfoAID);
 if (flags & aidBrowse)
    exBts[idxEx++]=new TSButton(nbotBrowse,cmBrowseAID);
 TSHzGroup *bt1=MakeHzGroup(new TSButton(nbotOk,cmOKApply,bfDefault),
                            new TSButton(nbotCan,cmCancelApply),
                            exBts[0],exBts[1],NULL);
 bt1->setGrowMode(gfMoveAccording);
 bt1->ySep=0;
 TSHzGroup *bt2;
 if (flags & aidInsert)
   {
    bt2=MakeHzGroup(new TSButton(nbotAdd,cmAddKey),
                    new TSButton(nbotIns,cmInsertKey),
                    new TSButton(nbotDel,cmDeleteKey),
                    0);
   }
 else
   {
    bt2=MakeHzGroup(new TSButton(nbotAdd,cmAddKey),
                    new TSButton(nbotDel,cmDeleteKey),
                    0);
   }
 bt2->setGrowMode(gfMoveAccording);

 col->insert(xTSCenter,yTSUp,upper);
 col->insert(xTSCenter,yTSUnder,bt2,0,upper);
 col->insert(xTSCenter,yTSUnder,bt1,0,bt2);
 col->doIt();
 d->options|=options;
 d->flags=flags;
 return d;
}

// That creates the dialog, is generic and is reused
// Used to: (1) Choose a comand (2) Choose a macro
TDialog *CreateChooseDialog(int x, int y, const char *name, int h, int w,
                            unsigned options)
{
 unsigned opsDiag=0;
 if (x<=0)
   {
    x=1;
    opsDiag|=ofCenterX;
   }
 if (y<=0)
   {
    y=1;
    opsDiag|=ofCenterY;
   }
 TSViewCol *col=new TSViewCol(new TDialog(TRect(x,y,1,1),name));

 unsigned scrlBars=tsslbVertical;
 unsigned hzMax=w;
 if (options & aidHzScroll)
   {
    scrlBars|=tsslbHorizontal;
    hzMax=256;
   }
 TSView *slb;
 if (options & aidStringable)
    slb=new TSStringableListBox(w,h+1,scrlBars,1,hzMax);
 else
    slb=new TSSortedListBox(w,h+1,scrlBars,1,hzMax);
 col->insert(xTSCenter,yTSUp,slb);

 TSView *bts;
 if (options & aidNoCancel)
    bts=new TSButton(nbotOk,cmOK,bfDefault);
 else
    bts=MakeHzGroup(new TSButton(nbotOk,cmOK,bfDefault),
                    new TSButton(nbotCan,cmCancel),0);
 col->insert(xTSCenter,yTSDown,bts);

 TDialog *d=col->doIt();
 d->options|=opsDiag;

 return d;
}

void TGrowDialogZ::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if (event.what==evCommand || event.what==evBroadcast)
   {
    switch (event.message.command)
      {
       case cmeZoom:
            event.message.command=cmZoom;
            TDialog::handleEvent(event);
            break;
       default:
           break;
      }
   }
}

