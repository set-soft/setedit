/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_stdio
#define Uses_string

#define Uses_snprintf
#define Uses_MsgBox
#define Uses_TDialog
#define Uses_TRadioButtons
#define Uses_TButton
#define Uses_TSItem
#define Uses_TLabel
#define Uses_TRect
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TScreen
#define Uses_TInputLine
#define Uses_TSortedListBox
#define Uses_TScrollBar
#define Uses_TStringCollection
#define Uses_TCheckBoxes
#define Uses_TCEditor
#define Uses_TDeskTopClock
#define Uses_TApplication
#define Uses_TCEditWindow
#define Uses_TStaticText
#define Uses_fpstream
#define Uses_TSOSListBox
#define Uses_TVCodePage
#define Uses_TVFontCollection

// EasyDiag requests
#define Uses_TSButton
#define Uses_TSLabelRadio
#define Uses_TSLabelCheck
#define Uses_TSRadioButtons
#define Uses_TSInputLine
#define Uses_TSVeGroup
#define Uses_TSSortedListBox
#define Uses_TSHzGroup
#define Uses_TSCheckBoxes
#define Uses_TSHzLabel
#define Uses_TSStaticText
#define Uses_TFileCollection

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#include <diaghelp.h>
#include <stdlib.h>
#define Uses_SETAppVarious
#define Uses_SETAppDialogs
#define Uses_TSetEditorApp
#define Uses_SETAppConst
#define Uses_SETAppFiles
#include <setapp.h>
#include <codepage.h>
#include <advice.h>
#include <edcollec.h>
#include <edspecs.h>
#define STANDALONE
#define Uses_TSOSListBoxMsg
#include <edmsg.h>

// Forced width of the encodings and fonts list boxes
const int wForced=24;

/******************************* File Open Dialog options ****************************/
typedef struct
{
 uint32 sortType;
 uint32 caseType;
 uint32 parentSort;
 uint32 dotFiles;
 uint32 exclude;
} BoxFO;

class TDiaFO : public TDialog
{
public:
 TDiaFO();
 virtual void handleEvent(TEvent& event);
 void updateState(unsigned sType);

 int parentSortEnabled;
 TRadioButtons32 *sortType;
 TView *parentSort;
};

TDiaFO::TDiaFO() :
         TWindowInit(&TDiaFO::initFrame),
         TDialog(TRect(1,1,1,1),__("Open File options"))
{
 parentSortEnabled=1;
}

void TDiaFO::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if (event.what==evBroadcast)
   {
    if (event.message.command==cmClusterPress || event.message.command==cmClusterMovedTo)
      { // OK, a child changed now see if that's the sortType
       if (event.message.infoPtr==sortType)
         { // I know how to use &&, but I preffer it ;-)
          uint32 sType;
          sortType->getData(&sType);
          updateState(sType);
         }
      }
   }
}

void TDiaFO::updateState(unsigned sType)
{
 if (parentSortEnabled && sType==fcolAlphabetical)
   {
    parentSort->setState(sfDisabled,True);
    parentSortEnabled=0;
   }
 else
 if (!parentSortEnabled && sType!=fcolAlphabetical)
   {
    parentSort->setState(sfDisabled,False);
    parentSortEnabled=1;
   }
}

int SetFileOpenDialogOptions(void)
{
 TDiaFO *d=new TDiaFO();
 TSViewCol *col=new TSViewCol(d);

 BoxFO box;
 //-------------- Transfer options
 unsigned oldOptions=TFileCollection::sortOptions;
 box.sortType=TFileCollection::sortOptions & fcolTypeMask;
 box.caseType=(TFileCollection::sortOptions & fcolCaseInsensitive) >> fcolCaseBit;
 box.parentSort=(TFileCollection::sortOptions & fcolParentLast) >> fcolParentBit;
 box.dotFiles=(TFileCollection::sortOptions & fcolDotsLast) >> fcolDotsBit;
 box.exclude=(TFileCollection::sortOptions & fcolHideMask) >> fcolHideBit;

 // ACDEFGIKLNPRSW
 TSLabel *Sort=TSLabelRadio(__("~S~ort type"),__("~A~lphabetical"),
                            __("~D~irectories first"),__("~F~iles first"),0);
 d->sortType=(TRadioButtons32 *)Sort->linked->view;
 TSLabel *Case=TSLabelRadio(__("Case style"),__("~C~apital letters go first"),
                            __("Case ~i~nsensitive"),0);
 TSLabel *Parent=TSLabelRadio(__("Parent directory (..)"),__("First in the ~l~ist"),
                              __("At the ~e~nd of the list"),0);
 d->parentSort=Parent->linked->view;
 d->updateState(box.sortType);

 // Note: MinGW have a really nasty namespace pollution, the grp1 and grp2
 // names are defined as numeric constants in dlgs.h, which I don't request.
 // I'm sick of these faults, MinGW is really bad.
 TSVeGroup *Grp1=MakeVeGroup(1,Sort,Case,Parent,0);
 Grp1->makeSameW();

 TSLabel *DotFiles=TSLabelRadio(__("Files starting with a dot"),__("~N~ormally sorted"),
                                __("After the ~r~est"),0);
 TSLabel *Exclude=TSLabelCheck(__("Exclude files"),__("Ending ~w~ith tilde"),
                               __("Ending with .bk~p~"),__("Startin~g~ with ."),0);

 TSVeGroup *Grp2=MakeVeGroup(1,DotFiles,Exclude,0);
 Grp2->makeSameW();

 col->insert(xTSLeft,yTSUp,Grp1);
 col->insert(xTSRightOf,yTSUp,Grp2,Grp1);
 EasyInsertOKCancel(col);

 col->doIt();
 delete col;
 d->options|=ofCentered;
 d->helpCtx=cmeFileOpenOptions;

 if (execDialog(d,&box)==cmOK)
   {
    //-------------- Transfer options
    TFileCollection::sortOptions=box.sortType | (box.caseType << fcolCaseBit) |
            (box.parentSort << fcolParentBit) | (box.dotFiles << fcolDotsBit) |
            (box.exclude << fcolHideBit);
    if (oldOptions!=TFileCollection::sortOptions)
      { // Some things aren't really intuitive so give some comments:
       if (box.sortType==fcolAlphabetical)
         {
          if (box.caseType==fcolCaseSensitive)
             GiveAdvice(gadvFOShiftCase);
         }
       else
         {
          if (box.caseType==fcolCaseSensitive)
             GiveAdvice(gadvFOShFuzzy);
          else
             GiveAdvice(gadvFOShiftDirs);
         }
      }
    return 1;
   }
 return 0;
}

