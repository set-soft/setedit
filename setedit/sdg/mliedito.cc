/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

 Module: Lisp Interpreter for the editor
 Comments:
 This module defines a derived class from TMLIBase that's suitable for the
editor. @x{TMLIBase (class)}.@p

***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#define Uses_string
#define Uses_TLispVariableCol
#define Uses_TMLIBase
#define Uses_TLispBaseVars
#define Uses_TLispVarDefs
#define Uses_TMLIArrayBase
#define Uses_TMLIEditor
#define Uses_TLispSDGstring
#define Uses_MsgBox
#define Uses_TNoCaseStringCollection
#include <settvuti.h>
#include <mli.h>
#define Uses_TCEditor_Commands
#include <ceditor.h>
#include <rhutils.h>
#include <dyncat.h>
#include <runprog.h>
#include <edmsg.h>

// Open a file and insert it in the desktop
// This function should be defined by RHIDE and is already needed
// by loadfunc.cc. The prototype is defined in setapp.h but this
// shouldn't be needed by RHIDE.
extern void OpenFileFromEditor(char *fullName);

extern char *strncpyZ(char *dest, const char *orig, int size);
extern char *strndup(char *source, int size);

void TMacrosColl::freeItem(void *item)
{
 MLIMacro *m=(MLIMacro *)item;
 delete[] m->start;
 delete m;
}

TMLIEditor::TMLIEditor(TMLIArrayBase *a, TLispVariableCol *v, FILE *f) :
            TMLIBase(a,v,f)
{
 Macros=new TMacrosColl();
}

TMLIEditor::~TMLIEditor()
{
 CLY_destroy(Macros);
 Macros=0;
}

Command TMLIEditor::WhatCommand(char *s)
{
 int i;
 for (i=0; i<MLIEditorCommands; i++)
     if (strcmp(s,cNames[i])==0)
        return cComms[i];
 return TMLIBase::WhatCommand(s);
}

TLispVar *TMLIEditor::WhatConstant(char *s)
{// Editor commands are cmc*
 if (*s=='c' && *(s+1)=='m' && *(s+2)=='c')
   {
    int command=SearchEdCommand(s+3);
    if (command>=0)
       return new TLispInteger(command+cmbBaseNumber);
   }
 // Editor Flags
 if (*s=='e' && *(s+1)=='d' && *(s+2)=='f')
   {
    unsigned val;
    if (SearchEditFlag(s+3,val))
       return new TLispInteger(val);
   }
 return TMLIBase::WhatConstant(s);
}


// That's the real send command
void MLIEditorSendCommand(TMLIBase *o,int start ,int cant)
{
 int i,ret=0;
 LocVarInt(command);

 CheckNumParams(cant<1);

 for (i=0; i<cant; i++)
    {
     GetInteger(i,command);
     if (TMLIEditor::SendCommand(command->val))
        ret++;
     destroyFloatVar(command);
     command=NULL;
    }
 MLIRetInt(ret);

CleanUp:
 destroyFloatVar(command);
}

// (SendCommands value ...)
DecFun(MLISendCommNoSel)
{
 MLIEditorSendCommand(o,start,cant);
}

// (SendCommSel value ...)
/*DecFun(MLISendCommSel)
{
 MLIEditorSendCommand(o,start,cant,1);
}*/

// (InsertText "str" [select] [move])
DecFun(MLIEditorInsertText)
{
 int select=0,move=-1;
 LocVarStr(string);

 CheckNumParams(cant<1 || cant>3);

 GetString(0,string);

 // Get the select setting
 if (cant>=2)
   {
    TLispVar *p=o->Solve(start+1);
    select=o->MLIBooleanValOf(p);
    destroyFloatVar(p);
   }

 // Get the move
 if (cant==3)
   {
    TLispVar *p=o->Solve(start+2);
    move=o->MLIBooleanValOf(p);
    destroyFloatVar(p);
   }

 MLIRetInt(TMLIEditor::InsertText(string->str,string->len,select,move));

CleanUp:
 destroyFloatVar(string);
}

DecFun(MLIdefmacro)
{
 ccIndex ind;
 int len;
 MLIMacro *macro;
 TMacrosColl *Macros=((TMLIEditor *)o)->Macros;
 LocVarStr(string);
 LocVarCode(code);

 CheckNumParams(cant!=2);
 GetString(0,string);
 GetCode(1,code);

 macro=new MLIMacro;
 // Copy the name
 len=min(31,string->len);
 strncpy(macro->Name,string->str,len);
 macro->Name[len]=0;
 // Copy the code
 len=code->end-code->start;
 macro->start=new char[len+1];
 memcpy(macro->start,code->start,len);
 macro->start[len]=0;

 if (Macros->search(macro->Name,ind))
   { // Exists, so replace it
    Macros->atFree(ind);
   }
 Macros->insert(macro);

 MLIRetInt(1);

CleanUp:
 destroyFloatVar(string);
 destroyFloatVar(code);
}

