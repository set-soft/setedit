/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/******************************************************************************

  This file is part of the TCEditor class by SET.

******************************************************************************/

/*------------------------------------------------------------*/
/* filename -       sedhelp.cc                                */
/*                                                            */
/* Registeration objects for the following classes:           */
/*                      TEditorCollection                     */
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
/*                                                            */
/*  Needed to write the objects to a stream.                  */
/*                                                            */
/*------------------------------------------------------------*/

// That's the first include because is used to configure the editor.
#include "ceditint.h"

#define Uses_TCollection
#define Uses_TListBox
#define Uses_TStringCollection
#define Uses_TWindow
#define Uses_TStreamableClass
#include <tv.h>

#define Uses_TCEditor
#define Uses_TCFileEditor
#define Uses_TCEditWindow
#include <ceditor.h>

//#include <ed.h>

TStreamableClass RDskWinHelp( TDskWinHelp::name,
                                    TDskWinHelp::build,
                                    __DELTA(TDskWinHelp)
                                   );
