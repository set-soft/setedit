/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TProgressBar
#define Uses_TDialog
#define Uses_TRect
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TNoStaticText
#include <easydia1.h>
#include <settvuti.h>
#include <easydiag.h>

static TDialog *d;
static TNoStaticText *c1;
static TNoStaticText *c2;
static TProgressBar *pbar;
static TView *prevFocus;

char ProgBar_DefaultChar='²';
char ProgBar_CurrentChar='²';

void ProgBar_Init(const char *title, int max, const char *comment1, const char *comment2)
{
 prevFocus=TProgram::deskTop->current;
 // Calculate the height
 int h=5;
 if (comment1)
    h++;
 if (comment2)
    h++;
 if (comment1 || comment2)
    h++;

 d=new TDialog(TRect(0,0,64,h),title);
 // Don't allow the user close it
 d->flags&=~wfClose;
 d->options|=ofCentered;

 pbar=new TProgressBar(TRect(2,2,62,3),max,ProgBar_CurrentChar);
 d->insert(pbar);

 if (comment1)
   {
    c1=new TNoStaticText(TRect(2,4,62,5),comment1);
    d->insert(c1);
   }

 if (comment2)
   {
    c2=new TNoStaticText(TRect(2,5,62,6),comment2);
    d->insert(c2);
   }

 TProgram::deskTop->insert(d);
}

void ProgBar_DeInit(void)
{
 TProgram::deskTop->remove(d);
 if (prevFocus)
    prevFocus->setState(sfActive,True);
}

void ProgBar_UpDate(int pos)
{
 pbar->update(pos);
}

void ProgBar_SetComments(const char *comment1, const char *comment2)
{
 if (comment1)
    c1->setText((char *)comment1);
 if (comment2)
    c2->setText((char *)comment2);
}
