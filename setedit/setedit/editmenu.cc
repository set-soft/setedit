/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// That's the first include because is used to configure the editor.
#include "ceditint.h"
#define Uses_stdio
#define Uses_string

#define Uses_TApplication
#define Uses_TMenuBar
#define Uses_TRect
#define Uses_TMenu
#define Uses_TSubMenu
#define Uses_TMenuItem
#define Uses_TStatusLine
#define Uses_TStatusItem
#define Uses_TStatusDef
#define Uses_TPoint
#define Uses_MsgBox
#define Uses_TFileDialog
#define Uses_TDeskTop
#define Uses_TStringCollection
#define Uses_TSortedListBox
#define Uses_TKeys
#define Uses_TVCodePage
#define Uses_TScreen
// InfView requests
#include <infr.h>

#define Uses_TInputLinePipedConst
#define Uses_TCEditWindow
#define Uses_TCEditor_Commands
#include <ceditor.h>

#include <setconst.h>

#define Uses_TSetEditorApp
#define Uses_TMultiMenu
#define Uses_SETAppConst
#define Uses_SETAppDialogs
#include <setapp.h>

#include <stdlib.h>
#include <stdarg.h>

#include <inf.h>
#include <editcoma.h>

char *ExpandFileNameToThePointWhereTheProgramWasLoaded(const char *s);

