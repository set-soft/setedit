/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// That's the first include because is used to configure the editor.
#include <ceditint.h>

#include <stdlib.h>
#include <stdio.h>

#define Uses_TDialog
#define Uses_TDeskTop
#define Uses_TProgram
#define Uses_TApplication
#define Uses_TCheckBoxes
#define Uses_TSItem
#define Uses_TStaticText
#define Uses_TCEditor_Internal

// EasyDiag requests
#define Uses_TSButton
#define Uses_TSStaticText
#define Uses_TSTextScroller

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#define Uses_SETAppVarious
#include <setapp.h>

#include <edspecs.h>
#include <advice.h>

static const char *cSlogan=__("A friendly text editor.");

ushort execDialog( TDialog *d, void *data )
{
 TView *p=TProgram::application->validView( d );
 if (p==0)
    return cmCancel;
 else
  {
   if (data!=0)
      p->setData(data);
   ushort result=TProgram::deskTop->execView(p);
   if (result!=cmCancel && data!=0)
      p->getData( data );
   TObject::CLY_destroy( p );
   return result;
  }
}

int AboutStartBox(void)
{
 if (EnvirGetBits("SET_VARIOUS1",svr1DontShowAbout))
    return 0;

 TDialog *d=new TDialog(TRect(0,0,54,12),__("About"));
 d->options|=ofCentered;

 TRect r(2,2,52,3);
 char b[54];
 TVIntl::snprintf(b,54,__("That's SET's Editor v%lX.%lX.%lX, (c) 1996-2003"),
                  TCEDITOR_VERSION>>16,(TCEDITOR_VERSION>>8) & 0xFF,
                  TCEDITOR_VERSION & 0xFF);
 d->insert(new TStaticText(r,b));
 r.move(0,1);
 d->insert(new TStaticText(r,__("by Salvador Eduardo Tropea")));
 r.move(0,2);
 d->insert(new TStaticText(r,cSlogan));
 r.move(0,2);
 d->insert(new TCheckBoxes32(r,new TSItem(__("Don't show it next time"),0)));
 r.a.x+=15;
 r.b.x-=15;
 r.a.y+=2;
 r.b.y+=3;
 d->insert(new TButton(r,__("~O~K"),cmOK,bfDefault));
 d->selectNext(False);

 uint32 op=0;
 execDialog(d,&op);
 if (op)
    EnvirSetBits("SET_VARIOUS1",svr1DontShowAbout);

 return 1;
}

#ifdef TVOS_UNIX
void ShowUserScreenDialog()
{
 GiveAdvice(gadvUserScreen);
}
#endif

void FullAboutBox(void)
{
 char b[54];
 TSStaticText *ant,*cur;

 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("About")));

 TSView::yDefSep=0;
 TVIntl::snprintf(b,54,__("SET's Editor v%lX.%lX.%lX, (c) 1996-2003"),
                  TCEDITOR_VERSION>>16,(TCEDITOR_VERSION>>8) & 0xFF,
                  TCEDITOR_VERSION & 0xFF);
 ant=new TSStaticText(b);
 col->insert(xTSCenter,2,ant);

 #define i(a)  cur=new TSStaticText(a); \
               col->insert(xTSCenter,yTSUnder,cur,0,ant); \
               ant=cur
 i(__("by Salvador Eduardo Tropea"));
 i(cSlogan);

 TSView::yDefSep=1;
 i(__("Made in Argentina"));
 #undef i
 #define i(a)  cur=new TSStaticText(a); \
               col->insert(2,yTSUnder,cur,0,ant); \
               ant=cur
 i(__("Thanks to:"));
 TSView::yDefSep=0;
 TNSCollection *text=new TNSCollection(12,5);
 #undef i
 #define i(a) text->insert((void *)TVIntl::getTextNew(a));
 i(__("The FSF and GNU people for such good tools"));
 i(__("DJ Delorie and collaborators for porting it to DOS"));
 i(__("Robert Hoehne for porting TVision and making RHIDE"));
 i(__("Vadim Belodorov for contributing the first port of TV to Win32"));
 i(__("Anatoli Soltan for porting to Win32 using BC++"));
 i(__("VA Linux and the people who maintain Source Forge"));
 i(__("The University of California, Berkeley and its contributors"));
 #ifdef HAVE_PCRE_LIB
 i(__("The University of Cambridge for the PCRE package"));
 #endif
 #ifdef HAVE_ALLEGRO
 i(__("Shawn Hargreaves for Allegro; used as the sound engine"));
 # ifdef SUP_MP3
 i(__("Tomislav Uzelac for AMP, MP3 engine and Ove Kaaven for adapting to Allegro"));
 # endif // SUP_MP3
 #endif // HAVE_ALLEGRO
 i(__("Jean-loup Gailly and Mark Adler for the zlib"));
 #ifdef HAVE_BZIP2
 i(__("Julian R. Seward for bzip2 routines included"));
 #endif
 i(__("Bjorn Reese for a lot of ideas about the stack debugger of UNIX"));
 i(__("All my friends that support my project (Laszlo, Marek, Ivan,"));
 i(__("Grzegorz, etc.)."));
 #undef i
 TSTextScroller *txt=new TSTextScroller(70,10,text,0,1,70);
 col->insert(2,yTSUnder,txt,0,ant);

 TSView::yDefSep=1;
 col->insert(xTSCenter,yTSUnder,new TSButton(__("O~K~"),cmOK,bfDefault),0,txt);

 col->exec(0);
 CLY_destroy(text);
 delete col;
}

