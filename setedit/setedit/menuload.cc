/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

 Module: Menu Loader
 Comments:
 This module is used to load the editor's menu from a text file.

***************************************************************************/
#include <ceditint.h>
#include <stdio.h>
#define Uses_stdlib
#define Uses_string
#define Uses_ctype
#define Uses_TMenu
#define Uses_TMenuItem
#define Uses_TSubMenu
#define Uses_TMenuBar
#define Uses_TKeys
#define Uses_TRect
#define Uses_MsgBox
#define Uses_TCEditor
#define Uses_TCEditor_Commands
#define Uses_TStringCollection
#define Uses_TStatusLine
#define Uses_TStatusItem
#define Uses_TStatusDef
#define Uses_TVCodePage
#define Uses_TScreen
#define Uses_AllocLocal
#define Uses_snprintf
#include <ceditor.h>
#define Uses_SETAppConst
#define Uses_SETAppVarious
#define Uses_TMultiMenu
#include <setapp.h>

#include <keytrans.h>
#define Uses_TMLIEditorDefs
#include <mli.h>

const int maxLineLen=255;
const int errMLExpectedStr=1,
          errMLUnclosedStr=2,
          errMLNoKeyInSubMenu=3,
          errMLWrongKeyName=4,
          errMLExtraCharsInLine=5,
          errMLNoNumberForContext=6,
          errMLSyntax=7,
          errMLEmptySubMenu=8,
          errMLUnclosedSubMenu=9,
          errMLNoCommandInMenuItem=10,
          errMLWrongCommandName=11,
          errMLNoMenuDefinition=12,
          errMLWrongCtxName=13,
          errMLUnfinishedMacro=14,
          errMLCantRegisterMacro=15,
          errMLNoStatusDefinition=16,
          errMLNoFromInRange=17,
          errMLNoToInRange=18,
          errMLNoNumber=19,
          errMLFromLessTo=20,
          errMLUnclosedStatusRange=21,
          errMLEmptyStatusLine=22,
          errMLNoHelpContext=23,
          errMLTooDeep=24,
          errMLUnmatchedEndIf=25,
          errMLMacroRedefined=26;

static const char *ErrorNames[] =
{
 __("No error"),
 __("String expected"),
 __("String not closed"),
 __("No key for SubMenu"),
 __("Wrong key name"),
 __("Extra characters in line"),
 __("No number for a context"),
 __("Syntax error"),
 __("Empty submenu"),
 __("SubMenu not closed "),
 __("Missing command value"),
 __("Wrong command name"),
 __("No menu definition"),
 __("Wrong context name"),
 __("Unfinished macro name"),
 __("Unable to register macro"),
 __("No status line definition"),
 __("No `from' value in range"),
 __("No `quantity' value in range"),
 __("Number expected"),
 __("Invalid range"),
 __("StatusRange not closed"),
 __("Empty status line"),
 __("Wrong help context name or number"),
 __("Too much conditionals nested"),
 __("Unmatched $endif"),
 __("Preprocessor value redefined")
};

static int Error=0;
static int Line;
static char *FileName=0;
const int maxDepth=32;

typedef struct
{
 char start;
 char escape;
 char *sIf,*sElse,*sEnd;
 char *sDefined,*sIfDef,*sIfNDef;
 char *sAnd,*sOr,sNot;
 char *sDefine, *sUndef;
 int  depth;
 char stack[maxDepth];
} stPreproInfo;

static stPreproInfo PreproInfo={'$',0,"if","else","endif","defined","ifdef","ifndef",
                                "and","or",'!',"define","undef"};
static DynStrCatStruct res={0,NULL};

const unsigned prliASCIIZ=1,prliNoASCIIZ=0;
const unsigned prliEatSpaces=2,prliNoEatSpaces=0;
const int prlisNoPL=0,prlisPLContinue=1,prlisPLOK=2;
const int prlieNoErr=0,prlieSyntax=-1,prlieElse=2,prlieEnd=3;
const int prlieNoOp=4,prlieAnd=5,prlieOr=6,prlieDefine=7,prlieUndef=8;

class TDef;

static int ReadPreprocessor(char *s, stPreproInfo *p, DynStrCatStruct *str,
                            int PreproValue, TDef *defs, char *buf, FILE *f);
static char *ExpandPreprocessor(char *buf, stPreproInfo *p, TDef *defs);

#define GetLine() { fgets(buf,maxLineLen,f); Line++; }

struct stDef
{
 char *var;
 char *val;
};

class TDef : public TStringCollection
{
public:
 TDef() : TStringCollection(20,5) {};

 stDef *At(ccIndex i) { return (stDef *)at(i); };
 virtual void *keyOf(void *item);
 virtual void freeItem(void *s);
 void insert(const char *var, const char *val=NULL);
 Boolean isDefined(char *s, int l);
 stDef  *getMacro(char *s, int l);
 Boolean undef(char *s, int l);
};

