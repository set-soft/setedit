/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TCEditWindow
#define Uses_TWindow

#define Uses_TStreamableClass
#define Uses_TStreamable
#include <ceditor.h>
#include <dskwin.h>
#include <dskprj.h>

TStreamableClass RDskWinMessage( TDskWinPrj::name,
                                    TDskWinPrj::build,
                                    __DELTA(TDskWinPrj)
                                   );
