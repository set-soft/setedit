/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#define Uses_stdio
#define Uses_snprintf
#define Uses_string
#define Uses_AllocLocal
#define Uses_TNoCaseStringCollection
#define Uses_TCEditor
#define Uses_TCEditor_Commands
#define Uses_TCEditor_Internal
#define Uses_TProgram
// EasyDiag requests
#define Uses_TSButton
#define Uses_TSInputLinePiped
#define Uses_TSLabel
// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#define Uses_TLispVariableCol
#define Uses_TMLIBase
#define Uses_TLispBaseVars
#define Uses_TLispVarDefs
#define Uses_TMLIArrayBase
#define Uses_TMLIEditor
#define Uses_MsgBox
#define Uses_TDialog
#define Uses_TSortedListBox
#define Uses_TRect
#define Uses_TButton
#define Uses_TLabel
#include <mli.h>
#undef GetString
class TSOSListBox;
#include <edmsg.h>
#define Uses_SETAppDialogs
#define Uses_SETAppVarious
#include <setapp.h>
#include <completi.h>
#include <edhists.h>
#include <slpinter.h>
#define Uses_TKeyTranslate
#define Uses_TKeySeqCol
#define Uses_TComSeqCol
#include <keytrans.h>

TCEditor *TMLIEditor::Editor;


char *TMLIEditor::GetWordUnderCursor(int lenMax, int &len, unsigned options)
{
 char *s=Editor->WordUnderCursor(lenMax,options);
 if (s)
   {
    len=strlen(s);
    return s;
   }
 len=0;
 // Allocate a dummy just in case
 s=new char[1];
 *s=0;
 return s;
}

char *TMLIEditor::GetEditorName(void)
{
 return Editor->fileName;
}

char *TMLIEditor::GetSelection(int &len)
{
 if (Editor->hasSelection() && !Editor->selHided)
   {
    len=Editor->selEnd-Editor->selStart;
    char *s=new char[len+1];
    memcpy(s,&Editor->buffer[Editor->selStart],len);
    s[len]=0;
    return s;
   }
 char *s=new char[1];
 s[0]=0; // Just in case
 len=0;
 return s;
}

int TMLIEditor::SendCommand(int command)
{
 if (command>=cmbBaseNumber && command<=cmbBaseNumber+cmbLastCommand)
   {// Editor commands
    if (Editor)
      {
       TEvent event;
       event.what=evCommand;
       event.message.command=command;
       Editor->handleEvent(event);
       return event.what==evNothing;
      }
   }
 else
   {// Application commands
    TEvent event;
    event.what=evCommand;
    event.message.command=command;
    TProgram::application->handleEvent(event);
    return event.what==evNothing;
   }
 return 0;
}

/*
inline static
void Replace2By1(char *str, int ind, int len, char rep)
{
 str[ind-1]=rep;
 memmove(&str[ind],&str[ind+1],len-ind-1);
}

inline static
void Replace2By2(char *str, int ind, const char *rep)
{
 str[ind-1]=*rep;
 str[ind]=rep[1];
}

static int AdjustString(char *str, int len)
{
 int ind,lret=len;

 for (ind=0; ind<len; ind++)
     if (str[ind]=='\\')
       {
        ind++;
        switch (str[ind])
          {
           case 'n':
                #ifdef CLY_UseCrLf
                Replace2By2(str,ind,CLY_crlf);
                #else
                Replace2By1(str,ind,len,'\n');
                lret--;
                #endif
                break;
           case 't':
                Replace2By1(str,ind,len,9);
                lret--;
                break;
           default:
                Replace2By1(str,ind,len,str[ind]);
                lret--;
          }
       }
 return lret;
}
*/

// ONLY FOR CR LF!!!!! not more than 2 characters!!
static
char *AdjustCRLF(char *str, int &len)
{
 int l=len,num=0,i;
 const char *crlf=CLY_crlf;

 if (l<2)
    return str;
 for (i=0; i<l; i++)
     if (str[i]==crlf[0] && str[i+1]!=crlf[1])
        num++;

 if (num)
   {
    len+=num;
    char *s=new char[len+1],*s2;
    for (s2=s, i=0; i<l; s2++, i++)
        if (str[i]==crlf[0] && str[i+1]!=crlf[1])
           *(s2++)=crlf[0], *s2=crlf[1];
        else
           *s2=str[i];
    *s2=0;
    return s;
   }
 return str;
}