const int cmTestScrv=0x2220,cmInfoScrv=0x2221,cmHelpScrv=0x2222;

class TDiaScrSaver : public TDialog
{
public:
 TDiaScrSaver(void);
 virtual void handleEvent(TEvent& event);
 void UpdateCommands(char *s);
 TCollection *savers;
 TSortedListBox *list;
 TView *xtOpt;
 int commandsOn;
};

TDiaScrSaver::TDiaScrSaver(void) :
           TWindowInit(&TDiaScrSaver::initFrame),
           TDialog(TRect(1,1,1,1),__("Screen saver"))
{
 commandsOn=1;
}


static
void ShowText(char *Text, char *Title)
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),Title));

 char *s=Text;
 for (; *s; s++)
     if (*s=='\r')
        *s=' ';
 TSStaticText *text=new TSStaticText(Text,72);
 col->insert(xTSCenter,1,text);
 col->insert(xTSCenter,yTSUnder,new TSButton(__("O~K~"),cmOK,bfDefault,10),0,text);

 TDialog *d=col->doIt();
 delete col;
 d->options|=ofCentered;
 
 execDialog(d,0);
 delete[] Text;
}

void TDiaScrSaver::UpdateCommands(char *s)
{
 int type;
 WichSaverIs(s,type);
 if (type==scsvInternal)
   {
    if (commandsOn)
      {
       disableCommand(cmInfoScrv);
       disableCommand(cmHelpScrv);
       xtOpt->setState(sfDisabled,True);
       commandsOn=0;
      }
   }
 else
   {
    if (!commandsOn)
      {
       enableCommand(cmInfoScrv);
       enableCommand(cmHelpScrv);
       xtOpt->setState(sfDisabled,False);
       commandsOn=1;
      }
   }
}

void TDiaScrSaver::handleEvent(TEvent& event)
{
 char *s,*s2,oldSaverState;
 TDialog::handleEvent(event);
 if (event.what==evCommand)
   {
    switch (event.message.command)
      {
       case cmTestScrv:
            // Keep current state
            s=TSetEditorApp::WhichScrSaver;
            oldSaverState=TSetEditorApp::UseScreenSaver;
            // Set it for testing
            TSetEditorApp::UseScreenSaver=1;
            TSetEditorApp::WhichScrSaver=strdup((char *)(savers->at(list->focused)));
            s2=strdup(TSetEditorApp::ExtScrSaverOpts);
            xtOpt->getData(TSetEditorApp::ExtScrSaverOpts);
            // Try it
            editorApp->screenSaver();
            // Restore all
            strcpy(TSetEditorApp::ExtScrSaverOpts,s2);
            free(s2);
            delete TSetEditorApp::WhichScrSaver;
            TSetEditorApp::WhichScrSaver=s;
            TSetEditorApp::UseScreenSaver=oldSaverState;
            break;

       case cmInfoScrv:
            s=GetScrSaverInfo((char *)(savers->at(list->focused)));
            if (s)
               ShowText(s,__("Information"));
            break;

       case cmHelpScrv:
            s=GetScrSaverHelp((char *)(savers->at(list->focused)));
            if (s)
               ShowText(s,__("Help for screen saver"));
            break;

       default:
            return;
      }
    clearEvent(event);
   }
 else
 if (event.what==evBroadcast && event.message.command==cmListItemFocused)
   { // The TListBox changed
    UpdateCommands((char *)(savers->at(list->focused)));
    clearEvent(event);
   }
}

#pragma pack(1)
typedef struct
{
 uint32  scr_on                CLY_Packed;
 TCollection *savers           CLY_Packed;
 ccIndex scr_sel               CLY_Packed;
 char    xtOp[extscrsParMxLen] CLY_Packed;
 char    time[5]               CLY_Packed;
 char    timeM[5]              CLY_Packed;
} BoxSavers;
#pragma pack()

void SetScreenSaversOptions(void)
{
 if (!TScreen::useScreenSaver())
   {
    messageBox(__("The screen saver can't be used on this terminal"),mfInformation | mfOKButton);
    return;
   }

 // EFHIMNOPRTX
 TDiaScrSaver *diaPrefs=new TDiaScrSaver();
 TSViewCol *col=new TSViewCol(diaPrefs);

 TSSortedListBox *scrv=new TSSortedListBox(24,5,tsslbVertical);
 diaPrefs->list=(TSortedListBox *)scrv->view;
 TSInputLine *extOptIn=new TSInputLine(extscrsParMxLen,24);
 diaPrefs->xtOpt=extOptIn->view;
 TSLabel *extOpts=new TSLabel(__("E~x~ternal options"),extOptIn);
 TSVeGroup *Scrv=new TSVeGroup(
   MakeVeGroup(0,TSLabelRadio(__("Sc~r~een saver (?)"),__("OF~F~"),__("O~N~"),0),
               scrv,extOpts,0), // First 3 joined
   MakeHzGroup(new TSButton(__("T~e~st"),cmTestScrv),
               new TSButton(__("~I~nfo"),cmInfoScrv),
               new TSButton(__("~H~elp"),cmHelpScrv),0)); // Buttons separated
 Scrv->makeSameW();

 TSHzLabel *timeL=new TSHzLabel(__("~T~ime"),new TSInputLine(5));
 TSStaticText *seconds=new TSStaticText(__("seconds"));
 TSHzGroup *Time=new TSHzGroup(timeL,seconds);
 Scrv=new TSVeGroup(Scrv,Time,0);

 TSHzGroup *TimeM=new TSHzGroup(new TSHzLabel(__("Ti~m~e"),new TSInputLine(5)),
                                new TSStaticText(__("mouse sec.")));
 TSVeGroup *ScreenSaver=new TSVeGroup(Scrv,TimeM,0);

 col->insert(2,1,ScreenSaver);
 EasyInsertOKCancel(col,4);

 TDialog *d=col->doIt();
 delete col;
 d->options|=ofCentered;
 d->helpCtx=cmeEdGralOptions;

 BoxSavers box;
 memset(&box,0,sizeof(BoxSavers));

 box.scr_on=TSetEditorApp::UseScreenSaver;

 char buf[32];
 sprintf(buf,"%d",TSetEditorApp::screenSaverTime);
 strncpy(box.time,buf,4);
 sprintf(buf,"%d",TSetEditorApp::screenSaverTimeMouse);
 strncpy(box.timeM,buf,4);
 strcpy(box.xtOp,TSetEditorApp::ExtScrSaverOpts);

 diaPrefs->savers=box.savers=GetScreenSaverList();
 ccIndex w;
 if (TSetEditorApp::WhichScrSaver &&
     ((TStringCollection *)box.savers)->search(TSetEditorApp::WhichScrSaver,w))
    box.scr_sel=w;
 else
    box.scr_sel=0;
 diaPrefs->UpdateCommands(TSetEditorApp::WhichScrSaver);

 if (execDialog(d,&box)==cmOK)
   {
    TSetEditorApp::UseScreenSaver=box.scr_on;
    delete TSetEditorApp::WhichScrSaver;
    TSetEditorApp::WhichScrSaver=strdup((char *)(box.savers->at(box.scr_sel)));
    TSetEditorApp::screenSaverTime=atoi(box.time);
    TSetEditorApp::screenSaverTimeMouse=atoi(box.timeM);
    strcpy(TSetEditorApp::ExtScrSaverOpts,box.xtOp);
   }
}


