/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#define Uses_string

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

/**[txh]********************************************************************

  Description:
  That's common between DOS and Linux.

***************************************************************************/

static
void PrimaryEncoding(TSSortedListBox *&enco1, TSLabel *&lbe1, int disableFonts)
{
 //-------------- Encoding Primary -------------------
 enco1=new TSSortedListBox(wForced,6,tsslbVertical);
 lbe1=new TSLabel(disableFonts ? _("No fonts found") : _("Primary ~e~ncoding"),enco1);
 //----------- End of Encoding Primary ---------------
}


#ifndef TVCompf_djgpp

TDialog *CreateScreenOpsDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),_("Screen Options")));

 TSSortedListBox *enco1;
 TSLabel *lbe1;
 PrimaryEncoding(enco1,lbe1,0);

 col->insert(2,2,lbe1);
 EasyInsertOKCancel(col);

 TDialog *d=col->doIt();
 delete col;

 d->options|=ofCentered;
 d->helpCtx=cmeSetScreenOps;
 return d;
}

#pragma pack(1)
typedef struct
{
 TCollection *font_enco  CLY_Packed;
 ccIndex font_encs       CLY_Packed;
} ScreenBox;
#pragma pack()

void SetScreenOps(void)
{
 if (!TScreen::codePageVariable())
   {
    messageBox(_("This terminal have a fixed code page"),mfError | mfOKButton);
    return;
   }
 ccIndex oldE1;
 char setDta=1;

 ScreenBox box;
 box.font_enco=GetCodePagesList();
 oldE1=box.font_encs=CodePageIDToIndex(GetCodePageFont(1));

 TDialog *d=CreateScreenOpsDialog();

 if (execDialogNoDestroy(d,&box,setDta)==cmOK)
   {
    if (oldE1!=box.font_encs)
      {
       ChangeEncodingOfFont(1,IndexToCodePageID(box.font_encs));
       ResetVideoMode(TScreen::screenMode);
      }
   }

 destroy(d);
}

#else
#include <tpaltext.h>

extern void InsertEnviromentVar(char *variable,char *contents);
extern void setIntenseState(void);
// From edfonts.cc
TStringCollection *CreateListOfFonts(char *dir, uint32 &IsPrimOn, ccIndex &indexPrim,
                                     uint32 &IsSecoOn, ccIndex &indexSeco);

const ushort OtherOp=13;