Boolean TDef::isDefined(char *s, int l)
{
 char val=s[l];
 s[l]=0;
 ccIndex pos;
 Boolean ret=search(s,pos);
 s[l]=val;

 return ret;
}

stDef *TDef::getMacro(char *s, int l)
{
 char val=s[l];
 s[l]=0;
 ccIndex pos;
 Boolean ret=search(s,pos);
 s[l]=val;

 return ret ? At(pos) : NULL;
}

Boolean TDef::undef(char *s, int l)
{
 char val=s[l];
 s[l]=0;
 ccIndex pos;
 Boolean ret=search(s,pos);
 s[l]=val;
 if (ret)
    atFree(pos);

 return ret;
}

void *TDef::keyOf(void *item)
{
 return ((stDef *)item)->var;
}

void TDef::freeItem(void *s)
{
 stDef *p=(stDef *)s;
 delete[] p->var;
 delete[] p->val;
 delete p;
}

void TDef::insert(const char *var, const char *val)
{
 stDef *p=new stDef;
 p->var=newStr(var);
 p->val=newStr(val);
 TStringCollection::insert(p);
}

char *SkipBlanks(char *buf)
{
 char *s;
 for (s=buf; *s!='\n' && *s && ucisspace(*s); s++);
 return s;
}

static
char *SkipToEndNoBlanks(char *buf)
{
 char *s, *last;
 for (last=s=buf; *s!='\n' && *s; s++)
     if (!ucisspace(*s))
        last=s;
 if (!ucisspace(*last))
    last++;
 return last;
}

static
char *GetString(char *s)
{
 if (*s!='\"')
   {
    Error=errMLExpectedStr;
    return 0;
   }
 for (s++; *s && *s!='\n' && *s!='\"'; s++);
 if (*s!='\"')
   {
    Error=errMLUnclosedStr;
    return 0;
   }
 *s=0;
 return s;
}

static
char *GetNumber(char *s, int &key, int error1, int error2, int Optional=0,
                int FirstInLine=0)
{
 char *ret;

 s=SkipBlanks(s);
 if (Optional && (*s=='\n' || *s==0))
    return s;
 if (!FirstInLine)
   {
    if (*s!=',')
      {
       Error=error1;
       return 0;
      }
    s=SkipBlanks(s+1);
   }
 if (!ucisdigit(*s))
   {
    Error=error2;
    return 0;
   }
 key=strtol(s,&ret,0);
 return ret;
}

static
char *GetKey(char *s, int &key, int error1, int Optional=0)
{
 char *ret;

 s=SkipBlanks(s);
 if (Optional && (*s=='\n' || *s==0))
    return s;
 if (*s!=',')
   {
    Error=error1;
    return 0;
   }
 s=SkipBlanks(s+1);

 if (*s!='k' || *(s+1)!='b')
   {
    Error=error1;
    return 0;
   }
 s+=2;
 for (ret=s; *ret && (TVCodePage::isAlNum(*ret) || *ret=='_'); ret++);
 char v=*ret; *ret=0;
 ushort code;
 if (InterpretKeyName(s,code))
   {
    *ret=v;
    Error=errMLWrongKeyName;
    return 0;
   }
 *ret=v;
 key=code;
 return ret;
}

static
char *GetCommand(char *s, int &command, int FirstInLine=0)
{
 char *ret;
 int isAMacro=0;

 s=SkipBlanks(s);
 if (!FirstInLine)
   {
    if (*s!=',')
      {
       Error=errMLNoCommandInMenuItem;
       return 0;
      }
    s=SkipBlanks(s+1);
   }

 if (*s!='c' || *(s+1)!='m')
   {
    Error=errMLNoCommandInMenuItem;
    return 0;
   }
 s+=2;
 /*if (*s!='c' && *s!='e' && *s!='(')
   {
    Error=errMLWrongCommandName;
    return 0;
   }*/
 isAMacro= *s=='(';
 s++;

 if (isAMacro)
   {
    ret=s;
    if (*ret=='(')
      {// cm((lisp code))
       char *end;
       if (MLIEdIsolateCode(ret,end)!=SLP_OK)
         {
          Error=errMLUnfinishedMacro;
          return 0;
         }
       ret=end+1;
      }
    else
       // cm(macro name)
       for (; *ret && *ret!=')'; ret++);
    if (*ret!=')')
      {
       Error=errMLUnfinishedMacro;
       return 0;
      }
    char v=*ret; *ret=0;
    command=RegisterMacroCommand(s);
    //printf("Registrando %s\n",s);
    *ret=v;
    ret++; // Skip the last parethesis
    if (command==-1)
      {
       Error=errMLCantRegisterMacro;
       return 0;
      }
   }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
 else
   {
    for (ret=s; *ret && (TVCodePage::isAlNum(*ret) || *ret=='_'); ret++);
    char v=*ret; *ret=0;
    if (*(s-1)=='c')
      {
       command=SearchEdCommand(s);
       if (command>=0)
          command+=cmbBaseNumber;
      }
    else
    if (*(s-1)=='e')
      {
       command=SearchEditCommand(s);
       if (command>=0)
          command+=cmeBase;
      }
    else
       command=SearchSimpleCommand(s-1);
      
    *ret=v;
    if (command==-1)
      {
       Error=errMLWrongCommandName;
       return 0;
      }
   }
 return ret;
}

