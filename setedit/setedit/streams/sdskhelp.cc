/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TCEditWindow
#define Uses_TWindow

#define Uses_TStreamableClass
#define Uses_TStreamable
#define Uses_string
// InfView requests
#include <infr.h>
#include <ceditor.h>
#include <dskwin.h>
#include <dskhelp.h>

TStreamableClass RDskWinHelp( TDskWinHelp::name,
                                    TDskWinHelp::build,
                                    __DELTA(TDskWinHelp)
                                   );