#pragma pack(1)
typedef struct
{
 uint32 ops          CLY_Packed;
 uint32 clk_on       CLY_Packed;
 uint32 clk_mode     CLY_Packed;
 char   editors[5]   CLY_Packed;
 char   closed[5]    CLY_Packed;
} BoxGral;
#pragma pack()

static
unsigned SetGeneralEditorOptionsMain(void)
{
 // ABCDEHIKLMOPRSTUVWY
 TSViewCol *col=new TSViewCol(__("General editor options"));

 TSLabel *tcb=TSLabelCheck(__("~S~ave/Desktop options"),
              __("Make ~b~ackups"),
              #define MAKE_BKPS 1
              __("~U~NIX-style backups"),
              #define UNIX_STYLE_BKPS 2
              __("H~i~dden backups"),
              #define HIDDEN_BKPS 4
              __("R~e~member bkps to delete"),
              #define REMM_BKPS 8
              __("~D~on't create desktop files"),
              #define DONT_CREATE_DST 16
              __("Save desktop files ~h~idden"),
              #define SAVE_HIDDEN_DST 32
              __("Tile windows ~v~ertically first"),
              #define TILE_VERT 64
 #if defined(TVOS_DOS) || (defined(TVOS_Win32) && defined(TVCompf_Cygwin))
              __("Save UNIX files ~a~s UNIX"),
 #else
              __("Save DOS files ~a~s DOS"),
 #endif
              #define SAVE_ASIS 128
              __("Don't remember cursor ~p~osition"),
              #define NO_CURSOR_POS 256
              __("Don't ~w~arn about read-only files"),
              #define NO_RO_WARNING 512
              __("Open ~r~ead-only files as R.O. buffers"),
              #define RO_AS_RO 1024
              0);

 TSHzGroup *Clock=new TSHzGroup(TSLabelRadio(__("~C~lock"),__("OFF"),__("ON"),0),
                                TSLabelRadio(__("St~y~le"),__("AM/PM"),__("24 hs"),0));

 TSHzLabel *MaxEds=new TSHzLabel(__("~M~ax. editor copies"),new TSInputLine(5));
 TSHzLabel *MaxClo=new TSHzLabel(__("Max. c~l~osed to remember"),new TSInputLine(5));

 TSVeGroup *FirstCol=MakeVeGroup(0,tcb,Clock,MaxEds,MaxClo,0);
 FirstCol->makeSameW();

 col->insert(xTSLeft,yTSUp,FirstCol);
 col->insert(xTSCenter,yTSDown,
             MakeHzGroup(new TSButton(__("O~K~"),cmOK,bfDefault),
                         new TSButton(__("Cancel"),cmCancel),
                         new TSButton(__("+ Desk~t~op"),cmYes),
                         new TSButton(__("+ Others"),cmNo),0));

 TDialog *d=col->doItCenter(cmeEdGralOptions);
 delete col;

 BoxGral box;
 memset(&box,0,sizeof(BoxGral));

 //------ Arrange the "Save options" ----------
 if (TCEditor::editorFlags & efBackupFiles)
    box.ops|=MAKE_BKPS;
 if (TCEditor::editorFlags & efSaveEOLasis)
    box.ops|=SAVE_ASIS;
 if (TCEditor::editorFlags & efUNIXBkpStyle)
    box.ops|=UNIX_STYLE_BKPS;
 if (TCEditor::editorFlags & efHiddenBkps)
    box.ops|=HIDDEN_BKPS;
 if (TCEditor::editorFlags & efDoNotWarnRO)
    box.ops|=NO_RO_WARNING;
 if (TCEditor::editorFlags & efROasRO)
    box.ops|=RO_AS_RO;

 // To avoid filling the disk with .dst files
 unsigned DesktopFilesOptions=GetDSTOptions();
 if (!(DesktopFilesOptions & dstCreate))
    box.ops|=DONT_CREATE_DST;
 // To be less annoying
 if (DesktopFilesOptions & dstHide)
    box.ops|=SAVE_HIDDEN_DST;
 // If files are specified in the command line open only these files
 if (DesktopFilesOptions & dstNoCursorPos)
    box.ops|=NO_CURSOR_POS;
 // Store the list of backups in the desktop file so an Alt+Q can delete it
 if (DesktopFilesOptions & dstRemmeberFK)
    box.ops|=REMM_BKPS;
 // Better to compare files
 unsigned dsktOps=TApplication::deskTop->getOptions();
 if (dsktOps & dsktTileVertical)
    box.ops|=TILE_VERT;
 //------ End of the "Save options" ----------

 box.clk_on=TSetEditorApp::ShowClock;
 box.clk_mode=TDeskTopClock::mode;

 char buf[32];
 sprintf(buf,"%d",TSetEditorApp::maxOpenEditorsSame);
 strncpy(box.editors,buf,4);
 sprintf(buf,"%d",TEditorCollection::maxClosedToRemember);
 strncpy(box.closed,buf,4);

 unsigned command=execDialog(d,&box.ops);
 if (command!=cmCancel)
   {
    //------ Decode the "Save options" ----------
    #define O(a,b) if (box.ops & a) TCEditor::editorFlags|=b; else \
                   TCEditor::editorFlags&=~b
    O(MAKE_BKPS,       efBackupFiles);
    O(UNIX_STYLE_BKPS, efUNIXBkpStyle);
    O(HIDDEN_BKPS,     efHiddenBkps);
    O(NO_RO_WARNING,   efDoNotWarnRO);
    O(RO_AS_RO,        efROasRO);
    #undef O

    // Filter options of this dialog
    DesktopFilesOptions&=dstEdMask | dstOwMask | dstCfMask;
    if (!(box.ops & DONT_CREATE_DST))
       DesktopFilesOptions|=dstCreate;
    if (box.ops & SAVE_HIDDEN_DST)
       DesktopFilesOptions|=dstHide;
    if (box.ops & NO_CURSOR_POS)
       DesktopFilesOptions|=dstNoCursorPos;
    if (box.ops & REMM_BKPS)
       DesktopFilesOptions|=dstRemmeberFK;
    if (box.ops & TILE_VERT)
       TApplication::deskTop->setOptions(dsktOps | dsktTileVertical);
    else
       TApplication::deskTop->setOptions(dsktOps & (~dsktTileVertical));
    EnvirSetIntVar("SET_CREATE_DST",DesktopFilesOptions);

    if (box.ops & SAVE_ASIS)
       TCEditor::editorFlags|=efSaveEOLasis;
    else
       TCEditor::editorFlags&=~efSaveEOLasis;
    //------ End of the "Save options" ----------

    TSetEditorApp::ShowClock=box.clk_on;
    TDeskTopClock::mode=box.clk_mode;
    TSetEditorApp::maxOpenEditorsSame=atoi(box.editors);
    TEditorCollection::maxClosedToRemember=atoi(box.closed);
    if (TEditorCollection::maxClosedToRemember<0)
       TEditorCollection::maxClosedToRemember=0;
    if (TEditorCollection::maxClosedToRemember>200)
       TEditorCollection::maxClosedToRemember=200;
    /* Avoid dangerous values */
    if (TSetEditorApp::maxOpenEditorsSame<1)
       TSetEditorApp::maxOpenEditorsSame=1;
   }
 return command;
}