static
char *GetHelpCtx(char *s, int &context, int FirstInLine=0)
{
 char *ret;

 s=SkipBlanks(s);
 if (!FirstInLine)
   {
    if (*s!=',')
      {
       Error=errMLNoCommandInMenuItem;
       return 0;
      }
    s=SkipBlanks(s+1);
   }

 if (*s!='h' || *(s+1)!='c')
    return GetNumber(s,context,0,errMLNoHelpContext,0,1);
 s+=2;

 for (ret=s; *ret && (TVCodePage::isAlNum(*ret) || *ret=='_'); ret++);
 char v=*ret; *ret=0;

 context=SearchHelpCtxCommand(s);
 if (context<0)
   { // Try with an editor command, they are mapped one to one with the
     // corresponding help context
    context=SearchEdCommand(s);
    if (context>=0)
       context+=cmbBaseNumber;
    else
      {
       context=SearchEditCommand(s);
       if (context>=0)
          context+=cmeBase;
      }
   }
 *ret=v;
 if (context<0)
   {
    Error=errMLNoHelpContext;
    return 0;
   }
 return ret;
}

// MenuItem: "Name", Command[, Key [, Context [,"KeyName"]]]
// MenuItemC: "Name", Command[, Key [,"KeyName"]] -> copyContext
static
TMenuItem *GetMenuItem(char *s,int copyContext)
{
 char *sEnd,*name,*extra=0;
 int key=kbNoKey,context=hcNoContext,command;

 // Now we'll parse the MenuItem definition.

 // Get the name
 s=SkipBlanks(s);
 sEnd=GetString(s);
 if (Error) return 0;
 name=s+1;

 // Get the command
 //s=GetNumber(sEnd+1,command,errMLNoCommandInMenuItem,errMLNoNumberForCommand);
 s=GetCommand(sEnd+1,command);
 if (Error) return 0;
 // Get the key
 s=GetKey(s,key,errMLExtraCharsInLine,1);
 if (Error) return 0;
 // Get the context
 if (copyContext)
    context=command;
 else
   {
    s=GetNumber(s,context,errMLExtraCharsInLine,errMLWrongCtxName,1);
    if (Error) return 0;
   }
 // Get the extra string
 s=SkipBlanks(s);
 if (*s!='\n' && *s!=0)
   {
    if (*s!=',')
      {
       Error=errMLExtraCharsInLine;
       return 0;
      }
    s=SkipBlanks(s+1);
    sEnd=GetString(s);
    if (Error) return 0;
    extra=s+1;
   }

 return new TMenuItem(name,command,key,context,extra);
}

static
TSubMenu *GetSubMenu(FILE *f, char *buf, char *s, stPreproInfo *PreproInfo,
                     DynStrCatStruct *Cat, TDef *defs, int &PreproValue)
{
 char *sEnd,*name;
 int key,context=hcNoContext;

 s=SkipBlanks(s);

 // Now we'll parse the SubMenu definition.
 // SubMenu: "Name", Key [,Context]

 // Get the name
 sEnd=GetString(s);
 if (Error) return 0;
 name=s+1;
 // Get the key
 s=GetKey(sEnd+1,key,errMLNoKeyInSubMenu);
 if (Error) return 0;
 // Get context
 s=GetNumber(s,context,errMLExtraCharsInLine,errMLNoNumberForContext,1);
 if (Error) return 0;

 // We had luck ;-)
 TSubMenu *sub=new TSubMenu(name,key,context);
 TMenuItem *firstMenu=0;
 TMenuItem *lastMenu=0;
 TMenuItem *newMenu;

 // Now we'll get the items:
 GetLine();
 while (!feof(f) && !Error)
   {
    s=SkipBlanks(buf);
    if (*s=='$')
      {
       PreproValue=ReadPreprocessor(s,PreproInfo,Cat,PreproValue,defs,buf,f);
      }
    else
    if (*s!='#' && *s!='\n' && PreproValue) // Skip comment lines
      {
       newMenu=0;
       s=ExpandPreprocessor(s,PreproInfo,defs);
       if (strncasecmp(s,"SubMenu:",8)==0)
         { // Recursive menues
          newMenu=(TMenuItem *)GetSubMenu(f,buf,s+8,PreproInfo,Cat,defs,PreproValue);
          if (!newMenu)
             return 0; // Fail
         }
       else
       if (strncasecmp(s,"MenuItem:",9)==0)
         { // Just an item
          newMenu=GetMenuItem(s+9,0);
          if (!newMenu)
             return 0; // Fail
         }
       else
       if (strncasecmp(s,"MenuItemC:",10)==0)
         { // Just an item
          newMenu=GetMenuItem(s+10,1);
          if (!newMenu)
             return 0; // Fail
         }
       else
       if (strncasecmp(s,"MenuSeparator",13)==0)
         { // A separator
          newMenu=&newLine();
         }
       else
       if (strncasecmp(s,"EndSubMenu",10)==0)
         { // The end of the submenu
          if (firstMenu)
            {
             sub->subMenu=new TMenu(*firstMenu);
             return sub;
            }
          else
            {
             Error=errMLEmptySubMenu;
             return 0;
            }
         }
       else
         {
          Error=errMLSyntax;
          return 0; // Fail
         }
       // Link it
       if (newMenu)
         {
          if (firstMenu)
            {
             lastMenu->next=newMenu;
             lastMenu=newMenu;
            }
          else
             lastMenu=firstMenu=newMenu;
         }
      }
    if (!Error)
       GetLine();
   }
 if (!Error)
    Error=errMLUnclosedSubMenu;
 return 0;
}

