/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
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
 TSubMenu& sub1 = *new TSubMenu( _("~F~ile"), kbAltF ) +
   *new TMenuItem( _("~N~ew InfView window"), cmInfView, kbNoKey ) +
   *new TMenuItem( _("~O~pen info file"), cmInfMainOpen, kbF3, hcNoContext, "F3" ) +
   *new TMenuItem( _("Open ~m~anpage"), cmManPage, kbF4, hcNoContext, "F4" ) +
   #if defined(TVOS_DOS) || (defined(TVOS_Win32) && !defined(TVCompf_Cygwin))
   *new TMenuItem( _("~D~OS shell"), cmCallShell, kbNoKey ) +
   #else
   *new TMenuItem( _("S~h~ell"), cmCallShell, kbNoKey ) +
   #endif
   *new TMenuItem( _("E~x~it"), cmQuit, kbAltX, hcNoContext, "Alt+X" );

 TSubMenu& sub2 = *new TSubMenu( _("~G~oto"), kbAltG ) +
   *new TMenuItem( _("Goto: (~F~ile)Node"), cmInfGoto, kbCtrlG, hcNoContext, "Ctrl+G" ) +
   *new TMenuItem( _("~N~ode list"), cmInfNodes, kbCtrlO, hcNoContext, "Ctrl+O" ) +
   *new TMenuItem( _("N~e~xt node"), chcdNext, kbCtrlN, hcNoContext, "Ctrl+N" ) +
   *new TMenuItem( _("~P~revious node"), chcdPrev, kbCtrlP, hcNoContext, "Ctrl+P" ) +
   *new TMenuItem( _("~U~pper node"), chcdUp, kbCtrlU, hcNoContext, "Ctrl+U" ) +
   *new TMenuItem( _("~T~op node"), cmInfTop, kbCtrlT, hcNoContext, "Ctrl+T" ) +
   *new TMenuItem( _("~D~irectory"), cmInfDir, kbCtrlD, hcNoContext, "Ctrl+D" );
              
 TSubMenu& sub3 = *new TSubMenu( _("~S~earch"), kbAltS ) +
   *new TMenuItem( _("~S~earch"), cmcFind, kbCtrlS, hcNoContext, "Ctrl+S" ) +
   *new TMenuItem( _("Search ~a~gain"), cmcSearchAgain, kbCtrlL, hcNoContext, "Ctrl+L");
   
 TSubMenu& sub4 = *new TSubMenu( _("~I~nfview"), kbAltI ) +
   *new TMenuItem( _("~C~onfiguration"), chcdConfigDia, kbNoKey ) +
   *new TMenuItem( _("~H~istory"), cmInfControl, kbAltF10, hcNoContext, "Alt+F10") +
   *new TMenuItem( _("~B~ookmarks"), cmInfBookM, kbCtrlB, hcNoContext, "Ctrl+B") +
   *new TMenuItem( _("~S~creen configuration"), cmScreenConf, kbNoKey ) +
   *new TMenuItem( _("~A~bout"), cmAbout, kbNoKey);

 TSubMenu& sub5 = *new TSubMenu( _("~W~indows"), kbAltW ) +
   *new TMenuItem( _("~S~ize/move"),cmResize, kbCtrlF5, hcNoContext, "Ctrl+F5" ) +
   *new TMenuItem( _("~Z~oom"), cmZoom, kbF5, hcNoContext, "F5" ) +
   *new TMenuItem( _("~T~ile"), cmTile, kbNoKey ) +
   *new TMenuItem( _("C~a~scade"), cmCascade, kbNoKey ) +
   *new TMenuItem( _("~N~ext"), cmNext, kbF6, hcNoContext, "F6" ) +
   *new TMenuItem( _("~P~revious"), cmPrev, kbShiftF6, hcNoContext, "Shift+F6" ) +
   *new TMenuItem( _("~C~lose"), cmClose, kbAltF3, hcNoContext, "Alt+F3" );

 r.b.y = r.a.y+1;
 return new TMenuBar(r, sub1+sub2+sub3+sub4+sub5);
}

TStatusLine *TEditorMiApp::initStatusLine( TRect r )
{
 r.a.y = r.b.y-1;
 return new TStatusLine( r,
     *new TStatusDef( hcInfView, hcInfView ) +
         *new TStatusItem(_("~Alt+F1~ Back"), kbAltF1, cmInfBack ) +
         *new TStatusItem(_("~Alt+F10~ Control"), kbAltF10, cmInfControl ) +
         *new TStatusItem(_("~F1~ Help"), kbF1, cmInfHelp ) +
         *new TStatusItem(_("~Ctrl+O~ Index"), kbCtrlO, cmInfNodes ) +
         *new TStatusItem( _("~F5~ Zoom"), kbF5, cmZoom ) +
     *new TStatusDef( 0, 0xFFFF ) +
         *new TStatusItem( _("~F5~ Zoom"), kbF5, cmZoom ) +
         *new TStatusItem( _("~F6~ Next"), kbF6, cmNext ) +
         *new TStatusItem( _("~F10~ Menu"), kbF10, cmMenu ) +
         *new TStatusItem( 0, kbCtrlF5, cmResize )
         );
}

void TEditorMiApp::outOfMemory()
{
 messageBox(_("Not enough memory for this operation."), mfError | mfOKButton );
}