#pragma pack(1)
typedef struct
{
 uint32 editors      CLY_Packed;
 uint32 otherWindows CLY_Packed;
 uint32 closed       CLY_Packed;
} BoxMoreDst;
#pragma pack()

static
uint32 DecodeMask(unsigned val, unsigned op1, unsigned op2, unsigned mask)
{
 val&=mask;
 if (!val) return 0;
 if (val==op1) return 1;
 if (val==op2) return 2;
 return 3;
}

static
unsigned EncodeMask(uint32 val, unsigned op1, unsigned op2)
{
 if (val==1) return op1;
 if (val==2) return op2;
 return 0;
}

static
unsigned SetGeneralEditorOptionsMoreDst(void)
{
 // CEKLMNORVY
 TSViewCol *col=new TSViewCol(__("More desktop options"));

 TSLabel *editors=TSLabelRadio(__("Remember editor windows"),
         __("~A~lways"),__("~O~nly if no file specified"),__("~N~ever"),0);
 TSLabel *others=TSLabelRadio(__("Remember other windows"),
         __("Al~w~ays"),__("On~l~y if no file specified"),__("Neve~r~"),0);
 TSLabel *closed=TSLabelRadio(__("Remember ~c~losed windows"),
         __("Alway~s~"),__("Onl~y~ if no file specified"),__("Ne~v~er"),0);

 col->insert(xTSLeft,yTSUp,MakeVeGroup(0,editors,others,closed,0));
 col->insert(xTSCenter,yTSDown,
             MakeHzGroup(new TSButton(__("O~K~"),cmOK,bfDefault),
                         new TSButton(__("Cancel"),cmCancel),
                         new TSButton(__("~M~ain options"),cmYes),0));

 TDialog *d=col->doItCenter(cmeEdGralOptions);
 delete col;

 BoxMoreDst box;
 unsigned DstOption=GetDSTOptions();
 box.editors=DecodeMask(DstOption,dstEdOnlyIfNoCL,dstEdNever,dstEdMask);
 box.otherWindows=DecodeMask(DstOption,dstOwOnlyIfNoCL,dstOwNever,dstOwMask);
 box.closed=DecodeMask(DstOption,dstCfOnlyIfNoCL,dstCfNever,dstCfMask);

 unsigned command=execDialog(d,&box);
 if (command!=cmCancel)
   {
    // Filter options of this dialog
    DstOption&=~(dstEdMask | dstOwMask | dstCfMask);
    DstOption|=EncodeMask(box.editors,dstEdOnlyIfNoCL,dstEdNever);
    DstOption|=EncodeMask(box.otherWindows,dstOwOnlyIfNoCL,dstOwNever);
    DstOption|=EncodeMask(box.closed,dstCfOnlyIfNoCL,dstCfNever);
    EnvirSetIntVar("SET_CREATE_DST",DstOption);
   }
 return command;
}

#pragma pack(1)
typedef struct
{
 uint32 end      CLY_Packed;
 uint32 beep     CLY_Packed;
 uint32 opts     CLY_Packed;
 uint32 opsAv    CLY_Packed;
 char   width[5] CLY_Packed;
 uint32 opsZoom  CLY_Packed;
} BoxOthers;
#pragma pack()