// (RunProgram "prg1;prg2;..." [options_flags] [compiler])
DecFun(MLIRunProgram)
{
 int Options=0,flags=repDontFork|repDontShowDialog;
 char *comp="GNU";
 LocVarStr(string);
 LocVarInt(option);
 LocVarStr(compiler);

 CheckNumParams(cant!=1 && cant!=2 && cant!=3);
 GetString(0,string);
 if (cant>=2)
   {
    GetInteger(1,option);
    Options=option->val;
   }
 if (Options & 1)
    flags|=repRestoreScreen;
 if (Options & 2)
    flags|=repNoRedirOut;
 //fprintf(stderr,"Options %d\n",Options);
 if (cant>=3)
   {
    GetString(2,compiler);
    comp=compiler->str;
   }

 RunExternalProgram(string->str,flags,comp);
 MLIRetInt(1);

CleanUp:
 destroyFloatVar(string);
 destroyFloatVar(option);
}

// (RunProgramRedir "prg1;prg2;..." [input_string])
DecFun(MLIRunProgramRedir)
{
 char *s;
 int l;
 unsigned flags=repDontShowAsMessage|repDontFork|repDontShowDialog;
 LocVarStr(string);
 LocVarStr(input);

 CheckNumParams(cant!=1 && cant!=2);

 GetString(0,string);
 if (cant==2)
   {
    GetString(1,input);
    RunExternalProgramSetInRedir(input->str,input->len);
    flags|=repRedirIn;
   }
 // Even when strings are stored as char * + len they are ASCIIZ
 RunExternalProgram(string->str,flags);
 if (cant==2)
    RunExternalProgramRemoveInRedir();

 // Ok now we have the redirected result in a file, get it
 l=0x100000;
 s=RunExternalProgramGetFile(l);
 MLIRetStrLenExists(s,l);

CleanUp:
 destroyFloatVar(string);
 destroyFloatVar(input);
}

//
// (WhichEditor [n])
// n is optional by now are supported only:
// 0 the name as-is (default)
// 1 the name without extention
// 2 the path
// 3 the drive
// 4 the extention
// 5 the name of the file without extention
//
DecFun(MLIWhichEditor)
{
 int Option=0,l;
 char *s;
 int remove=0;
 LocVarInt(option);

 CheckNumParams(cant>1);

 if (cant)
   {
    GetInteger(0,option);
    Option=option->val;
   }

 s=TMLIEditor::GetEditorName();
 l=strlen(s);
 switch (Option)
   {
    case 1:
        {
         char *lastSlash=strrchr(s,'/');
         if (!lastSlash)
            lastSlash=strrchr(s,'\\');
         char *lastDot=strrchr(s,'.');
         if (lastDot && (!lastSlash || lastSlash<lastDot))
            l=lastDot-s;
        }
         break;
    case 2:
        {
         char drive,*dir,*name,*ext;
         DynStrCatStruct str;
         split_fname(s,drive,dir,name,ext);
         if (drive)
           {
            char tmp[3]={drive,':',0};
            DynStrCatInit(&str,tmp,2);
            DynStrCat(&str,dir);
            s=str.str;
            l=str.len;
            string_free(dir);
           }
         else
           {
            s=dir;
            l=strlen(dir);
           }
         string_free(name);
         string_free(ext);
         remove=1;
        }
         break;
    case 3:
        {
         char drive,*dir,*name,*ext;
         split_fname(s,drive,dir,name,ext);
         s=new char[3];
         if (drive)
           {
            s[0]=drive;
            s[1]=':';
            s[2]=0;
            l=2;
           }
         else
           {
            s[0]=0;
            l=0;
           }
         string_free(dir);
         string_free(name);
         string_free(ext);
         remove=1;
        }
         break;
    case 4:
        {
         char drive,*dir,*name,*ext;
         split_fname(s,drive,dir,name,ext);
         s=ext;
         l=strlen(ext);
         string_free(dir);
         string_free(name);
         remove=1;
        }
         break;
    case 5:
        {
         char drive,*dir,*name,*ext;
         split_fname(s,drive,dir,name,ext);
         s=name;
         l=strlen(name);
         string_free(dir);
         string_free(ext);
         remove=1;
        }
         break;
   }
 MLIRetStrLen(s,l);
 if (remove)
    delete s;

CleanUp:
 destroyFloatVar(option);
}