// StatusEntry: "Name", Command, Key
// StatusHiddenEntry: Command, Key -> hidden
static
TStatusItem *GetStatusItem(char *s, int hidden)
{
 char *name=0,*sEnd;
 int command,key;

 // Now we'll parse the StatusEntry definition.

 s=SkipBlanks(s);
 sEnd=s;
 // Get the name
 if (!hidden)
   {
    sEnd=GetString(s)+1;
    if (Error) return 0;
    name=s+1;
   }

 // Get the command
 s=GetCommand(sEnd,command,hidden);
 if (Error) return 0;
 // Get the key
 s=GetKey(s,key,errMLExtraCharsInLine);
 if (Error) return 0;

 return new TStatusItem(name ? name : 0,key,command);
}


static
TStatusDef *GetStatusDef(FILE *f, char *buf, char *s, stPreproInfo *PreproInfo,
                         DynStrCatStruct *Cat, TDef *defs,
                         int &PreproValue)
{
 int from,quantity,to;

 s=SkipBlanks(s);

 // Now we'll parse the StatusRange definition.
 // StatusRange: From, Quantity

 // Get the from
 s=GetHelpCtx(s,from,1);
 if (Error) return 0;
 // Get the quantity
 s=GetNumber(s,quantity,errMLNoToInRange,errMLNoNumber);
 if (Error) return 0;
 to=from+quantity;
 if (from>to)
   {
    Error=errMLFromLessTo;
    return 0;
   }

 TStatusDef *def=new TStatusDef(from,to);
 TStatusItem *firstItem=0;
 TStatusItem *lastItem=0;
 TStatusItem *newItem;

 // Now we'll get the items:
 GetLine();
 while (!feof(f) && !Error)
   {
    s=SkipBlanks(buf);
    if (*s=='$')
      {
       PreproValue=ReadPreprocessor(s,PreproInfo,Cat,PreproValue,defs,buf,f);
      }
    else
    if (*s!='#' && *s!='\n' && PreproValue) // Skip comment lines
      {
       s=ExpandPreprocessor(s,PreproInfo,defs);
       newItem=0;
       if (strncasecmp(s,"StatusEntry:",12)==0)
         {
          newItem=GetStatusItem(s+12,0);
          if (!newItem)
             return 0; // Fail
         }
       else
       if (strncasecmp(s,"StatusHiddenEntry:",18)==0)
         {
          newItem=GetStatusItem(s+18,1);
          if (!newItem)
             return 0; // Fail
         }
       else
       if (strncasecmp(s,"EndStatusRange",14)==0)
         {
          if (firstItem)
            {
             def->items=firstItem;
             return def;
            }
          else
            {
             Error=errMLEmptyStatusLine;
             return 0;
            }
         }
       else
         {
          Error=errMLSyntax;
          return 0; // Fail
         }
       // Link it
       if (newItem)
         {
          if (firstItem)
            {
             lastItem->next=newItem;
             lastItem=newItem;
            }
          else
             lastItem=firstItem=newItem;
         }
      }
    if (!Error)
       GetLine();
   }
 Error=errMLUnclosedStatusRange;
 return 0;
}