static
unsigned SetGeneralEditorOptionsOthers(void)
{
 TSViewCol *col=new TSViewCol(__("Other options"));

 // ENG: ACIJKLMNPRUVW
 // ESP: ADEIKMNPRSUVY
 TSVeGroup *MsgWin=MakeVeGroup(0,
   TSLabelRadio(__("At the end of error list in message window"),
                __("~J~ust stop"),
                __("~I~ndicate with a message"),
                __("Wrap (~c~ircular list)"),0),
   new TSCheckBoxes(new TSItem(__("Make a beep"),0)),
   TSLabelCheck(__("When creating message and similar windows"),
                __("Use the ~v~ertical direction"),
                __("Use the ~r~ight side"),0),
   TSLabelRadio(__("When opening files"),
                __("~U~se reserved width or 7 (hz dir)"),
                __("~A~void message and project windows"),0),
   new TSHzLabel(__("Reserved ~w~idth"),new TSInputLine(5)),
   TSLabelRadio(__("Zoom windows when"),
                __("No ~p~roject window"),
                __("No prj. wi~n~dow or it's zoomed"),
                __("A~l~ways"),0),
   0);
 MsgWin->makeSameW();

 col->insert(xTSLeft,yTSUp,MsgWin);
 col->insert(xTSCenter,yTSDown,
             MakeHzGroup(new TSButton(__("O~K~"),cmOK,bfDefault),
                         new TSButton(__("Cancel"),cmCancel),
                         new TSButton(__("~M~ain options"),cmYes),0));

 TDialog *d=col->doItCenter(cmeEdGralOptions);
 delete col;

 BoxOthers box;
 box.end=TSOSListBoxMsg::opsEnd;
 box.beep=TSOSListBoxMsg::opsBeep;
 box.opts=TSetEditorApp::geFlags & geMask1;
 box.opsAv=TSetEditorApp::geFlags & geAvoidPrjAndMsg ? 1 : 0;
 char buf[32];
 sprintf(buf,"%d",TSetEditorApp::widthVertWindows);
 strncpy(box.width,buf,4);
 box.opsZoom=(TSetEditorApp::geFlags & geMask2)>>geShift2;

 unsigned command=execDialog(d,&box);
 if (command!=cmCancel)
   {
    TSOSListBoxMsg::opsEnd=box.end;
    TSOSListBoxMsg::opsBeep=box.beep;
    TSetEditorApp::geFlags=box.opts;
    if (box.opsAv)
       TSetEditorApp::geFlags|=geAvoidPrjAndMsg;
    if (box.opsZoom)
        TSetEditorApp::geFlags|=(box.opsZoom<<geShift2) & geMask2;
    TSetEditorApp::widthVertWindows=atoi(box.width);
    if (TSetEditorApp::widthVertWindows<6)
       TSetEditorApp::widthVertWindows=6;
    int mWidth=TDisplay::getCols()-12;
    if (TSetEditorApp::widthVertWindows>mWidth)
       TSetEditorApp::widthVertWindows=mWidth;
   }
 return command;
}

void SetGeneralEditorOptions(void)
{
 int dialog=0;
 unsigned command=0;
 do
   {
    switch (dialog)
      {
       case 0:
            command=SetGeneralEditorOptionsMain();
            if (command==cmYes)
               dialog=1;
            else if (command==cmNo)
               dialog=2;
            break;
       case 1:
            command=SetGeneralEditorOptionsMoreDst();
            if (command==cmYes)
               dialog=0;
            break;
       case 2:
            command=SetGeneralEditorOptionsOthers();
            if (command==cmYes)
               dialog=0;
            break;
      }
   }
 while (command==cmYes || command==cmNo);
}

/************************** Code page convert dialogs **************************/
static int fromCP=-1, toCP=-1;
static uint32 CPNoLow=0;
const uchar Version=1;

void SaveConvCPOptions(fpstream& s)
{
 s << Version << fromCP << toCP << (ushort)CPNoLow;
}

void LoadConvCPOptions(fpstream& s)
{
 uchar version;
 ushort aux;
 s >> version >> fromCP >> toCP >> aux;
 CPNoLow=aux;
}


#pragma pack(1)
typedef struct
{
 TCollection *lFrom CLY_Packed;
 ccIndex sFrom      CLY_Packed;
 TCollection *lTo   CLY_Packed;
 ccIndex sTo        CLY_Packed;
 uint32  ops        CLY_Packed;
} FromToBox;
#pragma pack()

int TSetEditorApp::ChooseConvCPs(int &From, int &To, uint32 &ops)
{
 // Look for some default
 int idDefScr, idDefApp, idDefInp;
 TVCodePage::GetDefaultCodePages(idDefScr,idDefApp,idDefInp);
 if (so && so->enForceApp)
    idDefApp=so->enApp;
 if (so && so->enForceScr)
    idDefScr=so->enScr;

 FromToBox box;
 box.lFrom=box.lTo=TVCodePage::GetList();
 box.sFrom=TVCodePage::IDToIndex(fromCP<0 ? idDefApp : fromCP);
 box.sTo  =TVCodePage::IDToIndex(toCP<0   ? idDefScr : toCP);
 box.ops  =CPNoLow;

 TSHzGroup *cps=new TSHzGroup(
                 new TSLabel(__("~F~rom code page"),
                  new TSSortedListBox(wForced,6,tsslbVertical)),
                 new TSLabel(__("~T~o code page"),
                  new TSSortedListBox(wForced,6,tsslbVertical)));
 TSVeGroup *all=new TSVeGroup(cps,
                     new TSCheckBoxes(
                         new TSItem(__("~D~on't remap codes below 32"),
                         new TSItem(__("~O~nly selected text"),0))));
 all->makeSameW();

 TSViewCol *col=new TSViewCol(__("Code page remap"));
 col->insert(2,1,all);
 EasyInsertOKCancel(col);
 TDialog *d=col->doIt();
 delete col;
 d->options|=ofCentered;
 d->helpCtx=cmeRemapCodePage;

 int ret=execDialog(d,&box)==cmOK;
 fromCP=From=TVCodePage::IndexToID(box.sFrom);
 toCP=To=TVCodePage::IndexToID(box.sTo);
 ops=CPNoLow=box.ops;

 return ret;
}

char *TSetEditorApp::CreateTitle(const char *title)
{
 const char *t=TVIntl::getTextNew(title);
 const char *d=TScreen::getDriverShortName();
 char *res=new char[strlen(t)+3+strlen(d)+1];
 strcpy(res,t);
 strcat(res," - ");
 strcat(res,d);
 DeleteArray(t);
 return res;
}

stScreenOptions *TSetEditorApp::so=NULL;

/************************** Code pages dialogs **************************/
#pragma pack(1)
typedef struct
{
 uint32  appForce      CLY_Packed;
 TCollection *appList  CLY_Packed;
 ccIndex appCP         CLY_Packed;
 uint32  inpForce      CLY_Packed;
 TCollection *inpList  CLY_Packed;
 ccIndex inpCP         CLY_Packed;
 uint32  scrForce      CLY_Packed;
 TCollection *scrList  CLY_Packed;
 ccIndex scrCP         CLY_Packed;
 uint32  sndForce      CLY_Packed;
 TCollection *sndList  CLY_Packed;
 ccIndex sndCP         CLY_Packed;
} EncodingBox;
#pragma pack()