TDialog *CreateScreenOpsDialog(int disableFonts)
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),_("Screen Options")));

 //------------ Video mode stuff ------------------
 TSLabel *mode=TSLabelRadio(__("~S~creen mode"),
          "8~0~x25",  // 0x003 8/9x16 VGA
          "80x~2~8",  // 0x103        VGA+scanlines
          "~8~0x35",  // 0x203
          "80x~4~0",  // 0x303
          "80x4~3~",  // 0x403 8x8
          "80x~5~0",  // 0x503 8/9x8
          "80x30",    // 0x703 8x16   VGA+tweaked
          "80x34",    // 0x803 8x14
          "90x30",    // 0x903 8x16
          "90x34",    // 0xA03 8x14
          "94x30",    // 0xB03 8x16
          "94x34",    // 0xC03 8x14
          "82x25",    // 0xD03 8x16 fonts
          __("other"),0);
 // Other mode
 TSInputLine *other=new TSInputLine(10);
 TSVeGroup *Mode=new TSVeGroup(mode,other,0);
 Mode->makeSameW();
 //-------- End of Video mode stuff ---------------

 //---------------- Primary Font ---------------------
 TSLabel *font=TSLabelRadio(disableFonts ? __("No fonts found") :
                                           __("~P~rimary font (intense)"),
                            __("~R~OM BIOS fonts"),_("O~t~her font"),0);
 TSSortedListBox *lb=new TSSortedListBox(wForced,4,tsslbVertical);
 TSVeGroup *PFont=new TSVeGroup(font,lb,0);
 PFont->makeSameW();
 //------------ End of Primary Font ------------------

 //---------------- Secondary Font -------------------
 TSLabel *font2=TSLabelRadio(disableFonts ? __("No fonts found") :
                                            __("~S~econdary font"),
                             __("~D~on't use it"),_("Other ~f~ont"),0);
 TSSortedListBox *lb2=new TSSortedListBox(wForced,4,tsslbVertical);
 TSVeGroup *SFont=new TSVeGroup(font2,lb2,0);
 SFont->makeSameW();
 //------------ End of Secondary Font ----------------

 //-------------- Encoding Primary -------------------
 TSSortedListBox *enco1;
 TSLabel *lbe1;
 PrimaryEncoding(enco1,lbe1,disableFonts);
 //----------- End of Encoding Primary ---------------

 //-------------- Encoding Secondary -----------------
 TSSortedListBox *enco2=new TSSortedListBox(wForced,6,tsslbVertical);
 TSLabel *lbe2=new TSLabel(disableFonts ? _("No fonts found") :
                                          _("Secondary encoding"),enco2);
 //----------- End of Encoding Secondary -------------

 //----------- External video mode -------------------
 TSHzGroup *ext=new TSHzGroup(new TSCheckBoxes(new TSItem(_("Use external program"),0)),
                              new TSInputLine(80,36),
                              0);
 ext->ySep=0;
 //--------- End of External video mode --------------

 if (disableFonts)
   {
    ((TSRadioButtons *)(font->linked))->view->options&=~ofSelectable;
    lb->view->options&=~ofSelectable;
    ((TSRadioButtons *)(font2->linked))->view->options&=~ofSelectable;
    lb2->view->options&=~ofSelectable;
    enco1->view->options&=~ofSelectable;
    enco2->view->options&=~ofSelectable;
   }

 col->insert(2,1,Mode);
 col->insert(xTSRightOf,1,PFont,Mode);
 col->insert(xTSRightOf,yTSUnder,SFont,Mode,PFont);
 col->insert(xTSRightOf,1,lbe1,PFont);
 col->insert(xTSRightOf,yTSUnder,lbe2,SFont,lbe1);
 col->insert(2,yTSUnder,ext,0,Mode);
 EasyInsertOKCancel(col);

 TDialog *d=col->doIt();
 delete col;

 d->options|=ofCentered;
 d->helpCtx=cmeSetScreenOps;
 return d;
}

#pragma pack(1)
typedef struct
{
 uint32 mode             CLY_Packed;
 char   other[10]        CLY_Packed;
 uint32 font_flg         CLY_Packed;
 TCollection *font_list  CLY_Packed;
 ccIndex font_sel        CLY_Packed;
 uint32 font2_flg        CLY_Packed;
 TCollection *font2_list CLY_Packed;
 ccIndex font2_sel       CLY_Packed;
 TCollection *font_enco  CLY_Packed;
 ccIndex font_encs       CLY_Packed;
 TCollection *font2_enco CLY_Packed;
 ccIndex font2_encs      CLY_Packed;
 uint32 ext_prg          CLY_Packed;
 char   command[80]      CLY_Packed;
} ScreenBox;
#pragma pack()

static
ushort ModeToOption(ushort mode)
{
 if ((mode & 0xFF)==3)
   {
    ushort m=mode>>8;
    if (m>5)
       m--;
    if (m>12)
       return OtherOp;
    return m;
   }
 return OtherOp;
}

static
ushort OptionToMode(ScreenBox &box)
{
 char *end;

 if (box.mode==OtherOp)
    return strtol(box.other,&end,0);
 if (box.mode>5)
    box.mode++;
 return box.mode<<8 | 3;
}


