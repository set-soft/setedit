/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdio.h>
#include <stdlib.h>
#define Uses_string
#define Uses_unistd
class TRect;
#define Uses_TDialog
#define Uses_TPoint
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TScroller
#define Uses_TNSCollection
#define Uses_MsgBox

#define Uses_TSLabel
#define Uses_TSVeGroup
#define Uses_TSInputLine
#define Uses_TSButton

// First include creates the dependencies
#include <easydia1.h>
#include <tv.h>
// Second request the headers
#include <easydiag.h>
#include <dskwin.h>
#include <dskman.h>

#define Uses_TManWindow
#define Uses_TManPageView
#define Uses_TEnhancedText
#include <manview.h>

#define Uses_SETAppDialogs
#include <setapp.h>

#include <rhutils.h>

extern TView *setFocusTo;
extern Boolean focusChanged;

TStreamable *TDskWinMan::build()
{
 return new TDskWinMan(streamableInit);
}

void TDskWinMan::write(opstream& os)
{
 TManWindow *mw=(TManWindow *)view;
 os << mw << (int)(TProgram::deskTop->indexOf(mw));
}

void *TDskWinMan::read(ipstream& is)
{
 TManWindow *mw;
 is >> mw >> ZOrder;
 view=mw;

 return this;
}

char *TDskWinMan::GetText(char *dest, short maxLen)
{
 char *s=_("   Man page: ");
 if (strlen(s)<(size_t)maxLen)
    strcpy(dest,s);
 else
    *dest=0;
 TManWindow *mw=(TManWindow *)view;
 TDskWin::GetText(dest,(char *)mw->getFileName(),maxLen);
 return dest;
}

TDskWinMan::TDskWinMan(const char *file, const char *sections, const char *extraOps)
{
 type=dktMan;
 CanBeSaved=1;
 CanBeDeletedFromDisk=0;
 view=CreateManWindow(file,sections,extraOps);
}

TDskWinMan::~TDskWinMan()
{
}

#if defined(SEOS_DOS) || defined(SEOS_Win32)
/*
  This routine checks if man is there. If we can't find it we must put a warning
*/
#ifdef __TURBOC__
//$todo: Check it better (to SAA)
#define RET_OK 0
#else
#define RET_OK 1
#endif

int CheckForMan(void)
{
 static int isManInstalled=0;

 if (!isManInstalled)
   {
    char *err=open_stdout();
    int ret=TV_System("man");
    close_stdout();
    unlink(err);

    if (ret==1)
       isManInstalled=RET_OK;
    else
       messageBox(_("You must install man to use it!"), mfError | mfOKButton);
   }

 return isManInstalled;
}
#else
int CheckForMan(void)
{
 return 1;
}
#endif

const int prgLen=80,sectLen=20,extraLen=80,visibleLen=60;

#pragma pack(1)
typedef struct
{
 char program[prgLen]   __attribute__((packed));
 char section[sectLen]  __attribute__((packed));
 char options[extraLen] __attribute__((packed));
} ManPageOptions;
#pragma pack()

ManPageOptions op={"setedit","",""};
extern char *strncpyZ(char *dest, char *orig, int size);

TDskWinMan *ManPageView(const char *name)
{
 if (!CheckForMan())
    return 0;

 if (name)
    strncpyZ(op.program,(char *)name,80);

 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),_("Man page to view")));

 TSVeGroup *options=
 MakeVeGroup(new TSLabel(_("~M~an page for ..."),new TSInputLine(prgLen,visibleLen)),
             new TSLabel(_("~S~ection"),new TSInputLine(sectLen,visibleLen)),
             new TSLabel(_("~E~xtra options"),new TSInputLine(extraLen,visibleLen)),
             0);

 col->insert(2,2,options);
 EasyInsertOKCancel(col);

 TDialog *d=col->doIt();
 delete col;
 d->options|=ofCentered;

 TDskWinMan *ret=0;
 if (execDialog(d,&op)==cmOK)
   {
    ret=new TDskWinMan(op.program,op.section,op.options);
   }
 return ret;
}