// New code pages dialogs
void TSetEditorApp::EncodingOptions()
{
 if (!so) return; // Sanity check
 // Compute the height of the list boxes to use most of the desktop
 TRect dkt=TProgram::deskTop->getExtent();
 int height=dkt.b.y-dkt.a.y-10;
 if (TScreen::codePageVariable())
    height=(height-2)/2;

 TSVeGroup *appEncode=NULL,*scrEncode=NULL,*sndEncode=NULL,*inpEncode=NULL;

 appEncode=new TSVeGroup(
   TSLabelCheck(__("~A~pplication"),__("Force encoding"),0),
   new TSSortedListBox(wForced,height,tsslbVertical),
   0);
 appEncode->makeSameW();

 inpEncode=new TSVeGroup(
   TSLabelCheck(__("~I~nput"),__("Force encoding"),0),
   new TSSortedListBox(wForced,height,tsslbVertical),
   0);
 inpEncode->makeSameW();

 TSView *upperCPs=MakeHzGroup(appEncode,inpEncode,0);
 TSView *lowerCPs=NULL;

 if (TScreen::codePageVariable())
   {// Only if the code page is variable
    scrEncode=new TSVeGroup(
      TSLabelCheck(__("~S~creen"),__("Force encoding"),0),
      new TSSortedListBox(wForced,height,tsslbVertical),
      0);
    scrEncode->makeSameW();

    if (TScreen::canSetSBFont())
      {// Only if the secondary font exists
       sndEncode=new TSVeGroup(
         TSLabelCheck(__("Second ~f~ont"),__("Force encoding"),0),
         new TSSortedListBox(wForced,height,tsslbVertical),
         0);
       sndEncode->makeSameW();
       lowerCPs=MakeHzGroup(scrEncode,sndEncode,0);
      }
    else
       lowerCPs=scrEncode;
   }


 char *title=CreateTitle(__("Encodings"));
 TSViewCol *col=new TSViewCol(title);
 DeleteArray(title);
 col->insert(xTSLeft,yTSUp,upperCPs);
 if (lowerCPs)
    col->insert(xTSCenter,yTSUnder,lowerCPs,0,upperCPs);
 col->insert(xTSCenter,yTSDown,
             MakeHzGroup(new TSButton(__("O~K~"),cmOK,bfDefault),
                         new TSButton(__("Cancel"),cmCancel),
                         new TSButton(__("Set ~D~efaults"),cmYes),0));
 TDialog *d=col->doIt();
 delete col;
 d->options|=ofCentered;
 d->helpCtx=cmeEncodings;
 EncodingBox box;

 // Current TV settings
 int idDefScr, idDefApp, idDefInp;
 TVCodePage::GetDefaultCodePages(idDefScr,idDefApp,idDefInp);

 //fprintf(stderr,"SO: App %d Inp %d Scr %d\n",so->enApp,so->enInp,so->enScr);
 //fprintf(stderr,"DrvDef: App %d Inp %d Scr %d\n",idDefApp,idDefInp,idDefScr);

 // Currently selected values
 // Note: Here if the user isn't forcing the code page what we show is what the
 // driver is currently using. The other option is to show what was selected last
 // time it was forced but in this way:
 // 1) The user doesn't know what's using currently, maybe s/he doesn't really want
 // to force it.
 // 2) The first code to handle v0.5.0 left it in 0 which is invalid.
 int appCP, scrCP, sndCP, inpCP;
 appCP=TVCodePage::IDToIndex(so->enForceApp && so->enApp!=-1 ? so->enApp : idDefApp);
 inpCP=TVCodePage::IDToIndex(so->enForceInp && so->enInp!=-1 ? so->enInp : idDefInp);
 scrCP=TVCodePage::IDToIndex(so->enForceScr && so->enScr!=-1 ? so->enScr : idDefScr);
 sndCP=so->enSnd!=-1 ? TVCodePage::IDToIndex(so->enSnd) : scrCP;

 // Data box
 box.appForce=so->enForceApp;
 box.inpForce=so->enForceInp;
 box.scrForce=so->enForceScr;
 box.sndForce=so->enForceSnd;
 box.appCP=appCP;
 box.inpCP=inpCP;
 box.scrCP=scrCP;
 box.sndCP=sndCP;
 box.appList=box.inpList=box.scrList=box.sndList=TVCodePage::GetList();

 unsigned ret=execDialog(d,&box);
 if (ret==cmYes)
   {// Set defaults
    int priChanged=so->enForceScr || (so->enForceScr && idDefScr!=scrCP);
    int sndChanged=so->enForceSnd || (so->enForceSnd && idDefScr!=sndCP);
    so->enForceApp=so->enForceInp=so->enForceScr=so->enForceSnd=0;
    so->enApp=idDefApp;
    so->enInp=idDefInp;
    so->enScr=so->enSnd=idDefScr;
    TVCodePage::SetCodePage(so->enApp,so->enScr,so->enInp);
    SetEditorFontsEncoding(priChanged,idDefScr,sndChanged,idDefScr);
    // This is a full redraw, not just a refresh from the buffers
    TProgram::application->Redraw();
   }
 else if (ret==cmOK)
   {
    int appChanged=box.appForce!=(uint32)so->enForceApp || (so->enForceApp && box.appCP!=appCP);
    int inpChanged=box.inpForce!=(uint32)so->enForceInp || (so->enForceInp && box.inpCP!=inpCP);
    int priChanged=box.scrForce!=(uint32)so->enForceScr || (so->enForceScr && box.scrCP!=scrCP);
    int sndChanged=box.sndForce!=(uint32)so->enForceSnd || (so->enForceSnd && box.sndCP!=sndCP);
    if (appChanged || inpChanged || priChanged || sndChanged)
      {// At least one changed       
       so->enForceApp=box.appForce;
       so->enForceInp=box.inpForce;
       so->enForceScr=box.scrForce;
       so->enForceSnd=box.sndForce;
       // Transfer the settings or just revert to defaults
       so->enApp=so->enForceApp ? TVCodePage::IndexToID(box.appCP) : idDefApp;
       so->enInp=so->enForceInp ? TVCodePage::IndexToID(box.inpCP) : idDefInp;
       so->enScr=so->enForceScr ? TVCodePage::IndexToID(box.scrCP) : idDefScr;
       so->enSnd=so->enForceSnd ? TVCodePage::IndexToID(box.sndCP) : so->enScr;
       TVCodePage::SetCodePage(so->enApp,so->enScr,so->enInp);
       SetEditorFontsEncoding(priChanged,so->enForceScr ? so->enScr : idDefScr,
                              sndChanged,so->enForceSnd ? so->enSnd : idDefScr);
       // This is a full redraw, not just a refresh from the buffers
       TProgram::application->Redraw();
      }
   }
}