DecFun(MLIGetSelection)
{
 CheckNumParams(cant!=0);

 int l;
 char *s=TMLIEditor::GetSelection(l);
 MLIRetStrLen(s,l);
}


//
// (WordUnderCursor [n])
//
// Returns the word under the cursor, n is the maximun length we will use, 256 by
// default.
// Note: if n<4, n is 4. If n>32Kb n is 32Kb
//
DecFun(MLIWordUnderCursor)
{
 int lenWord=256,Options=0;
 char *s;
 LocVarInt(len);
 LocVarInt(options);

 CheckNumParams(cant>2);

 if (cant)
   {
    GetInteger(0,len);
    lenWord=len->val;
    // Safety limits, I don't want to crash because the user asked 1Gb ;-)
    if (lenWord<4)
       lenWord=4;
    if (lenWord<0x8000)
       lenWord=0x8000;
    if (cant>1)
      {
       GetInteger(1,options);
       Options=options->val;
      }
   }

 int l;
 s=TMLIEditor::GetWordUnderCursor(lenWord,l,Options);
 MLIRetStrLenExists(s,l);

CleanUp:
 destroyFloatVar(len);
}

// (ShowInStatusLine string)
DecFun(MLIShowInStatusLine)
{
 LocVarStr(string);

 CheckNumParams(cant!=1);
 GetString(0,string);
 TMLIEditor::ShowInStatusLine(string->str,string->len);
 MLIRetInt(1);

CleanUp:
 destroyFloatVar(string);
}

// (ComplChoose options delimiter [flags])
DecFun(MLIComplChoose)
{
 LocVarStr(options);
 LocVarStr(delimiter);
 LocVarInt(oFlags);
 char *str;
 unsigned flags=0;

 CheckNumParams(cant!=2 && cant!=3);
 GetString(0,options);
 GetString(1,delimiter);

 // Get the flags
 if (cant>2)
   {
    GetInteger(0,oFlags);
    flags=oFlags->val;
   }

 str=TMLIEditor::CompletionChoose(options->str,delimiter->str,flags);
 if (!str)
    str=newStr("");
 MLIRetStrLenExists(str,strlen(str));

CleanUp:
 destroyFloatVar(options);
 destroyFloatVar(delimiter);
 destroyFloatVar(oFlags);
}

// (getenv name)
DecFun(MLIgetenv)
{
 LocVarStr(string);
 const char *value;

 CheckNumParams(cant!=1);
 GetString(0,string);
 value=GetVariable(string->str);
 if (!value)
    value="";
 MLIRetString((char *)value);

CleanUp:
 destroyFloatVar(string);
}

DecFun(MLIGetSyntaxAtCursor)
{
 CheckNumParams(cant);
 MLIRetInt(TMLIEditor::GetSyntaxAtCursor());
}

DecFun(MLIForceUpdate)
{
 CheckNumParams(cant);
 MLIRetInt(TMLIEditor::ForceUpdate());
}

DecFun(MLIAskString)
{
 LocVarStr(title);
 LocVarStr(message);
 char *str;

 CheckNumParams(cant!=2);

 GetString(0,title);
 GetString(1,message);
 str=TMLIEditor::AskString(title->str,message->str);
 MLIRetStrLenExists(str,strlen(str));

CleanUp:
 destroyFloatVar(title);
 destroyFloatVar(message);
}

DecFun(MLIOpenFile)
{
 LocVarStr(file);

 CheckNumParams(cant!=1);

 GetString(0,file);
 OpenFileFromEditor(file->str);
 MLIRetInt(1);

CleanUp:
 destroyFloatVar(file);
}

DecFun(MLIMessageBox)
{
 unsigned ops=mfError|mfOKButton;
 LocVarStr(message);
 LocVarInt(options);

 CheckNumParams(cant!=1 && cant!=2);

 GetString(0,message);
 if (cant>1)
   {
    GetInteger(1,options);
    ops=options->val;
   }

 MLIRetInt(messageBox(message->str,ops));

CleanUp:
 destroyFloatVar(message);
 destroyFloatVar(options);
}

