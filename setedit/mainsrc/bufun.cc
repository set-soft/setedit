/****************************************************************************

  Busca Funciones (BuFun), Copyright (c) 1996-2003 by Salvador E. Tropea (SET)

  Designed for the TCEditor class to be used by Robert Hîhne in your RHIDE.

  If you want to use this code contact me first.

  E-Mail: salvador@inti.gov.ar

  Telephone: (+5411) 4759-0013

  Postal Address:
  Salvador E. Tropea
  CurapaligÅe 2124
  (1678) Caseros - 3 de Febrero
  Prov: Buenos Aires
  Argentina

  These routines are compatible with Borland's TVision 1.03 and the port of
those routines to DJGPP.

  int SelectFunctionToJump(char *b, unsigned l)

  Is the external entry point, this routine calls to one routine that parses
the C source searching for functions, the methode is heuristic and doesn't
detect all the possible cases, in particular the K & R edition I style of
functions (totally obsolete).
  The routine creates a StringCollection with the functions found and then
creates a dialog box to select one. If the user select a function the
routine returns the line where the function starts if there are no functions
or the user choose cancel the routine returns -1.
  The parameters passed to the routine are:
  char *b: The buffer where the routines will search.
  unsigned l: The length of the buffer.

  Defining the label TEST you can test the module as an standalone program.

****************************************************************************/

// That's the first include because is used to configure the editor.
#include "ceditint.h"

#define Uses_stdio
#define Uses_ctype
#define Uses_string
#define Uses_stdlib
#define Uses_itoa

#define Uses_MsgBox
#define Uses_TLabel
#define Uses_TScrollBar
#define Uses_TRect
#define Uses_TButton
#define Uses_TDialog
#define Uses_TEvent
#define Uses_TApplication
#define Uses_TVCodePage

#define Uses_TCEditor_External // For mode constants
#define Uses_TCEditor_Commands // For the cmcJumpToFunction context

#define Uses_TSSOSSortedListBox
#define Uses_TSLabel
#define Uses_TSButton
#define Uses_TSHzGroup
#define Uses_TSStaticText
#include <easydia1.h>
#include <ceditor.h>
#include <easydiag.h>

#define Uses_TGrowDialog
#define Uses_SOStack
#define Uses_TNoCaseSOSStringCollection
#define Uses_TSOSSortedListBox
#include <settvuti.h>

#define Uses_SETAppConst
#include <setapp.h>
#include <edmsg.h>
#include <bufun.h>
#include <splinman.h>

extern ushort execDialog( TDialog *d, void *data );

//#define TEST

inline int IsWordChar(char c)
{
 return c=='_' || TVCodePage::isAlpha(c);
}

char bfBuffer[MaxLenWith0];
char bfNomFun[MaxLenWith0];
char bfTempNomFun[MaxLenWith0];
static char Alone;
static int Used,UsedNom;
static int Line,LineFun;
typedef struct
{
 int line;
 ccIndex index;
} stItem;

#ifdef TEST
static FILE *in;
#else
static char *buffer;
static unsigned IndexB;
static unsigned BufLen;
#endif

static int TakeWord(int TakeOneCharToo=0);


#ifdef TEST
static int GetAChar()
{
 char c=getc(in);
 if (c=='\n')
    Line++;
 return c;
}

static void UnGetAChar(int c)
{
 ungetc(c,in);
 if (c=='\n')
    Line--;
}
#else
static int GetAChar()
{
 if (IndexB>=BufLen)
    return EOF;
 int c=(unsigned char)buffer[IndexB++];
 if (c=='\n')
    Line++;
 return c;
}

static void UnGetAChar(int c)
{
 IndexB--;
 if (c=='\n')
    Line--;
}
#endif


/**[txh]********************************************************************

  Description:
  Puts the next word in Buffer. It skips white spaces, comments,
preprocesor lines, etc. So only words that can be interesting are collected.
If TakeOneCharToo!=0 the routine also collects single chars in Alone global
variable.

  Return:
  0 nothing found.
  1 a word.
  2 a single character
  3 a preprocessor line. The buffer contains the #xxx that started the line.

***************************************************************************/

