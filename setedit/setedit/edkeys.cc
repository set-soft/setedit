/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdio.h>

#define Uses_TDialog
#define Uses_TSortedListBox
#define Uses_TRect
#define Uses_MsgBox
#define Uses_TScrollBar
#define Uses_TButton
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TNoCaseNoOwnerStringCollection
#define Uses_TNoCaseStringCollection
#define Uses_TCEditor_Internal
#define Uses_TCEditor_Commands
#define Uses_TFileDialog
#define Uses_TInputScanKey
#define Uses_TRadioButtons
#define Uses_TSItem
#define Uses_TLabel
#define Uses_TStringable
#define Uses_TStringableListBox
#define Uses_TStaticText
#define Uses_TKeys
#define Uses_TKeys_Extended
#define Uses_TDialogAID
#define Uses_TGKey

#define Uses_TSLabelRadio
#define Uses_TSLabel
#define Uses_TSButton

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#define Uses_TMLIEditorDefs
#include <mli.h>
#include <loadkbin.h>
#include <edmsg.h>
#define Uses_SETAppDialogs
#define Uses_SETAppConst
#include <setapp.h>
#include <edhists.h>
#include <ctype.h>
#define Uses_TKeyTranslate
#define Uses_TKeySeqCol
#define Uses_TComSeqCol
#include <keytrans.h>
#include <diaghelp.h>
// For the helpCtx:
#include <editcoma.h>

//------------------------------ Forward declarations

static int InitVariables(void);
static void DeInitVariables(void);
static int DeleteKey(int wich);
static int AddNewKey(void);
static int DeleteKeyInSeq(int wich);
static int AddNewKeyInSeq(void);
static int InsNewKeyInSeq(int wich);
static int DeleteComInSeq(int wich);
static int AddNewComInSeq(void);
static int InsNewComInSeq(int wich);
static char *AddMacro(void);
static TComSeqCol *AddCommands(void);
static int addKey(TKeySeqCol *sKeys, void *data, int Type);
static int OkApply(void);
static int CancelApply(void);
static int AddCommOrMacro(void);

//------------------------------ Constants

const int lNameKeys=32;
const int lAddb=11,lDelb=10;
const int lOKb=10,lCancelb=12,lSepb=2;
const int ktyCommand=1,ktyMacro=2;
// Columns for the dialogs
const int col1=1,col2=12,col3=23,col4=34;

//------------------------------ Types

typedef struct
{
 TStringable *items;
 int selected;
 uint32 ops;
} TStrLB;

//------------------------------ Static variables

static int Modified,wasModified;

static TDialogAID *ChooseFirstDialog=0;
static TStringableListBoxRec FirstSel;
static char lGroupSetData;

static TKeySeqCol *KSequence;
static TComSeqCol *CSequence;

static KeyTTable *Original;
static int CanBeDeleted;

static TDialog *MacrosDialog=0;
static TListBoxRec MacBox;

static TDialogAID *AddKeyDialog;
static TStrLB NewKeyBox;

static TDialogAID *AddComDialog;
static TStringableListBoxRec NewComBox;

static TDialog *CommandsDialog=0;
static TListBoxRec ComBox;
static TNoCaseNoOwnerStringCollection *Commands=0;

//------------------------------ Helper functions, can be moved

#define DisableCommands() \
 Boolean oldOk=TView::commandEnabled(cmOKApply); \
 Boolean oldDel=TView::commandEnabled(cmDeleteKey); \
 Boolean oldIns=TView::commandEnabled(cmInsertKey); \
 TView::disableCommand(cmOKApply); \
 TView::disableCommand(cmDeleteKey); \
 TView::disableCommand(cmInsertKey)

#define RestoreCommands() \
 if (oldOk) \
    TView::enableCommand(cmOKApply); \
 else  \
    TView::disableCommand(cmOKApply); \
 if (oldDel) \
    TView::enableCommand(cmDeleteKey); \
 else \
    TView::disableCommand(cmDeleteKey); \
 if (oldIns) \
    TView::enableCommand(cmInsertKey); \
 else \
    TView::disableCommand(cmInsertKey)

//------------------------------ Classes

class TDialogK : public TDialog
{
public:
 TDialogK(const TRect& bounds, const char *aTitle) :
   TDialog(bounds,aTitle),
   TWindowInit(&TDialogK::initFrame) {}
 virtual void handleEvent(TEvent& event);
};

//---------- Entry point

/**[txh]********************************************************************

  Description:
  Edits the keyboard binding data using a friendly interface.

  Return:
  0 if no modifications were done.

***************************************************************************/

int KeyBindEdit(void)
{
 InitVariables();
 TView::enableCommand(cmOKApply);
 TView::enableCommand(cmAddKey);
 TView::enableCommand(cmInsertKey);
 execDialogNoDestroy(ChooseFirstDialog,&FirstSel,lGroupSetData);
 DeInitVariables();
 return wasModified;
}