static
void GetNewMenuID(FILE *f, char *buf, char *s, unsigned &min, unsigned &max)
{
 int from,quantity,to;

 // Same as StatusRange
 s=SkipBlanks(s);
 // Get the from
 s=GetHelpCtx(s,from,1);
 if (Error) return;
 // Get the quantity
 s=GetNumber(s,quantity,errMLNoToInRange,errMLNoNumber);
 if (Error) return;
 to=from+quantity;
 if (from>to)
    Error=errMLFromLessTo;
 min=from;
 max=to;
}

static
int PreproLine_Start(char *line, unsigned len, unsigned flags, stPreproInfo *p,
                     DynStrCatStruct *str)
{
 char *s;

 if (flags & prliASCIIZ)
    len=strlen(line);
 if (flags & prliEatSpaces)
   {
    s=SkipBlanks(line);
    len-=s-line;
   }
 else
    s=line;
 // Eliminate the \n
 if (s[len-1]=='\n')
   {
    len--;
    s[len]=0;
   }
 // Is a valid preprocessor line
 if (*s!=p->start)
    return prlisNoPL;
 // Skip de preprocessor character
 s++;
 len--;
 // Start concatenating escaped lines
 DynStrCatInit(str,s,len);
 // Is multiline?
 if (s[len-1]==p->escape)
   { // Yes, ask for more and eliminate the escape char
    str->str[len-1]=' ';
    return prlisPLContinue;
   }
 return prlisPLOK;
}

static
int PreproLine_Continue(char *line, unsigned len, unsigned flags, stPreproInfo *p,
                        DynStrCatStruct *str)
{
 if (flags & prliASCIIZ)
    len=strlen(line);
 // Eliminate the \n
 if (line[len-1]=='\n')
   {
    len--;
    line[len]=0;
   }
 // Concatenate
 DynStrCat(str,line,len);
 // Is multiline?
 if (line[len-1]==p->escape)
   { // Yes, ask for more and eliminate the escape char
    str->str[len-1]=' ';
    return prlisPLContinue;
   }
 return prlisPLOK;
}

static
int GetLenOfToken(char *&str)
{
 char *s=str;
 while (*s && ucisspace(*s)) s++;
 str=s;
 while (*s && !ucisspace(*s)) s++;
 int r=s-str;
 //str=s;
 return r;
}

static
int GetLenOfWord(char *&str)
{
 char *s=str;
 while (*s && ucisspace(*s)) s++;
 str=s;
 while (*s && (TVCodePage::isAlNum(*s) || *s=='_')) s++;
 int r=s-str;
 //str=s;
 return r;
}

static inline
char *SkipString(char *s)
{
 for (s++; *s && *s!='\n' && *s!='\"'; s++);
 if (*s=='"') s++;
 return s;
}

static
int GetLenOfWholeWord(char *&str)
{
 char *s=str;
 while (*s && !(TVCodePage::isAlpha(*s)  || *s=='_'))
   {
    if (*s=='"')
       s=SkipString(s);
    else
       s++;
   }
 str=s;
 while (*s && (TVCodePage::isAlNum(*s) || *s=='_')) s++;
 int r=s-str;
 return r;
}

static
int PreproLine_InterpretIfDef(char *s, stPreproInfo *, TDef *defs,
                              int yes)
{
 int l=GetLenOfWord(s),Value;

 Value=defs->isDefined(s,l)==True ? 1 : 0;

 if (!yes)
    Value=Value ? 0 : 1;

 return Value;
}

static
int PreproLine_InterpretIf(char *&str, stPreproInfo *p, TDef *defs,
                           int nested=0);

static
int PreproLine_SolveOperand(char *&str, stPreproInfo *p, TDef *defs)
{
 char *s=str;

 s=SkipBlanks(s);
 if (*s=='(')
   {
    str=s+1;
    return PreproLine_InterpretIf(str,p,defs,1);
   }
 int invert=0;
 if (*s==p->sNot)
   {
    s++;
    s=SkipBlanks(s);
    invert=1;
   }
 int Value=prlieSyntax;
 int l=GetLenOfWord(s);
 if (strncasecmp(s,p->sDefined,l)==0)
   {
    s+=l;
    s=SkipBlanks(s);
    if (*s!='(') return prlieSyntax;
    s++;
    s=SkipBlanks(s);
    l=GetLenOfWord(s);
    if (!l) return prlieSyntax;
    Value=defs->isDefined(s,l)==True ? 1 : 0;
    if (invert) Value=Value ? 0 : 1;
    s+=l;
    s=SkipBlanks(s);
    if (*s!=')') return prlieSyntax;
    s++;
   }
 else
   {
    if (!l) return prlieSyntax;
    Value=defs->isDefined(s,l)==True ? 1 : 0;
    if (invert) Value=Value ? 0 : 1;
    s+=l;
   }
 str=s;
 return Value;
}