TMenuBar *TSetEditorApp::initMenuBar(TRect r)
{
 r.b.y = r.a.y+1;

 multiMenuBar=GetTVMenu(ExpandFileNameToThePointWhereTheProgramWasLoaded("menubind.smn"),r);
 if (multiMenuBar)
    return multiMenuBar;

 // Spanish: ABCEHMPRV
 // English: CEFHMPSVW
 TSubMenu& sub1 = *new TSubMenu( __("~F~ile"), kbAltF ) +
   // E: ACHNOSUX
   *new TMenuItem( __("~O~pen..."), cmeOpen, kbF3 ) +
   *new TMenuItem( __("~N~ew"), cmeNew, kbNoKey ) +
   *new TMenuItem( __("~S~ave"), cmcSave, kbF2, hcNoContext, "F2" ) +
   *new TMenuItem( __("S~a~ve as..."), cmcSaveAs, kbNoKey ) +
   *new TMenuItem( __("Save as ~U~NIX..."), cmcSaveAsConvertEOL, kbNoKey ) +
          newLine() +
   *new TMenuItem( __("~C~hange dir..."), cmeChangeDrct, kbNoKey ) +
   *new TMenuItem( __("S~h~ell"), cmeDosShell, kbNoKey ) +
   *new TMenuItem( __("E~x~it"), cmeQuit, kbAltX, hcNoContext, "Alt-X" ) +
   *new TMenuItem( __("~Q~uit"), cmeQuitDelete, kbAltQ, hcNoContext, "Alt-Q" );

 TSubMenu& sub2 = *new TSubMenu( __("~E~dit"), kbAltE ) +
   *new TMenuItem( __("~U~ndo"), cmcUndo, kbAltBack, hcNoContext, "Alt+BackSpace" ) +
   *new TMenuItem( __("~R~edo"), cmcRedo, kbNoKey ) +
          newLine() +
   *new TMenuItem( __("Cu~t~"), cmcCut, kbShiftDel, hcNoContext, "Shift-Del" ) +
   *new TMenuItem( __("~C~opy"), cmcCopy, kbCtrlIns, hcNoContext, "Ctrl-Ins" ) +
   *new TMenuItem( __("~P~aste"), cmcPaste, kbShiftIns, hcNoContext, "Shift-Ins" ) +
   *new TMenuItem( __("~S~how clipboard"), cmeShowClip, kbNoKey ) +
          newLine() +
   *new TMenuItem( __("~C~lear"), cmcClear, kbCtrlDel, hcNoContext, "Ctrl-Del" ) +
          newLine() +
   *new TMenuItem( __("S~e~t Local"), cmcSetLocalOptions, kbAltL, hcNoContext, "Alt-L" ) +
   *new TMenuItem( __("Set ~G~lobal"), cmcSetGlobalOptions, kbAltG, hcNoContext, "Alt-G" ) +
          newLine() +
   *new TMenuItem( __("E~x~pand all tabs"), cmcExpandAllTabs, kbNoKey ) +
   *new TMenuItem( __("C~o~mpact text"), cmcCompactBuffer, kbNoKey ) +
          newLine() +
   *new TMenuItem( __("~J~ump to function"), cmcJumpToFunction, kbNoKey ) +
   *new TMenuItem( __("~P~rofile Editor"), cmcProfileEditor, kbNoKey );

 TSubMenu& sub3 = *new TSubMenu( __("~S~earch"), kbAltS ) +
   *new TMenuItem( __("~F~ind..."), cmcFind, kbNoKey ) +
   *new TMenuItem( __("~R~eplace..."), cmcReplace, kbNoKey ) +
   *new TMenuItem( __("~S~earch again"), cmcSearchAgain, kbNoKey );

 TSubMenu& sub4 = *new TSubMenu( __("~M~acro"), kbAltM ) +
   *new TMenuItem( __("~R~ecord"), cmcRecordMacro, kbShiftF10, hcNoContext, "Shift+F10" ) +
   *new TMenuItem( __("~S~top"), cmcStopMacro, kbAltF10, hcNoContext, "Alt+F10" ) +
   *new TMenuItem( __("~P~lay"), cmcPlayMacro, kbCtrlF10, hcNoContext, "Ctrl+F10" );

 TSubMenu& sub5 = *new TSubMenu( __("Re~c~tangle"), kbAltR ) +
   *new TMenuItem( __("~S~tart"), cmcSelRectStart, kbNoKey ) +
   *new TMenuItem( __("~E~nd"),   cmcSelRectEnd,   kbNoKey ) +
   *new TMenuItem( __("~H~ide"), cmcSelRectHide,   kbNoKey ) +
          newLine() +
   *new TMenuItem( __("~C~opy"),  cmcSelRectCopy,  kbNoKey ) +
   *new TMenuItem( __("~P~aste"), cmcSelRectPaste, kbNoKey ) +
   *new TMenuItem( __("Cu~t~"),   cmcSelRectCut,   kbNoKey ) +
   *new TMenuItem( __("C~l~ear"), cmcSelRectDel,   kbNoKey ) +
   *new TMenuItem( __("~M~ove"),  cmcSelRectMove,  kbNoKey );

 TSubMenu& sub6 = *new TSubMenu( __("~W~indows"), kbAltW ) +
   *new TMenuItem( __("~S~ize/move"),cmeResize, kbCtrlF5, hcNoContext, "Ctrl-F5" ) +
   *new TMenuItem( __("~Z~oom"), cmeZoom, kbF5, hcNoContext, "F5" ) +
   *new TMenuItem( __("~T~ile"), cmeTile, kbNoKey ) +
   *new TMenuItem( __("C~a~scade"), cmeCascade, kbNoKey ) +
   *new TMenuItem( __("~N~ext"), cmeNext, kbF6, hcNoContext, "F6" ) +
   *new TMenuItem( __("~P~revious"), cmePrev, kbShiftF6, hcNoContext, "Shift-F6" ) +
   *new TMenuItem( __("~C~lose"), cmeClose, kbAltF3, hcNoContext, "Alt-F3" ) +
   *new TMenuItem( __("~L~ist"), cmeListWin, kbAlt0, hcNoContext, "Alt-0" ) +
   *new TMenuItem( __("~U~ser Screen"), cmeUserScreen, kbAltF5, hcNoContext, "Alt-F5" );

 TSubMenu& sub7 = *new TSubMenu( __("~H~elp"), kbAltH ) +
   *new TMenuItem( __("~I~NF View"), cmeInfView, kbF1, hcNoContext, "F1" ) +
   *new TMenuItem( __("~A~nother InfView"), cmeAnotherInfView, kbNoKey );

 TSubMenu& sub8 = *new TSubMenu( __("~V~arious"), kbAltV ) +
   *new TMenuItem( __("~C~alculator"), cmeCalculator, kbAltF4, hcNoContext, "Alt+F4" ) +
   *new TMenuItem( __("~S~DG"), cmeSDG, kbF9, hcNoContext, "F9" ) +
   *new TMenuItem( __("SDG ~O~ptions"), cmeSDGDialog, hcNoContext, kbNoKey) +
   *new TMenuItem( __("Copy to ~W~indows Clip."), cmcCopyClipWin, hcNoContext, kbNoKey) +
   *new TMenuItem( __("Paste ~f~rom Wind. Clip."), cmcPasteClipWin, hcNoContext, kbNoKey) +
   *new TMenuItem( __("Co~l~ors"), cmeSetColors, hcNoContext, kbNoKey);

 TSubMenu& sub9 = *new TSubMenu( __("~P~roject"), kbAltP ) +
   *new TMenuItem( __("~O~pen..."), cmeOpenPrj, kbNoKey ) +
   *new TMenuItem( __("~C~lose"), cmeClosePrj, kbNoKey );

 TMultiMenu *m=new TMultiMenu();
 m->add(&(sub1+sub2+sub3+sub4+sub5+sub6+sub7+sub8+sub9));
 multiMenuBar=new TMultiMenuBar(r,m);
 return multiMenuBar;
}