//----------- Routines to init, deinit and apply the things done

static int InitVariables(void)
{
 Original=KeyTrans.expand(CanBeDeleted);

 ChooseFirstDialog=CreateAddInsDelDialog(col1,1,_("Key assignment"),12,lNameKeys+4,0);
 ChooseFirstDialog->helpCtx=hcEditKeys;
 FirstSel.items=&KeyTrans;
 FirstSel.selection=0;
 lGroupSetData=1;
 ChooseFirstDialog->DelAction=DeleteKey;
 ChooseFirstDialog->AddAction=AddNewKey;
 ChooseFirstDialog->OkAction=OkApply;
 ChooseFirstDialog->CancelAction=CancelApply;

 Modified=0;
 wasModified=0;

 return 0;
}

static void ApplyChanges(void)
{
 wasModified=1;
}

static void DeInitVariables(void)
{
 if (wasModified)
   {
    if (CanBeDeleted)
       delete Original;
    KeyTrans.compact();
   }
 else
   {
    KeyTrans.ChangeTable(Original,CanBeDeleted ? kbtDynamic : kbtStatic);
   }

 Destroy(ChooseFirstDialog)
 Destroy(CommandsDialog)
 Destroy(Commands)
 Destroy(MacrosDialog)
}

//---------- First dialog: Add/Delete keys

// Action for delete button
static int DeleteKey(int wich)
{
 KeyTrans.deleteKey(wich);
 Modified=1;
 return 1;
}

static
int OkApply(void)
{
 if (Modified)
   {
    if (messageBox(_("Accepting that you'll change the SET's editor behavior"),mfOKCancel)==cmOK)
      {
       ApplyChanges();
       return 1;
      }
   }
 else
   return 1;
 return 0;
}

static
int CancelApply(void)
{
 if (Modified)
   {
    if (messageBox(_("Do you really want to discard the changes?"),mfOKCancel)==cmOK)
       return 1;
   }
 else
   return 1;
 return 0;
}


//---------- Second level of dialog: Add a key

static int AddNewKey(void)
{
 AddKeyDialog=CreateAddInsDelDialog(col2,3,_("Sequence of keys"),6,24,
                                    aidInsert | aidComMac);
 AddKeyDialog->helpCtx=hcEditKeysSeq;
 KSequence=new TKeySeqCol(2,2);
 NewKeyBox.items=KSequence;
 NewKeyBox.selected=0;
 NewKeyBox.ops=0;
 AddKeyDialog->DelAction=DeleteKeyInSeq;
 AddKeyDialog->AddAction=AddNewKeyInSeq;
 AddKeyDialog->InsAction=InsNewKeyInSeq;
 AddKeyDialog->OkAction=AddCommOrMacro;

 DisableCommands();
 int added=(execDialog(AddKeyDialog,&NewKeyBox)==cmOK);
 RestoreCommands();

 destroy(KSequence);
 return added;
}

static
int AddCommOrMacro(void)
{
 AddKeyDialog->getData(&NewKeyBox);

 if (NewKeyBox.ops)
   {
    char *m=AddMacro();
    if (m)
       return addKey(KSequence,strdup(m),kbtIsMacro);
   }
 else
   {
    TComSeqCol *p=AddCommands();
    if (p)
      {
       int ret=addKey(KSequence,p,kbtIsSeq);
       destroy(p);
       return ret;
      }
   }
 return 0;
}

static
int addKey(TKeySeqCol *sKeys, void *data, int Type)
{
 int ret=KeyTrans.addKey(sKeys,data,Type);
 if (ret>=0)
   {
    if (messageBox(_("This key is already in use, do you want to replace it?"),mfYesButton | mfNoButton)==cmYes)
      {
       KeyTrans.deleteKey(ret);
       KeyTrans.addKey(sKeys,data,Type);
       Modified=1;
       return 1;
      }
    return 0;
   }
 else
   if (ret==-1)
     {
      messageBox(_("This sequence is used for more than one key, delete these assignments first"),mfOKButton);
      return 0;
     }
 Modified=1;
 return 1;
}

void TDialogK::handleEvent(TEvent& event)
{
 if (event.what==evKeyDown)
   {
    endModal(event.keyDown.keyCode);
    clearEvent(event);
    return;
   }
 TDialog::handleEvent(event);
}

static char *press_key=__("Press a key");

unsigned short TCEditor_SelectAKey(void)
{
 TDialogK *d=new TDialogK(TRect(0,0,30,5),_("Key selector"));
 d->options|=ofCentered;

 const char *message=_(press_key);
 int x=(30-strlen(message))/2;
 TStaticText *ts=new TStaticText(TRect(x,2,29,3),message);
 d->insert(ts);

 return execDialog(d,0);
}

static
int DeleteKeyInSeq(int wich)
{
 KSequence->atRemove(wich);
 return 1;
}

static
int AddNewKeyInSeq(void)
{
 unsigned short k=TCEditor_SelectAKey();
 if (k)
   {
    KSequence->insert(k);
    return 1;
   }
 return 0;
}