static
int PreproLine_GetOperation(char *&str, stPreproInfo *p, TDef *)
{
 int l=GetLenOfWord(str);
 if (!l)
    return prlieNoOp;
 if (strncasecmp(str,p->sAnd,l)==0)
   {
    str+=l;
    return prlieAnd;
   }
 else
 if (strncasecmp(str,p->sOr,l)==0)
   {
    str+=l;
    return prlieOr;
   }

 return prlieNoOp;
}

static
int PreproLine_InterpretIf(char *&str, stPreproInfo *p, TDef *defs,
                           int nested)
{
 int Value,Operand=prlieNoOp,RetVal=prlieSyntax;
 char *s=str;

 if (!*s) return prlieSyntax;
 while (*s)
   {
    Value=PreproLine_SolveOperand(s,p,defs);
    if (Value==prlieSyntax) return prlieSyntax;
    if (RetVal==prlieSyntax)
       RetVal=Value;
    else
      {
       switch (Operand)
         {
          case prlieAnd:
               RetVal&=Value;
               break;
          case prlieOr:
               RetVal|=Value;
               break;
         }
      }
    Operand=PreproLine_GetOperation(s,p,defs);
    if (Operand==prlieSyntax) return prlieSyntax;
    if (Operand==prlieNoOp)
      {
       if (nested)
         {
          if (*s==')')
            {
             s++;
             break;
            }
          else
             return prlieSyntax;
         }
       else
         {
          if (*s) return prlieSyntax;
         }
      }
   }
 str=s;
 return RetVal;
}

static
int PreproLine_InterpretDefine(char *str, stPreproInfo *p, TDef *defs,
                               int PreproValue)
{
 if (!PreproValue)
    return prlieDefine;
 char *s=str;

 // Get macro name
 // TODO: support for name(args)
 int l=GetLenOfWord(s);
 if (!l) return prlieSyntax;

 // Is already there?
 if (defs->isDefined(s,l))
   {
    Error=errMLMacroRedefined;
    return prlieDefine;
   }

 // Copy the name
 AllocLocalStr(name,l+1);
 memcpy(name,s,l);
 name[l]=0;

 // Get the definition
 s+=l;
 s=SkipBlanks(s);
 char *end=SkipToEndNoBlanks(s);
 char v=*end; *end=0;
 defs->insert(name,s);
 *end=v;

 return prlieDefine;
}

static
int PreproLine_InterpretUndef(char *str, stPreproInfo *p, TDef *defs,
                              int PreproValue)
{
 if (PreproValue)
   {
    char *s=str;
   
    // Get macro name
    int l=GetLenOfWord(s);
    if (!l) return prlieSyntax;
   
    // Remove it
    defs->undef(s,l);
   }

 return prlieUndef;
}

static
int PreproLine_Interpret(stPreproInfo *p, DynStrCatStruct *str, TDef *defs,
                         int PreproValue)
{
 char *s=str->str;
 int l,ret=prlieSyntax;

 l=GetLenOfToken(s);
 if (strncasecmp(s,p->sIf,l)==0)
   {
    s+=l;
    ret=PreproLine_InterpretIf(s,p,defs);
   }
 else if (strncasecmp(s,p->sIfDef,l)==0)
    ret=PreproLine_InterpretIfDef(s+l,p,defs,1);
 else if (strncasecmp(s,p->sIfNDef,l)==0)
    ret=PreproLine_InterpretIfDef(s+l,p,defs,0);
 else if (strncasecmp(s,p->sElse,l)==0)
    ret=prlieElse;
 else if (strncasecmp(s,p->sEnd,l)==0)
    ret=prlieEnd;
 else if (strncasecmp(s,p->sDefine,l)==0)
    ret=PreproLine_InterpretDefine(s+l,p,defs,PreproValue);
 else if (strncasecmp(s,p->sUndef,l)==0)
    ret=PreproLine_InterpretUndef(s+l,p,defs,PreproValue);
 free(str->str);
 return ret;
}

static inline
int PreproPush(stPreproInfo *p, int PreproValue, int newState)
{
 if (p->depth>=maxDepth)
   {
    Error=errMLTooDeep;
    return PreproValue;
   }
 p->stack[p->depth++]=PreproValue;
 return PreproValue ? newState : 0;
}

static inline
int PreproPop(stPreproInfo *p)
{
 if (p->depth==0)
   {
    Error=errMLUnmatchedEndIf;
    return 0;
   }
 return p->stack[--p->depth];
}

static inline
int PreproToggle(stPreproInfo *p, int PreproValue)
{
 int prev=1;
 if (p->depth)
    prev=p->stack[p->depth-1];
 return prev ? (PreproValue ? 0 : 1) : 0;
}

