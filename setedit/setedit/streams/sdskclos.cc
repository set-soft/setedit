/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TCEditWindow
#define Uses_TWindow

#define Uses_TStreamableClass
#define Uses_TStreamable
#define Uses_string
#include <ceditor.h>
#include <dskwin.h>
#include <dskclose.h>

TStreamableClass RDskWinClosed( TDskWinClosed::name,
                                    TDskWinClosed::build,
                                    __DELTA(TDskWinClosed)
                                   );