static
int TakeWord(int TakeOneCharToo)
{
 int c;
 char last;

 do
  {
   c=GetAChar();
   if (c==EOF)
      return 0;
   if (IsWordChar(c))
     {
      Used=0;
      do
       {
        bfBuffer[Used++]=c;
        c=GetAChar();
       }
      while (c!=EOF && Used<MaxLen && (TVCodePage::isAlNum(c) || c==':' || c=='_' || c=='~'));
      bfBuffer[Used]=0;
      if (c!=EOF)
         UnGetAChar(c);
      if (Used==0)
         return 0;
      return 1;
     }
   else
   if (ucisspace(c))
     {
      do
       {
        c=GetAChar();
       }
      while (c!=EOF && ucisspace(c));
      UnGetAChar(c);
     }
   else
     {
      switch (c)
        {
         case '\"':
              do
               {
                c=GetAChar();
                if (c=='\\')
                  {
                   c=GetAChar();
                   if (c!=EOF) c=0;
                  }
               }
              while (c!=EOF && c!='\"');
              break;

         case '\'':
              do
               {
                c=GetAChar();
                if (c=='\\')
                  {
                   c=GetAChar();
                   if (c!=EOF) c=0;
                  }
               }
              while (c!=EOF && c!='\'');
              break;

         case '/':
              c=GetAChar();
              if (c=='/')
                {
                 do
                  {
                   do
                    {
                     last=c;
                     c=GetAChar();
                    }
                   while (c!=EOF && c!='\n' && c!='\r');
                   if (c=='\r') c=GetAChar();
                  }
                 while (last=='\\');
                }
              else
                if (c=='*')
                  {
                   do
                    {
                     c=GetAChar();
                     if (c=='*')
                       {
                        c=GetAChar();
                        if (c=='/')
                           break;
                        UnGetAChar(c);
                       }
                    }
                   while (c!=EOF);
                  }
              break;

         case '#':
              Used=0;
              do
               {
                bfBuffer[Used++]=c;
                c=GetAChar();
               }
              while (c!=EOF && Used<MaxLen && TVCodePage::isAlpha(c));
              bfBuffer[Used]=0;
              if (c==EOF || Used==0)
                 return 0;
              UnGetAChar(c);
              do
               {
                do
                 {
                  last=c;
                  c=GetAChar();
                 }
                while (c!=EOF && c!='\n' && c!='\r');
                if (c=='\r') c=GetAChar();
               }
              while (last=='\\');
              return 3;


         default:
              // Is a single char
              if (TakeOneCharToo)
                {
                 Alone=c;
                 return 2;
                }
        }
     }
  }
 while (c!=EOF);

 return 0;
}

const int maxPreproStates=64;

static int SearchBalance(char ref, char ref2)
{
 int r,bal=1;
 int preproLevel=0;
 static char preproStates[maxPreproStates];

 preproStates[0]=0;
 do
  {
   r=TakeWord(1);
   if (r==2)
     {
      if (preproStates[preproLevel]) continue;
      if (Alone==ref)
         bal--;
      else
         if (Alone==ref2) bal++;
     }
   else if (r==3)
     {
      if (strncmp(bfBuffer,"#if",3)==0)
        {
         preproLevel++;
         if (preproLevel==maxPreproStates) preproLevel--;
         preproStates[preproLevel]=0;
        }
      else if (strncmp(bfBuffer,"#else",5)==0 || strncmp(bfBuffer,"#elif",5)==0)
        {
         preproStates[preproLevel]=1;
        }
      else if (strncmp(bfBuffer,"#endif",6)==0)
        {
         preproLevel--;
         if (preproLevel<0) preproLevel=0;
        }
     }
   if (!bal) break;
  }
 while (r);

 return r;
}


static
int SearchBalanceCopy(char ref, char ref2)
{
 int r,bal=1;

 if (Used<MaxLen)
    bfNomFun[Used++]=ref2;
 UsedNom=Used;
 do
  {
   r=TakeWord(1);
   if (r==2)
     {
      if (Alone==ref)
         bal--;
      else
         if (Alone==ref2) bal++;
      if (UsedNom<MaxLen)
        {
         if (Alone==')' && bfNomFun[UsedNom-1]==' ')
            bfNomFun[UsedNom-1]=Alone;
         else
         if (Alone==',' && bfNomFun[UsedNom-1]==' ')
           {
            bfNomFun[UsedNom-1]=Alone;
            bfNomFun[UsedNom++]=' ';
           }
         else
            bfNomFun[UsedNom++]=Alone;
        }
     }
   else
     if (r==1)
       {
        if (UsedNom+Used<MaxLen_1)
          {
           strcpy(&bfNomFun[UsedNom],bfBuffer);
           UsedNom+=Used;
           bfNomFun[UsedNom++]=' ';
          }
       }
   if (!bal) break;
  }
 while (r);
 bfNomFun[UsedNom++]=0;

 return r;
}