void SetScreenOps(void)
{
 int repeat=1,fontChanged=0;
 ushort oldmode=TScreen::screenMode;
 ushort oldFontFlg,oldFont2Flg;
 ccIndex oldFontSel,oldFont2Sel,oldE1,oldE2;
 char setDta=1,extChange;

 ScreenBox box;
 box.mode=ModeToOption(oldmode);
 sprintf(box.other,"0x%03X",oldmode);
 box.font_list=box.font2_list=CreateListOfFonts((char *)GetVariable("SET_FILES"),
                                                box.font_flg,box.font_sel,box.font2_flg,
                                                box.font2_sel);
 box.font_enco=box.font2_enco=GetCodePagesList();
 oldE1=box.font_encs=CodePageIDToIndex(GetCodePageFont(1));
 oldE2=box.font2_encs=CodePageIDToIndex(GetCodePageFont(2));

 oldFontFlg=box.font_flg;
 oldFontSel=box.font_sel;
 oldFont2Flg=box.font2_flg;
 oldFont2Sel=box.font2_sel;
 TDialog *d=CreateScreenOpsDialog(box.font_list->getCount()==0);
 box.ext_prg=TSetEditorApp::UseExternPrgForMode;
 strcpy(box.command,TSetEditorApp::ExternalPrgMode);

 while (repeat)
   {
    repeat=0;
    if (execDialogNoDestroy(d,&box,setDta)==cmOK)
      {
       if (oldFontSel!=box.font_sel || oldFontFlg!=box.font_flg ||
           oldFont2Flg!=box.font2_flg || oldFont2Sel!=box.font2_sel)
         {
          fontChanged=1;
          if (box.font_flg==0) // ROM?
             LoadEditorFonts(0,0,IndexToCodePageID(box.font_encs),
                             IndexToCodePageID(box.font2_encs));
          else
            {
             char *p=(char *)(box.font_list->at(box.font_sel));
             p+=strlen(p)+1;
             char *s=0;
             if (box.font2_flg==1)
               {
                s=(char *)(box.font2_list->at(box.font2_sel));
                s+=strlen(s)+1;
               }
             LoadEditorFonts(p,s,IndexToCodePageID(box.font_encs),
                             IndexToCodePageID(box.font2_encs));
            }
         }
       else
       if (oldE1!=box.font_encs || oldE2!=box.font2_encs)
         {
          if (oldE1!=box.font_encs)
            {
             ChangeEncodingOfFont(1,IndexToCodePageID(box.font_encs));
             fontChanged=1;
            }
          if (oldE2!=box.font2_encs)
            {
             ChangeEncodingOfFont(2,IndexToCodePageID(box.font2_encs));
             fontChanged=1;
            }
         }
       extChange=strcmp(TSetEditorApp::ExternalPrgMode,box.command)!=0;
       strcpy(TSetEditorApp::ExternalPrgMode,box.command);
       if (TSetEditorApp::UseExternPrgForMode!=(char)box.ext_prg ||
           (box.ext_prg && extChange))
          fontChanged=1; // Not the font but force it ;-)
       TSetEditorApp::UseExternPrgForMode=box.ext_prg;

       ushort newmode=OptionToMode(box);
       if (oldmode!=newmode || fontChanged)
         {
          if (ResetVideoMode(newmode))
            {
             messageBox(mfError | mfOKButton,_("This video mode (0x%03x) is not"
                        " supported by the Turbo Vision library"),newmode);
             repeat=1;
             //fprintf(stderr,"Modo no soportado: %03x, seg£n TV: %03x\n",newmode,TScreen::screenMode);
            }
         }
      }
   }

 destroy(box.font_list);
 destroy(d);
}
#endif

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
         TDialog(TRect(1,1,1,1),_("File Open options")),
         TWindowInit(&TDiaFO::initFrame)
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
 TSLabel *Case=TSLabelRadio(__("Case style"),__("~C~apital letters goes first"),
                            __("Case ~i~nsensitive"),0);
 TSLabel *Parent=TSLabelRadio(__("Parent directory (..)"),__("First in the ~l~ist"),
                              __("At the ~e~nd of the list"),0);
 d->parentSort=Parent->linked->view;
 d->updateState(box.sortType);

 TSVeGroup *grp1=MakeVeGroup(1,Sort,Case,Parent,0);
 grp1->makeSameW();

 TSLabel *DotFiles=TSLabelRadio(__("Files starting with a dot"),__("~N~ormally sorted"),
                                __("After the ~r~est"),0);
 TSLabel *Exclude=TSLabelCheck(__("Exclude files"),__("Ending ~w~ith tilde"),
                               __("Ending with .bk~p~"),__("Startin~g~ with ."),0);

 TSVeGroup *grp2=MakeVeGroup(1,DotFiles,Exclude,0);
 grp2->makeSameW();

 col->insert(xTSLeft,yTSUp,grp1);
 col->insert(xTSRightOf,yTSUp,grp2,grp1);
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
           TDialog(TRect(1,1,1,1),_("Screen saver")),
           TWindowInit(&TDiaScrSaver::initFrame)
{
 commandsOn=1;
}


