/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef EDHISTS_H_INCLUDED
#define EDHISTS_H_INCLUDED
/* Here I define the history IDs for the editor.
   I start from 120 to avoid any conflict even when RHIDE workarounds it. */

const int
hID_Start             = 120,
hID_Cant              = 36;

const int
hID_TextSearchEditor  = hID_Start,
hID_TextReplaceEditor = hID_Start+1,
hID_TCalcDialogExp    = hID_Start+2,
hID_TextSearchInf     = hID_Start+3,
hID_GotoInf           = hID_Start+4,
hID_FileOpen          = hID_Start+5,
hID_OpenInfo          = hID_Start+6,
hID_SaveBlock         = hID_Start+7,
hID_GrepFiles         = hID_Start+8,
hID_GrepPlaces        = hID_Start+9,
hID_FileSave          = hID_Start+10,
hID_ConfigFiles       = hID_Start+11,
hID_ProjectFiles      = hID_Start+12,
hID_OpenMP3           = hID_Start+13,
hID_SaveMP3           = hID_Start+14,
hID_GotoLine          = hID_Start+15,
hID_sLispMacros       = hID_Start+16,
hID_SelectTagFile     = hID_Start+17,
hID_ExportProjectItems= hID_Start+18,
hID_sLispKeyCode      = hID_Start+19,
hID_ArbitraryIndent   = hID_Start+20,
hID_RunProgram        = hID_Start+21,
hID_DbgEvalModifyExp  = hID_Start+22,
hID_DbgEvalModifyNewV = hID_Start+23,
hID_DbgBkFilename     = hID_Start+24,
hID_DbgBkFunction     = hID_Start+25,
hID_DbgBkLine         = hID_Start+26,
hID_DbgBkAddress      = hID_Start+27,
hID_DbgBkCondition    = hID_Start+28,
hID_DbgBkCount        = hID_Start+29,
hID_DbgBkThread       = hID_Start+30,
hID_DbgCommand        = hID_Start+31,
hID_DbgGDB            = hID_Start+32,
hID_DbgXTerm          = hID_Start+33,
hID_DbgMainFunc       = hID_Start+34,
hID_DbgSourceLoc      = hID_Start+35,
hID_DbgBinary         = hID_Start+36;
#endif
