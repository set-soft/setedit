/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// That's the first include because is used to configure the editor.
#include <ceditint.h>

#define Uses_stdio
#define Uses_stdlib

#define Uses_TDialog
#define Uses_TDeskTop
#define Uses_TProgram
#define Uses_TApplication
#define Uses_TCheckBoxes
#define Uses_TSItem
#define Uses_TStaticText
#define Uses_TCEditor_Internal
#define Uses_TScreen

// EasyDiag requests
#define Uses_TSButton
#define Uses_TSStaticText
#define Uses_TSTextScroller
#define Uses_TSCheckBoxes
#define Uses_TSVeGroup

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#define Uses_SETAppVarious
#include <setapp.h>

#include <edspecs.h>
#include <advice.h>
#include <vername.h>

static const char *cSlogan=__("\x3 A friendly text editor.");
static const char *cFormatVersion=__("\x3Version: %lX.%lX.%lX   Revision: %d");
static const char *cFormatName=__("\x3That's SET's Editor \"%s\", (c) %s");
static const char *cFormatPlatform=__("\x3Platform: %s  Driver: %s");
static const char *cSET=__("\x3 by Salvador Eduardo Tropea");

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

const int bufWidth=80;

int AboutStartBox(void)
{
 if (EnvirGetBits("SET_VARIOUS1",svr1DontShowAbout))
    return 0;

 TSViewCol *col=new TSViewCol(__("About"));

 char v1[bufWidth],v2[bufWidth];
 TVIntl::snprintf(v1,bufWidth,cFormatName,VERSION_NAME,TCEDITOR_C_YEAR);
 TVIntl::snprintf(v2,bufWidth,cFormatVersion,
                  TCEDITOR_VERSION>>16,(TCEDITOR_VERSION>>8) & 0xFF,
                  TCEDITOR_VERSION & 0xFF,VERSION_REV);

 TSVeGroup *ant=
  MakeVeGroup(tsveMakeSameW,
              new TSStaticText(v1),
              new TSStaticText(v2),
              new TSStaticText(cSET),
              new TSStaticText(cSlogan),0);

 TSVeGroup *all=
  MakeVeGroup(1 | tsveMakeSameW,
              ant,
              new TSCheckBoxes(new TSItem(__("Don't show it next time"),0)),
              new TSButton(__("~O~K"),cmOK,bfDefault),0);

 col->insert(xTSCenter,yTSUpSep,all);

 TDialog *d=col->doItCenter();

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

// Platform ID
#ifdef SEOSf_STR
 #define PLAT_OS SEOS_STR "/" SEOSf_STR
#else
 #define PLAT_OS SEOS_STR
#endif
#ifdef SECompf_STR
 #define PLAT_Comp SEComp_STR "/" SECompf_STR
#else
 #define PLAT_Comp SEComp_STR
#endif
#define PLAT_STR PLAT_OS "," SECPU_STR "," PLAT_Comp

void FullAboutBox(void)
{
 TSViewCol *col=new TSViewCol(__("About"));

 char v1[bufWidth],v2[bufWidth],v3[bufWidth];
 TVIntl::snprintf(v1,bufWidth,cFormatName,VERSION_NAME,TCEDITOR_C_YEAR);
 TVIntl::snprintf(v2,bufWidth,cFormatVersion,
                  TCEDITOR_VERSION>>16,(TCEDITOR_VERSION>>8) & 0xFF,
                  TCEDITOR_VERSION & 0xFF,VERSION_REV);
 TVIntl::snprintf(v3,bufWidth,cFormatPlatform,PLAT_STR,
                  TScreen::getDriverShortName());

 TSVeGroup *ant=
  MakeVeGroup(tsveMakeSameW,
              new TSStaticText(v1),
              new TSStaticText(v2),
              new TSStaticText(v3),
              new TSStaticText(cSET),
              new TSStaticText(cSlogan),0);

 TNSCollection *text=new TNSCollection(12,5);
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
 i(__("Grzegorz, Andris, etc.)."));
 #undef i
 TSTextScroller *txt=new TSTextScroller(70,8,text,0,1,70);

 TSVeGroup *thanks=
  MakeVeGroup(tsveMakeSameW,
              new TSStaticText(__("Thanks to:")),
              txt,0);

 TSVeGroup *all=
  MakeVeGroup(1 | tsveMakeSameW,
              ant,
              new TSStaticText(__("\x3Made in Argentina")),
              thanks,
              new TSButton(__("~O~K"),cmOK,bfDefault),0);

 col->insert(xTSCenter,yTSUpSep,all);
 col->exec(0);
 CLY_destroy(text);
 delete col;
}

