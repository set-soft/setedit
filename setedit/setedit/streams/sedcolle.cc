/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TStreamableClass
#define Uses_TWindow
#define Uses_TListBox
#define Uses_TCEditWindow
#include <ceditor.h>
#include <dskwin.h>
#include <edcollec.h>

TStreamableClass REditorCollection( TEditorCollection::name,
                                    TEditorCollection::build,
                                    __DELTA(TEditorCollection)
                                   );

