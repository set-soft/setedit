/* Copyright (C) 2000-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  Clipper parser used by bufun.cc when the syntax is Clipper 5.x.

***************************************************************************/

#define Uses_string
#define Uses_stdio
#define Uses_ctype
#define Uses_TVCodePage
#include <tv.h>

#include <bufun.h>

#if 0
FILE *f;

static
int GetChar()
{
 return fgetc(f);
}
#else
static char *buffer;
static unsigned lenBuffer, indexBuffer;

static
int GetChar()
{
 return indexBuffer>=lenBuffer ? EOF : buffer[indexBuffer++];
}
#endif

const int LargoMax=255;
static char FirstCol=1;
static int  Line=1;

static
int GetUpToEOL()
{
 int c;
 do
   {
    c=GetChar();
   }
 while (c!='\n' && c!=EOF);
 FirstCol=1;
 Line++;
 return c;
}

static
int GetWord()
{
 int c,IndiceW=0;
 do
   {
    c=GetChar();
    switch (c)
      {
       case '"':
            FirstCol=0;
            do
              {
               c=GetChar();
              }
            while (c!='"' && c!='\n' && c!=EOF);
            if (c=='\n')
              {
               FirstCol=1;
               Line++;
              }
            if (IndiceW) return IndiceW;
            break;

       case '\'':
            FirstCol=0;
            do
              {
               c=GetChar();
              }
            while (c!='\'' && c!='\n' && c!=EOF);
            if (c=='\n')
              {
               FirstCol=1;
               Line++;
              }
            if (IndiceW) return IndiceW;
            break;

       case '*':
            FirstCol=0;
            c=GetChar();
            if (c==' ')
              {
               c=GetChar();
               if (c=='-')
                 { // That's an EOL comment.
                  c=GetUpToEOL();
                 }
              }
            if (IndiceW) return IndiceW;
            break;

       case '/':
            FirstCol=0;
            c=GetChar();
            if (c=='/')
              { // That's an EOL comment.
               c=GetUpToEOL();
              }
            else
              {
               if (c=='*')
                 { // Large comment
                  c=GetChar();
                  if (c=='\n') Line++;
                  do
                    {
                     if (c=='*')
                       {
                        c=GetChar();
                        if (c=='/')
                           break;
                       }
                     else
                        c=GetChar();
                     if (c=='\n') Line++;
                    }
                  while (c!=EOF);
                  FirstCol=0;
                 }
              }
            if (IndiceW) return IndiceW;
            break;

       case '&':
            FirstCol=0;
            c=GetChar();
            if (c=='&')
               c=GetUpToEOL();
            if (IndiceW) return IndiceW;
            break;

       case '#':
            if (FirstCol)
               c=GetUpToEOL();
            if (IndiceW) return IndiceW;
            break;

       default:
            if (TVCodePage::isAlpha(c) || c=='_')
              {
               if (IndiceW<MaxLen)
                  bfBuffer[IndiceW++]=c;
              }
            else
              {
               if (c=='\n')
                 {
                  FirstCol=1;
                  Line++;
                 }
               else
                 {
                  if (!isspace(c))
                     FirstCol=0;
                 }
               if (IndiceW) return IndiceW;
              }
      }
   }
 while (c!=EOF);
 return IndiceW;
}

static
int CheckStr(char *word, char *toCheck)
{
 int l=0;
 while (*toCheck && *word)
   {
    if (toupper(*toCheck)!=*word) return 0;
    toCheck++; word++;
    l++;
   }      
 if (*toCheck) return 0;
 return l>=4;
}

static
int Parsear(tAddFunc AddFunc, int mode)
{
 int r,f,p,line=-1,funs=0,Static=0,lastReturn=-1,lenTemp=0;
 FirstCol=1;
 Line=1;
 do
   {
    r=GetWord();
    if (r)
      {
       bfBuffer[r]=0; p=0;
       // Memorize the last return/quit
       if (CheckStr("RETURN",bfBuffer) || CheckStr("QUIT",bfBuffer))
         {
          lastReturn=Line;
          Static=0;
          continue;
         }
       // Memorize if the static modifier is there
       if (!Static)
         {
          Static=CheckStr("STATIC",bfBuffer);
          if (Static) continue;
         }
       // Check if that's a function or procedure
       f=CheckStr("FUNCTION",bfBuffer);
       if (!f) p=CheckStr("PROCEDURE",bfBuffer);
       if (f || p)
         {
          if (line>=0)
            { // We have one cached, add it
             AddFunc(bfNomFun,lenTemp,line,lastReturn);
             lastReturn=-1;
            }
          line=Line;
          r=GetWord();
          bfBuffer[r]=0;
          if (r<MaxLen-5 && (mode & modeBFClassSep))
            {
             sprintf(bfNomFun,"%s (%c%c)",bfBuffer,
                     f ? 'F' : 'P',         // Function or Procedure
                     Static ? 'S' : ' ');   // Static or normal
             lenTemp=r+6;
            }
          else
            {
             strcpy(bfNomFun,bfBuffer);
             lenTemp=r+1;
            }
          //printf("%s %s %d\n",f ? "Function" : "Procedure",bfNomFun,line);
          funs++;
         }
       Static=0;
      }
   }
 while (r);
 if (line>=0) // We have one cached, add it
    AddFunc(bfNomFun,lenTemp,line,lastReturn);

 return funs;
}

int SearchClipperFuncs(char *b, unsigned l, int mode, tAddFunc AddFunc)
{
 buffer=b;
 lenBuffer=l;
 indexBuffer=0;
 return Parsear(AddFunc,mode);
}

#if 0
int main(int argc, char *argv[])
{
 if (argc!=2)
   {
    printf("Prueba de parser de Clipper, Copyright 2000 by Salvador E. Tropea\n");
    printf("Uso: pclipper archivo\n");
    return 1;
   }
 f=fopen(argv[1],"rt");
 if (!f)
   {
    printf("No se pudo abrir %s\n",argv[1]);
    return 2;
   }
 Parsear();
 fclose(f);
 return 0;
}
#endif