static
int InsNewKeyInSeq(int wich)
{
 unsigned short k=TCEditor_SelectAKey();
 if (k)
   {
    KSequence->atInsert(wich,(void *)(unsigned)k);
    return 1;
   }
 return 0;
}

//---------- Third level of dialog: Add a Macro

static
char *AddMacro(void)
{
 char isFirstTime=0;

 if (!MacrosDialog)
   {
    isFirstTime=1;
    MacrosDialog=CreateChooseDialog(col3,5,_("Macros"),10,32);
    MacrosDialog->helpCtx=hcEditKeysMac;
    MacBox.items=GetMacrosList();
    MacBox.selection=0;
   }

 if (execDialogNoDestroy(MacrosDialog,&MacBox,isFirstTime)==cmCancel)
    return 0;
 if (MacBox.items->getCount() == 0)
    return 0;
 return (char *)(MacBox.items->at(MacBox.selection));
}

static
TComSeqCol *AddCommands(void)
{
 AddComDialog=CreateAddInsDelDialog(col3,5,_("Commands"),6,32,aidInsert);
 AddComDialog->helpCtx=hcEditKeysCom;
 CSequence=new TComSeqCol(2,6);
 NewComBox.items=CSequence;
 NewComBox.selection=0;
 AddComDialog->DelAction=DeleteComInSeq;
 AddComDialog->AddAction=AddNewComInSeq;
 AddComDialog->InsAction=InsNewComInSeq;

 DisableCommands();
 int ret=execDialog(AddComDialog,&NewComBox);
 RestoreCommands();
 if (ret==cmOK)
    return CSequence;

 destroy(CSequence);
 return 0;
}

//---------- Fourth level of dialog: Choose a command

static
int SelectACom(void)
{
 char isFirstTime=0;

 if (!CommandsDialog)
   {
    isFirstTime=1;
    CommandsDialog=CreateChooseDialog(col4,7,_("A command"),10,20);
    Commands=CreateEdCommandCol();
    ComBox.items=Commands;
    ComBox.selection=0;
   }

 if (execDialogNoDestroy(CommandsDialog,&ComBox,isFirstTime)==cmCancel)
    return -1;

 int command=SearchEdCommand((char *)(Commands->at(ComBox.selection)));
 if (command>0)
    command+=cmbBaseNumber;
 return command;
}

static
int DeleteComInSeq(int wich)
{
 CSequence->atRemove(wich);
 return 1;
}

static
int AddNewComInSeq(void)
{
 int c=SelectACom();
 if (c>=0)
   {
    CSequence->insert(c);
    return 1;
   }
 return 0;
}

static
int InsNewComInSeq(int wich)
{
 int c=SelectACom();
 if (c>=0)
   {
    CSequence->atInsert(wich,(void *)c);
    return 1;
   }
 return 0;
}

//--------------- Other facilities

void SeeScanCodes(void)
{
 TDialog *d=new TDialog(TRect(0,0,30,7),_("Scan Viewer"));
 d->options|=ofCentered;
 d->helpCtx=cmeSeeScanCodes;
 TPoint p;
 p.x=5; p.y=2;
 d->insert(new TInputScanKey(p));
 d->insert(new TButton(TRect(10,4,20,6),_("~O~k"),cmOK,bfNormal));
 d->selectNext(False);
 execDialog(d,0);
}

int AltKeysSetUp(void)
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),_("Alt keys settings")));

 TSLabel *Hotkey=TSLabelRadio(__("~K~eys used by the menues, etc."),
                              __("~L~eft Alt"),__("~R~ight Alt"),
                              __("~B~oth Alt"),0);

 col->insert(2,2,Hotkey);
 EasyInsertOKCancel(col);

 TDialog *d=col->doIt();
 d->options|=ofCentered;
 d->helpCtx=cmeSetUpAltKeys;
 delete col;

 uint32 which=TGKey::GetAltSettings();

 if (execDialog(d,&which)==cmOK)
   {
    TGKey::SetAltSettings(which);
    return 1;
   }
 return 0;
}

int KeyPadSetUp(void)
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),_("Key Pad behavior")));

 TSLabel *Behave=TSLabelRadio(__("Behavior"),__("Mantain the ~B~IOS default"),
                              __("Ever interpret shift+arrow as ~m~ovement"),0);
 col->insert(2,2,Behave);
 EasyInsertOKCancel(col);

 TDialog *d=col->doIt();
 d->options|=ofCentered;
 d->helpCtx=cmeKeyPadBehavior;
 delete col;

 uint32 b=TGKey::GetKbdMapping(TGKey::dosTranslateKeypad);
 if (execDialog(d,&b)==cmOK)
   {
    TGKey::SetKbdMapping(b ? TGKey::dosTranslateKeypad : TGKey::dosNormalKeypad);
    return 1;
   }
 return 0;
}

