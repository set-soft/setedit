/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string
#define Uses_TRect
#define Uses_TColorDialog
#define Uses_TColorGroup
#define Uses_TColorItem
#define Uses_TPalette
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TCEditWindow
#define Uses_TStringable

#define Uses_TSStringableListBox
#define Uses_TSButton

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>
#define Uses_SETAppConst
#include <setapp.h>
#include <diaghelp.h>

static TPalette *temp_pal=0;
// Used to export a palette as text suitable for an include file
//#define EXPORT_PAL
#ifdef EXPORT_PAL
static FILE *f;
static unsigned char *palData;
static const char *nameColors[]=
{
 "Black",
 "Blue",
 "Green",
 "Cyan",
 "Red",
 "Magenta",
 "Brown",
 "Lightgray",
 "Darkgray",
 "Lightblue",
 "Lightgreen",
 "Lightcyan",
 "Lightred",
 "Lightmagenta",
 "Yellow",
 "White"
};
#endif

static void addItem(TColorGroup *&group,int index, const char *name, const char *group_name)
{
 if (strcmp(name,"reserved")==0)
   {
    #ifdef EXPORT_PAL
    fprintf(f,"\\\n  S(%02X,%s,%s,%s) \\\n",index,nameColors[palData[index]&0xF],nameColors[palData[index]>>4],name);
    #endif
    return;
   }
 if (*group_name)
   {
    if (!group)
       group=new TColorGroup(group_name);
    else
       *group=*group + *new TColorGroup(group_name);
    #ifdef EXPORT_PAL
    fprintf(f,"\\\n  SS(%02X,%s,%s,%s,%s) \\\n",index,nameColors[palData[index]&0xF],nameColors[palData[index]>>4],name,group_name);
    #endif
   }
 #ifdef EXPORT_PAL
 else
    fprintf(f,"  S(%02X,%s,%s,%s) \\\n",index,nameColors[palData[index]&0xF],nameColors[palData[index]>>4],name);
 #endif
 *group=*group + *new TColorItem(name, index);
}

#include <pal.h>
#undef S
#undef SS
#define S(index,foreground,background,name) \
  addItem(group,0x##index,#name,"");
#define SS(index,foreground,background,name,_group) \
  addItem(group,0x##index,#name,#_group);

static TColorDialog *GetColorDialog()
{
 TColorDialog *c;
 TColorGroup *group = NULL;

 #ifdef EXPORT_PAL
 TPalette &palette=TProgram::application->getPalette();
 palData=palette.data;
 f=fopen("Exportada.Pal","wt");
 // That creates the color items dynamically
 #endif
 SE_cpColor
 #ifdef EXPORT_PAL
 fclose(f);
 #endif

 c=new TColorDialog(&TProgram::application->getPalette(),group);
 c->helpCtx=cmeSetColors;
 temp_pal=new TPalette(TProgram::application->getPalette());
 c->setData(&TProgram::application->getPalette());
 return c;
}

void Colors()
{
 TColorDialog *c = GetColorDialog();
 if (TProgram::application->validView(c)!=0)
   {
    if (TProgram::deskTop->execView( c )==cmCancel)
       // restore the old palette
       TProgram::application->getPalette()=*temp_pal;
    // force to reread the cached colors for the editor
    TCEditor::colorsCached = 0;
  
    TProgram::application->Redraw();
    CLY_destroy(c);
   }
 delete temp_pal;
 temp_pal=0;
}

extern char SEcpColor[]; // Defined in editmain
#undef S
#undef SS
#include <pal.h>
#include <palfte.h>
#include <palconv.h>
#include <paldjd.h>
#include <palbcc.h>
#include <palmc.h>

static char SEcpFTE[]={ SE_cpColorFTE 0 };
static char SEcpDARKJDI[]={ SE_cpColorDarkJDI 0 };
static char SEcpConsoleVIM[]={ SE_cpColorConsoleVIM 0 };
static char SEcpColorBCC[]={ SE_cpColorBCC 0 };
static char SEcpMidnight[]={ SE_cpMidnight 0 };

typedef struct
{
 char name[20];
 char *palette;
} palTheme;

static palTheme Themes[]=
{
 {"Default",            SEcpColor      },
 {"FTE like",           SEcpFTE        },
 {"Console + VIM",      SEcpConsoleVIM },
 {"Dark JDI",           SEcpDARKJDI    },
 {"Classic Borland",    SEcpColorBCC   },
 {"Midnight Commander", SEcpMidnight   },
};

class TColorThemes : public TStringable
{
public:
 TColorThemes() : TStringable() { Count=sizeof(Themes)/sizeof(palTheme); };
 virtual ~TColorThemes() {};

 virtual void getText(char *dest, unsigned item, int maxLen);
};

void TColorThemes::getText(char *dest, unsigned item, int maxLen)
{
 const char *ori=Themes[item].name;
 strncpy(dest,ori,maxLen);
 dest[maxLen]=EOS;
}

static TColorThemes cThemes;

void ColorTheme()
{
 TSViewCol *col=new TSViewCol(__("Color Themes"));

 col->insert(xTSCenter,yTSUp,new TSStringableListBox(20,GetDeskTopRows()-9,tsslbVertical));
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmeColorTheme);
 delete col;

 TStringableListBoxRec box;
 box.items=&cThemes;
 box.selection=0;

 if (execDialog(d,&box)==cmOK)
   {
    temp_pal=new TPalette(TProgram::application->getPalette());
    memcpy(temp_pal->data+1,Themes[box.selection].palette,temp_pal->data[0]);
    TProgram::application->getPalette()=*temp_pal;
    // force to reread the cached colors for the editor
    TCEditor::colorsCached=0;
    TProgram::application->Redraw();
   }
}

