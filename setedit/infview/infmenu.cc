/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string
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
// InfView requests
#include <infr.h>

#define Uses_TCEditor_Commands
#include <ceditor.h>

#include <stdarg.h>
#include "infalone.h"
#include "inf.h"

TMenuBar *TEditorMiApp::initMenuBar( TRect r )
{
 TSubMenu& sub1 = *new TSubMenu( __("~F~ile"), kbAltF ) +
   *new TMenuItem( __("~N~ew InfView window"), cmInfView, kbNoKey ) +
   *new TMenuItem( __("~O~pen info file"), cmInfMainOpen, kbF3, hcNoContext, "F3" ) +
   *new TMenuItem( __("Open ~m~anpage"), cmManPage, kbF4, hcNoContext, "F4" ) +
   #if defined(TVOS_DOS) || (defined(TVOS_Win32) && !defined(TVCompf_Cygwin))
   *new TMenuItem( __("~D~OS shell"), cmCallShell, kbNoKey ) +
   #else
   *new TMenuItem( __("S~h~ell"), cmCallShell, kbNoKey ) +
   #endif
   *new TMenuItem( __("E~x~it"), cmQuit, kbAltX, hcNoContext, "Alt+X" );

 TSubMenu& sub2 = *new TSubMenu( __("~G~oto"), kbAltG ) +
   *new TMenuItem( __("Goto: (~F~ile)Node"), cmInfGoto, kbCtrlG, hcNoContext, "Ctrl+G" ) +
   *new TMenuItem( __("~N~ode list"), cmInfNodes, kbCtrlO, hcNoContext, "Ctrl+O" ) +
   *new TMenuItem( __("N~e~xt node"), cmhNext, kbCtrlN, hcNoContext, "Ctrl+N" ) +
   *new TMenuItem( __("~P~revious node"), cmhPrev, kbCtrlP, hcNoContext, "Ctrl+P" ) +
   *new TMenuItem( __("~U~pper node"), cmhUp, kbCtrlU, hcNoContext, "Ctrl+U" ) +
   *new TMenuItem( __("~T~op node"), cmInfTop, kbCtrlT, hcNoContext, "Ctrl+T" ) +
   *new TMenuItem( __("~D~irectory"), cmInfDir, kbCtrlD, hcNoContext, "Ctrl+D" );
              
 TSubMenu& sub3 = *new TSubMenu( __("~S~earch"), kbAltS ) +
   *new TMenuItem( __("~S~earch"), cmcFind, kbCtrlS, hcNoContext, "Ctrl+S" ) +
   *new TMenuItem( __("Search ~a~gain"), cmcSearchAgain, kbCtrlL, hcNoContext, "Ctrl+L");
   
 TSubMenu& sub4 = *new TSubMenu( __("~I~nfview"), kbAltI ) +
   *new TMenuItem( __("~C~onfiguration"), cmhConfigDia, kbNoKey ) +
   *new TMenuItem( __("~H~istory"), cmInfControl, kbAltF10, hcNoContext, "Alt+F10") +
   *new TMenuItem( __("~B~ookmarks"), cmInfBookM, kbCtrlB, hcNoContext, "Ctrl+B") +
   *new TMenuItem( __("~S~creen configuration"), cmScreenConf, kbNoKey ) +
   *new TMenuItem( __("~A~bout"), cmAbout, kbNoKey);

 TSubMenu& sub5 = *new TSubMenu( __("~W~indows"), kbAltW ) +
   *new TMenuItem( __("~S~ize/move"),cmResize, kbCtrlF5, hcNoContext, "Ctrl+F5" ) +
   *new TMenuItem( __("~Z~oom"), cmZoom, kbF5, hcNoContext, "F5" ) +
   *new TMenuItem( __("~T~ile"), cmTile, kbNoKey ) +
   *new TMenuItem( __("C~a~scade"), cmCascade, kbNoKey ) +
   *new TMenuItem( __("~N~ext"), cmNext, kbF6, hcNoContext, "F6" ) +
   *new TMenuItem( __("~P~revious"), cmPrev, kbShiftF6, hcNoContext, "Shift+F6" ) +
   *new TMenuItem( __("~C~lose"), cmClose, kbAltF3, hcNoContext, "Alt+F3" );

 r.b.y = r.a.y+1;
 return new TMenuBar(r, sub1+sub2+sub3+sub4+sub5);
}

TStatusLine *TEditorMiApp::initStatusLine( TRect r )
{
 r.a.y = r.b.y-1;
 return new TStatusLine( r,
     *new TStatusDef( hcInfView, hcInfView ) +
         *new TStatusItem(__("~Alt+F1~ Back"), kbAltF1, cmInfBack ) +
         *new TStatusItem(__("~Alt+F10~ Control"), kbAltF10, cmInfControl ) +
         *new TStatusItem(__("~F1~ Help"), kbF1, cmInfHelp ) +
         *new TStatusItem(__("~Ctrl+O~ Index"), kbCtrlO, cmInfNodes ) +
         *new TStatusItem( __("~F5~ Zoom"), kbF5, cmZoom ) +
     *new TStatusDef( 0, 0xFFFF ) +
         *new TStatusItem( __("~F5~ Zoom"), kbF5, cmZoom ) +
         *new TStatusItem( __("~F6~ Next"), kbF6, cmNext ) +
         *new TStatusItem( __("~F10~ Menu"), kbF10, cmMenu ) +
         *new TStatusItem( 0, kbCtrlF5, cmResize )
         );
}

void TEditorMiApp::outOfMemory()
{
 messageBox(__("Not enough memory for this operation."), mfError | mfOKButton );
}