TStatusLine *TSetEditorApp::initStatusLine( TRect r )
{
 // Note: any help context that belongs to a modal window (dialogs) must
 // contain F1 to get help.
 r.a.y = r.b.y-1;

 TStatusLine *st=GetTVStatusLine(ExpandFileNameToThePointWhereTheProgramWasLoaded("menubind.smn"),r);
 if (st)
    return st;

 st=
 new TStatusLine( r,
     *new TStatusDef( 0, 255 ) +
         *new TStatusItem( __("~F2~ Save"), kbF2, cmcSave ) +
         *new TStatusItem( __("~F3~ Open"), kbF3, cmeOpen ) +
         *new TStatusItem( __("~Alt+F3~ Close"), kbAltF3, cmeClose ) +
         *new TStatusItem( __("~F5~ Zoom"), kbF5, cmeZoom ) +
         *new TStatusItem( __("~F6~ Next"), kbF6, cmeNext ) +
         *new TStatusItem( __("~F10~ Menu"), kbF10, cmMenu ) +
         *new TStatusItem( 0, kbCtrlF5, cmeResize ) +
         *new TStatusItem( 0, kbAltF1, cmeLastHelp ) +
         *new TStatusItem( 0, kbAltF8, cmeNextMessage ) +
         *new TStatusItem( 0, kbAltF7, cmePrevMessage ) +
         *new TStatusItem( 0, kbF1, cmeInfView ) +
     *new TStatusDef( hcInfView, hcInfView+4 ) +
         *new TStatusItem( __("~Alt+F1~ Back"), kbAltF1, cmInfBack ) +
         *new TStatusItem( __("~Alt+F10~ Control"), kbAltF10, cmInfControl ) +
         *new TStatusItem( __("~^H~ Help"), kbCtrlH, cmInfHelp ) +
         *new TStatusItem( __("~ESC~ Close"), kbEsc, cmClose ) +
         *new TStatusItem( __("~Alt+I~ Include"), kbAltI, cmInfPasteIn ) +
         *new TStatusItem( __("~Alt+B~ Bookmark"), kbAltB, cmInfBookM ) +
         *new TStatusItem( 0, kbF5, cmZoom ) +
     *new TStatusDef( hcListWin, hcListWin+4 ) +
         *new TStatusItem( __("~Del~ Close"), kbDel, cmDelete ) +
         *new TStatusItem( __("~Ins~ Open"), kbIns, cmInsert ) +
         *new TStatusItem( __("~F1~ Help"), kbF1, cmeInfView ) +
         *new TStatusItem( __("~Ctrl+Del~ Delete File"), kbCtrlDel, cmDelFile ) +
     *new TStatusDef( hcEditorProjectWindow, hcEditorProjectWindow+4 ) +
         *new TStatusItem( __("~Ins~ Insert"), kbIns, cmInsert ) +
         *new TStatusItem( __("~Del~ Delete"), kbDel, cmDelete ) +
         *new TStatusItem( __("~F5~ Zoom"), kbF5, cmZoom ) +
         *new TStatusItem( 0, kbAltF1, cmeLastHelp ) +
     *new TStatusDef( hcMessageWindow, hcMessageWindow+1 ) +
         *new TStatusItem( __("~Alt+F7~ Previous"), kbAltF7, cmePrevMessage ) +
         *new TStatusItem( __("~Alt+F8~ Next"), kbAltF8, cmeNextMessage ) +
         *new TStatusItem( __("~Ctrl+C~ Stop"), kbCtrlC, cmeStopChild ) +
         *new TStatusItem( __("Save"), kbNoKey, cmcSaveAs ) +
         *new TStatusItem( __("~Ctrl+Ins~ Copy"), kbCtrlIns, cmcCopy ) +
         *new TStatusItem( 0, kbF5, cmZoom ) +
         *new TStatusItem( 0, kbAltF1, cmeLastHelp ) +
     *new TStatusDef( hcCalculator, hcCalculator+4 ) +
         *new TStatusItem( __("~ENTER~ Evaluate"), kbEnter, cmEval ) +
         *new TStatusItem( __("~Ctrl+Ins~ Copy"), kbCtrlIns, cmtilCopy ) +
         *new TStatusItem( __("~Shift+Ins~ Paste"), kbShiftIns, cmtilPaste ) +
         *new TStatusItem( __("~F1~ Help"), kbF1, cmeInfView ) +
     // Default for all the othe contexts
     *new TStatusDef( 0, 0xFFFF ) +
         *new TStatusItem( __("~F1~ Help"), kbF1, cmeInfView ) +
         *new TStatusItem( __("~F5~ Zoom"), kbF5, cmZoom )
         );
 return st;
}

