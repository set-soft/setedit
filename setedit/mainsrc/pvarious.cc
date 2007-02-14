/**[txh]********************************************************************

  Copyright (c) 2001-2005 by Salvador Eduardo Tropea.
  This program is covered by the GPL license.

  Description:
  Parses a .shl, .txi, .pmc or assembler file looking for function
definitions.
  It can be compiled as an standalone program by defining STANDALONE.

***************************************************************************/

#define Uses_stdio
#define Uses_ctype
#define Uses_string
#define Uses_TVCodePage
#define Uses_snprintf
#include <settvuti.h>

#include <bufun.h>

//#define STANDALONE

static unsigned Index,Line,Len;
static unsigned char *Buffer;
typedef unsigned char uchar;

static
void ReplaceCRby0(char *s)
{
 for (; *s && *s!='\n' && *s!='\r'; s++);
 *s=0;
}

static
char *MoveAfterEqual(char *s)
{
 for (; *s && *s!='='; s++);
 if (*s) s++;
 for (; *s && isspace((uchar)*s); s++);
 return s;
}

static
char *MoveAfterColon(char *s)
{
 for (; *s && *s!=':'; s++);
 if (*s) s++;
 for (; *s && isspace((uchar)*s); s++);
 return s;
}

static
void GetLine()
{
 int i=0;
 while (Index<Len && Buffer[Index]!='\n' && i<MaxLen)
    bfBuffer[i++]=Buffer[Index++];
 bfBuffer[i]=0;
 while (Index<Len && Buffer[Index]!='\n') Index++;
 if (Index<Len && Buffer[Index]=='\n') Index++;
}

int SearchSHLDefs(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
 unsigned lineFound=0,funcs=0;
 Index=0; Line=0; Len=len; Buffer=(uchar *)buffer;
 while (Index<len)
   {
    GetLine(); Line++;
    if (strncasecmp(bfBuffer,"Name",4)==0 && !TVCodePage::isAlpha((uchar)bfBuffer[4]))
      {
       char *pos=MoveAfterEqual(bfBuffer);
       ReplaceCRby0(pos);
       strcpy(bfNomFun,pos);
       lineFound=Line;
      }
    else if (strncasecmp(bfBuffer,"End",3)==0)
      {
       AddFunc(bfNomFun,strlen(bfNomFun)+1,lineFound,Line);
       funcs++;
      }
   }
 return funcs;
}

struct Entry
{
 const char *name;
 int len;
};

static
Entry Sections[]=
{
{"chapter",7},
{"section",7},
{"subsection",10},
{"subsubsection",13},
{"top",3},
{"unnumbered",10},
{"unnumberedsec",13},
{"unnumberedsubsec",16},
{"unnumberedsubsubsec",19},
{"appendix",8},
{"appendixsec",11},
{"appendixsubsec",14},
{"appendixsubsubsec",17},
{0,0}};

int SearchTxiSecs(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
 unsigned funcs=0;
 Index=0; Line=0; Len=len; Buffer=(uchar *)buffer;
 while (Index<len)
   {
    GetLine(); Line++;
    if (bfBuffer[0]=='@')
      {
       int i;
       for (i=0; Sections[i].name; i++)
          {
           if (strncmp(Sections[i].name,bfBuffer+1,Sections[i].len)==0 &&
               isspace((uchar)bfBuffer[1+Sections[i].len]))
             {
              char *pos=bfBuffer+1+Sections[i].len;
              for (; *pos && isspace((uchar)*pos); pos++);
              ReplaceCRby0(pos);
              strcpy(bfNomFun,pos);
              int l=strlen(bfNomFun);
              if (l+3+Sections[i].len<MaxLen)
                {
                 strcpy(bfNomFun+l," (");
                 strcpy(bfNomFun+l+2,Sections[i].name);
                 strcpy(bfNomFun+l+2+Sections[i].len,")");
                 l+=3+Sections[i].len;
                }
              AddFunc(bfNomFun,l+1,Line,-1);
              funcs++;
              break;
             }
          }
      }
   }
 return funcs;
}

static
int IsAsmChar(uchar s)
{
 return isalnum(s) || s=='_' || s=='@' || s=='$';
}

static
int GetLabel(int &len)
{
 int i=0;
 while (IsAsmChar(bfBuffer[i]))
   {
    bfNomFun[i]=bfBuffer[i];
    i++;
   }
 bfNomFun[i]=0;
 len=i;
 return bfBuffer[i]==':' && i!=0;
}

int SearchAsmLabels(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
 unsigned funcs=0;
 int l;
 Index=0; Line=0; Len=len; Buffer=(uchar *)buffer;
 while (Index<len)
   {
    GetLine(); Line++;
    if (GetLabel(l))
      {
       AddFunc(bfNomFun,l+1,Line,-1);
       funcs++;
      }
   }
 return funcs;
}