static
int ReadPreprocessor(char *s, stPreproInfo *p, DynStrCatStruct *str, int PreproValue,
                     TDef *defs, char *buf, FILE *f)
{
 int ret;
 ret=PreproLine_Start(s,0,prliNoEatSpaces | prliASCIIZ,p,str);
 if (ret==prlieSyntax)
    Error=errMLSyntax;
 else
   {
    while (ret==prlisPLContinue)
      {
       GetLine();
       ret=PreproLine_Continue(buf,0,prliNoEatSpaces | prliASCIIZ,p,str);
      }
    if (ret==prlieSyntax)
       Error=errMLSyntax;
    else
      {
       ret=PreproLine_Interpret(p,str,defs,PreproValue);
       switch (ret)
         {
          case 0:
               PreproValue=PreproPush(p,PreproValue,0);
               break;
          case 1:
               PreproValue=PreproPush(p,PreproValue,1);
               break;
          case prlieElse:
               PreproValue=PreproToggle(p,PreproValue);
               break;
          case prlieEnd:
               PreproValue=PreproPop(p);
               break;
          case prlieSyntax:
               Error=errMLSyntax;
               break;
         }
      }
   }
 return PreproValue;
}

static
char *ExpandPreprocessor(char *buf, stPreproInfo *p, TDef *defs)
{
 int l;
 char *s=buf, *prev=buf;
 if (res.str)
   {
    free(res.str);
    res.str=NULL;
   }

 do
   {// Get by words
    l=GetLenOfWholeWord(s);
    if (l)
      {// Search this word
       stDef *p=defs->getMacro(s,l);
       if (p)
         {// Replace it
          if (res.str==NULL)
            {// Not started, copy upto here
             DynStrCatInit(&res,buf,s-buf);
            }
          else
             DynStrCat(&res,prev,s-prev);
          if (p->val)
             DynStrCat(&res,p->val);
          s+=l;
          prev=s;
         }
       else
         {// Not found
          s+=l;
         }
      }
   }
 while (l);

 if (res.str)
   {
    if (prev!=s)
       DynStrCat(&res,prev,s-prev);
    return res.str;
   }
 return buf;
}

/*****************************************************************************
  TMultiMenu and TMultiMenuBar members.
*****************************************************************************/

void TMultiMenu::add(TMenuItem *m)
{
 if (last)
   {
    last->next=m;
    last=m;
   }
 else
    items=last=m;
}

TMultiMenu::~TMultiMenu()
{
 if (next)
    delete next;
}

TMultiMenuBar *TMultiMenuBar::createMultiMenuBar(const TRect &bounds,
                                                 TMultiMenu *aMenu)
{
 TMultiMenu *m=aMenu;
 while (m->next)
    m=m->next;
 return new TMultiMenuBar(bounds,aMenu,m);
}

void TMultiMenuBar::findMenu()
{// The last is the default
 TMultiMenu *p=menuList;
 while (p->next && (helpCtx<p->min || helpCtx>p->max))
   p=p->next;
 if (p!=menu)
   {
    menu=p;
    computeLength();
    drawView();
   }
}

void TMultiMenuBar::update()
{
 // Avoid changing the menu if the user is using it ;-)
 if (state & sfModal)
    return;
 TView *p=TopView();
 unsigned h=p ? p->getHelpCtx() : hcNoContext;
 if (helpCtx!=h)
   {
    helpCtx=h;
    findMenu();
   }
}

TMultiMenuBar::~TMultiMenuBar()
{// Delete all of them
 delete menuList;
 // Prevent farther deletions in ~TMenuBar
 menu=NULL;
}

/*****************************************************************************
  Helpers to construct a TMultiMenu
*****************************************************************************/

static char MenuAndStatusLoaded=0;
static TStatusDef *statusDef=NULL;
// List of menues
static TMultiMenu *firstMenu=NULL;

static
void InitAddSubMenu()
{
 firstMenu=new TMultiMenu();
}

static
void AddNewMenu(unsigned min, unsigned max)
{
 TMultiMenu *mm=new TMultiMenu();
 mm->min=min;
 mm->max=max;
 mm->next=firstMenu;
 firstMenu=mm;
}

static
void AddSubMenu(TSubMenu *m)
{
 firstMenu->add(m);
}

/**[txh]********************************************************************

 Description:
 Reads the menu from the file fileName and creates a TMenuBar with the
provided rectangle as size.

 Return:
 0 if something fails. To display the error call @x{ShowMenuLoadError}.

***************************************************************************/