void TSetEditorApp::outOfMemory()
{
 messageBox(__("Not enough memory for this operation."), mfError | mfOKButton);
}

#if 0
// The following are here just for internationalization purposes
_("Open Read-only ~c~opy")
_("Save w/ same ~t~ime...")
_("Save a~l~l")
_("~P~rint")
_("Pr~i~nt Setup...")
_("S~e~t Local options")
_("Set ~G~lobal options")
_("Pus~h~ cursor position")
_("Pop cursor pos~i~tion")
_("C~a~se (upper/lower)")
_("Block to ~u~pper")
_("Block to ~l~ower")
_("~C~haracter toggle")
_("Block ~i~nvert")
_("Block ~a~lternate")
_("~G~o to line")
_("Jump to ~p~rototype")
_("~N~ame current function")
_("~C~hoose...")
_("R~e~peat")
_("~G~enerate Code")
_("Ru~n~ selected code")
_("Enter c~o~de to run")
_("Pse~u~do macros...")
_("To ~u~pper")
_("To l~o~wer")
_("Poc~k~et calculator")
_("~R~un program")
_("~A~SCII Chart")
_("Ca~l~endar")
_("MP~3~ songs")
_("~S~elect a song")
_("~P~lay selected")
_("S~t~op selected")
_("~C~onvert to WAV")
_("~E~dit/Create list")
_("P~l~ay list")
_("St~o~p list")
_("~H~TML Accents")
_("Convert ~a~ccents to tags")
_("Convert ~t~ags to accents")
_("E~x~port as HTML...")
_("Re~m~ap code page...")
_("~B~lock quoted printable decode")
_("Un/~I~ndent block")
_("Indent ~o~ne space")
_("Unindent o~n~e character")
_("Indent one ~t~ab/gap")
_("~U~nindent one tab/gap")
_("~C~omment indent")
_("Comment unin~d~ent")
_("~A~rbitrary indent")
_("Paste ~E~macs mode")
_("Test of macro from menu")
_("~D~elete memorized backups")
_("Redra~w~ screen")
_("~C~olors")
_("~C~ustomize...")
_("~P~alette...")
_("~T~heme...")
_("~L~ocal edition...")
_("Glo~b~al edition...")
_("Editor ~G~eneral...")
_("Scr~e~en saver...")
_("S~D~G Options...")
_("~R~un program (which one)...")
_("~K~eyboard")
_("~K~ey assignment")
_("~S~etup Alt keys")
_("Key ~p~ad behavior")
_("~B~ack to defaults")
_("Consult ~s~can codes")
_("~S~creen Options...")
_("Encod~i~ngs...")
_("Fon~t~s...")
_("~U~ser words...")
_("De~f~ault global edition...")
_("~O~pen file dialog...")
_("Do~n~'t create backups for...")
_("~T~ip of the day")
_("~S~yntax help")
_("~F~iles to search")
_("~M~an page View")
_("A~b~out")
_("Searc~h~ files under cursor in...")
_("Insert key ~n~ame")
_("Sc~r~oll Up")
_("Scroll ~D~own")
_("Cop~y~ to file Clipboard")
_("Paste ~f~rom file Clipboard")
_("Copy to OS Clipboar~d~")
_("Paste from OS Clip~b~oard")
_("Save ~d~esktop here")
_("Sa~v~e as DOS...")
_("S~h~ell")
_("T~a~g files")
_("~L~ist of tag files...")
_("~O~ptions...")
_("Jump ~t~o symbol")
_("~C~lass browser")
_("~W~ord completion")
_("E~x~port project")
_("~I~mport project items")
_("Save As")
_("Calendar (holida~y~s)...")
_("Checking for ~m~odified files...")
_("Advice dialogs...")
_("Jump to ~l~ast cursor position")
_("Jump to last ~u~ndo position")
_("Insert ~n~ew line (don't move)")
_("Pro~j~ect Window")
_("~M~essage Window")
_("~E~dition Windows")
_("Window  ~2~")
_("Window  ~3~")
_("Window  ~4~")
_("Window  ~5~")
_("Window  ~6~")
_("Window  ~7~")
_("Window  ~8~")
_("Window  ~9~")
_("Window 10")
_("Window 11")
_("Window 12")
_("Window 13")
_("Window 14")
_("Window 15")
_("Window 16")
_("Window 17")
_("Window 18")
_("Window 19")
_("Pus~h~ cursor position and window")
_("P~o~p cursor position and window (tag ret)")
_("Debu~g~ger Window")
_("~W~atches Window")
_("~D~ebug")
_("~O~ptions")
_("~P~rogram and mode...")
_("Path for ~s~ources...")
_("~M~essages displayed...")
_("~A~dvanced...")
_("Go to 'Connected' ~1~")
_("Go to 'Ready to run' ~2~")
_("~B~reakpoint")
_("~R~un/Continue/Atach")
_("~S~tep over")
_("~T~race into")
_("~G~o to cursor")
_("~U~ntil return")
_("Return ~n~ow")
_("Sto~p~")
_("Restart (~K~ill)")
_("~E~valuate/Modify...")
_("~W~atch an expression")
_("~C~alling stack")
_("~D~ebug session")
_("De~t~ach")
_("C~l~ose")
_("~D~estroy (DANGER!)")
_("~E/+~ Expand")
_("~C/-~ Collapse")
_("~+~ Enable")
_("~-~ Disable")
_("~Ins~ Send command")
_("~Ctrl+A/+~ Add w/scope")
_("~R~egenerate central file")
_("Ed~i~t breakpoints...")
_("Edit watchpoints... ~3~")
_("Se~l~ect thread...")
_("Disasse~m~bler Window...")
_("~N~ormal watch...")
_("~W~ith scope...")
_("~U~sing the Inspector...")
_("~D~ata window...")
_("~S~tack window")
_("Clear deb~u~g elements...")
_("~R~ead block...")
_("~W~rite block...")
_("~U~p")
_("~D~own")
_("~R~ight")
_("~L~eft")
_("Page d~o~wn")
_("Page u~p~")
_("Fir~s~t column")
_("L~a~st column")
_("First ro~w~")
_("Las~t~ row")
_("First addr. ~i~ncrement")
_("~F~irst addr. decrement")
_("~A~ddress")
_("Change ~b~ase address...")
_("~G~o to new address...")
_("Follow ~p~ointer")
_("Follow pointer in ~n~ew window")
_("~R~ecompute address")
_("M~o~de")
_("Toggle ~a~uto follow")
_("Change ~d~isplay mode")
_("Toggle ~e~ndian mode")
_("Change ~r~adix")
_("~B~lock")
_("~F~ill...")
_("~C~lear...")
_("~M~ove...")
_("~V~arious")
_("~L~ess bytes per line")
_("~M~ore bytes per line")
_("~U~pdate memory")
_("Go to st~a~te")
_("~C~onnected")
_("~R~eady to run")
_("~O~pen info file...")
_("Con~f~iguration dialog...")
_("~B~ookmarks...")
_("List of ~n~odes...")
_("~G~o to '(file)node'...")
_("~N~avigation")
_("~L~ast visited topic")
_("~N~ext topic")
_("~P~revious topic")
_("Go ~u~p in herarchy")
_("Main menu for ~t~his file")
_("Info ~d~irectory")
_("~J~ump to last link")
_("~G~o to link number")
_("Link ~1~")
_("Link ~2~")
_("Link ~3~")
_("Link ~4~")
_("Link ~5~")
_("Link ~6~")
_("Link ~7~")
_("Link ~8~")
_("Link ~9~")
_("How to use the ~h~elp")
_("Con~t~rol dialog...")
#endif