/***************************** Fonts dialogs ****************************/

#pragma pack(1)
typedef struct
{
 uint32  priUse         CLY_Packed;
 TCollection *priList   CLY_Packed;
 ccIndex priFont        CLY_Packed;
 TCollection *priSizes  CLY_Packed;
 ccIndex priSize        CLY_Packed;
 uint32  secUse         CLY_Packed;
 TCollection *secList   CLY_Packed;
 ccIndex secFont        CLY_Packed;
} FontsBox;
#pragma pack()

// An easydiag wrapper for TVBitmapFontDescLBox
ListBoxSpecialize(TSVBitmapFontDescLBox);
ListBoxImplement(VBitmapFontDescLBox);
// An easydiag wrapper for TVBitmapFontSizeLBox
ListBoxSpecialize(TSVBitmapFontSizeLBox);
ListBoxImplement(VBitmapFontSizeLBox);

// A TDialog class to connect the primary font with the available sizes
class TDiaFont : public TDialog
{
public:
 TDiaFont(const char *aTitle);
 virtual void handleEvent(TEvent& event);

 TVBitmapFontDescLBox *pri;
 TSortedListBox *sizes;
 TVBitmapFontDescCol *fonts;
 int selected;
};

TDiaFont::TDiaFont(const char *aTitle) :
         TWindowInit(&TDiaFont::initFrame),
         TDialog(TRect(1,1,1,1),aTitle)
{
 options|=ofCentered;
 helpCtx=cmeFonts;
}

void TDiaFont::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if (event.what==evBroadcast)
   {
    if (event.message.command==cmListItemFocused &&
        event.message.infoPtr==pri &&
        pri->focused!=selected)
      {
       selected=pri->focused;
       TVBitmapFontDesc *p=(TVBitmapFontDesc *)fonts->at(selected);

       // Check if the sizes box is already initialized.
       if (!sizes->list())
          return;
       TListBoxRec box;
       if (!p->sizes->search((void *)sizes->list()->at(sizes->focused),box.selection))
         {
          unsigned w,h;
          if (TScreen::getFontGeometry(w,h))
            {
             TVBitmapFontSize sz={w,h};
             if (!p->sizes->search((void *)&sz,box.selection))
                box.selection=0;
            }
          else
             box.selection=0;
         }
       box.items=p->sizes;
       sizes->setData(&box,False);
      }
   }
}

void TSetEditorApp::FontsOptions()
{
 if (!so) return; // Sanity check
 if (!TScreen::canSetBFont())
   {
    messageBox(__("This terminal doesn't support changing fonts"),mfInformation | mfOKButton);
    return;
   }
 unsigned wmin,wmax,hmin,hmax;
 if (!TScreen::getFontGeometry(wmin,hmin))
   {
    messageBox(__("Can't determine fonts geometry"),mfError | mfOKButton);
    return;
   }
 if (!TScreen::getFontGeometryRange(wmin,hmin,wmax,hmax))
   {
    wmax=wmin;
    hmax=hmin;
   }
 TVBitmapFontDescCol *fonts=
   TVFontCollection::CreateListOfFonts(GetVariable("SET_FILES"),wmin,wmax,
                                       hmin,hmax);
 if (!fonts)
   {
    messageBox(__("No fonts available for current video mode"),mfInformation | mfOKButton);
    return;
   }
 // Ok, we have fonts and we can use them
 // Fill the data box
 FontsBox box;
 box.priUse=so->foPriLoad;
 box.secUse=so->foSecLoad;
 box.priList=box.secList=fonts;
 // That's a gcc 3.4 requirement:
 ccIndex aux;
 if (!so->foPriName || !fonts->search(so->foPriName,aux))
    aux=0;
 box.priFont=aux;
 if (!so->foSecName || !fonts->search(so->foSecName,aux))
    aux=0;
 box.secFont=aux;

 TVBitmapFontDesc *pri=(TVBitmapFontDesc *)fonts->at(box.priFont),*sec;
 box.priSizes=pri->sizes;
 int filled=0;
 TVBitmapFontSize sizeSt;
 sizeSt.w=so->foPriW; sizeSt.h=so->foPriH;
 if (!so->foPriName || !pri->sizes->search(&sizeSt,aux))
   {
    unsigned w,h;
    if (TScreen::getFontGeometry(w,h))
      {
       sizeSt.w=w; sizeSt.h=h;
       if (pri->sizes->search(&sizeSt,aux))
          filled=1;
      }
    if (!filled)
       aux=0;
   }
 box.priSize=aux;

 // Create the dialog
 TRect dkt=TProgram::deskTop->getExtent();
 int height=dkt.b.y-dkt.a.y-10;

 int retry;
 do
   {
    // Primary font label, check box and list
    TSLabel *priLBl=TSLabelCheck(__("~P~rimary"),__("~L~oad font"),0);
    TSVBitmapFontDescLBox *priLB=new TSVBitmapFontDescLBox(wForced,height-1,tsslbVertical);
    TSVeGroup *priOps=new TSVeGroup(priLBl,priLB,0);
    priOps->makeSameW();
   
    // Size
    TSVBitmapFontSizeLBox *priSz=new TSVBitmapFontSizeLBox(12,height,tsslbVertical);
    TSLabel *priSzl=new TSLabel(__("S~i~ze"),priSz);
   
    // Secondary font options, only if available
    TSVeGroup *secOps=NULL;
    if (TScreen::canSetSBFont())
      {
       secOps=new TSVeGroup(TSLabelCheck(__("~S~econdary"),__("Lo~a~d font"),0),
                            new TSVBitmapFontDescLBox(wForced,height-1,tsslbVertical),0);
       secOps->makeSameW();
      }

    retry=0;
    char *title=CreateTitle(__("Fonts"));
    TDiaFont *d=new TDiaFont(title);
    DeleteArray(title);
    // Setup the members used to do the connection
    d->pri=(TVBitmapFontDescLBox *)priLB->view;
    d->sizes=(TSortedListBox *)priSz->view;
    d->selected=box.priSize;
    d->fonts=fonts;
    // Now create the EasyDiag collection
    TSViewCol *col=new TSViewCol(d);
    col->insert(xTSLeft,yTSUp,MakeHzGroup(priOps,priSzl,secOps,0));
    EasyInsertOKCancel(col);
    col->doIt();
    delete col;
   
    if (execDialog(d,&box)==cmOK)
      {
       pri=(TVBitmapFontDesc *)fonts->at(box.priFont);
       sec=(TVBitmapFontDesc *)fonts->at(box.secFont);
       TVBitmapFontSize *fontSize=(TVBitmapFontSize *)pri->sizes->at(box.priSize);
       // We know the requested size for the primary font will work, but we don't
       // know if the secondary font supports it.
       if (box.secUse)
         {
          TVBitmapFontSize *s;
          if (box.priUse)
             s=fontSize;
          else
            {
             unsigned w=8,h=16;
             TScreen::getFontGeometry(w,h);
             sizeSt.w=w; sizeSt.h=h;
             s=&sizeSt;
            }
          ccIndex pos;
          if (!sec->sizes->search((void *)s,pos))
            {
             retry=1;
             messageBox(mfError | mfOKButton,__("The selected secondary font doesn't support the primary size (%dx%d)"),s->w,s->h);
            }
         }
       if (!retry)
          SetEditorFonts(box.priUse,newStr(pri->name),newStr(pri->file),fontSize,
                         box.secUse,newStr(sec->name),newStr(sec->file));
      }
   }
 while (retry);
}

