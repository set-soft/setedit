/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
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
#include <ceditor.h>
#define Uses_SETAppConst
#include <setapp.h>

static TPalette *temp_pal=0;

static void addItem(TColorGroup *&group,int index, const char *name, const char *group_name)
{
  if (strcmp(name,"reserved")==0)
    return;
  if (*group_name)
  {
    if (!group)
      group = new TColorGroup(_(group_name));
    else
      *group = *group + *new TColorGroup(_(group_name));
  }
  *group = *group + *new TColorItem(_(name), index);
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

 // That creates the color items dynamically
 SE_cpColor

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
    destroy(c);
   }
 delete temp_pal;
 temp_pal=0;
}