static
void ShowText(char *Text, char *Title)
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),_(Title)));

 char *s=Text;
 for (; *s; s++)
     if (*s=='\r')
        *s=' ';
 TSStaticText *text=new TSStaticText(Text,72);
 col->insert(xTSCenter,1,text);
 col->insert(xTSCenter,yTSUnder,new TSButton(_("O~K~"),cmOK,bfDefault,10),0,text);

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
    messageBox(_("The screen saver isn't useful for this terminal"),mfInformation | mfOKButton);
    return;
   }

 // EFHIMNOPRTX
 TDiaScrSaver *diaPrefs=new TDiaScrSaver();
 TSViewCol *col=new TSViewCol(diaPrefs);

 TSSortedListBox *scrv=new TSSortedListBox(24,5,tsslbVertical);
 diaPrefs->list=(TSortedListBox *)scrv->view;
 TSInputLine *extOptIn=new TSInputLine(extscrsParMxLen,24);
 diaPrefs->xtOpt=extOptIn->view;
 TSLabel *extOpts=new TSLabel(_("E~x~ternal options"),extOptIn);
 TSVeGroup *Scrv=new TSVeGroup(
   MakeVeGroup(0,TSLabelRadio(__("Sc~r~een saver (?)"),__("OF~F~"),__("O~N~"),0),
               scrv,extOpts,0), // First 3 joined
   MakeHzGroup(new TSButton(_("T~e~st"),cmTestScrv),
               new TSButton(_("~I~nfo"),cmInfoScrv),
               new TSButton(_("~H~elp"),cmHelpScrv),0)); // Buttons separated
 Scrv->makeSameW();

 TSHzLabel *timeL=new TSHzLabel(_("~T~ime"),new TSInputLine(5));
 TSStaticText *seconds=new TSStaticText(_("seconds"));
 TSHzGroup *Time=new TSHzGroup(timeL,seconds);
 Scrv=new TSVeGroup(Scrv,Time,0);

 TSHzGroup *TimeM=new TSHzGroup(new TSHzLabel(_("Ti~m~e"),new TSInputLine(5)),
                                new TSStaticText(_("mouse sec.")));
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
              __("~U~NIX style backups"),
              #define UNIX_STYLE_BKPS 2
              __("H~i~dden backups"),
              #define HIDDEN_BKPS 4
              __("R~e~mmember bkps to delete"),
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
              __("Don't remmember cursor ~p~osition"),
              #define NO_CURSOR_POS 256
              __("Don't ~w~arn about read-only files"),
              #define NO_RO_WARNING 512
              __("Open ~r~ead-only files as R.O. buffers"),
              #define RO_AS_RO 1024
              0);

 TSHzGroup *Clock=new TSHzGroup(TSLabelRadio(__("~C~lock"),__("OFF"),__("ON"),0),
                                TSLabelRadio(__("St~y~le"),__("AM/PM"),__("24 hs"),0));

 TSHzLabel *MaxEds=new TSHzLabel(_("~M~ax. editor copies"),new TSInputLine(5));
 TSHzLabel *MaxClo=new TSHzLabel(_("Max. c~l~osed to remember"),new TSInputLine(5));

 TSVeGroup *FirstCol=MakeVeGroup(0,tcb,Clock,MaxEds,MaxClo,0);
 FirstCol->makeSameW();

 col->insert(xTSLeft,yTSUp,FirstCol);
 col->insert(xTSCenter,yTSDown,
             MakeHzGroup(new TSButton(_("O~K~"),cmOK,bfDefault),
                         new TSButton(_("Cancel"),cmCancel),
                         new TSButton(_("+ Desk~t~op"),cmYes),
                         new TSButton(_("+ Others"),cmNo),0));

 TDialog *d=col->doItCenter(cmeEdGralOptions);
 delete col;

 BoxGral box;
 memset(&box,0,sizeof(BoxGral));

 //------ Arrange the "Save options" ----------
 if (TCEditor::editorFlags & efBackupFiles)
    box.ops|=MAKE_BKPS;
 if (TCEditor::editorFlags & efSaveUNIXasis)
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
       TCEditor::editorFlags|=efSaveUNIXasis;
    else
       TCEditor::editorFlags&=~efSaveUNIXasis;
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

 TSLabel *editors=TSLabelRadio(__("Remmember editor windows"),
         __("~E~ver"),__("~O~nly if no file specified"),__("~N~ever"),0);
 TSLabel *others=TSLabelRadio(__("Remmember other windows"),
         __("E~v~er"),__("On~l~y if no file specified"),__("Neve~r~"),0);
 TSLabel *closed=TSLabelRadio(__("Remmember ~c~losed windows"),
         __("Ever"),__("Onl~y~ if no file specified"),__("Never"),0);

 col->insert(xTSLeft,yTSUp,MakeVeGroup(0,editors,others,closed,0));
 col->insert(xTSCenter,yTSDown,
             MakeHzGroup(new TSButton(_("O~K~"),cmOK,bfDefault),
                         new TSButton(_("Cancel"),cmCancel),
                         new TSButton(_("~M~ain options"),cmYes),0));

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
 char   width[5] CLY_Packed;
} BoxOthers;
#pragma pack()