static
int LoadTVMenuAndStatus(char *fileName)
{
 FILE *f;
 char buf[maxLineLen+1];
 char *s;
 DynStrCatStruct Cat;
 int PreproValue=1;
 InitAddSubMenu();
 TStatusDef *lastStatusDef=NULL;

 Error=0; Line=0;
 f=fopen(fileName,"rt");
 if (!f)
    return 0;
 FileName=newStr(fileName);
 TDef *defs=new TDef();

 // All of the configuration details
 defs->insert(SEOS_STR);
 #ifdef SEOSf_STR
 defs->insert(SEOSf_STR);
 #endif
 defs->insert(SECPU_STR);
 defs->insert(SEComp_STR);
 #ifdef SECompf_STR
 defs->insert(SECompf_STR);
 #endif
 // The current TV driver
 const char *drv=TScreen::getDriverShortName();
 int l=strlen(drv)+4;
 AllocLocalStr(drvName,l);
 CLY_snprintf(drvName,l,"Drv%s",drv);
 defs->insert(drvName);

 // For compatibility (Win32!=WIN32)
 #if defined(SEOS_Win32)
 defs->insert("WIN32");
 #endif

 #if WITH_MP3
 defs->insert("MP3");
 #endif
 #if HAVE_PCRE_LIB
 defs->insert("PCRE");
 #endif
 #if HAVE_BZIP2
 defs->insert("BZIP2");
 #endif
 #if HAVE_MIXER
 defs->insert("MIXER");
 #endif
 #if HAVE_GDB_MI
 defs->insert("DEBUG");
 #endif
 #if HAVE_CALCULATOR
 defs->insert("CALCULATOR");
 #endif
 #if HAVE_CALENDAR
 defs->insert("CALENDAR");
 #endif

 PreproInfo.depth=0;
 GetLine();
 while (!feof(f) && !Error)
   {
    s=SkipBlanks(buf);
    if (*s=='$') // Preprocessor lines
      {
       PreproValue=ReadPreprocessor(s,&PreproInfo,&Cat,PreproValue,defs,buf,f);
      }
    else
    if (*s!='#' && *s!='\n' && PreproValue) // Skip comment lines
      {
       s=ExpandPreprocessor(s,&PreproInfo,defs);
       if (strncasecmp(s,"SubMenu:",8)==0)
         {
          TSubMenu *m=GetSubMenu(f,buf,s+8,&PreproInfo,&Cat,defs,PreproValue);
          if (!Error)
            {
             if (strcasecmp(m->name,"Editor Right Click")==0)
                TCEditor::RightClickMenu=m;
             else
                AddSubMenu(m);
            }
         }
       else
       if (strncasecmp(s,"NewMenu:",8)==0)
         {
          unsigned min, max;
          GetNewMenuID(f,buf,s+8,min,max);
          if (!Error)
             AddNewMenu(min,max);
         }
       else
       if (strncasecmp(s,"StatusRange:",12)==0)
         {
          TStatusDef *st=GetStatusDef(f,buf,s+12,&PreproInfo,&Cat,defs,PreproValue);
          if (!Error)
            {
             if (statusDef)
               {
                lastStatusDef->next=st;
                lastStatusDef=st;
               }
             else
                statusDef=lastStatusDef=st;
            }
         }
       else
          Error=errMLSyntax;
      }
    if (!Error)
       GetLine();
   }

 fclose(f);
 CLY_destroy(defs);
 if (!Error)
   {
    if (!firstMenu->items)
       Error=errMLNoMenuDefinition;
    else
    if (!statusDef)
       Error=errMLNoStatusDefinition;
    else
       return 1;
   }
 return 0;
}

int LoadMenuAndStatus(char *fileName, int forceReload)
{
 if (MenuAndStatusLoaded && !forceReload) return 1;
 Error=0; firstMenu=NULL; statusDef=NULL;
 return LoadTVMenuAndStatus(fileName);
}

TMultiMenuBar *GetTVMenu(char *fileName, TRect &rect)
{
 if (Error) return 0;
 if (!firstMenu && !LoadMenuAndStatus(fileName)) return 0;
 return TMultiMenuBar::createMultiMenuBar(rect,firstMenu);
}

TStatusLine *GetTVStatusLine(char *fileName, TRect &rect)
{
 if (Error) return 0;
 if (!statusDef && !LoadMenuAndStatus(fileName)) return 0;
 return new TStatusLine(rect,*statusDef);
}

/**[txh]********************************************************************

  Description:
  It just deletes anything allocated by this module.

***************************************************************************/

void UnLoadTVMenu(void)
{
 delete[] FileName;
 FileName=0;
 UnRegisterMacroCommands();
 free(res.str);
 res.str=NULL;
}


/**[txh]********************************************************************

 Description:
 Shows the error using a messageBox. If no error just returns.

***************************************************************************/

void ShowMenuLoadError(void)
{
 if (Error && FileName)
   {
    char *aux=TVIntl::getTextNew(ErrorNames[Error]);
    messageBox(mfError | mfOKButton,__("Error loading menu: (%d) %s in line %d of %s."),
               Error,aux,Line,FileName);
    DeleteArray(aux);
    Error=0;
    delete FileName;
    FileName=0;
   }
}
