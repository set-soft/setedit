/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  This module is used to translate commands to macros. This technique is
used by the menu. As the TVision menu classes can't trigger a macro this
routine allocates a range of values and translate the command into a macro.@p
  Needs: isAnEditor, SLPSearchMacro.

***************************************************************************/

#define Uses_TStringCollection
#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TCEditWindow
#define Uses_TCEditor
#include <ceditor.h>
#include <slpinter.h>
#define Uses_SETAppVarious
#include <setapp.h>

typedef struct
{
 unsigned command;
 char *macro;
} stCommToMacro;

const unsigned cmmBase=0x2600;

class TCommToMacroCol : public TStringCollection
{
public:
 TCommToMacroCol() : TStringCollection(5,4) {};
 virtual int compare(void *key1, void *key2);
 virtual void *keyOf(void *item) { return (void *)(long)(((stCommToMacro *)item)->command); };
 virtual void freeItem( void *item );
};

int TCommToMacroCol::compare(void *key1, void *key2)
{
 return (unsigned long)key1-(unsigned long)key2;
}

void TCommToMacroCol::freeItem(void *item)
{
 stCommToMacro *p=(stCommToMacro *)item;
 delete[] p->macro;
 delete p;
}

static unsigned LastUsed=cmmBase-1;
static TCommToMacroCol *List=0;

char *TranslateCommandToMacro(unsigned command)
{
 if (!List || !(command>=cmmBase && command<=LastUsed))
    return 0;

 ccIndex pos;
 if (!List->search((void *)(unsigned long)command,pos))
    return 0;

 return ((stCommToMacro *)(List->at(pos)))->macro;
}


int RegisterMacroCommand(char *name)
{
 if (!List)
    List=new TCommToMacroCol();
 if (!List)
    return -1;

 stCommToMacro *entry=new stCommToMacro;
 entry->command=++LastUsed;
 entry->macro=newStr(name);

 List->insert(entry);
 return (int)LastUsed;
}

/**[txh]********************************************************************

  Description:
  Unregisters all the macro commands. Just to free the allocated memory.
Called by menuload.cc.

***************************************************************************/

void UnRegisterMacroCommands(void)
{
 CLY_destroy(List);
 List=0;
}


int isAMacroInMenu(unsigned command)
{
 TCEditor *target;
 TView *p;

 char *s=TranslateCommandToMacro(command);
 if (!s)
    return 0;

 p=TApplication::deskTop->current;
 if (!p || !IsAnEditor(p))
   {
    target=TCEditor::clipboard;
    if (!target)
       return 0;
   }
 else
   target=((TCEditWindow *)p)->editor;

 SLPSearchMacro(target,s,False);
 return 1;
}