static
int SearchCFuncs(char *b, unsigned l, int mode, tAddFunc AddFunc)
{
 int funs=0;
 int opLen=0;
 char opBuf[8];
 int rearrageName=mode & modeBFClassSep;
 mode&=modeBFFunctions; // Only function/prototype setting

 buffer=b;
 IndexB=0;
 BufLen=l;

 Line=1;
 int r;
 do
  {
   r=TakeWord();
   if (!r) break;
   if (strcmp(bfBuffer,"operator")==0 || (Used>10 && strcmp(bfBuffer+Used-10,"::operator")==0))
     { // C++ operators are a special case.
      // Here we collect the name of the operator, that's symbols until
      // the ( [We can't get fooled by () operator because we always skip
      // the first]
      opLen=1;
      TakeWord(1);
      opBuf[0]=Alone;
      do
        {
         r=TakeWord(1);
         if (r==2 && opLen<4 && Alone!='(')
            opBuf[opLen++]=Alone;
        }
      while (r==2 && Alone!='(');
      opBuf[opLen]=0;
     }
   else
     { // Not an operator, it can be the return type of a function or any
       // other thing. Collect the last word before anything that isn't a
       // word. That's potentially the name of a function
      do
       {
        r=TakeWord(1);
       }
      while(r==1);
      if (!r) break;
     }
   // Ok, we should have the name in Buffer and Alone should be (
   if (Alone=='(')
     {
      strcpy(bfNomFun,bfBuffer);
      if (opLen)
        { // operators
         strcat(bfNomFun," ");
         strcat(bfNomFun,opBuf);
         strcat(bfNomFun," ");
         Used+=opLen+2;
         opLen=0;
        }
      LineFun=Line;
      // Collect all the parameters
      r=SearchBalanceCopy(')','(');
      if (!r) break;
      // Now we should find: (a) a { if that's a function or (b) a ; if that's
      // a prototype.
      do
        {
         r=TakeWord(1);
        }
      while (r==3 && bfBuffer[0]=='#');
      if (!r) break;

      int SearchOpen=0,SkipCatch=0;
      int Eureka=0,isProto=0;

      // Another special case for C++ const member atributes
      // For: proto const ...
      if (r==1 && strcmp(bfBuffer,"const")==0)
         r=TakeWord(1);
      // List of exceptions that can be thrown
      // For: proto throw(...) ...
      if (r==1 && strcmp(bfBuffer,"throw")==0)
        {
         r=TakeWord(1);
         if (r==2 && Alone=='(') // throw(
           {
            r=SearchBalance(')','(');
            if (!r) break;
            // throw(...)
            r=TakeWord(1);
           }
        }

      if (mode==modeBFPrototypes && r==2 && Alone==';')
        {// Looking for a prototype
         Eureka=isProto=1;
        }

      if (!isProto)
        {// Looking for a function
         if (r==2)
           {
            if (Alone=='{')
               Eureka=1;
            else
              if (Alone==':') // Special case for C++ constructors with
                              // initialization lists.
                 SearchOpen=Eureka=1;
           }
         else
           {
            // A bizarre case: try/catch not inside the body
            if (strcmp(bfBuffer,"try")==0)
               SkipCatch=SearchOpen=Eureka=1;
           }
        }
      if (Eureka)
        {
         // Delay the insertion until we know the line of ending
         int used=UsedNom;
         if (mode!=modeBFPrototypes || isProto)
            strcpy(bfTempNomFun,bfNomFun);
            
         if (!isProto)
           {// If the { was delayed look now for it
            if (SearchOpen)
              {
               do
                {
                 r=TakeWord(1);
                 if (r==2 && Alone=='{') break;
                }
               while (r);
               if (!r) break;
              }
            // Skip the body of the function
            r=SearchBalance('}','{');
            if (!r) break;
           }
         // We found a `try', we should find a `catch'.
         // If found skip it adding to the function body these lines.
         if (SkipCatch)
           {
            unsigned pIndexB=IndexB;
            int pLine=Line;
            do
              {
               r=TakeWord(0);
              }
            while (r==3);
            if (r==1 && (strcmp(bfBuffer,"catch")==0))
              {
               r=TakeWord(1);
               if (r==2 && Alone=='(')
                 {
                  if (SearchBalance(')','('))
                    {
                     r=TakeWord(1);
                     if (r==2 && Alone=='{')
                        SearchBalance('}','{');
                    }
                 }
              }
            else
              {
               IndexB=pIndexB;
               Line=pLine;
              }
           }

         // Now insert it
         if (mode!=modeBFPrototypes || isProto)
           {
            //stkHandler s=StrDup(bfTempNomFun,LineFun,Line,used,stk,rearrageName);
            //FunList->insert(s);
            char *member=NULL, *aux;

            // Look for the member name. But only in the function name.
            aux=strchr(bfTempNomFun,'(');
            if (aux)
              {
               *aux=0;
               // Look for the *last* :: (namespace::class::member)
               // Pointed out by Andris.
               member=strrchr(bfTempNomFun,':');
               if (member && member!=bfTempNomFun && member[-1]==':')
                  member--;
               else
                  member=NULL;
               *aux='(';
              }
           
            if (!rearrageName || member==NULL)
              {
               strcpy(bfNomFun,bfTempNomFun);
              }
            else
              { // Change class::member to member (class)
               used++;
               char *source=member+2,*dest=bfNomFun;
               for (; *source; source++,dest++) *dest=*source;
               *(dest++)=' ';
               *(dest++)='(';
               char *s=bfTempNomFun;
               for (;s<member; s++,dest++) *dest=*s;
               *(dest++)=')';
               *dest=0;
              }
            AddFunc(bfNomFun,used,LineFun,Line);
            funs++;
           }
         #ifdef TEST
         printf("Function: %s, Line %d, Line end %d\n",bfTempNomFun,LineFun,Line);
         #endif
        }
     }
  }
 while(r);

 return funs;
}


