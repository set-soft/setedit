/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef EDHISTS_H_INCLUDED
#define EDHISTS_H_INCLUDED
/* Here I define the history IDs for the editor.
   I start from 120 to avoid any conflict even when RHIDE workarounds it. */

const int
hID_Start             = 120,
hID_Cant              = 17;

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
hID_ExportProjectItems= hID_Start+18;
#endif
