/****************************************************************************

  Busca Funciones(BuFun), Copyright (c) 1996-2002 by Salvador E. Tropea (SET)

  Designed for the TCEditor class to be used by Robert Hîhne in your RHIDE.

  If you want to use this code contact me first.

  E-Mail: salvador@inti.edu.ar

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
the C source searching for functions, the methode is heuristic and don't
detect all the possible cases, in particular the K & R edition I style of
functions (totally obsolet).
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
#include <stdio.h>
#include <ctype.h>
#define Uses_string
#include "ucdefs.h"

#define Uses_SOStack
#define Uses_TNoCaseSOSStringCollection
#define Uses_TSOSSortedListBox
#define Uses_TVCodePage
#include <settvuti.h>

inline int IsWordChar(char c)
{
 return c=='_' || TVCodePage::isAlpha(c);
}

static char Buffer[256];
static char NomFun[256];
static char Alone;
static int Used,UsedNom;
static int Line,LineFun;

static char *buffer;
static unsigned IndexB;
static unsigned BufLen;

static int TakeWord(int TakeOneCharToo=0);


static char GetAChar()
{
 if (IndexB>=BufLen)
    return EOF;
 char c=buffer[IndexB++];
 if (c=='\n')
    Line++;
 return c;
}

static void UnGetAChar(char c)
{
 IndexB--;
 if (c=='\n')
    Line--;
}


static stkHandler StrDup(char *s,int line,int len,SOStack &stk)
{
 stkHandler h=stk.alloc(len+sizeof(int));
 char *d=stk.GetStrOf(h);
 strcpy(d,s);
 int *l=(int *)&d[len];
 *l=line;
 return h;
}

static int TakeWord(int TakeOneCharToo)
{
 char c;
 char last;

 do
  {
   c=GetAChar();
   if (IsWordChar(c))
     {
      Used=0;
      do
       {
        Buffer[Used++]=c;
        c=GetAChar();
       }
      while (c!=EOF && Used<255 && (TVCodePage::isAlNum(c) || c==':' || c=='_' || c=='~'));
      Buffer[Used]=0;
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
                   c=GetAChar();
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
                   c=GetAChar();
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
              break;


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


static int SearchBalance(char ref, char ref2)
{
 int r,bal=1;

 do
  {
   r=TakeWord(1);
   if (r==2)
     {
      if (Alone==ref)
         bal--;
      else
         if (Alone==ref2) bal++;
     }
   if (!bal) break;
  }
 while (r);

 return r;
}

static int SearchBalanceCopy(char ref, char ref2)
{
 int r,bal=1;

 if (Used<255)
    NomFun[Used++]=ref2;
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
      if (UsedNom<255)
        {
         if (Alone==')' && NomFun[UsedNom-1]==' ')
            NomFun[UsedNom-1]=Alone;
         else
         if (Alone==',' && NomFun[UsedNom-1]==' ')
           {
            NomFun[UsedNom-1]=Alone;
            NomFun[UsedNom++]=' ';
           }
         else
            NomFun[UsedNom++]=Alone;
        }
     }
   else
     if (r==1)
       {
        if (UsedNom+Used<254)
          {
           strcpy(&NomFun[UsedNom],Buffer);
           UsedNom+=Used;
           NomFun[UsedNom++]=' ';
          }
       }
   if (!bal) break;
  }
 while (r);
 NomFun[UsedNom++]=0;

 return r;
}


static int SearchFuncs(TNoCaseSOSStringCollection *FunList,SOStack &stk,
                       unsigned )
{
 int funs=0;

 Line=1;
 int r;
 do
  {
   r=TakeWord();
   if (!r) break;
   do
    {
     r=TakeWord(1);
    }
   while(r==1);
   if (!r) break;
   if (Alone=='(')
     {
      strcpy(NomFun,Buffer);
      LineFun=Line;
      r=SearchBalanceCopy(')','(');
      if (!r) break;
      r=TakeWord(1);
      if (!r) break;

      int SearchOpen=0;
      int Eureka=0;
      if (r==2)
        {
         if (Alone=='{')
            Eureka=1;
         else
           if (Alone==':')
              SearchOpen=Eureka=1;
        }
      else
        {
         if (strcmp(Buffer,"const")==0)
            SearchOpen=Eureka=1;
        }
      if (Eureka)
        {
#ifdef TEST
         printf("Function: %s, Line %d\n",NomFun,LineFun);
#endif
         stkHandler s=StrDup(NomFun,LineFun,UsedNom,stk);
         FunList->insert(s);
         funs++;
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
         r=SearchBalance('}','{');
         if (!r) break;
        }
     }
  }
 while(r);

 return funs;
}

int CreateFunctionList(char *b, unsigned l, SOStack &stk,
                       TNoCaseSOSStringCollection *FunList, unsigned ops=0, char *p=0);

int CreateFunctionList(char *b, unsigned l, SOStack &stk,
                       TNoCaseSOSStringCollection *FunList, unsigned ops, char *p)
{
 buffer=b;
 IndexB=0;
 BufLen=l;

 int funcs=SearchFuncs(FunList,stk,ops);

 if (!funcs)
   {
    printf("Hmmm ... I can't find any function, are you sure?");
    return 1;
   }

 return 0;
}
