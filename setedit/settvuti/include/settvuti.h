/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

  (c) 1997-1998 by SET

  See the individual headers for details.

*****************************************************************************/

#ifdef Uses_TStreamableClass
#define _INCL_TVSETUTI
#endif

#ifdef Uses_TDialogAID
#define Uses_TGrowDialog
#define Uses_TStringableListBox
#define Uses_TScrollBar
#define _INCL_TDIAGAID
#endif

#ifdef Uses_TNoCaseStringCollection
#define _INCL_TNOCASTC
#define _INCL_STRING
#define _INCL_TVSETUTI
#define Uses_TStringCollection
#define Uses_string
#endif

#ifdef Uses_TStringCollectionW
#define _INCL_TVSETUTI
#define _INCL_TNOCASTC
#define Uses_TStringCollection
#endif

#ifdef Uses_TNoSortedStringCollection
#define _INCL_TNOSOSTR
#define _INCL_TVSETUTI
#define Uses_TStringCollection
#endif

#ifdef Uses_TNoCaseNoOwnerStringCollection
#define _INCL_TNOCASTC
#define _INCL_STRING
#define Uses_TStringCollection
#define Uses_string
#endif

#ifdef Uses_TSOSStringCollection
#define Uses_TNoCaseSOSStringCollection
#endif

#ifdef Uses_TNoCaseSOSStringCollection
#define _INCL_TNOCASTC
#define Uses_SOStack
#define Uses_TStringCollection
#endif

#ifdef Uses_TSOSSortedListBox
#define _INCL_TNOCASTC
#define Uses_SOStack
#define Uses_TSortedListBox
#endif

#ifdef Uses_TGrowDialog
#define _INCL_TDIAGROW
#define Uses_TDialog
#define Uses_TWindowInit
#endif

#ifdef Uses_TViewPlus
#define _INCL_VIEWPLUS
#define Uses_TView
#endif

#ifdef Uses_TInputLinePiped
#define _INCL_TINPPIPE
#define Uses_TInputLine
#endif

#ifdef Uses_TInputLinePipedConst
#define _INCL_TINPPIPE
#endif

#ifdef Uses_TNSSOSCol
#define _INCL_TNOCASTC
#define Uses_SOStack
#define Uses_TNSCollection
#endif

#ifdef Uses_TSOSCol
#define _INCL_TNOCASTC
#define Uses_SOStack
#define Uses_TCollection
#endif

#ifdef Uses_TSArray_Dec
#define Uses_TSArray_Def
#define _INCL_SARRAY_CC
#define _INCL_STDLIB
#endif

#ifdef Uses_TSArray_Def
#define _INCL_SARRAY_H
#endif

#ifdef Uses_TInputScanKey
#define Uses_TView
#define Uses_TRect
#define _INCL_INPUTSCAN_H
#endif

#ifdef Uses_TStringableListBox
#define Uses_TListViewer
#define Uses_TStringable
#define _INCL_TSTRLBOX_H
#endif

#ifdef Uses_TStringable
#define _INCL_TSTRINGA_H
#endif

#ifdef Uses_TDeskTopClock
#define Uses_TView
#define _INCL_DKTCLOCK_H
#define _INCL_TIME
#endif

#ifdef Uses_FileOpenAid
#define Uses_GenericFileDialog
#define _INCL_HISTS_H
#define _INCL_FIOPEAID_H
#endif

#ifdef Uses_GenericFileDialog
#define _INCL_FILEOPEN_H
#endif

#ifdef Uses_TProgressBar
#define _INCL_TPROGBAR_H
#define _INCL_TVSETUTI
#define Uses_TView
#define Uses_TRect
#endif

#ifdef Uses_TProgress
#define _INCL_TPROGRES_H
#define _INCL_TVSETUTI
#define Uses_TView
#define Uses_TRect
#endif

#ifdef Uses_ProgBar
#define _INCL_TPROBDIA_H
#endif

#ifdef Uses_Progress
#define _INCL_TPROGRES_H
#endif

#ifdef Uses_TNoStaticText
#define INCL_NOSTATEX
#define Uses_TStaticText
#endif

#ifdef Uses_SOStack
#define _INCL_SETSTACK
#define _INCL_STDLIB
#endif

#ifdef _INCL_STDLIB
#define Uses_stdlib
#endif

#ifdef _INCL_TIME
#define Uses_time
#endif

#include <tv.h>

extern char *strncpyZ(char *dest, const char *orig, int size);

#ifdef INCL_NOSTATEX
#include <nostatex.h>
#endif

#ifdef _INCL_TPROBDIA_H
#include <tprogdia.h>
#endif

#ifdef _INCL_TPROGRES_H
#include <tprogrdi.h>
#endif

#ifdef _INCL_TVSETUTI
#include <tvsetuti.h>
#endif

#ifdef _INCL_TPROGBAR_H
#include <tprogbar.h>
#endif

#ifdef _INCL_TPROGRES_H
#include <tprogres.h>
#endif

#ifdef _INCL_FILEOPEN_H
#include <fileopen.h>
#endif

#ifdef _INCL_HISTS_H
#include <edhists.h>
#endif

#ifdef _INCL_FIOPEAID_H
#include <fiopeaid.h>
#endif

#ifdef _INCL_DKTCLOCK_H
#include <dktclock.h>
#endif

#ifdef _INCL_TSTRINGA_H
#include <tstringa.h>
#endif

#ifdef _INCL_TSTRLBOX_H
#include <tstrlbox.h>
#endif

#ifdef _INCL_INPUTSCAN_H
#include <inputsca.h>
#endif

#ifdef _INCL_SARRAY_H
#include <sarray.h>
#endif

#ifdef _INCL_SARRAY_CC
#include <sarray.cc>
#endif

#ifdef _INCL_TINPPIPE
#include <tinppipe.h>
#endif

#ifdef _INCL_SETSTACK
#ifndef __SET_SOStack_H__
#include <setstack.h>
#endif
#endif

#ifdef _INCL_TDIAGROW
#include <tdiagrow.h>
#endif

#ifdef _INCL_TNOCASTC
#include <tnocastc.h>
#endif

#ifdef _INCL_TNOSOSTR
#include <tnosostr.h>
#endif

#ifdef _INCL_VIEWPLUS
#include <viewplus.h>
#endif

#ifdef _INCL_TDIAGAID
#include <tdiagaid.h>
#endif