int TMLIEditor::InsertText(char *str, int len, int select, int move)
{
 if (Editor)
   {
    Boolean moveToEnd;
   
    switch (move)
      {
       case SLP_MOVE_CURSOR:
            moveToEnd=True;
            break;
       case SLP_DONT_MOVE_CURSOR:
            moveToEnd=False;
            break;
       default: // SLP_DEFAULT_MOVE
            moveToEnd=TCEditor::staticNoMoveToEndPaste ? False : True;
      }
    //len=AdjustString(str,len);
    char *Str=str;
    if (CLY_LenEOL!=1)
       Str=AdjustCRLF(str,len);
    // I forgot it the first time and produced a hard bug
    if (Editor->IslineInEdition)
       Editor->MakeEfectiveLineInEdition();
    // Usually an insertion resets the "found" highlight.
    // So we must do the same here
    Editor->updateFlags&=~ufFound;
    int ret=Editor->insertBuffer(str,0,len,True,select ? True : False,moveToEnd);
    Editor->trackCursor(False);
    if (Str!=str)
       delete[] Str;
    return ret;
   }
 return 0;
}

void TMLIEditor::ShowInStatusLine(char *s, int l)
{
 // Limit to 1Kb screen width
 if (l>1024)
    l=1024;
 // Sanity check, I don't want to crash thanks to a macro.
 if (l<0 || !s)
    return;
 // Copy it to a buffer because the string will be altered if it contains tabs or \n
 AllocLocalStr(b,l+1);
 memcpy(b,s,l);
 b[l]=0;
 Editor->setStatusLine(b);
}

char *TMLIEditor::CompletionChoose(char *options, char *delimiter, unsigned flags)
{
 return ::CompletionChoose(options,delimiter,Editor->cursor.x+Editor->owner->origin.x,
                           Editor->cursor.y+Editor->owner->origin.y+1,flags);
}

int TMLIEditor::GetSyntaxAtCursor()
{
 if (!Editor)
    return 0;
 if (Editor->IslineInEdition)
    Editor->MakeEfectiveLineInEdition();

 uint32 attr=Editor->lenLines.getAttr(Editor->curPos.y);
 Editor->LineMeassure(Editor->curLinePtr,Editor->ColToPointer(),attr,NULL);
 return attr;
}

int TMLIEditor::ForceUpdate()
{
 if (!Editor) return 0;
 Editor->unlock();
 Editor->lock();
 return 1;
}

char *TMLIEditor::AskString(const char *title, const char *message)
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),title));
 TSLabel *label=new TSLabel(message,new TSInputLinePiped(1024,60));
 col->insert(2,1,label);
 EasyInsertOKCancel(col);
 TDialog *d=col->doIt(); delete col; d->options|=ofCentered;
 char b[1024]; *b=0;
 if (execDialog(d,b)!=cmCancel)
    return newStr(b);
 return newStr("");
}

Boolean TMLIEditor::SelectionExists()
{
 return Editor->hasVisibleSelection();
}

unsigned TMLIEditor::GetFindFlags()
{
 unsigned ret=Editor->editorFlags & efFindMaskSL;
 // Compact the global options inside the flags
 if (TCEditor::ReplaceStyle==efTagsText)
    ret|=efTagsTextSL;
 if (TCEditor::RegExStyle==efExtendedRegEx)
    ret|=efExtendedRegExSL;
 else if (TCEditor::RegExStyle==efPerlRegEx)
    ret|=efPerlRegExSL;
 if (TCEditor::CanOptimizeRegEx==efNoOptimizeRegEx)
    ret|=efNoOptimizeRegExSL;
 return ret;
}