static TNoCaseSOSStringCollection *glFunList;
static SOStack *glStk;

// A copy of this function is used in txhgen.cc, any update here should be
// reflected there until I find a better solution.
inline
void AlignLen(int &len)
{
 if (len & 3)
    len+=4-(len & 3);
}

static
void StrDup(char *s, int len, int line, int lineEnd)
{
 stkHandler h;
 char *d,b[64];
 ccIndex ind;
 int differentiate=0;

 if (glFunList->Search(s,ind))
   {// Already there
    itoa(line,b,10);
    len+=strlen(b)+1;
    differentiate=1;
   }

 AlignLen(len);
 h=glStk->alloc(len+sizeof(int)*2);
 d=glStk->GetStrOf(h);
 strcpy(d,s);
 if (differentiate)
   {
    strcat(d," ");
    strcat(d,b);
   }
 int *l=(int *)&d[len];
 l[0]=line;
 l[1]=lineEnd;
 glFunList->insert(h);
}

static stFuncsSHL FuncsAvail[]=
{
 {"Clipper 5.x",SearchClipperFuncs},
 {"Perl",SearchPerlFuncs},
 {"Syntax Highlight File",SearchSHLDefs},
 {"Texinfo source",SearchTxiSecs},
 {"80x86 asm (AT&T syntax)",SearchAsmLabels},
 {"80x86 asm (Intel syntax)",SearchAsmLabels},
 {"Netwide Assembler",SearchAsmLabels},
 {"8x51 asm",SearchAsmLabels},
 {"PICs asm",SearchAsmLabels},
 {"PDP11 asm",SearchAsmLabels},
 {"PHP",SearchPHPFuncs},
 {0,SearchCFuncs}
};

static
int SearchFuncs(char *b, unsigned l, TNoCaseSOSStringCollection *FunList,
                SOStack *stk, int mode, char *shl)
{
 if (!shl) return 0;
 glFunList=FunList;
 glStk=stk;
 int i=0;
 //FunList->duplicates=True;
 while (FuncsAvail[i].shl)
   {
    if (!strcmp(FuncsAvail[i].shl,shl))
       return FuncsAvail[i].func(b,l,mode,StrDup);
    i++;
   }
 return FuncsAvail[i].func(b,l,mode,StrDup);
}

#ifndef TEST

class TLFuns : public TGrowDialog
{
public:
 TLFuns( TRect r, const char *name, int extraOptions=0 ) :
      TWindowInit( &TLFuns::initFrame ),
      TGrowDialog(r,name,extraOptions) {};
 void handleEvent(TEvent& event);
};

void TLFuns::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if ( event.what == evCommand || event.what == evBroadcast)
   {
    switch ( event.message.command )
      {
       // a button
       case cmListItemSelected:
            endModal(event.message.command);
            clearEvent(event);
            break;
       case cmeZoom:
            event.message.command=cmZoom;
            TDialog::handleEvent(event);
            break;
       default:
           break;
      }
   }
}

