/**[txh]********************************************************************

  Copyright (c) 2001 by Salvador Eduardo Tropea.
  This program is covered by the GPL license.

  Description:
  Parses a Perl script looking for function definitions.
  It can be compiled as an standalone program by defining STANDALONE.
  That's just an heuristic, not a real parser and I'm quite sure it can be
fulled, but is fast and small.

***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <bufun.h>
#include <string.h>

//#define STANDALONE

static unsigned Index,Line,lenWord,Len;
static unsigned char *Buffer;

static
void GetEndOfStr(unsigned char end)
{
 for (Index++; Index<Len; Index++)
    {
     if (Buffer[Index]=='\n')
       {
        Line++;
        continue;
       }
     if (Buffer[Index]=='\\')
        Index++;
     else
        if (Buffer[Index]==end)
           break;
    }
 Index++;
}

static
void AddToWord(char val)
{
 if (lenWord<(unsigned)MaxLen_1)
   {
    bfBuffer[lenWord++]=val;
    bfBuffer[lenWord]=0;
   }
 Index++;
}

static
int isWordChar()
{
 return (isalnum(Buffer[Index]) || Buffer[Index]=='_');
}

static
void GetWordChars()
{
 while (Index<Len && isWordChar())
    AddToWord(Buffer[Index]);
}

static
int GetNextValue()
{
 lenWord=0;
 bfBuffer[0]=0;
 while (Index<Len)
   {// Eat spaces
    for (; Index<Len && isspace(Buffer[Index]); Index++)
        if (Buffer[Index]=='\n')
           Line++;
    switch (Buffer[Index])
      {
       case '#': // EOL comment eat upto the EOL
            for (; Index<Len && Buffer[Index]!='\n'; Index++);
            if (Buffer[Index]=='\n')
              {
               Index++;
               Line++;
              }
            break;
       case '"': // Parseable string, search the end
            GetEndOfStr('"');
            break;
       case '\'': // Simple string, search the end
            GetEndOfStr('\'');
            break;
       case '`': // Shell string, search the end
            GetEndOfStr('`');
            break;
       case '\\': // Escape sequence, mostly in search patterns
            Index+=2;
            break;
       case '$': // Scalar
            AddToWord('$');
            GetWordChars();
            return 2;
       case '@': // Array
            AddToWord('@');
            GetWordChars();
            return 2;
       default:
            AddToWord(Buffer[Index]);
            Index--;
            if (isWordChar())
              {
               Index++;
               GetWordChars();
               return 1;
              }
            Index++;
            return 0;
      }
   }
 return 0;
}

static
void GetWord()
{
 do
   {
    if (GetNextValue()==1)
       return;
   }
 while (Index<Len);
}

static
void SearchBalance(char start, char end)
{
 do
   {
    if (GetNextValue()==0 && bfBuffer[0]==start)
       break;
   }
 while (Index<Len);
 int level=1;
 do
   {
    if (GetNextValue()==0)
      {
       if (bfBuffer[0]==start)
          level++;
       else
         if (bfBuffer[0]==end)
           {
            level--;
            if (level==0)
               break;
           }
      }
   }
 while (Index<Len);
}

int SearchPerlFuncs(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
 unsigned lenFound,lineFound,funcs=0;
 Index=0; Line=1; Len=len; Buffer=(unsigned char *)buffer;
 while (Index<len)
   {
    GetWord();
    if (strcmp(bfBuffer,"sub")==0)
      {
       GetWord();
       lenFound=lenWord+1;
       lineFound=Line;
       strcpy(bfNomFun,bfBuffer);
       SearchBalance('{','}');
       AddFunc(bfNomFun,lenFound,lineFound,Line);
       funcs++;
      }
   }
 return funcs;
}

#ifdef STANDALONE
char bfBuffer[MaxLenWith0];
char bfNomFun[MaxLenWith0];
char bfTempNomFun[MaxLenWith0];

void PrintFunc(char *name, int len, int lineStart, int lineEnd)
{
 printf("%s [%d,%d]\n",name,lineStart,lineEnd);
}

int main(int argc, char *argv[])
{
 printf("Perl Functions Parser, Copyright 2001 by Salvador E. Tropea\n");
 if (argc!=2)
   {
    printf("Use: pperl file\n");
    return 1;
   }
 FILE *f=fopen(argv[1],"rt");
 if (!f)
   {
    printf("Can't open %s\n",argv[1]);
    return 2;
   }
 fseek(f,0,SEEK_END);
 long len=ftell(f);
 fseek(f,0,SEEK_SET);
 char *buffer=new char[len+1];
 long bread=fread(buffer,len,1,f);
 if (bread!=1)
   {
    printf("Error reading file %s\n",argv[1]);
    perror("Error");
    fclose(f);
    return 3;
   }
 fclose(f);
 buffer[len]=0;
 SearchPerlFuncs(buffer,len,0,PrintFunc);

 return 0;
}
#endif // STANDALONE
