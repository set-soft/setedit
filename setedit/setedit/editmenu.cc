/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// That's the first include because is used to configure the editor.
#include "ceditint.h"
#define Uses_stdio
#define Uses_string

#define Uses_TApplication
#define Uses_TMenuBar
#define Uses_TRect
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
// InfView requests
#include <infr.h>

#define Uses_TInputLinePipedConst
#define Uses_TCEditWindow
#define Uses_TCEditor_Commands
#include <ceditor.h>

#include <setconst.h>

#define Uses_TSetEditorApp
#define Uses_SETAppConst
#define Uses_SETAppDialogs
#include <setapp.h>

#include <stdlib.h>
#include <stdarg.h>
#include <iomanip.h>

#include <inf.h>
#include <editcoma.h>

int LoadMenuAndStatus(char *fileName, int forceReload=0);
TMenuBar    *GetTVMenu(char *fileName, TRect &rect);
TStatusLine *GetTVStatusLine(char *fileName, TRect &rect);

char *ExpandFileNameToThePointWhereTheProgramWasLoaded(const char *s);

TMenuBar *TSetEditorApp::initMenuBar( TRect r )
{
 r.b.y = r.a.y+1;

 TMenuBar *mb=GetTVMenu(ExpandFileNameToThePointWhereTheProgramWasLoaded("menubind.smn"),r);
 if (mb)
    return mb;

 // Spanish: ABCEHMPRV
 // English: CEFHMPSVW
 TSubMenu& sub1 = *new TSubMenu( _("~F~ile"), kbAltF ) +
   // E: ACDNOSUX
   *new TMenuItem( _("~O~pen..."), cmeOpen, kbF3 ) +
   *new TMenuItem( _("~N~ew"), cmeNew, kbNoKey ) +
   *new TMenuItem( _("~S~ave"), cmcSave, kbF2, hcNoContext, "F2" ) +
   *new TMenuItem( _("S~a~ve as..."), cmcSaveAs, kbNoKey ) +
   *new TMenuItem( _("Save as ~U~NIX..."), cmcSaveAsUNIX, kbNoKey ) +
          newLine() +
   *new TMenuItem( _("~C~hange dir..."), cmeChangeDrct, kbNoKey ) +
   *new TMenuItem( _("~D~OS shell"), cmeDosShell, kbNoKey ) +
   *new TMenuItem( _("E~x~it"), cmeQuit, kbAltX, hcNoContext, "Alt-X" ) +
   *new TMenuItem( _("~Q~uit"), cmeQuitDelete, kbAltQ, hcNoContext, "Alt-Q" );

 TSubMenu& sub2 = *new TSubMenu( _("~E~dit"), kbAltE ) +
   *new TMenuItem( _("~U~ndo"), cmcUndo, kbAltBack, hcNoContext, "Alt+BackSpace" ) +
   *new TMenuItem( _("~R~edo"), cmcRedo, kbNoKey ) +
          newLine() +
   *new TMenuItem( _("Cu~t~"), cmcCut, kbShiftDel, hcNoContext, "Shift-Del" ) +
   *new TMenuItem( _("~C~opy"), cmcCopy, kbCtrlIns, hcNoContext, "Ctrl-Ins" ) +
   *new TMenuItem( _("~P~aste"), cmcPaste, kbShiftIns, hcNoContext, "Shift-Ins" ) +
   *new TMenuItem( _("~S~how clipboard"), cmeShowClip, kbNoKey ) +
          newLine() +
   *new TMenuItem( _("~C~lear"), cmcClear, kbCtrlDel, hcNoContext, "Ctrl-Del" ) +
          newLine() +
   *new TMenuItem( _("S~e~t Local"), cmcSetLocalOptions, kbAltL, hcNoContext, "Alt-L" ) +
   *new TMenuItem( _("Set ~G~lobal"), cmcSetGlobalOptions, kbAltG, hcNoContext, "Alt-G" ) +
          newLine() +
   *new TMenuItem( _("E~x~pand all tabs"), cmcExpandAllTabs, kbNoKey ) +
   *new TMenuItem( _("C~o~mpact text"), cmcCompactBuffer, kbNoKey ) +
          newLine() +
   *new TMenuItem( _("~J~ump to function"), cmcJumpToFunction, kbNoKey ) +
   *new TMenuItem( _("~P~rofile Editor"), cmcProfileEditor, kbNoKey );

 TSubMenu& sub3 = *new TSubMenu( _("~S~earch"), kbAltS ) +
   *new TMenuItem( _("~F~ind..."), cmcFind, kbNoKey ) +
   *new TMenuItem( _("~R~eplace..."), cmcReplace, kbNoKey ) +
   *new TMenuItem( _("~S~earch again"), cmcSearchAgain, kbNoKey );

 TSubMenu& sub4 = *new TSubMenu( _("~M~acro"), kbAltM ) +
   *new TMenuItem( _("~R~ecord"), cmcRecordMacro, kbShiftF10, hcNoContext, "Shift+F10" ) +
   *new TMenuItem( _("~S~top"), cmcStopMacro, kbAltF10, hcNoContext, "Alt+F10" ) +
   *new TMenuItem( _("~P~lay"), cmcPlayMacro, kbCtrlF10, hcNoContext, "Ctrl+F10" );

 TSubMenu& sub5 = *new TSubMenu( _("Re~c~tangle"), kbAltR ) +
   *new TMenuItem( _("~S~tart"), cmcSelRectStart, kbNoKey ) +
   *new TMenuItem( _("~E~nd"),   cmcSelRectEnd,   kbNoKey ) +
   *new TMenuItem( _("~H~ide"), cmcSelRectHide,   kbNoKey ) +
          newLine() +
   *new TMenuItem( _("~C~opy"),  cmcSelRectCopy,  kbNoKey ) +
   *new TMenuItem( _("~P~aste"), cmcSelRectPaste, kbNoKey ) +
   *new TMenuItem( _("Cu~t~"),   cmcSelRectCut,   kbNoKey ) +
   *new TMenuItem( _("C~l~ear"), cmcSelRectDel,   kbNoKey ) +
   *new TMenuItem( _("~M~ove"),  cmcSelRectMove,  kbNoKey );

 TSubMenu& sub6 = *new TSubMenu( _("~W~indows"), kbAltW ) +
   *new TMenuItem( _("~S~ize/move"),cmeResize, kbCtrlF5, hcNoContext, "Ctrl-F5" ) +
   *new TMenuItem( _("~Z~oom"), cmeZoom, kbF5, hcNoContext, "F5" ) +
   *new TMenuItem( _("~T~ile"), cmeTile, kbNoKey ) +
   *new TMenuItem( _("C~a~scade"), cmeCascade, kbNoKey ) +
   *new TMenuItem( _("~N~ext"), cmeNext, kbF6, hcNoContext, "F6" ) +
   *new TMenuItem( _("~P~revious"), cmePrev, kbShiftF6, hcNoContext, "Shift-F6" ) +
   *new TMenuItem( _("~C~lose"), cmeClose, kbAltF3, hcNoContext, "Alt-F3" ) +
   *new TMenuItem( _("~L~ist"), cmeListWin, kbAlt0, hcNoContext, "Alt-0" ) +
   *new TMenuItem( _("~U~ser Screen"), cmeUserScreen, kbAltF5, hcNoContext, "Alt-F5" );

 TSubMenu& sub7 = *new TSubMenu( _("~H~elp"), kbAltH ) +
   *new TMenuItem( _("~I~NF View"), cmeInfView, kbF1, hcNoContext, "F1" ) +
   *new TMenuItem( _("~A~nother InfView"), cmeAnotherInfView, kbNoKey );

 TSubMenu& sub8 = *new TSubMenu( _("~V~arious"), kbAltV ) +
   *new TMenuItem( _("~C~alculator"), cmeCalculator, kbAltF4, hcNoContext, "Alt+F4" ) +
   *new TMenuItem( _("~S~DG"), cmeSDG, kbF9, hcNoContext, "F9" ) +
   *new TMenuItem( _("SDG ~O~ptions"), cmeSDGDialog, hcNoContext, kbNoKey) +
   *new TMenuItem( _("Copy to ~W~indows Clip."), cmcCopyClipWin, hcNoContext, kbNoKey) +
   *new TMenuItem( _("Paste ~f~rom Wind. Clip."), cmcPasteClipWin, hcNoContext, kbNoKey) +
   *new TMenuItem( _("Co~l~ors"), cmeSetColors, hcNoContext, kbNoKey);

 TSubMenu& sub9 = *new TSubMenu( _("~P~roject"), kbAltP ) +
   *new TMenuItem( _("~O~pen..."), cmeOpenPrj, kbNoKey ) +
   *new TMenuItem( _("~C~lose"), cmeClosePrj, kbNoKey );

 return new TMenuBar( r, sub1 + sub2 + sub3 + sub4 + sub5 + sub6 + sub7 + sub8 + sub9);
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
         *new TStatusItem( _("~F2~ Save"), kbF2, cmcSave ) +
         *new TStatusItem( _("~F3~ Open"), kbF3, cmeOpen ) +
         *new TStatusItem( _("~Alt+F3~ Close"), kbAltF3, cmeClose ) +
         *new TStatusItem( _("~F5~ Zoom"), kbF5, cmeZoom ) +
         *new TStatusItem( _("~F6~ Next"), kbF6, cmeNext ) +
         *new TStatusItem( _("~F10~ Menu"), kbF10, cmMenu ) +
         *new TStatusItem( 0, kbCtrlF5, cmeResize ) +
         *new TStatusItem( 0, kbAltF1, cmeLastHelp ) +
         *new TStatusItem( 0, kbAltF8, cmeNextMessage ) +
         *new TStatusItem( 0, kbAltF7, cmePrevMessage ) +
         *new TStatusItem( 0, kbF1, cmeInfView ) +
     *new TStatusDef( hcInfView, hcInfView+4 ) +
         *new TStatusItem( _("~Alt+F1~ Back"), kbAltF1, cmInfBack ) +
         *new TStatusItem( _("~Alt+F10~ Control"), kbAltF10, cmInfControl ) +
         *new TStatusItem( _("~^H~ Help"), kbCtrlH, cmInfHelp ) +
         *new TStatusItem( _("~ESC~ Close"), kbEsc, cmClose ) +
         *new TStatusItem( _("~Alt+I~ Include"), kbAltI, cmInfPasteIn ) +
         *new TStatusItem( _("~Alt+B~ Bookmark"), kbAltB, cmInfBookM ) +
         *new TStatusItem( 0, kbF5, cmZoom ) +
     *new TStatusDef( hcListWin, hcListWin+4 ) +
         *new TStatusItem( _("~Del~ Close"), kbDel, cmDelete ) +
         *new TStatusItem( _("~Ins~ Open"), kbIns, cmInsert ) +
         *new TStatusItem( _("~F1~ Help"), kbF1, cmeInfView ) +
         *new TStatusItem( _("~Ctrl+Del~ Delete File"), kbCtrlDel, cmDelFile ) +
     *new TStatusDef( hcEditorProjectWindow, hcEditorProjectWindow+4 ) +
         *new TStatusItem( _("~Ins~ Insert"), kbIns, cmInsert ) +
         *new TStatusItem( _("~Del~ Delete"), kbDel, cmDelete ) +
         *new TStatusItem( _("~F5~ Zoom"), kbF5, cmZoom ) +
         *new TStatusItem( 0, kbAltF1, cmeLastHelp ) +
     *new TStatusDef( hcMessageWindow, hcMessageWindow+1 ) +
         *new TStatusItem( _("~Alt+F7~ Previous"), kbAltF7, cmePrevMessage ) +
         *new TStatusItem( _("~Alt+F8~ Next"), kbAltF8, cmeNextMessage ) +
         *new TStatusItem( _("~Ctrl+C~ Stop"), kbCtrlC, cmeStopChild ) +
         *new TStatusItem( _("Save"), kbNoKey, cmcSaveAs ) +
         *new TStatusItem( _("~Ctrl+Ins~ Copy"), kbCtrlIns, cmcCopy ) +
         *new TStatusItem( 0, kbF5, cmZoom ) +
         *new TStatusItem( 0, kbAltF1, cmeLastHelp ) +
     *new TStatusDef( hcCalculator, hcCalculator+4 ) +
         *new TStatusItem( _("~ENTER~ Evaluate"), kbEnter, cmEval ) +
         *new TStatusItem( _("~Ctrl+Ins~ Copy"), kbCtrlIns, cmtilCopy ) +
         *new TStatusItem( _("~Shift+Ins~ Paste"), kbShiftIns, cmtilPaste ) +
         *new TStatusItem( _("~F1~ Help"), kbF1, cmeInfView ) +
     // Default for all the othe contexts
     *new TStatusDef( 0, 0xFFFF ) +
         *new TStatusItem( _("~F1~ Help"), kbF1, cmeInfView ) +
         *new TStatusItem( _("~F5~ Zoom"), kbF5, cmZoom )
         );
 return st;
}