Boolean TMLIEditor::FindOrReplaceString(char *str, char *repl, unsigned flags,
                                        char *&string, unsigned &len, Boolean again)
{
 // Save the current state
 unsigned tmpFlags=Editor->editorFlags;
 unsigned tmpStartOfSearch=Editor->StartOfSearch;
 unsigned tmpRegExStyle=TCEditor::RegExStyle;
 unsigned tmpReplaceStyle=TCEditor::ReplaceStyle;
 unsigned tmpCanOptimize=TCEditor::CanOptimizeRegEx;
 unsigned tmpSearchInSel=TCEditor::SearchInSel;
 Boolean ret=False;

 // Expand the flags
 if (flags & efTagsTextSL)
    TCEditor::ReplaceStyle=efTagsText;
 else
    TCEditor::ReplaceStyle=efNormalText;
 if (flags & efOnlySelectionSL)
    TCEditor::SearchInSel=1;
 else
    TCEditor::SearchInSel=0;
 switch (flags & efRexExStyleMask)
   {
    case efBasicRegExSL:
         TCEditor::RegExStyle=efBasicRegEx;
         break;
    case efExtendedRegExSL:
         TCEditor::RegExStyle=efExtendedRegEx;
         break;
    case efPerlRegExSL:
         TCEditor::RegExStyle=efPerlRegEx;
         break;
   }
 if (flags & efNoOptimizeRegExSL)
    TCEditor::CanOptimizeRegEx=efNoOptimizeRegEx;
 else
    TCEditor::CanOptimizeRegEx=efOptimizeRegEx;
 

 if (!again)
   {
    Editor->editorFlags=(flags & efFindMaskSL & ~efOptimizedRegex) | efNoFindFailMsg;
    if (repl)
       Editor->editorFlags|=efDoReplace;
    else
       Editor->editorFlags&=~efDoReplace;

    // Solve the scope of the search
    if (flags & efFromBegginingSL)
       Editor->StartOfSearch=0;
    else
       Editor->StartOfSearch=(unsigned)(Editor->ColToPointer()-Editor->buffer);
    // Memorize the string to search. It is needed during the "find again" and
    // what we have now are sLisp strings from the sLisp stack that will most
    // probably disapear before the call to FindAgain
    DeleteArray(findAgainStr);
    findAgainStr=newStr(str);
    // Do it!
    if (repl)
      {// Do a search & replace
       DeleteArray(replaceAgainStr);
       replaceAgainStr=newStr(repl);
       if (!Editor->CompileSearch(findAgainStr,replaceAgainStr))
          ret=Editor->doSearchReplace();
      }
    else
      {// Do the search
       Editor->CompileSearch(findAgainStr);
       ret=Editor->search(NULL,flags);
      }
    findAgainFlags=Editor->editorFlags | (flags & ~efFindMaskSL);
   }
 else
   {// Repeat the last operation
    Editor->editorFlags=flags & efFindMaskSL;
    Editor->StartOfSearch=(unsigned)(Editor->ColToPointer()-Editor->buffer)+1;
    ret=Editor->doSearchReplace();
   }
 if ((Editor->editorFlags & efDoReplace) && ret)
    Editor->updateFlags&=~ufFound;
 if ((Editor->editorFlags & efDoReplace) || !ret)
    string=NULL;
 else
   {
    len=Editor->selEndF-Editor->selStartF;
    string=newStrL(Editor->buffer+Editor->selStartF,len);
   }

 // Restore the original state
 Editor->editorFlags=tmpFlags;
 Editor->StartOfSearch=tmpStartOfSearch;
 TCEditor::RegExStyle=tmpRegExStyle;
 TCEditor::ReplaceStyle=tmpReplaceStyle;
 TCEditor::CanOptimizeRegEx=tmpCanOptimize;
 TCEditor::SearchInSel=tmpSearchInSel;

 return ret;
}

Boolean TMLIEditor::FindAgain(char *&string, unsigned &len)
{
 if (!findAgainStr)
    return False;
 return FindOrReplaceString(findAgainStr,NULL,findAgainFlags,string,len,True);
}

int  TMLIEditor::GetCursorX()
{
 return Editor->curPos.x;
}

int  TMLIEditor::GetCursorY()
{
 return Editor->curPos.y;
}

void TMLIEditor::SetCursorXY(int x, int y)
{
 Editor->MoveCursorTo(x,y,True);
}

const char *TMLIEditor::GetSyntaxLang()
{
 Editor->CacheSyntaxHLData(Editor->GenericSHL);
 return Editor->SyntaxHL!=shlNoSyntax ? Editor->strC.Name : "";
}