int SearchPMDefs(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
 unsigned lineFound=0,funcs=0,lenNom=0;
 Index=0; Line=0; Len=len; Buffer=(uchar *)buffer;
 while (Index<len)
   {
    GetLine(); Line++;
    ReplaceCRby0(bfBuffer);
    if (strncasecmp(bfBuffer,"Trigger:",8)==0 && isspace((uchar)bfBuffer[8]))
      {
       char *pos=MoveAfterColon(bfBuffer);
       strncpyZ(bfTempNomFun,pos,MaxLenWith0);
       lineFound=Line;
      }
    else if (strncasecmp(bfBuffer,"Name:",5)==0 && isspace((uchar)bfBuffer[5]))
      {
       char *pos=MoveAfterColon(bfBuffer);
       lenNom=CLY_snprintf(bfNomFun,MaxLenWith0,"%s %s",pos,bfTempNomFun);
      }
    else if (lineFound && lenNom && !bfBuffer[0]) // Empty line
      {
       AddFunc(bfNomFun,lenNom+1,lineFound,Line);
       lineFound=lenNom=0;
       funcs++;
      }
   }
 return funcs;
}


int SearchKICADLib(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
 unsigned lineFound=0,funcs=0,lenNom=0;
 Index=0; Line=0; Len=len; Buffer=(uchar *)buffer;
 while (Index<len)
   {
    GetLine(); Line++;
    ReplaceCRby0(bfBuffer);
    if (strncasecmp(bfBuffer,"DEF",3)==0 && isspace((uchar)bfBuffer[3]))
      {
       char *pos=bfBuffer+3;
       for (;*pos && isspace((uchar)*pos); pos++);
       for (lenNom=0;*pos && !isspace((uchar)*pos) && lenNom<(unsigned)MaxLen_1; pos++)
           bfNomFun[lenNom++]=*pos;
       bfNomFun[lenNom++]=0;
       lineFound=Line;
      }
    else if (strncasecmp(bfBuffer,"ENDDEF",6)==0 &&
             (bfBuffer[6]==0 || isspace((uchar)bfBuffer[6])))
      {
       AddFunc(bfNomFun,lenNom,lineFound,Line);
       lineFound=lenNom=0;
       funcs++;
      }
   }
 return funcs;
}

static
int IsMakeChar(uchar s)
{
 return isalnum(s) || s=='_' || s=='-';
}

static
int GetMakeLabel(int &len)
{
 int i=0;
 while (IsMakeChar(bfBuffer[i]))
   {
    if (i<MaxLen)
       bfNomFun[i]=bfBuffer[i];
    i++;
   }
 bfNomFun[i]=0;
 len=i;
 return bfBuffer[i]==':' && i!=0;
}

int SearchMakeLabels(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
 unsigned funcs=0;
 int l;
 Index=0; Line=0; Len=len; Buffer=(uchar *)buffer;
 while (Index<len)
   {
    GetLine(); Line++;
    if (GetMakeLabel(l))
      {
       AddFunc(bfNomFun,l+1,Line,-1);
       funcs++;
      }
   }
 return funcs;
}


static
char *GetHTMLAnchor(int &len, char *buffer)
{
 char *pos, *s, delim;

 s=buffer;
 while ((pos=strstr(s,"name"))!=NULL)
   {
    for (s=pos+4; *s && isspace((uchar)*s); s++);
    if (*s=='=')
      {
       for (s++; *s && isspace((uchar)*s); s++);
       if (*s=='\'' || *s=='"')
          delim=*s;
       else
          delim=0;
       s++;
       int l;
       for (l=0; l<MaxLen && ((delim && s[l]!=delim) ||
            (!delim && isspace((uchar)s[l]))); l++)
           bfNomFun[l]=s[l];
       bfNomFun[l]=0;
       s+=l+1;
       len=l;
       return s;
      }
   }
 return NULL;
}

int SearchHTMLAnchors(char *buffer, unsigned len, int mode, tAddFunc AddFunc)
{
 unsigned funcs=0;
 int l;

 Index=0; Line=0; Len=len; Buffer=(uchar *)buffer;
 while (Index<len)
   {
    GetLine(); Line++;
    char *buffer=bfBuffer;
    while ((buffer=GetHTMLAnchor(l,buffer))!=NULL)
      {
       AddFunc(bfNomFun,l+1,Line,-1);
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
 //printf(".SHL Definitions Parser, Copyright 2001 by Salvador E. Tropea\n");
 //printf(".TXI Definitions Parser, Copyright 2001 by Salvador E. Tropea\n");
 printf("Assembler Labels Parser, Copyright 2001 by Salvador E. Tropea\n");
 if (argc!=2)
   {
    printf("Use: pvarious file\n");
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
 //SearchSHLDefs(buffer,len,0,PrintFunc);
 //SearchTxiSecs(buffer,len,0,PrintFunc);
 SearchAsmLabels(buffer,len,0,PrintFunc);

 return 0;
}
#endif // STANDALONE