static
unsigned SetGeneralEditorOptionsOthers(void)
{
 TSViewCol *col=new TSViewCol(__("Other options"));

 // ENG: CIJKMRVW
 // ESP: ADIKPRSV
 TSVeGroup *MsgWin=MakeVeGroup(0,
   TSLabelRadio(__("At the end of errors in message window"),
          __("~J~ust stop"),
          __("~I~ndicate with a message"),
          __("Wrap (~c~ircular list)"),0),
   new TSCheckBoxes(new TSItem(_("Make a beep"),0)),
   new TSLabel(_("When creating message and similar windows"),
               new TSCheckBoxes(
                   new TSItem(_("Use the ~v~ertical direction"),
                   new TSItem(_("Use the ~r~ight side"),0)))),
   new TSHzLabel(_("Reserved ~w~idth"),new TSInputLine(5)),
   0);
 MsgWin->makeSameW();

 col->insert(xTSLeft,yTSUp,MsgWin);
 col->insert(xTSCenter,yTSDown,
             MakeHzGroup(new TSButton(_("O~K~"),cmOK,bfDefault),
                         new TSButton(_("Cancel"),cmCancel),
                         new TSButton(_("~M~ain options"),cmYes),0));

 TDialog *d=col->doItCenter(cmeEdGralOptions);
 delete col;

 BoxOthers box;
 box.end=TSOSListBoxMsg::opsEnd;
 box.beep=TSOSListBoxMsg::opsBeep;
 box.opts=TSetEditorApp::geFlags;
 char buf[32];
 sprintf(buf,"%d",TSetEditorApp::widthVertWindows);
 strncpy(box.width,buf,4);

 unsigned command=execDialog(d,&box);
 if (command!=cmCancel)
   {
    TSOSListBoxMsg::opsEnd=box.end;
    TSOSListBoxMsg::opsBeep=box.beep;
    TSetEditorApp::geFlags=box.opts;
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

int  ChooseConvCPs(int &From, int &To, uint32 &ops)
{
 FromToBox box;
 box.lFrom=box.lTo=GetCodePagesList();
 box.sFrom=CodePageIDToIndex(fromCP<0 ? GetCodePageFont(1) : fromCP);
 box.sTo  =CodePageIDToIndex(toCP<0   ? GetCodePageFont(2) : toCP);
 box.ops  =CPNoLow;

 TSHzGroup *cps=new TSHzGroup(
                 new TSLabel(_("~F~rom code page"),
                  new TSSortedListBox(wForced,6,tsslbVertical)),
                 new TSLabel(_("~T~o code page"),
                  new TSSortedListBox(wForced,6,tsslbVertical)));
 TSVeGroup *all=new TSVeGroup(cps,
                     new TSCheckBoxes(
                         new TSItem(_("~D~on't remap codes under 32"),
                         new TSItem(_("~O~nly selected text"),0))));
 all->makeSameW();

 TSViewCol *col=new TSViewCol(__("Code page remap"));
 col->insert(2,1,all);
 EasyInsertOKCancel(col);
 TDialog *d=col->doIt();
 delete col;
 d->options|=ofCentered;
 d->helpCtx=cmeRemapCodePage;

 int ret=execDialog(d,&box)==cmOK;
 fromCP=From=IndexToCodePageID(box.sFrom);
 toCP=To=IndexToCodePageID(box.sTo);
 ops=CPNoLow=box.ops;

 return ret;
}
