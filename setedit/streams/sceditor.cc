/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */

/*------------------------------------------------------------*/
/* filename -       sceditor.cc                               */
/*                                                            */
/* Registeration objects for the following classes:           */
/*                      TCEditor                              */
/*                      TCFileEditor                          */
/*                      TCEditWindow                          */
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
/*                                                            */
/*  Needed to write the objects to a stream.                  */
/*                                                            */
/*------------------------------------------------------------*/

// That's the first include because is used to configure the editor.
#include "ceditint.h"

#define Uses_TStringCollection
#define Uses_TView
#define Uses_TWindow
#define Uses_TStreamableClass

#define Uses_TCEditor
#define Uses_TCFileEditor
#define Uses_TCEditWindow
#include "ceditor.h"

__link( RView )
__link( RWindow )
__link( RScrollBar )
__link( RSIndicator )
// TIndicator is neede to read old desktop files. The patch from Robert to change
// it by TSIndicator was incomplete and was a source of a bug in 0.4.23 release.
__link( RIndicator )

TStreamableClass RCEditor( TCEditor::name,
                           TCEditor::build,
                           __DELTA(TCEditor)
                        );

TStreamableClass RCEditWindow( TCEditWindow::name,
                               TCEditWindow::build,
                                __DELTA(TCEditWindow)
                              );