static TDialog *createDialog( )
{
 TLFuns *d=new TLFuns(TRect(1,1,1,1),__("Jump to function"));
 TSViewCol *col=new TSViewCol(d);

 TRect r=TApplication::deskTop->getExtent();
 int h=r.b.y-r.a.y-10;
 int w=r.b.x-r.a.x-15;

 TSSOSSortedListBox *ListaH=new TSSOSSortedListBox(w,h,tsslbVertical|tsslbHorizontal,1,256);
 ((TListViewer *)ListaH->view)->handleSpace=False;
 ListaH->view->growMode=gfMoveBottomCorner;
 TSLabel *lista=new TSLabel(__("List of functions"),ListaH);

 col->insert(xTSLeft,yTSUp,lista);

 TSButton *ok=new TSButton(__("O~K~"),cmOK,bfDefault);
 TSButton *cancel=new TSButton(__("Cancel"),cmCancel);
 TSButton *browse=new TSButton(__("~B~rowse"),cmYes);
 ok->view->growMode=cancel->view->growMode=browse->view->growMode=gfGrowAll;
 TSHzGroup *but123=MakeHzGroup(ok,cancel,browse,0);
 col->insert(xTSCenter,yTSDown,but123);
 col->doItCenter(cmcJumpToFunction);
 delete col;
 return d;
}

static
char *GetNameLine(TNoCaseSOSStringCollection *FunList, int i, int *line)
{
 char *s=FunList->atStr(i);
 int length=strlen(s)+1;
 AlignLen(length);
 *line=*(int *)(s+length);
 return s;
}

static
int cmpStItem(const void *e1, const void *e2)
{
 stItem *i1=(stItem *)e1,*i2=(stItem *)e2;
 return i1->line-i2->line;
}

static
int AskSortList()
{
 TSViewCol *col=new TSViewCol(__("Sort mode"));

 col->insert(xTSCenter,yTSUp,new TSStaticText(__("Sort the functions")));
 col->insert(xTSCenter,yTSDown,
             new TSHzGroup(new TSButton(__("~A~lphabetically"),cmYes,bfDefault),
                           new TSButton(__("by ~L~ine"),cmNo)));
 TDialog *d=col->doItCenter(cmcJumpToFunction);
 delete col;
 return execDialog(d,0);
}

/**[txh]********************************************************************

  Description:
  Searchs functions and prototypes and ask for jumping.

  Include: ced_exte
  Return: Line to jump or -1

***************************************************************************/

int SelectFunctionToJump(char *b, unsigned l, char *word, int mode,
                         char *fileName, char *shl)
{
 SOStack stk;
 TNoCaseSOSStringCollection *FunList = new TNoCaseSOSStringCollection(20,5,&stk);

 int funcs=SearchFuncs(b,l,FunList,&stk,mode|modeBFClassSep,shl);

 struct TListBoxRec
 {
  TCollection *items;
  ccIndex selection;
 } br;

 br.items=FunList;
 br.selection=0;

 if (!funcs)
   {
    messageBox(__("Hmmm ... I can't find any function, are you sure?"),mfError | mfOKButton);
    delete FunList;
    return -1;
   }
 else
   {
    if (word)
      {
       FunList->Search(word,br.selection);
       if (br.selection>=FunList->getCount())
          br.selection=FunList->getCount()-1;
      }
    int ret=execDialog(createDialog(),&br);
    if (ret==cmCancel)
      {
       delete FunList;
       return -1;
      }
    // The Browse button have cmYes as command, just for commodity
    if (ret==cmYes)
      {
       ret=AskSortList();
       if (ret==cmCancel)
         {
          delete FunList;
          return -1;
         }
       TProgram::deskTop->lock();
       char *aux=TVIntl::getTextNew(__("Functions:"));
       EdShowMessage(aux,True);
       DeleteArray(aux);
       int i,c=FunList->getCount();
       FileInfo fInfo;
       fInfo.Column=0; fInfo.offset=fInfo.len=-1;
       if (ret==cmYes)
         {
          br.selection=0;
          // Add to the message window
          for (i=0; i<c; i++)
             {
              char *s=GetNameLine(FunList,i,&fInfo.Line);
              EdShowMessageFile(s,fInfo,fileName);
             }
         }
       else
         {
          // Sort by line number
          stItem *s=new stItem[c];
          for (i=0; i<c; i++)
             {
              GetNameLine(FunList,i,&s[i].line);
              s[i].index=i;
             }
          qsort(s,c,sizeof(stItem),cmpStItem);
          br.selection=s[0].index;
          // Add to the message window
          for (i=0; i<c; i++)
             {
              char *name=GetNameLine(FunList,s[i].index,&fInfo.Line);
              EdShowMessageFile(name,fInfo,fileName);
             }
          DeleteArray(s);
         }
       SpLinesUpdate();
       EdJumpToMessage(0);
       TProgram::deskTop->unlock();
      }
   }

 int line;
 GetNameLine(FunList,br.selection,&line);
 delete FunList;

 return line;
}

