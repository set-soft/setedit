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
#include <settvuti.h>
#include <diaghelp.h>


TDialogAID::TDialogAID(const TRect& bounds, const char *aTitle,
                       TScrollBar *sb, TStringableListBox *slb ) :
  TGrowDialog(bounds,aTitle),
  TWindowInit(&TDialogAID::initFrame)
{
 insert(sb);
 slb->growMode=gfMoveBottomCorner;
 insert(slb);
 List=slb;
 AddAction=0;
 InsAction=0;
 DelAction=0;
 OkAction=0;
 CancelAction=0;
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

static int lbotAdd=0;
static int lbotIns;
static int lbotDel;
static int lbotOk;
static int lbotCan;

const int lSepb=2;

static
void InitBotLens(void)
{
 if (!lbotAdd)
   {
    lbotAdd=strlen(_(nbotAdd));
    lbotIns=strlen(_(nbotIns));
    lbotDel=strlen(_(nbotDel));
    lbotOk =max(strlen(_(nbotOk )),8);
    lbotCan=strlen(_(nbotCan));
   }
}

TDialogAID *CreateAddInsDelDialog(int x, int y, const char *name, int h, int w,
                                  int flags)
{
 TDialogAID *d;
 TRect r=TProgram::deskTop->getExtent();

 InitBotLens();

 int anBots1=lbotOk+lbotCan+3*lSepb+2;
 int anBots2=lbotAdd+lbotDel+3*lSepb+2;
 if (flags & aidInsert)
    anBots2+=lbotIns+lSepb;
 int W=max(max(anBots1,anBots2),max(strlen(_(name)),w+3));
 int H=h+7;
 if (flags & aidComMac)
    H+=3;

 if (x<=0)
    x=(r.b.x-W)>>1;
 if (y<=0)
    y=(r.b.y-H)>>1;

 int X=x+W;
 int Y=y+H;

 TScrollBar *sb=new TScrollBar(TRect(W-3,1,W-2,h+1));
 TStringableListBox *slb=new TStringableListBox(TRect(2,1,W-3,h+1),1,sb);
 d=new TDialogAID(TRect(x,y,X,Y),name,sb,slb);

 if (flags & aidComMac)
   {
    TRadioButtons32 *type  = new TRadioButtons32( TRect(2,H-8,W-2,H-6 ),
                             new TSItem( __("Command~s~"),
                             new TSItem( __("~M~acro"), 0 )));
    type->growMode=gfGrowHiX | gfGrowHiY | gfGrowLoY;
    d->insert(type);
    TLabel *tl=new TLabel(TRect(2,H-9,W-2,H-8),__("Assignmen~t~"),type);
    tl->growMode=gfGrowHiX | gfGrowHiY | gfGrowLoY;
    d->insert(tl);
   }

 //---- Buttons
 int sep;
 sep=(W-lbotOk-lbotCan)/3;
 x=sep-1;

 TButton *tb=new TButton(TRect(x,H-5,x+lbotOk+2,H-3),nbotOk,cmOKApply,bfDefault);
 tb->growMode=gfMoveAccording;
 d->insert(tb);
 x+=lbotOk+sep;
 tb=new TButton(TRect(x,H-5,x+lbotCan+2,H-3),nbotCan,cmCancelApply,bfNormal);
 tb->growMode=gfMoveAccording;
 d->insert(tb);

 if (flags & aidInsert)
    sep=(W-(lbotAdd+lbotDel+lbotIns))/4;
 else
    sep=(W-(lbotAdd+lbotDel))/3;
 x=sep-1;
 tb=new TButton(TRect(x,H-3,x+lbotAdd+2,H-1),nbotAdd,cmAddKey,bfNormal);
 tb->growMode=gfMoveAccording;
 d->insert(tb);
 x+=lbotAdd+sep;
 if (flags & aidInsert)
   {
    tb=new TButton(TRect(x,H-3,x+lbotIns+2,H-1),nbotIns,cmInsertKey,bfNormal);
    tb->growMode=gfMoveAccording;
    d->insert(tb);
    x+=lbotIns+sep;
   }
 tb=new TButton(TRect(x,H-3,x+lbotDel+2,H-1),nbotDel,cmDeleteKey,bfNormal);
 tb->growMode=gfMoveAccording;
 d->insert(tb);

 d->selectNext(False);
 d->flags=flags;
 return d;
}

// That creates the dialog, is generic and is reused
// Used to: (1) Choose a comand (2) Choose a macro
TDialog *CreateChooseDialog(int x, int y, const char *name, int h, int w)
{
 TDialog *d;

 InitBotLens();

 int anBots=lbotOk+lbotCan+3*lSepb+2; // 2 for frames
 int W=max(anBots,max(strlen(_(name)),w+3));
 int H=h+5;
 int X=x+W;
 int Y=y+H;

 TScrollBar *sb=new TScrollBar(TRect(W-3,1,W-2,H-4));
 TSortedListBox *slb=new TSortedListBox(TRect(2,1,W-3,H-4),1,sb);
 d=new TDialog(TRect(x,y,X,Y),name);
 d->insert(sb);
 d->insert(slb);
 AddOKCancel(d);
 d->selectNext(False);
 return d;
}



