/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TCEditWindow
#define Uses_TWindow

#define Uses_TStreamableClass
#define Uses_TStreamable
#include <ceditor.h>
#include <dskwin.h>
#include <dskhelp.h>

TStreamableClass RDskWinHelp( TDskWinHelp::name,
                                    TDskWinHelp::build,
                                    __DELTA(TDskWinHelp)
                                   );