int CreateFunctionList(char *b, unsigned l, SOStack &stk,
                       TNoCaseSOSStringCollection *FunList, unsigned ops,
                       char *shl)
{
 int funcs=SearchFuncs(b,l,FunList,&stk,modeBFFunctions | ops,shl);
 return funcs==0;
}

static SOStack *stkBufun=0;
static TNoCaseSOSStringCollection *lstBufun=0;
static const char *FileName=0;
static unsigned FileID=0, numFuncs;

/**[txh]********************************************************************

  Description:
  Destroys a list of functions created with CreateFunctionList. You should
call it if you are sure the user won't need the list again.
@x{CreateFunctionList}.

***************************************************************************/

void DestroyFunctionList()
{
 CLY_destroy(lstBufun);
 delete stkBufun;
 DeleteArray(FileName);
 lstBufun=0;
 stkBufun=0;
 FileName=0;
 FileID=0;
}

/**[txh]********************************************************************

  Description:
  Creates a function list for this file. The information is hold internally
and the file name is memorized. That's done so if you call it twice for the
same file it won't create the list twice. In addition a number that changes
each time the file is modified should be suplied. In this way the list will
be computed if the file was changed. You must pass the pointer to the buffer
containing the code and the length.@p
  Is safe to call this function even when another list is cached because it
won't leak memory, the old list will be destroyed.

  Return:
  The number of functions found.

***************************************************************************/

int CreateFunctionList(char *b, unsigned l, const char *fileName, unsigned ID,
                       char *shl)
{// Check if that's the one we have cached
 if (stkBufun && FileName && strcmp(FileName,fileName)==0 && FileID==ID)
    return numFuncs;
 DestroyFunctionList();
 stkBufun=new SOStack;
 lstBufun=new TNoCaseSOSStringCollection(20,5,stkBufun);
 FileName=newStr(fileName);
 FileID=ID;
 numFuncs=CreateFunctionList(b,l,*stkBufun,lstBufun,modeBFClassSep,shl);
 return numFuncs;
}

/**[txh]********************************************************************

  Description:
  Searchs the name of the function that contains the provided line number.
It uses the list created by CreateFunctionList, so you must call it before.
@x{CreateFunctionList}.

  Return:
  !=0 => start, end and name were filled with the starting line of the
function, end line and the name of the function. The pointer to the name
of the function shouldn't be altered or released.

***************************************************************************/

int SearchFunctionByLine(int line, int &start, int &end, char *&name)
{
 if (!lstBufun) return 0;
 int i,c=lstBufun->getCount(),len,lineS,lineE,*l;

 // ToDo: a binary search, they are sorted!
 for (i=0; i<c; i++)
    {
     char *s=lstBufun->atStr(i);
     len=strlen(s)+1;
     AlignLen(len);
     l=(int *)(s+len);
     lineS=l[0];
     lineE=l[1];
     if (lineE==-1)
       {
        if (i+1<c)
          {
           char *s=lstBufun->atStr(i+1);
           len=strlen(s)+1;
           AlignLen(len);
           l=(int *)(s+len);
           lineE=l[0]-1;
          }
        else
           lineE=INT_MAX;
       }
     if (line>=lineS && line<lineE)
       {
        start=lineS; end=lineE; name=s;
        return 1;
       }
    }
 return 0;
}

#else
static void writeStr(void *p, void *)
{
 char *s=(char *)p;
 int l=strlen(s)+1;
 AlignLen(l);
 printf("%s %d\n",s,*(int *)(s+l));
}

int main(int argc, char *argv[])
{
 if (argc!=2)
    return 1;
 in=fopen(argv[1],"rb");

 TStringCollection *FunList = new TStringCollection(20,5);

 int funcs=SearchFuncs(FunList);

 printf("\n\nLista ordenada de las funciones: (%d)\n\n",funs);
 FunList->forEach(writeStr,NULL);
 return 0;
}
#endif
