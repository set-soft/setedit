/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string
#define Uses_TProgress
#define Uses_TDialog
#define Uses_TRect
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TNoStaticText
#include <settvuti.h>

static TDialog *d;
static TProgress *pbar;
static TView *prevFocus;

void Progress_Init(const char *title, const char *comment)
{
 prevFocus=TProgram::deskTop->current;
 int l=strlen(_(comment))+2;

 d=new TDialog(TRect(0,0,l+6,5),title);
 // Don't allow the user close it
 d->flags&=~wfClose;
 d->options|=ofCentered;

 pbar=new TProgress(TRect(3,2,l+3,3),comment);
 d->insert(pbar);

 TProgram::deskTop->insert(d);
}

void Progress_DeInit(void)
{
 TProgram::deskTop->remove(d);
 if (prevFocus)
    prevFocus->setState(sfActive,True);
}

void Progress_UpDate()
{
 pbar->update();
}