void TSetEditorApp::outOfMemory()
{
 messageBox(_("Not enough memory for this operation."), mfError | mfOKButton);
}

#if 0
// The following are here just for internationalization purposes
_("Open Read-only ~c~opy")
_("Save w/same ~t~ime...")
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
_("~C~hoose ...")
_("R~e~peat")
_("~G~enerate Code")
_("Ru~n~ selected code")
_("Enter c~o~de to run")
_("Pse~u~do macros ...")
_("To ~u~pper")
_("To l~o~wer")
_("Poc~k~et calculator")
_("~R~un program")
_("~A~SCII Chart")
_("Ca~l~endar")
_("~M~P3 songs")
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
_("E~x~port as HTML ...")
_("Re~m~ap code page ...")
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
_("~C~olors ...")
_("Color ~P~alette ...")
_("~L~ocal edition ...")
_("Glo~b~al edition ...")
_("Editor ~G~eneral ...")
_("Scr~e~en saver ...")
_("S~D~G Options ...")
_("~R~un program (which one) ...")
_("~K~eyboard")
_("~K~ey assignment")
_("~S~etup Alt keys")
_("Key ~p~ad behavior")
_("~B~ack to defaults")
_("Consult ~s~can codes")
_("~S~creen Options ...")
_("~U~ser words ...")
_("De~f~ault global edition ...")
_("File ~o~pen dialog ...")
_("Do~n~'t create backups for ...")
_("~T~ip of the day")
_("~S~yntax help")
_("~F~iles to search")
_("~M~an page View")
_("A~b~out")
_("Searc~h~ files under cursor in ...")
_("Insert key ~n~ame")
// Colors Window
_("~G~roup")
_("~I~tem")
_("~F~oreground")
_("~B~ackground")
_("~T~ry")
#endif
