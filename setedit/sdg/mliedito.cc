/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

 Module: Lisp Interpreter for the editor
 Comments:
 This module defines a derived class from TMLIBase that's suitable for the
editor. @x{TMLIBase (class)}.@p

***************************************************************************/

#include <configed.h>
#define Uses_stdio
#define Uses_ctype
#define Uses_string
#define Uses_AllocLocal
#define Uses_TLispVariableCol
#define Uses_TMLIBase
#define Uses_TLispBaseVars
#define Uses_TLispVarDefs
#define Uses_TMLIArrayBase
#define Uses_TMLIEditor
#define Uses_TLispSDGstring
#define Uses_MsgBox
#define Uses_TNoCaseStringCollection
#define Uses_TStringable // needed for keytrans.h
#define Uses_TScreen
#include <settvuti.h>
#include <mli.h>
#define Uses_TCEditor_Commands
#include <ceditor.h>
#include <rhutils.h>
#include <dyncat.h>
#include <runprog.h>
#include <edmsg.h>
#define Uses_TKeyTranslate
#define Uses_TKeySeqCol
#define Uses_TComSeqCol
#include <keytrans.h>
#define Uses_SETAppConst
#include <setapp.h>
#include <edspecs.h>

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

char *TMLIEditor::findAgainStr=NULL;
char *TMLIEditor::replaceAgainStr=NULL;
unsigned TMLIEditor::findAgainFlags;

TMLIEditor::TMLIEditor(TMLIArrayBase *a, TLispVariableCol *v, FILE *f) :
            TMLIBase(a,v,f)
{
 Macros=new TMacrosColl();
}