// (EvalString <string>)
DecFun(MLIEvalString)
{
 CheckNumParams(cant!=1);

 LocVarStr(code);
 TLispVar *return_value;

 GetString(0,code);
 return_value=o->Interpret(code->str);
 destroyFloatVar(code);

 if (!return_value)
   {
    MLIRetNULL();
    return;
   }

 MLIRetObj(return_value);

CleanUp:
 return;
}

// (ShowInMessageWindow var [clean])
DecFun(MLIShowInMessageWindow)
{
 LocVar(value);
 LocVar(clear);
 Boolean clearMW=False;
 char *v;
 int l;

 CheckNumParams(cant!=1 && cant!=2);
 GetVar(0,value);
 if (cant>1)
   {
    GetVar(1,clear);
    clearMW=o->MLIBooleanValOf(clear) ? True : False;
   }

 v=value->toStr();
 EdShowMessage(v,clearMW);
 l=strlen(v);
 delete[] v;
 // Return something like printf does
 MLIRetInt(l);

CleanUp:
 destroyFloatVar(value);
 destroyFloatVar(clear);
}

DecFun(MLISelectionExists)
{
 CheckNumParams(cant!=0);
 MLIRetInt(TMLIEditor::SelectionExists() ? 1 : 0);
}

DecFun(MLIFindString)
{
 int i=0;
 unsigned flags=0;
 LocVarStr(find_option);
 LocVarStr(find_str);

 CheckNumParams(cant<1);
 if (cant>1)
   {
    // get options
    while (i<cant-1)
      {
       GetString(i,find_option);
       if (strcmp("MatchCase",find_option->str)) flags|=1;
       else if (strcmp("MatchWord",find_option->str)) flags|=2;
       else if (strcmp("RegExp",find_option->str)) flags|=4;
       else if (strcmp("InsideComments",find_option->str))
         {
          if (flags&16==0) flags|=8;
         }
       else if (strcmp("OutsideComments",find_option->str))
         {
          if (flags&8==0) flags|=16;
         }
       i++;
       destroyFloatVar(find_option);
      }
   }
 GetString(i,find_str);
 MLIRetInt(TMLIEditor::FindString(find_str->str,flags));

CleanUp:
 destroyFloatVar(find_str);
 destroyFloatVar(find_option);
}

DecFun(MLIGetCursorX)
{
 CheckNumParams(cant!=0);
 MLIRetInt(TMLIEditor::GetCursorX());
}

DecFun(MLIGetCursorY)
{
 CheckNumParams(cant!=0);
 MLIRetInt(TMLIEditor::GetCursorY());
}

DecFun(MLISetCursorXY)
{
 int x,y;
 LocVarInt(X);
 LocVarInt(Y);

 CheckNumParams(cant<1);
 GetInteger(0,X);
 x=X->val;
 if (cant>1)
   {
    GetInteger(1,Y);
    y=Y->val;
   }
 else
    y=TMLIEditor::GetCursorY();
 TMLIEditor::SetCursorXY(x,y);
 // We must return something and y is optional
 MLIRetInt(TMLIEditor::GetCursorY());

CleanUp:
 destroyFloatVar(X);
 destroyFloatVar(Y);
}


char *TMLIEditor::cNames[MLIEditorCommands]=
{
 "SendCommands",
 "InsertText",
 //"SendCommSel",
 "defmacro",
 "RunProgram",
 "WhichEditor",
 "GetSelection",
 "WordUnderCursor",
 "RunProgramRedir",
 "ShowInStatusLine",
 "ComplChoose",
 "getenv",
 "GetSyntaxAtCursor",
 "ForceUpdate",
 "AskString",
 "OpenFile",
 "MessageBox",
 "EvalString",
 "ShowInMessageWindow",
 "SelectionExists",
 "Find",
 "GetCursorX",
 "GetCursorY",
 "SetCursorXY"
};

Command TMLIEditor::cComms[MLIEditorCommands]=
{
 MLISendCommNoSel,
 MLIEditorInsertText,
 //MLISendCommSel,
 MLIdefmacro,
 MLIRunProgram,
 MLIWhichEditor,
 MLIGetSelection,
 MLIWordUnderCursor,
 MLIRunProgramRedir,
 MLIShowInStatusLine,
 MLIComplChoose,
 MLIgetenv,
 MLIGetSyntaxAtCursor,
 MLIForceUpdate,
 MLIAskString,
 MLIOpenFile,
 MLIMessageBox,
 MLIEvalString,
 MLIShowInMessageWindow,
 MLISelectionExists,
 MLIFindString,
 MLIGetCursorX,
 MLIGetCursorY,
 MLISetCursorXY
};


