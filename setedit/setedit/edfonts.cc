/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string
#define Uses_TVCodePage
#define Uses_TScreen
#define Uses_TApplication
#define Uses_TVFontCollection
#include <tv.h>

#define Uses_TSetEditorApp
#include <setapp.h>

void TSetEditorApp::SetEditorFontsEncoding(int priChanged, int enPri,
                                           int sndChanged, int enSec)
{
 if (!so) return; // Sanity check
 int prC=0, seC=0;
 TScreenFont256 primary,secondary,*prF=NULL,*seF=NULL;

 if (priChanged && so->foPriLoaded && so->foPri)
   {
    so->foPri->SetCodePage(enPri);
    primary.w=so->foPriW;
    primary.h=so->foPriH;
    primary.data=so->foPri->GetFont(so->foPriW,so->foPriH);
    prF=&primary;
    prC=1;
   }
 if (sndChanged && so->foSecLoaded && so->foSec)
   {
    so->foSec->SetCodePage(enSec);
    secondary.w=so->foPriW;
    secondary.h=so->foPriH;
    secondary.data=so->foSec->GetFont(so->foPriW,so->foPriH);
    seF=&secondary;
    seC=1;
   }
 if (prC || seC)
   {
    TScreen::setFont(prC,prF,seC,seF);
    if (prF) DeleteArray(prF->data);
    if (seF) DeleteArray(seF->data);
   }
}


TScreenFont256 *TSetEditorApp::FontRequestCallBack(int which, unsigned w, unsigned h)
{
 if (!so) return NULL; // Sanity check
 TVFontCollection *col=which ? so->foSec : so->foPri;
 if (!col)
    return NULL;
 uchar *data=col->GetFont(w,h);
 if (!data)
    return NULL;
 TScreenFont256 *f=new TScreenFont256;
 so->foPriW=w;
 so->foPriH=h;
 f->data=data;
 f->w=w;
 f->h=h;
 fontCreated=1;
 if (which)
    so->foSecLoaded=1;
 else
    so->foPriLoaded=1;
 return f;
}

void TSetEditorApp::SetEditorFonts(uchar priUse, char *priName, char *priFile,
                                   TVBitmapFontSize *priSize,
                                   uchar secUse, char *secName, char *secFile)
{
 if (!so) return; // Sanity check
 // Transfer the options
 unsigned w,h;
 w=priSize->w; h=priSize->h;
 int sizeChanged=(w!=so->foPriW) || (h!=so->foPriH);
 if (sizeChanged)
   {
    so->foPriW=w;
    so->foPriH=h;
   }

 int priNameChanged=!so->foPriName || strcmp(so->foPriName,priName)!=0;
 if (priNameChanged)
   {
    DeleteArray(so->foPriName);
    so->foPriName=newStr(priName);
    DeleteArray(so->foPriFile);
    so->foPriFile=newStr(priFile);
   }
 else
   {
    DeleteArray(priName);
    DeleteArray(priFile);
   }

 int priUseChanged=priUse!=so->foPriLoad;
 so->foPriLoad=priUse;

 int secNameChanged=!so->foSecName || strcmp(so->foSecName,secName)!=0;
 if (secNameChanged)
   {
    DeleteArray(so->foSecName);
    so->foSecName=newStr(secName);
    DeleteArray(so->foSecFile);
    so->foSecFile=newStr(secFile);
   }
 else
   {
    DeleteArray(secName);
    DeleteArray(secFile);
   }

 int secUseChanged=secUse!=so->foSecLoad;
 so->foSecLoad=secUse;

 // Make it efective
 int idDefScr, idDefApp, idDefInp;
 TVCodePage::GetDefaultCodePages(idDefScr,idDefApp,idDefInp);
 if (!TScreen::canSetBFont()) return;

 TScreenFont256 primary,secondary,*prF=NULL,*seF=NULL;
 int secChanged=0;
 int priChanged=priNameChanged || priUseChanged || sizeChanged;
 if (priNameChanged)
   {// Create a collection for this font
    if (so->enScr==-1) so->enScr=idDefScr;
    if (so->foPri) delete so->foPri;
    so->foPri=new TVFontCollection(so->foPriFile,so->enScr);
    if (so->foPri->GetError())
      {
       if (so->foPri) delete so->foPri;
       so->foPri=NULL;
      }
   }
 if (priChanged && so->foPriLoad && so->foPri)
   {
    primary.w=so->foPriW;
    primary.h=so->foPriH;
    primary.data=so->foPri->GetFont(so->foPriW,so->foPriH);
    prF=&primary;
   }

 if (TScreen::canSetSBFont())
   {
    secChanged=secNameChanged || secUseChanged || sizeChanged;
    if (secNameChanged)
      {// Create a collection for this font
       if (so->enSnd==-1) so->enSnd=idDefScr;
       if (so->foSec) delete so->foSec;
       so->foSec=new TVFontCollection(so->foSecFile,so->enSnd);
       if (so->foSec->GetError())
         {
          delete so->foSec;
          so->foSec=NULL;
         }
      }
    if (secChanged && so->foSecLoad && so->foSec)
      {
       secondary.w=so->foPriW;
       secondary.h=so->foPriH;
       secondary.data=so->foSec->GetFont(so->foPriW,so->foPriH);
       seF=&secondary;
      }
   }
 if (TScreen::setFont(priChanged,prF,secChanged,seF,so->enScr))
   {
    if (priChanged)
       so->foPriLoaded=prF!=NULL;
    if (secChanged)
       so->foSecLoaded=seF!=NULL;
    if (!so->foCallBackSet)
      {
       TScreen::setFontRequestCallBack(FontRequestCallBack);
       so->foCallBackSet=1;
      }
   }
 if (prF) DeleteArray(prF->data);
 if (seF) DeleteArray(seF->data);
}