TMLIEditor::~TMLIEditor()
{
 CLY_destroy(Macros);
 Macros=0;
 DeleteArray(findAgainStr);
 DeleteArray(replaceAgainStr);
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
 if (*s=='c' && *(s+1)=='m' && *(s+2)=='e')
   {
    int command=SearchEditCommand(s+3);
    if (command>=0)
       return new TLispInteger(command+cmeBase);
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
 int len, i;
 MLIMacro *macro;
 TMacrosColl *Macros=((TMLIEditor *)o)->Macros;
 LocVarStr(string);
 LocVarCode(code);

 CheckNumParams(cant!=2);
 GetString(0,string);
 GetCode(1,code);

 for (i=0; i<string->len; i++)
     CheckForError(string->str[i]=='(' || string->str[i]==')',MLIInvaForName);

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
 if (Options & 4)
    flags|=repStopDebug;
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

// FindString(str [flags])
DecFun(MLIFindString)
{
 unsigned flags=TMLIEditor::GetFindFlags(), ret;
 LocVarStr(findStr);
 LocVarInt(findFlags);
 char *str;
 unsigned strLen;

 CheckNumParams(cant!=1 && cant!=2);
 GetString(0,findStr);
 if (cant>1)
   {
    // Get flags
    GetInteger(1,findFlags);
    flags=findFlags->val;
   }
 ret=TMLIEditor::FindString(findStr->str,flags,str,strLen);
 if (!str)
   {
    str=newStr("");
    strLen=0;
   }
 MLIRetStrLenExists(str,strLen);

CleanUp:
 destroyFloatVar(findStr);
 destroyFloatVar(findFlags);
}

// ReplaceString(to_find replacement [flags])
DecFun(MLIReplaceString)
{
 unsigned flags=TMLIEditor::GetFindFlags(), ret;
 LocVarStr(findStr);
 LocVarStr(replaceStr);
 LocVarInt(findFlags);
 char *str;
 unsigned strLen;

 CheckNumParams(cant!=2 && cant!=3);
 GetString(0,findStr);
 GetString(1,replaceStr);
 if (cant>2)
   {
    // Get flags
    GetInteger(2,findFlags);
    flags=findFlags->val;
   }
 ret=TMLIEditor::FindOrReplaceString(findStr->str,replaceStr->str,flags,str,strLen);
 if (str)
    DeleteArray(str);
 MLIRetInt(ret);

CleanUp:
 destroyFloatVar(findStr);
 destroyFloatVar(replaceStr);
 destroyFloatVar(findFlags);
}

DecFun(MLIFindAgain)
{
 char *str;
 unsigned strLen;

 CheckNumParams(cant);
 TMLIEditor::FindAgain(str,strLen);
 if (!str)
   {
    str=newStr("");
    strLen=0;
   }
 MLIRetStrLenExists(str,strLen);
}

DecFun(MLIReplaceAgain)
{
 char *str;
 unsigned strLen;

 CheckNumParams(cant);
 Boolean ret=TMLIEditor::FindAgain(str,strLen);
 if (str)
    DeleteArray(str);
 MLIRetInt(ret);
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

DecFun(MLIGetSyntaxLang)
{
 CheckNumParams(cant!=0);
 MLIRetString(TMLIEditor::GetSyntaxLang());
}

DecFun(MLISelectWindowNumber)
{
 LocVarInt(n);

 CheckNumParams(cant<1);
 GetInteger(0,n);
 MLIRetInt(TMLIEditor::SelectWindowNumber(n->val));

CleanUp:
 destroyFloatVar(n);
}

DecFun(MLIGetCurWindowNumber)
{
 CheckNumParams(cant!=0);
 MLIRetInt(TMLIEditor::GetCurWindowNumber());
}

DecFun(MLIGetMaxWindowNumber)
{
 CheckNumParams(cant!=0);
 MLIRetInt(TMLIEditor::GetMaxWindowNumber());
}

// (KeyBindings keyBindOp [keyBindOp ...])
DecFun(MLIKeyBindings)
{
 int i, abortOperation=0;
 LocVarKeyBind(key);

 CheckNumParams(cant<1);

 if (!TMLIEditor::StartKeyBind())
   {// Most probably a nested call
    MLIRetInt(0);
    goto CleanUp;
   }
 abortOperation=1;
 for (i=0; i<cant; i++)
    {
     GetKeyBind(i,key);
     if (!TMLIEditor::BindKey(key->sKeys,key->data,key->bindType))
       {
        MLIRetInt(0);
        goto CleanUp;
       }
     destroyFloatVar(key);
     key=NULL;
    }
 MLIRetInt(1);
 abortOperation=0;
 TMLIEditor::EndKeyBind();

CleanUp:
 destroyFloatVar(key);
 if (abortOperation)
    TMLIEditor::AbortKeyBind();
}

TLispKeyBind::TLispKeyBind(TKeySeqCol *aSKeys, void *aData, int aBindType)
{
 sKeys=aSKeys;
 data=aData;
 bindType=aBindType;
 type=MLITypeKeyBind;
}

TLispKeyBind::~TLispKeyBind()
{
 if (bindType!=kbtDelOp)
   {
    CLY_destroy(sKeys);
    if (bindType==kbtIsSeq)
       CLY_destroy((TComSeqCol *)data);
    else
       delete[] (char *)data;
   }
}

int TLispKeyBind::print(FILE *) { return 0; }
char *TLispKeyBind::toStr() { return NULL; }

TKeySeqCol *ParseKeySeq(char *str, int len)
{
 ushort code;
 // Make a local copy
 AllocLocalStr(s,len+1);
 memcpy(s,str,len);
 s[len]=0;
 // Create the collection
 TKeySeqCol *ret=new TKeySeqCol(2,2);
 // Parse it
 char *key=strtok(s,", ");
 do
   {
    if (InterpretKeyName(key,code))
      {
       CLY_destroy(ret);
       return NULL;
      }
    ret->insert(code);
    key=strtok(NULL,", ");
   }
 while (key);

 return ret;
}

// (BindKey "key1,key2" command [commands ...])
// (BindKey "key1,key2" "macro")
DecFun(MLIBindKey)
{
 LocVarStr(keySeqStr);
 LocVar(first);
 LocVarInt(command);
 TKeySeqCol *seq=NULL;
 TComSeqCol *comms=NULL;
 int i;

 CheckNumParams(cant<2);

 // Get the key sequence
 GetString(0,keySeqStr);
 seq=ParseKeySeq(keySeqStr->str,keySeqStr->len);
 CheckForError(!seq,MLIInvaKeySeq);

 // Get the first as a generic variable to determine the type
 GetVar(1,first);
 switch (GroupTypeOf(first))
   {
    case MLIGInteger:
         // Commands
         comms=new TComSeqCol(2,2);
         comms->insert(MLIAsIntVal(first));
         for (i=2; i<cant; i++)
            {
             GetInteger(i,command);
             comms->insert(command->val);
             destroyFloatVar(command);
             command=NULL;
            }
         MLIRetKeyBind(seq,comms,kbtIsSeq);
         // They are now part of the return value
         comms=NULL;
         seq=NULL;
         break;
    case MLIGString:
         // A macro or sLisp code
         CheckForError(cant>2,MLINumParam);
         MLIRetKeyBind(seq,strdup(MLIAsStrVal(first)),kbtIsMacro);
         // Now part of the return value
         seq=NULL;
         break;
    default:
         CheckForError(1,MLITypeParam);
   }

CleanUp:
 destroyFloatVar(keySeqStr);
 destroyFloatVar(first);
 destroyFloatVar(command);
 CLY_destroy(comms);
 CLY_destroy(seq);
}

// (GetSystemInfo which)
DecFun(MLIGetSystemInfo)
{
 LocVarInt(which);

 CheckNumParams(cant!=1);

 GetInteger(0,which);
 switch (which->val)
   {
    case edfInfTVDriver:
         MLIRetString(TScreen::getDriverShortName());
         break;
    case edfInfOS:
         MLIRetString(SEOS_STR);
         break;
    case edfInfOSFlavor:
         #ifdef SEOSf_STR
         MLIRetString(SEOSf_STR);
         #else
         MLIRetString("");
         #endif
         break;
    case edfInfCPU:
         MLIRetString(SECPU_STR);
         break;
    case edfInfCompiler:
         MLIRetString(SEComp_STR);
         break;
    case edfInfCompilerFlavor:
         #ifdef SECompf_STR
         MLIRetString(SECompf_STR);
         #else
         MLIRetString("");
         #endif
         break;
    default:
         MLIRetString("");
   }

CleanUp:
 destroyFloatVar(which);
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
 "FindString",
 "GetCursorX",
 "GetCursorY",
 "SetCursorXY",
 "GetSyntaxLang",
 "FindAgain",
 "ReplaceString",
 "ReplaceAgain",
 "SelectWindowNumber",
 "GetCurWindowNumber",
 "GetMaxWindowNumber",
 "KeyBindings",
 "BindKey",
 "GetSystemInfo"
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
 MLISetCursorXY,
 MLIGetSyntaxLang,
 MLIFindAgain,
 MLIReplaceString,
 MLIReplaceAgain,
 MLISelectWindowNumber,
 MLIGetCurWindowNumber,
 MLIGetMaxWindowNumber,
 MLIKeyBindings,
 MLIBindKey,
 MLIGetSystemInfo
};


