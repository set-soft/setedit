/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Module: CodePage
  Comments:
  Most of the code was moved to TV.

***************************************************************************/

#define Uses_TVCodePage
#define Uses_TScreen
#define Uses_TApplication
#define Uses_TVFontCollection
#define Uses_TCEditor
#define Uses_TCEditor_Internal
#define Uses_TWindow
#include <ceditor.h>

#define Uses_TSetEditorApp
#include <setapp.h>
#include <mp3play.h>
#include <calendar.h>
#include <tprogdia.h>
#include <ceditint.h> // SUP_MP3
// From the screen saver
extern char *coFormaScreenSaverStars[];
extern char *cFormaScreenSaverStars[];

TVCodePageCallBack
         TSetEditorApp::oldCPCallBack=NULL;

void TSetEditorApp::cpCallBack(ushort *map)
{
 int i;

 #define C(cla,name) TVCodePage::RemapString((uchar *)cla::name,(uchar *)cla::o##name,map)
 #ifdef SUP_MP3
 C(MP3Player,butRew);
 C(MP3Player,butStop);
 C(MP3Player,butPlay);
 C(MP3Player,butPause);
 C(MP3Player,butFfw);
 #endif
 #undef C
 #define C(cla,name) cla::name=TVCodePage::RemapChar(cla::o##name,map)
 C(TCEditor,TabChar);
 C(TCalendarView,upArrowChar);
 C(TCalendarView,downArrowChar);
 #undef C
 #define C(num,o,n) for (i=0; i<num; i++) n[i][0]=TVCodePage::RemapChar(o[i][0],map)
 C(4,coFormaScreenSaverStars,cFormaScreenSaverStars);
 #undef C
 ProgBar_CurrentChar=TVCodePage::RemapChar(ProgBar_DefaultChar,map);

 // Update the "is_word_char" bit
 for (i=0; i<256; i++)
     if (TVCodePage::isAlNum(i))
        TableTypesEditor[i]|=ttedIsWordChar;
     else
        TableTypesEditor[i]&=~ttedIsWordChar;
 TableTypesEditor[(int) '_']|=ttedIsWordChar;
}


