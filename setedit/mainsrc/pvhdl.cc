/**[txh]********************************************************************

  Copyright (c) 2001-2005 by Salvador Eduardo Tropea.
  This program is covered by the GPL license.

  Description:
  Parses a VHDL description looking for entities, architectures, components,
functions, procedures and packages definitions.
  It can be compiled as an standalone program by defining STANDALONE.
  That's just an heuristic, not a real parser and I'm quite sure it can be
fulled, but is fast and small.
  Based on the Perl parser.

***************************************************************************/

//#define STANDALONE_TEST

#ifndef STANDALONE_TEST
 #define Uses_snprintf
 #define Uses_string
 #define Uses_stdio
 #define Uses_ctype
 #include <compatlayer.h>
 #include <bufun.h>
#else
 const int MaxLenWith0=256,MaxLen=255,MaxLen_1=254;
 extern char bfBuffer[];
 extern char bfNomFun[];
 extern char bfTempNomFun[];
 typedef void (*tAddFunc)(char *name, int len, int lineStart, int lineEnd);
 #define CLY_snprintf snprintf
 #include <stdio.h>
 #include <ctype.h>
 #include <string.h>
#endif

static unsigned Index,Line,lenWord,Len;
static unsigned char *Buffer;
static int canBeStr=0;

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
void GetUptoStr()
{
 while (Index<Len && Buffer[Index]!='"')
    AddToWord(Buffer[Index]);
}

static
void EatSpaces()
{
 for (; Index<Len && isspace(Buffer[Index]); Index++)
     if (Buffer[Index]=='\n')
        Line++;
}

static
int GetNextValue()
{
 lenWord=0;
 bfBuffer[0]=0;
 while (Index<Len)
   {// Eat spaces
    EatSpaces();
    if (Buffer[Index]=='-' && Buffer[Index+1]=='-')
      { // EOL comment eat upto the EOL
       for (; Index<Len && Buffer[Index]!='\n'; Index++);
       if (Buffer[Index]=='\n')
         {
          Index++;
          Line++;
         }
       continue;
      }
    switch (Buffer[Index])
      {
       case '"': // String, search the end
            if (!canBeStr)
               GetEndOfStr('"');
            else
              {
               Index++;
               GetUptoStr();
               Index++;
               return 1;
              }
            break;
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

void GetFunction(const char *type, tAddFunc AddFunc)
{
 unsigned lenFound,lineFound;
 int proto=0;

 canBeStr=1;
 GetWord();
 canBeStr=0;
 lineFound=Line;
 strcpy(bfNomFun,bfBuffer);
 do
   {
    int nv=GetNextValue();
    if (nv==0)
      {
       if (bfBuffer[0]=='(')
          SearchBalance('(',')');
       else if (bfBuffer[0]==';')
         {
          proto=1;
          break;
         }
      }
    else if (nv==1 && strcasecmp(bfBuffer,"is")==0)
      {
       break;
      }
   }
 while (Index<Len);
 lenFound=CLY_snprintf(bfTempNomFun,MaxLenWith0,"%s [%s%s]",bfNomFun,type,
                       proto ? " Declaration" : "");
 AddFunc(bfTempNomFun,lenFound+1,lineFound,-1,NULL,0);
}

int SearchVHDLStuff(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
 unsigned lenFound,lineFound,funcs=0;
 Index=0; Line=1; Len=len; Buffer=(unsigned char *)buffer;
 while (Index<len)
   {
    GetWord();
    if (strcasecmp(bfBuffer,"entity")==0)
      {
       GetWord();
       lenFound=CLY_snprintf(bfTempNomFun,MaxLenWith0,"%s [Entity]",bfBuffer);
       AddFunc(bfTempNomFun,lenFound+1,Line,-1,NULL,0);
       funcs++;
      }
    else if (strcasecmp(bfBuffer,"architecture")==0)
      {
       GetWord();
       // Name of the architecture
       lineFound=Line;
       strcpy(bfNomFun,bfBuffer);
       // "of"
       GetWord();
       if (strcasecmp(bfBuffer,"of"))
          continue;
       // Name of the entity
       GetWord();
       lenFound=CLY_snprintf(bfTempNomFun,MaxLenWith0,"%s [Architecture of %s]",
                             bfNomFun,bfBuffer);
       AddFunc(bfTempNomFun,lenFound+1,lineFound,-1,NULL,0);
      }
    else if (strcasecmp(bfBuffer,"component")==0)
      {
       GetWord();
       lenFound=CLY_snprintf(bfTempNomFun,MaxLenWith0,"%s [Component]",
                             bfBuffer);
       AddFunc(bfTempNomFun,lenFound+1,Line,-1,NULL,0);
       funcs++;
      }
    else if (strcasecmp(bfBuffer,"function")==0)
      {
       GetFunction("Function",AddFunc);
       funcs++;
      }
    else if (strcasecmp(bfBuffer,"procedure")==0)
      {
       GetFunction("Procedure",AddFunc);
       funcs++;
      }
    else if (strcasecmp(bfBuffer,"package")==0)
      {
       GetWord();
       if (strcasecmp(bfBuffer,"body")==0)
         {
          GetWord();
          lenFound=CLY_snprintf(bfTempNomFun,MaxLenWith0,"%s [Package body]",
                                bfBuffer);
         }
       else
         {
          lenFound=CLY_snprintf(bfTempNomFun,MaxLenWith0,
                                "%s [Package declaration]",bfBuffer);
         }
       AddFunc(bfTempNomFun,lenFound+1,Line,-1,NULL,0);
       funcs++;
      }
    else if (strcasecmp(bfBuffer,"end")==0)
      {// Get the next word to avoid confusing things like "End Entity XXXX"
       // First make sure we have a word
       EatSpaces();
       if (Buffer[Index]!=';')
         {
          canBeStr=1; // For overloaded operators
          GetWord();
          canBeStr=0;
         }
      }
   }
 return funcs;
}

#ifdef STANDALONE_TEST
char bfBuffer[MaxLenWith0];
char bfNomFun[MaxLenWith0];
char bfTempNomFun[MaxLenWith0];

void PrintFunc(char *name, int len, int lineStart, int lineEnd)
{
 printf("%s [%d,%d]\n",name,lineStart,lineEnd);
}

int main(int argc, char *argv[])
{
 printf("VHDL Pseudo-parser, Copyright 2001-2005 by Salvador E. Tropea\n");
 if (argc!=2)
   {
    printf("Use: pvhdl file\n");
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
 SearchVHDLStuff(buffer,len,0,PrintFunc);

 return 0;
}
#endif // STANDALONE