/***************************** Screen dialogs ****************************/

#pragma pack(1)
typedef struct
{
 uint32  options        CLY_Packed;
 char sizeW[5]          CLY_Packed;
 char sizeH[5]          CLY_Packed;
 char sizeCW[5]         CLY_Packed;
 char sizeCH[5]         CLY_Packed;
 char command[80]       CLY_Packed;
 char mode[10]          CLY_Packed;
} ScreenSizeBox;
#pragma pack()

static
void ToStr(int val, char *dest)
{
 char buf[32];
 sprintf(buf,"%d",val);
 strncpy(dest,buf,4);
}

void TSetEditorApp::ScreenOptions()
{
 if (!so) return; // Sanity check
 if (!TScreen::canSetVideoSize())
   {
    messageBox(__("This terminal has a fixed size"),mfInformation | mfOKButton);
    return;
   }

 ScreenSizeBox box;
 box.options=so->scOptions;
 ToStr(so->scWidth,box.sizeW);
 ToStr(so->scHeight,box.sizeH);
 ToStr(so->scCharWidth,box.sizeCW);
 ToStr(so->scCharHeight,box.sizeCH);
 sprintf(box.mode,"0x%03X",so->scModeNumber);
 if (so->scCommand)
    strcpy(box.command,so->scCommand);
 else
    box.command[0]=0;

 int retry;
 do
   {
    retry=0;
    // CDEGHIMNSWX
    TSLabel *options=TSLabelRadio(__("Screen size options"),
             __("~D~on't force"),
             __("~S~ame as last run"),
             __("~E~xternal program"),
             __("~C~losest to specified size"),
             __("Specified ~m~ode number"),0);
   
    TSHzGroup *sizes=MakeHzGroup(
                new TSVeGroup(new TSHzLabel(__("~W~idth "),new TSInputLine(5)),
                              new TSHzLabel(__("~H~eight"),new TSInputLine(5)),0),
                new TSVeGroup(new TSHzLabel(__("Chars w~i~dth "),new TSInputLine(5)),
                              new TSHzLabel(__("Chars hei~g~ht"),new TSInputLine(5)),0),
                0);
   
    TSLabel *external=new TSLabel(__("E~x~ternal program"),
                                 new TSInputLine(80,36));

    TSHzLabel *mode=new TSHzLabel(__("Mode ~n~umber"),new TSInputLine(10));
   
    TSVeGroup *all=MakeVeGroup(options,sizes,external,mode,0);
    all->makeSameW();

    char *title=CreateTitle(__("Screen size"));
    TSViewCol *col=new TSViewCol(title);
    DeleteArray(title);
    col->insert(xTSLeft,yTSUp,all);
    EasyInsertOKCancel(col);
    TDialog *d=col->doIt();
    delete col;
    d->options|=ofCentered;
    d->helpCtx=cmeSetScreenOps;
   
    if (execDialog(d,&box)==cmOK)
      {
       unsigned nW,nH,nCH,nCW;
       nW=atoi(box.sizeW);
       nH=atoi(box.sizeH);
       nCW=atoi(box.sizeCW);
       nCH=atoi(box.sizeCH);
       if (nW<80 || nW>250 || nH<25 || nH>250)
         {
          messageBox(__("Please specify a screen size of at least 80x25 and no more than 250x250"),mfError | mfOKButton);
          retry=1;
         }
       else if (nCW<5 || nCW>32 || nCH<7 || nCW>32)
         {
          messageBox(__("Please specify a character size of at least 5x7 and no more than 32x32"),mfError | mfOKButton);
          retry=1;
         }
       else
         {
          char *end;
          so->scOptions=box.options;
          so->scWidth=atoi(box.sizeW);
          so->scHeight=atoi(box.sizeH);
          so->scCharWidth=atoi(box.sizeCW);
          so->scCharHeight=atoi(box.sizeCH);
          so->scModeNumber=strtol(box.mode,&end,0);
          DeleteArray(so->scCommand);
          so->scCommand=newStr(box.command);
          resetVideoMode();
         }         
      }
   }
 while (retry);
}

void TSetEditorApp::SetModifCheckOptions()
{
 TSViewCol *col=new TSViewCol(__("Checking for modified files"));

 col->insert(xTSCenter,yTSUpSep,
  MakeVeGroup(tsveMakeSameW,
              new TSStaticText(__("When a file on disk is newer than a file in edition:\n")),
              new TSHzLabel(__("Seconds between checks"),new TSInputLine(5)),
              TSLabelCheck(__("Related options"),
                           __("~D~on't check after executing an external program"),
                           __("Don't check while ~i~dle"),0),
              0));
 EasyInsertOKCancel(col);
 TDialog *d=col->doItCenter(cmeSetModiCkOps);
 delete col;

 struct
 {
  char time[5]    CLY_Packed;
  uint32 ops      CLY_Packed;
 } box;
 CLY_snprintf(box.time,5,"%d",TCEditor::minDifModCheck);
 box.ops=modifFilesOps;
 if (execDialog(d,&box)==cmOK)
   {
    TCEditor::minDifModCheck=atoi(box.time);
    modifFilesOps=box.ops;
   }
}

