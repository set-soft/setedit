/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TApplication
#define Uses_TMenuBar
#define Uses_TRect
#define Uses_TSubMenu
#define Uses_TKeys
#define Uses_TKeys
#define Uses_TMenuItem
#define Uses_TStatusLine
#define Uses_TStatusItem
#define Uses_TStatusDef
#define Uses_TPoint
#define Uses_TEditor
#define Uses_MsgBox
#define Uses_TFileDialog
#define Uses_TDeskTop

#define Uses_TCEditor_Commands
#include <ceditor.h>

#include <stdarg.h>
#include <strstream.h>
#include <iomanip.h>
#include "infalone.h"
#include "inf.h"

TMenuBar *TEditorMiApp::initMenuBar( TRect r )
{
 TSubMenu& sub1 = *new TSubMenu( "~F~ile", kbAltF ) +
   *new TMenuItem( "~O~pen info file", cmInfOpen, kbF3, hcNoContext, "F3" ) +
   #ifdef TVOS_DOS
   *new TMenuItem( "~D~OS shell", cmDosShell, kbNoKey ) +
   #endif
   *new TMenuItem( "E~x~it", cmQuit, kbAltX, hcNoContext, "Alt+X" );

 TSubMenu& sub2 = *new TSubMenu( "~G~oto", kbAltG ) +
   *new TMenuItem( "Goto: (~F~ile)Node", cmInfGoto, kbCtrlG, hcNoContext, "Ctrl+G" ) +
   *new TMenuItem( "~N~ode list", cmInfNodes, kbCtrlO, hcNoContext, "Ctrl+O" ) +
   *new TMenuItem( "N~e~xt node", chcdNext, kbCtrlN, hcNoContext, "Ctrl+N" ) +
   *new TMenuItem( "~P~revious node", chcdPrev, kbCtrlP, hcNoContext, "Ctrl+P" ) +
   *new TMenuItem( "~U~pper node", chcdUp, kbCtrlU, hcNoContext, "Ctrl+U" ) +
   *new TMenuItem( "~T~op node", cmInfTop, kbCtrlT, hcNoContext, "Ctrl+T" ) +
   *new TMenuItem( "~D~irectory", cmInfDir, kbCtrlD, hcNoContext, "Ctrl+D" );
              
 TSubMenu& sub3 = *new TSubMenu( "~S~earch", kbAltS ) +
   *new TMenuItem( "~S~earch", cmcFind, kbCtrlS, hcNoContext, "Ctrl+S" ) +
   *new TMenuItem( "Search ~a~gain", cmcSearchAgain, kbCtrlL, hcNoContext, "Ctrl+L");
   
 TSubMenu& sub4 = *new TSubMenu( "~I~nfview", kbAltI ) +
   *new TMenuItem( "~N~ew InfView window", cmInfView, kbNoKey ) +
   *new TMenuItem( "~C~onfiguration", chcdConfigDia, kbNoKey ) +
   *new TMenuItem( "~H~istory", cmInfControl, kbAltF10, hcNoContext, "Alt+F10") +
   *new TMenuItem( "~B~ookmarks", cmInfBookM, kbCtrlB, hcNoContext, "Ctrl+B") +
   *new TMenuItem( "~A~bout", cmAbout, kbNoKey);

 TSubMenu& sub5 = *new TSubMenu( "~W~indows", kbAltW ) +
   *new TMenuItem( "~S~ize/move",cmResize, kbCtrlF5, hcNoContext, "Ctrl+F5" ) +
   *new TMenuItem( "~Z~oom", cmZoom, kbF5, hcNoContext, "F5" ) +
   *new TMenuItem( "~T~ile", cmTile, kbNoKey ) +
   *new TMenuItem( "C~a~scade", cmCascade, kbNoKey ) +
   *new TMenuItem( "~N~ext", cmNext, kbF6, hcNoContext, "F6" ) +
   *new TMenuItem( "~P~revious", cmPrev, kbShiftF6, hcNoContext, "Shift+F6" ) +
   *new TMenuItem( "~C~lose", cmClose, kbAltF3, hcNoContext, "Alt+F3" );

 r.b.y = r.a.y+1;
 return new TMenuBar(r, sub1+sub2+sub3+sub4+sub5);
}

TStatusLine *TEditorMiApp::initStatusLine( TRect r )
{
 r.a.y = r.b.y-1;
 return new TStatusLine( r,
     *new TStatusDef( 0, 255 ) +
         *new TStatusItem( "~F5~ Zoom", kbF5, cmZoom ) +
         *new TStatusItem( "~F6~ Next", kbF6, cmNext ) +
         *new TStatusItem( "~F10~ Menu", kbF10, cmMenu ) +
         *new TStatusItem( 0, kbCtrlF5, cmResize ) +
     *new TStatusDef( hcInfView, hcInfView ) +
         *new TStatusItem("~Alt+F1~ Back", kbAltF1, cmInfBack ) +
         *new TStatusItem("~Alt+F10~ Control", kbAltF10, cmInfControl ) +
         *new TStatusItem("~F1~ Help", kbF1, cmInfHelp ) +
         *new TStatusItem("~Ctrl+O~ Index", kbCtrlO, cmInfNodes ) +
         *new TStatusItem( "~F5~ Zoom", kbF5, cmZoom )
         );
}

void TEditorMiApp::outOfMemory()
{
 messageBox("Not enough memory for this operation.", mfError | mfOKButton );
}