int TMLIEditor::SelectWindowNumber(int num)
{
 int ret=::SelectWindowNumber(num);
 if (ret)
   {// Change the sLisp target editor if that's an editor
    TCEditor *p=GetCurrentIfEditor();
    if (p)
      {
       Editor->update(ufView);
       Editor=p;
      }
   }
 return ret;
}

int TMLIEditor::GetCurWindowNumber()
{
 return ((TWindow *)Editor->owner)->number;
}

int TMLIEditor::GetMaxWindowNumber()
{
 return ::GetMaxWindowNumber();
}

/*****************************************************************************
  Key binding operations
*****************************************************************************/

KeyTTable *TMLIEditor::oriKeyTable=NULL;
int TMLIEditor::oriCanBeDeleted;

int TMLIEditor::StartKeyBind()
{
 if (oriKeyTable)
    return 0;
 oriKeyTable=KeyTrans.expand(oriCanBeDeleted);
 return oriKeyTable!=NULL;
}

void TMLIEditor::EndKeyBind()
{
 if (!oriKeyTable)
    return;
 if (oriCanBeDeleted)
    delete oriKeyTable;
 KeyTrans.compact();
 oriKeyTable=NULL;
}

void TMLIEditor::AbortKeyBind()
{
 if (!oriKeyTable)
    return;
 KeyTrans.ChangeTable(oriKeyTable,oriCanBeDeleted ? kbtDynamic : kbtStatic);
 oriKeyTable=NULL;
}

int TMLIEditor::BindKey(TKeySeqCol *sKeys, void *data, int Type)
{
 int keyBranch;
 int ret=KeyTrans.addKey(sKeys,data,Type,&keyBranch);
 if (ret>=0)
   {
    KeyTrans.deleteKey(ret);
    KeyTrans.addKey(sKeys,data,Type);
   }
 else
   if (ret==-1)
     {// This is something not allowed from the dialogs: to simply destroy
      // a group of assigments to replace it by just one assignment.
      KeyTrans.deleteKey(keyBranch);
      KeyTrans.addKey(sKeys,data,Type);
     }
 return 1;
}

#define IntMessage1(a,b)     aux=TVIntl::getTextNew(a); \
                             CLY_snprintf(buf,256,aux,b); \
                             EdShowMessage(buf); \
                             DeleteArray(aux);
static void SLPShowError(void)
{
 char buf[256],*aux;
 EdShowMessageI(__("Error in sLisp interpreter:"),True);
 IntMessage1(__("Type of error: %s"),MLIEditorTypeError)
 EdShowMessage(MLIEditorErrorName);
 IntMessage1(__("Code: ...%s..."),MLIEditorErrorCode)
 EdShowMessageI(__("End of SLP"));
}

static char *slpFile=0;

void SLPInterfaceInit(char *file)
{
 if (slpFile)
    return;
 FILE *f;
 // avoid problems if somebody tries to access an editor
 TMLIEditor::Editor=0;
 if (!InitLispEditor())
    return;
 f=fopen(file,"rb");
 if (f)
   {
    fseek(f,0,SEEK_END);
    int len=ftell(f);
    fseek(f,0,SEEK_SET);
    slpFile=new char[len+1];
    if (slpFile)
      {
       fread(slpFile,len,1,f);
       slpFile[len]=0;
       if (!InterpretLispEditorFile(slpFile))
          SLPShowError();
      }
    fclose(f);
   }
}

void SLPInterfaceDeInit(void)
{
 DeInitLispEditor();
 delete[] slpFile;
 slpFile=0;
}

void SLPInterfaceRunSelection(TCEditor *ed)
{
 if (ed->hasVisibleSelection())
   {
    if (ed->IslineInEdition)
       ed->MakeEfectiveLineInEdition();
    unsigned l=ed->selEnd-ed->selStart;
    char *s=(char *)malloc(l+1);
    if (s)
      {
       memcpy(s,ed->buffer+ed->selStart,l);
       s[l]=0;
       SLPInterfaceRunString(ed,s,True);
       free(s);
      }
   }
}

int SLPInterfaceRunString(TCEditor *ed, char *code, Boolean verbose)
{
 int ret=SLP_OK;
 TMLIEditor::Editor=ed;
 if (!InterpretLispEditor(code,verbose))
   {
    SLPShowError();
    ret=SLP_ERROR;
   }
 ed->update(ufView);
 TMLIEditor::Editor=0;

 return ret;
}

void SLPInterfaceRunAsk(TCEditor *ed, char *code)
{
 TSViewCol *col=new TSViewCol(__("Enter sLisp code to interprete"));
 TSInputLinePiped *inp=new TSInputLinePiped(maxRunAskCode-1,1,hID_sLispMacros,60);
 col->insert(xTSCenter,yTSUp,inp);
 EasyInsertOKCancel(col);
 TDialog *d=col->doIt(); delete col; d->options|=ofCentered;
 char b[maxRunAskCode];
 if (code)
    strcpy(b,code);
 else
    *b=0;
 if (execDialog(d,b)!=cmCancel)
   {
    b[maxRunAskCode-1]=0;
    SLPInterfaceRunString(ed,b,True);
   }
}

void SLPInterfaceRun(TCEditor *ed)
{
 if (!slpFile)
    return;
 TMLIEditor::Editor=ed;

 switch (ChooseAndRunLispEditor())
   {
    case SLP_ERROR:
         SLPShowError();
         break;
    case SLP_NO_MACROS:
         messageBox(__("No macros defined"),mfWarning | mfOKButton);
         break;
   }
 // Is better to redraw it because in some cases is needed, for example when the
 // line is in edition and the series of commands moves the cursor outside the line
 ed->update(ufView);
 TMLIEditor::Editor=0;
}

void SLPInterfaceReRun(TCEditor *ed)
{
 if (!slpFile)
    return;
 TMLIEditor::Editor=ed;

 if (ReRunLastChooseLispEditor()==SLP_ERROR)
    SLPShowError();
 // Is better to redraw it because in some cases is needed, for example when the
 // line is in edition and the series of commands moves the cursor outside the line
 ed->update(ufView);
 TMLIEditor::Editor=0;
}

#define AnDiag 36
// List
#define XL  2
#define AnL 31
#define X2L (XL+AnL)
#define YL  2
#define AlL 12
#define Y2L (YL+AlL)
// Buttons
#define AnOK 10
#define AnCan 12
#define XOK  ((AnDiag-AnOK-AnCan-2)/2)
#define YOK  (Y2L+1)
#define XCan (XOK+AnOK+2)
#define YCan YOK

#define AlDiag YOK+3

static TDialog *SLPCreateDialog(void)
{
 TDialog *d=new TDialog(TRect(0,0,AnDiag,AlDiag),__("sLisp macros"));

 d->options |= ofCentered;
 d->helpCtx =  cmcChooseMacro;

 TScrollBar *sb=new TScrollBar(TRect(X2L,YL,X2L+1,Y2L));
 d->insert(sb);

 TSortedListBox *Lista=new TSortedListBox(TRect(XL,YL,X2L,Y2L),1,sb);
 d->insert(Lista);
 d->insert(new TLabel(TRect(XL,YL-1,X2L,YL),__("~M~acros"),Lista));

 d->insert(new TButton(TRect(XOK,YOK,XOK+AnOK,YOK+2),__("O~K~"),cmOK,bfDefault));
 d->insert(new TButton(TRect(XCan,YCan,XCan+AnCan,YCan+2),__("Cancel"),cmCancel,bfNormal));

 d->selectNext( False );
 return d;
}

ccIndex SLPChoose(TNoCaseStringCollection *Col)
{
 struct TListBoxRec
 {
     TCollection *items;
     ccIndex selection;
 } br;

 br.items=Col;
 br.selection=0;

 if (execDialog(SLPCreateDialog(),&br)==cmCancel)
    return -1;

 return br.selection;
}

int SLPSearchMacro(TCEditor *ed, char *name, Boolean verbose)
{
 if (!name)
    return SLP_NO_CHOOSE;

 // Macro names can't start with (, it must be a piece of code
 if (*name=='(')
    return SLPInterfaceRunString(ed,name,verbose);

 TMLIEditor::Editor=ed;

 int ret=MLIEdSeachAndRunCom(name,verbose);
 if (ret==SLP_ERROR)
    SLPShowError();

 ed->update(ufView);
 TMLIEditor::Editor=0;
 return ret;
}

