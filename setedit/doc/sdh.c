/*****************************************************************************

 SET's Documentation Helper (SDH) Copyright(c) 1997-2004
 by Salvador Eduardo Tropea

 This program is used to automate the creation of .info, .txt, .html, .dvi
and .ps files from one source file (.tx).

 See copyrigh file for details. Normally located in the root of the
distribution. This file can be distributed under the terms of the GPL
license.

*****************************************************************************/

#define Uses_stdlib
#define Uses_stdio
#define Uses_string
#define Uses_ctype
#define Uses_limits
#define Uses_unistd
#define Uses_getcwd
#include <compatlayer.h>

/*
 returns:
 1 => bad command line.
 2 => Can't open input file.
 3 => Hierarchy error.
 4 => Can't create output
 5 => Error when spawning
 6 => Cross reference error
*/
//char *outf=NULL;
char *inf=NULL;
FILE *fi,*fo,*fo2,*fo3;
#define MAX_BL 200
char bl[MAX_BL];
char b2[MAX_BL];
int TopFound=0;
int CurLevel=0;
int KindOfLevel;
int PutPoint=0;
#define MAX_IND 4
char *IndexNames[MAX_IND];
char *FilesIndex[MAX_IND];
char *FilesIndexTXT[MAX_IND];
int   IndexFound[MAX_IND];
int cantIndices=0;
int makeClean=0;
char *OutPathOrig=NULL;
char  OutPath[PATH_MAX];
#define MAX_SEARCH 10
char *SeachPaths[MAX_SEARCH];
int   CantSearchs=0;
char  Include[PATH_MAX];
int   NoNumberInInf=0;

//#define DEBUG

// Kind of numbers
// The top isn't numbered and is special
#define K_TOP     0
// Numbered chapters, sections, subsections and subsubsections
#define K_NORMAL  1
// Unnumbered sections
#define K_UNNUM   2
// Appendices with roman numbers?, I'll implement letters first
#define K_ROMAN   3

#define F_INFO  1
#define F_TXT   2
#define F_HTML  4
#define F_DVI   8
#define F_PS   16
#define F_ALL  31
int outFormats=0;

typedef struct
{
 int Level;
 int Type;
 char *name;
 char *Number;
} aNode;

#define Inic_Nodes 100
#define Delta_Nodes 20
aNode *Nodes=NULL;
int cantNodes=0;
int cantAllocNodes=0;

char *SearchNode(char *name);

void PutCopy(void)
{
 printf("SET's Documentation Helper (SDH)\n"
        "Copyright (c) 1997-2004 by Salvador Eduardo Tropea\n\n");
}

void PutHelp(int ret)
{
 PutCopy();
 printf("sdh -i input_file [-p] [-n \"name\" file file_txt] [-c] [-oihtdpa] [-I path]\n"
        "   [-O path]\n\n"
        "in_file is the BASE name of the documentation (.tx)\n"
        "-p: Put X. instead of X in chapter numbers\n"
        "-n \"name\" file file_txt: for indices\n"
        "-N: don't put numbers in .info file, let makeinfo do it\n"
        "-c: clean, deletes intermediate files\n"
        "-oX: output format, i=.info h=.html t=.txt d=.dvi p=.ps a=ALL\n"
        "-I path: adds the path to the search paths\n"
        "-O path: output path\n"
        "\n");
 if (ret)
    exit(ret);
}

void PutPathIn(char *name,char *path)
{
 char *v;

 if (path)
   {
    strcpy(name,path);
    v=name+strlen(name)-1;
    if (*v!='/' && *v!='\\')
      {
       *(v+1)='/';
       *(v+2)=0;
      }
   }
 else
   *name=0;
}

void ParseCommandLine(int argc, char *argv[])
{
 int i,OneMore,ThreeMore,j;

 if (argc<2)
    PutHelp(1);
 for (i=1; i<argc; i++)
    {
     OneMore=i<(argc-1);
     ThreeMore=i<(argc-3);
     if (*argv[i]=='-')
       {
        switch (argv[i][1])
          {
           case 'o':
                for (j=2; argv[i][j]; j++)
                   {
                    switch (argv[i][j])
                      {
                       case 'i':
                            outFormats|=F_INFO;
                            break;
                       case 't':
                            outFormats|=F_TXT;
                            break;
                       case 'h':
                            outFormats|=F_HTML;
                            break;
                       case 'd':
                            outFormats|=F_DVI;
                            break;
                       case 'p':
                            outFormats|=F_PS;
                            break;
                       case 'a':
                            outFormats|=F_ALL;
                            break;
                       default:
                            printf("Invalid output format\n");
                            exit(1);
                      }
                   }
                break;
           case 'i':
                if (OneMore)
                  {
                   inf=argv[i+1];
                   i++;
                  }
                else
                  PutHelp(1);
                break;
           case 'n':
                if (ThreeMore)
                  {
                   if (cantIndices==MAX_IND)
                     {
                      printf("Only %d indices are supported, please recompile\n",MAX_IND);
                      exit(3);
                     }
                   IndexNames[cantIndices]=argv[i+1];
                   FilesIndex[cantIndices]=argv[i+2];
                   FilesIndexTXT[cantIndices]=argv[i+3];
                   cantIndices++;
                   i+=3;
                  }
                else
                  PutHelp(1);
                break;
           case 'N':
                NoNumberInInf=1;
                break;
           case 'p':
                PutPoint=1;
                break;
           case 'c':
                makeClean=1;
                break;
           case 'O':
                if (OneMore)
                   OutPathOrig=argv[++i];
                else
                   PutHelp(1);
                break;
           case 'I':
                if (OneMore)
                  {
                   if (CantSearchs==MAX_SEARCH)
                     {
                      printf("Only 10 -I paths supported, recompile\n");
                      exit(1);
                     }
                   SeachPaths[CantSearchs++]=argv[++i];
                  }
                else
                  PutHelp(1);
                break;
          }
       }
     else
       PutHelp(1);
    }
 if (!inf)
   {
    printf("No input file\n");
    PutHelp(1);
   }
 if (!outFormats)
   {
    PutHelp(0);
    printf("No output format defined use -oX\n");
    exit(1);
   }
 PutPathIn(OutPath,OutPathOrig);
}

FILE *TryOpenFile(char *path,char *name,char *ext)
{
 char n[PATH_MAX];
 PutPathIn(n,path);
 strcat(n,name);
 strcat(n,ext);
 return fopen(n,"rt");
}

#define TryWith(path) { f=TryOpenFile(path,inf,ext); if(f) return f; }

FILE *OpenInFile(char *ext)
{
 FILE *f=NULL;
 int i;

 TryWith(NULL);
 TryWith(OutPathOrig);
 for (i=0; i<CantSearchs; i++)
     TryWith(SeachPaths[i]);
 printf("Can't open %s%s\n",inf,ext);
 exit(2);
 return f;
}


void CreateFile(char *ext)
{
 char name[PATH_MAX];
 PutPathIn(name,OutPathOrig);
 strcat(name,inf);
 strcat(name,ext);
 fo=fopen(name,"wt");
 if (!fo)
   {
    printf("Can't create %s\n",name);
    exit(4);
   }
}

FILE *CreateAuxFile(char *nombre)
{
 FILE *f;
 char name[PATH_MAX];
 PutPathIn(name,OutPathOrig);
 strcat(name,nombre);
 f=fopen(name,"wt");
 if (!f)
   {
    printf("Can't create %s\n",name);
    exit(4);
   }
 return f;
}

int GetLevel(void)
{
#define s(x,l,k) if (strncmp(bl,"@"#x,sizeof(#x)) == 0) {KindOfLevel=k; return l;}
  s(chapter,1,K_NORMAL);
  s(section,2,K_NORMAL);
  s(subsection,3,K_NORMAL);
  s(subsubsection,4,K_NORMAL);
  s(top,0,K_TOP);
  s(unnumbered,1,K_UNNUM);
  s(unnumberedsec,2,K_UNNUM);
  s(unnumberedsubsec,3,K_UNNUM);
  s(unnumberedsubsubsec,4,K_UNNUM);
  s(appendix,1,K_ROMAN);
  s(appendixsec,2,K_ROMAN);
  s(appendixsubsec,3,K_ROMAN);
  s(appendixsubsubsec,4,K_ROMAN);
#undef s
  return -1;
}

void AddNode(int Level)
{
 char *s=bl,*p;

 if (!TopFound)
   {
    if (Level!=0)
      {
       printf("Use @top as the first entry.\n");
       if (Level==-1)
          printf("Don't use @node Top.\n");
       exit(3);
      }
    TopFound=1;
   }

 if (cantNodes==0)
   {
    Nodes=(aNode *)malloc(Inic_Nodes*sizeof(aNode));
    cantAllocNodes=Inic_Nodes;
   }
 else
   if (cantNodes==cantAllocNodes)
     {
      cantAllocNodes+=Delta_Nodes;
      Nodes=(aNode *)realloc(Nodes,cantAllocNodes*sizeof(aNode));
     }
 if (Level==-1)
   {
    Level=CurLevel;
    KindOfLevel=K_NORMAL; // What can I do?
   }
 Nodes[cantNodes].Level=Level;
 for (;*s && !ucisspace(*s); s++);
 for (;*s &&  ucisspace(*s); s++);
 for (p=s;*p && *p!='\n'; p++);
 if (!*s)
   {
    printf("Empty sectioning command:\n%s\n",bl);
    exit(3);
   }
 *p=0;
 Nodes[cantNodes].name=strdup(s);
 Nodes[cantNodes].Type=KindOfLevel;
 cantNodes++;
 if (Level>CurLevel && (Level-CurLevel)>1)
   {
    printf("You are jumping from level %d to level %d!",CurLevel,Level);
    exit(3);
   }
 CurLevel=Level;
}

void ProcessLine(void)
{
 int Level=GetLevel();

 if (strncmp(bl,"@node",5)==0)
   {
    printf("Don't use @node, use @unnumbered instead\n");
    exit(3);
    //AddNode(-1);
   }
 if (Level>=0)
   {
    AddNode(Level);
    return;
   }
}

void ReadLine(void)
{
 fgets(bl,MAX_BL,fi);
}

void Rewind(void)
{
 rewind(fi);
}

void ScanFile(void)
{
 do
   {
    ReadLine();
    ProcessLine();
   }
 while (!feof(fi));
 Rewind();
}

#ifdef DEBUG
void DumpNodes(void)
{
 int i,j;
 for (i=0; i<cantNodes; i++)
    {
     j=Nodes[i].Level;
     if (j==-1)
        printf("-----> ");
     else
        for (;j>=0;--j)
            printf("   ");
     printf("%s Name: %s (%d)\n",Nodes[i].Number,Nodes[i].name,Nodes[i].Type);
    }
}
#else
#define DumpNodes()
#endif

char *emptyStr=" ";

char *ProxNodeOf(int i)
{
 if ((i+1)<cantNodes)
   {
    int lev=Nodes[i].Level,nlev;
    for (i++; i<cantNodes; i++)
       {
        nlev=Nodes[i].Level;
        if (lev==nlev)
           return Nodes[i].name;
        if (nlev<lev)
           break;
       }
   }
 return emptyStr;
}

void GenMenu(int i, FILE *f, int use_sp_end)
{
 int lev=Nodes[i].Level;
 fputs("@menu\n",f);
 for (;i<cantNodes; i++)
    {
     if (Nodes[i].Level==lev)
        fprintf(f,"@mitem{%s}\n",Nodes[i].name);
     else
       if (Nodes[i].Level<lev)
          break;
    }
 if (use_sp_end)
    fputs("@end-menu\n\n",f);
 else
    fputs("@end menu\n\n",f);
}

void InsertNumber(char *s,char *n,FILE *f)
{
 for (;*s && !ucisspace(*s); s++) fputc(*s,f);
 for (;*s &&  ucisspace(*s); s++) fputc(*s,f);
 fputs(n,f);
 fputs(s,f);
}

int HTMLInMenu;

void HTMLPrep(char *s,FILE *f)
{
 char *p,*s1;
 #define c(x) (strncmp(s,"@"x,sizeof(x))==0)
 #define r(x) if (strncmp(s,"@end "#x,sizeof(#x)+4)==0) fputs("@end-"#x"\n",f); else
 if (c("menu"))
   {
    HTMLInMenu=1;
    //fputs("@menu-\n",f);
    fputs(s,f);
   }
 else
 if (HTMLInMenu && s[0]=='*')
   {
    int found;
    for (s++; *s && ucisspace(*s); s++);
    for (p=s; *p && !(*p==':' && *(p+1)==':'); p++);
    found=*p==':';
    *p=0;
    s1=s;
    if (found)
      {
       for (s=p+2; *s && ucisspace(*s); s++);
       if (*s)
         {
          for (p=s; *p && *p!='\n'; p++);
          *p=0;
         }
      }
    else
       s=emptyStr;
    if (s[0])
       fprintf(f,"@mitem2{%s,%s}\n",s1,s);
    else
       fprintf(f,"@mitem{%s}\n",s1);
   }
 else
 if (c("end-menu") || c("end menu"))
   {
    HTMLInMenu=0;
    fputs("@end-menu\n",f);
   }
 else
 r(format)
 r(table)
 r(itemize)
 r(enumerate)
 r(example)
 r(smallexample)
 r(display)
 r(quotation)
 r(menu)
 fputs(s,f);
 #undef r
 #undef s
 #undef c
}

void ReplaceEndTable(char *b, FILE *f)
{
 if (strncmp(b,"@endtable-{}",12)==0)
    fputs("@end itemize\n@p{}\n",f);
 else
    fputs(b,f);
 if (strncmp(b,"@table",6)==0)
    fputs("@itemize @bullet\n",f);
}

char *dirStr="(dir)";

void GenerateTXI_NUM(void)
{
 int i,nlev,levant=0,lev;
 char *prev=dirStr;
 char *up[6],*name,*aprev[5];
 HTMLInMenu=0;

 CreateFile(".txi");
 fo2=fo;
 // Special hack! a .txi but with numbers, for .info using makeinfo 4.7 or newer
 // where we can't use @end xxx inside a macro.
 CreateFile(".nut");
 fo3=fo;
 CreateFile(".num");
 up[0]=dirStr;
 aprev[0]=dirStr;
 i=0;
 do
   {
    ReadLine();
    lev=GetLevel();
    if (lev>=0)
      {
       nlev=Nodes[i].Level;
       name=nlev ? Nodes[i].name : "Top";
       if (nlev>levant)
         {
          aprev[nlev]=prev;
          GenMenu(i,fo,1);
          GenMenu(i,fo2,0);
          GenMenu(i,fo3,0);
         }
       fprintf(fo2,"@node %s, %s, %s, %s\n",name,ProxNodeOf(i),aprev[nlev],up[nlev]);
       fprintf(fo3,"@node %s, %s, %s, %s\n",name,ProxNodeOf(i),aprev[nlev],up[nlev]);
       fprintf(fo,"@node-{%s, %s, %s, %s}\n",name,ProxNodeOf(i),aprev[nlev],up[nlev]);
       prev=name;
       aprev[nlev]=prev;
       up[nlev+1]=name;
       levant=nlev;
       i++;
      }
    ReplaceEndTable(bl,fo2);
    if (lev<0)
      {
       HTMLPrep(bl,fo);
       ReplaceEndTable(bl,fo3);
      }
    else
      {
       InsertNumber(bl,Nodes[i-1].Number,fo);
       if (NoNumberInInf)
          fputs(bl,fo3);
       else
          InsertNumber(bl,Nodes[i-1].Number,fo3);
      }
    if (lev>=0 && strcmp(name,"Top"))
      {
       fprintf(fo2,"@cindex %s\n",name);
       fprintf(fo3,"@cindex %s\n",name);
       fprintf(fo,"@cindex %s\n",name);
      }
   }
 while (!feof(fi));
 fclose(fo);
 fclose(fo2);
 fclose(fo3);
 Rewind();
}

char *emptyStr2="";

void Numerate(void)
{
 int num[2][6];
 int i,lev,levant=0,type,sub,j;
 char b[80],b1[20];

 memset(num,0,6*2*sizeof(int));
 for (i=0; i<cantNodes; i++)
    {
     type=Nodes[i].Type;
     if (type==K_UNNUM || type==K_TOP)
       {
        Nodes[i].Number=emptyStr2;
        continue;
       }
     sub=type==K_ROMAN ? 1 : 0;
     lev=Nodes[i].Level;
     if (lev<levant)
        do
          {
           num[sub][levant--]=0;
          }
        while(levant>lev);
     num[sub][lev]++;
     if (sub)
        sprintf(b,"%c",'A'+num[sub][1]-1);
     else
        sprintf(b,"%d",num[sub][1]);
     for (j=2; num[sub][j]; j++)
        {
         sprintf(b1,".%d",num[sub][j]);
         strcat(b,b1);
        }
     if (PutPoint && num[sub][2]==0)
        strcat(b,". ");
     else
        strcat(b,emptyStr);
     Nodes[i].Number=strdup(b);
     levant=lev;
    }
}

void GenerateCTX(void)
{
 int i,nlev,curlev=0;

 CreateFile(".ctx");
 for (i=1; i<cantNodes; i++)
    {
     nlev=Nodes[i].Level;
     if (nlev>curlev)
        fputs("<MENU>\n",fo);
     else
       if (nlev<curlev)
          do
            {
             fputs("</MENU>\n",fo);
            }
          while(--curlev>nlev);
     fprintf(fo,"@w{<LI> <A HREF=\"#%s\">%s%s</A>}\n",Nodes[i].name,Nodes[i].Number,Nodes[i].name);
     curlev=nlev;
    }
 if (curlev)
    do
      {
       fputs("</MENU>\n",fo);
      }
    while(--curlev);
 fclose(fo);
}

void GenerateINFO(void)
{
 int error;
 char s[6*PATH_MAX];
 // No validate because makeinfo can't see through mitem{} (another weak point)
 sprintf(s,"makeinfo %s --no-split --fill-column 75 --no-validate -o %s%s.inf %s%s.nut",Include,OutPath,inf,OutPath,inf);
 //sprintf(s,"makertf %s --no-split --fill-column 75 --no-validate -o %s%s.inf %s%s.num",Include,OutPath,inf,OutPath,inf);
 error=system(s);
 if (error)
   {
    printf("Failed executing:\n%s\n",s);
    exit(5);
   }
}

char *Conv[256]={
NULL, // 0
NULL, // 1
NULL, // 2
NULL, // 3
NULL, // 4
NULL, // 5
NULL, // 6
NULL, // 7
NULL, // 8
NULL, // 9
NULL, // 10
NULL, // 11
NULL, // 12
NULL, // 13
NULL, // 14
NULL, // 15
NULL, // 16
NULL, // 17
NULL, // 18
NULL, // 19
NULL, // 20
NULL, // 21
NULL, // 22
NULL, // 23
NULL, // 24
NULL, // 25
NULL, // 26
NULL, // 27
NULL, // 28
NULL, // 29
NULL, // 30
NULL, // 31
NULL, // 32
NULL, // 33
NULL, // 34
NULL, // 35
NULL, // 36
NULL, // 37
NULL, // 38
NULL, // 39
NULL, // 40
NULL, // 41
NULL, // 42
NULL, // 43
NULL, // 44
NULL, // 45
NULL, // 46
NULL, // 47
NULL, // 48
NULL, // 49
NULL, // 50
NULL, // 51
NULL, // 52
NULL, // 53
NULL, // 54
NULL, // 55
NULL, // 56
NULL, // 57
NULL, // 58
NULL, // 59
"@lt{}", // 60
NULL, // 61
"@gt{}", // 62
NULL, // 63
"@@", // 64
NULL, // 65
NULL, // 66
NULL, // 67
NULL, // 68
NULL, // 69
NULL, // 70
NULL, // 71
NULL, // 72
NULL, // 73
NULL, // 74
NULL, // 75
NULL, // 76
NULL, // 77
NULL, // 78
NULL, // 79
NULL, // 80
NULL, // 81
NULL, // 82
NULL, // 83
NULL, // 84
NULL, // 85
NULL, // 86
NULL, // 87
NULL, // 88
NULL, // 89
NULL, // 90
NULL, // 91
NULL, // 92
NULL, // 93
NULL, // 94
NULL, // 95
NULL, // 96
NULL, // 97
NULL, // 98
NULL, // 99
NULL, // 100
NULL, // 101
NULL, // 102
NULL, // 103
NULL, // 104
NULL, // 105
NULL, // 106
NULL, // 107
NULL, // 108
NULL, // 109
NULL, // 110
NULL, // 111
NULL, // 112
NULL, // 113
NULL, // 114
NULL, // 115
NULL, // 116
NULL, // 117
NULL, // 118
NULL, // 119
NULL, // 120
NULL, // 121
NULL, // 122
"@{", // 123
NULL, // 124
"@}", // 125
NULL, // 126
NULL, // 127
NULL, // 128
"@value{uuml}", // 129
"@value{eacute}", // 130
NULL, // 131
"@value{auml}", // 132
"@value{agrave}", // 133
NULL, // 134
NULL, // 135
NULL, // 136
"@value{euml}", // 137
"@value{egrave}", // 138
"@value{iuml}", // 139
NULL, // 140
"@value{igrave}", // 141
NULL, // 142
NULL, // 143
NULL, // 144
NULL, // 145
NULL, // 146
NULL, // 147
"@value{ouml}", // 148
"@value{ograve}", // 149
NULL, // 150
"@value{ugrave}", // 151
NULL, // 152
NULL, // 153
"@value{Uuml}", // 154
NULL, // 155
NULL, // 156
NULL, // 157
NULL, // 158
NULL, // 159
"@value{aacute}", // 160
"@value{iacute}", // 161
"@value{oacute}", // 162
"@value{uacute}", // 163
"@value{ntilde}", // 164
"@value{Ntilde}", // 165
NULL, // 166
NULL, // 167
"@questiondown{}", // 168
NULL, // 169
NULL, // 170
NULL, // 171
NULL, // 172
"@exclamdown{}", // 173
NULL, // 174
NULL, // 175
NULL, // 176
NULL, // 177
NULL, // 178
NULL, // 179
NULL, // 180
NULL, // 181
NULL, // 182
NULL, // 183
NULL, // 184
NULL, // 185
NULL, // 186
NULL, // 187
NULL, // 188
NULL, // 189
NULL, // 190
NULL, // 191
NULL, // 192
NULL, // 193
NULL, // 194
NULL, // 195
NULL, // 196
NULL, // 197
NULL, // 198
NULL, // 199
NULL, // 200
NULL, // 201
NULL, // 202
NULL, // 203
NULL, // 204
NULL, // 205
NULL, // 206
NULL, // 207
NULL, // 208
NULL, // 209
NULL, // 210
NULL, // 211
NULL, // 212
NULL, // 213
NULL, // 214
NULL, // 215
NULL, // 216
NULL, // 217
NULL, // 218
NULL, // 219
NULL, // 220
NULL, // 221
NULL, // 222
NULL, // 223
NULL, // 224
NULL, // 225
NULL, // 226
NULL, // 227
NULL, // 228
NULL, // 229
NULL, // 230
NULL, // 231
NULL, // 232
NULL, // 233
NULL, // 234
NULL, // 235
NULL, // 236
NULL, // 237
NULL, // 238
NULL, // 239
NULL, // 240
NULL, // 241
NULL, // 242
NULL, // 243
NULL, // 244
NULL, // 245
NULL, // 246
NULL, // 247
NULL, // 248
NULL, // 249
NULL, // 250
NULL, // 251
NULL, // 252
NULL, // 253
NULL, // 254
NULL  // 255
};

static void ConvASCII(unsigned char *s, unsigned char *e, FILE *f)
{
 for (;s<e; s++)
     if (Conv[*s])
        fputs(Conv[*s],f);
     else
        fputc(*s,f);
}

static void ConvertToHTML(char *s,FILE *f)
{
 char *start1,*end1;
 char *start2,*end2;

// f=stdout;
 if (*s!='*')
    return;
 for (end1=start1=s+2; *end1 && *end1!=':'; end1++);
 if (*end1!=':')
    return;
 for (start2=end1+1; *start2 && ucisspace(*start2); start2++);
 if (!*start2)
    return;
 for (end2=start2; *end2 && *end2!='.'; end2++);
 if (*end2!='.')
    return;
 fprintf(f,"<li><A HREF=\"#@w{");
 ConvASCII(start2,end2,f);
 fputs("}\"> @w{",f);
 ConvASCII(start1,end1,f);
 fputs("} </A>@*\n",f);
}

static void ConvertToTXT(char *s,FILE *f)
{
 char *p;

 for (p=s; *p && *p!=':'; p++);
 if (!*p)
    return;
 #if 0
 if (/*p[-1]=='>' || p[-1]=='<' || */*p=='/')
    return;
 else
    printf("%s",p);
 #endif
 for (p++; *p && ucisspace(*p); p++);
 if (!*p)
    return;
 ConvASCII(s,p,f);
 s=p;
 for (;*p && *p!='.'; p++);
 *p=0;
 fputs(SearchNode(s),f);
 fputs("\n",f);
}

int NextNode(FILE *f)
{
 char *pos,*s;
 while (*bl!=31 && !feof(f))
   {
    fgets(bl,MAX_BL,f);
   }
 if (feof(f))
    return 0;
 do
   {
    fgets(bl,MAX_BL,f);
   }
 while (ucisspace(*bl) && !feof(f));
 pos=strstr(bl,"Node:");
 if (!pos)
    return 0;
 for (pos+=6; *pos && ucisspace(*pos); pos++);
 for (s=bl; *pos!=','; pos++,s++) *s=*pos;
 *s=0;
 return 1;
}

void GenerateIDX(int ForTXT)
{
 char name[PATH_MAX],*n;
 FILE *f,*o;
 int i;
 void (*Convert)(char *s,FILE *f);

 Convert=ForTXT ? ConvertToTXT : ConvertToHTML;
 if (!cantIndices)
    return;
 memset(IndexFound,0,MAX_IND*sizeof(int));
 f=OpenInFile(".inf");
 if (!f)
   {
    printf("Can't open %s\n",name);
    exit(2);
   }
 *bl=0;
 do
   {
    if (NextNode(f))
      {
       for (i=0; i<cantIndices; i++)
          {
           if (!IndexFound[i] && strcmp(bl,IndexNames[i])==0)
             {
              n=ForTXT ? FilesIndexTXT[i]: FilesIndex[i];
              o=CreateAuxFile(n);
              if (!o)
                {
                 printf("Can't create %s\n",n);
                 exit(4);
                }
              do
                {
                 fgets(bl,MAX_BL,f);
                 if (*bl=='*' && *(bl+1)!='*' && strncmp(bl,"* Menu:",7)!=0)
                    break;
                }
              while (!feof(f));
              do
                {
                 Convert(bl,o);
                 fgets(bl,MAX_BL,f);
                 if (*bl!='*')
                    break;
                }
              while(!feof(f));
              fclose(o);
              IndexFound[i]=1;
              break;
             }
          }
      }
   }
 while(!feof(f));
 fclose(f);
}

void GenerateHTML(void)
{
 int error;
 char s[6*PATH_MAX];
 sprintf(s,"makeinfo %s --no-split --fill-column 200 --no-headers --no-split --no-validate -Dhtml -o %s%s.html %s%s.tx3",Include,OutPath,inf,OutPath,inf);
 error=system(s);
 if (error)
   {
    printf("Failed executing:\n%s\n",s);
    exit(5);
   }
}

void GenerateC_IDX(void)
{
 int i,j;
 char *name="contents.idx";
 FILE *f;

 f=CreateAuxFile(name);
 if (!f)
   {
    printf("Can't create %s\n",name);
    exit(4);
   }

 for (i=0; i<cantNodes; i++)
    {
     j=Nodes[i].Level;
     if (!j)
        continue;
     if (j!=1)
        fprintf(f,"  ");
     fprintf(f,"%s%s\n",Nodes[i].Number,Nodes[i].name);
    }
 fclose(f);
}

char *NameLevel[5] =
{
 "Top!",
 "Chapter",
 "Section",
 "Section",
 "Section"
};


char *SearchNode(char *name)
{
 int i;
 for (i=0; i<cantNodes; i++)
    {
     if (strcmp(name,Nodes[i].name)==0)
       {
        i=sprintf(b2,"%s %s",NameLevel[Nodes[i].Level],Nodes[i].Number);
        b2[i-1]=0;
        return b2;
       }
    }
 printf("Cross reference to %s doesn't exists\n",name);
 exit(6);
}

void GenerateTX1(void)
{
 char *pos,*end,*s,v;
 int i=0;

 CreateFile(".tx1");
 do
   {
    ReadLine();
    if (feof(fi))
       break;
    // Here we need real nodes again, sorry
    if (strncmp(bl,"@node-{",7)==0)
      {
       fprintf(fo,"@node %s\n",Nodes[i++].name);
       continue;
      }
    s=bl;
    do
      {
       char *refSt="@x{";
       pos=strstr(s,refSt);
       if (!pos)
         {
          refSt="@xp{";
          pos=strstr(s,refSt);
         }
       if (pos)
         {
          *pos=0;
          fputs(s,fo);
          fputs(refSt,fo);
          for (pos+=strlen(refSt); *pos && ucisspace(*pos); pos++);
          for (end=pos; *end && *end!=',' && *end!='}'; end++)
              if (*end=='@')
                {
                 if (*(end+1)=='@')
                    end+=2;
                 else
                    end++;
                }
          if (!*end)
            {
             printf("Error, please don't cut a line containing a cross reference\n");
             exit(6);
            }
          v=*end;
          *end=0;
          fputc('"',fo);
          fputs(pos,fo);
          fputs("\" ",fo);
          fputs(SearchNode(pos),fo);
          fputc(v,fo);
          s=end+1;
         }
       else
          fputs(s,fo);
      }
    while(pos);
   }
 while(!feof(fi));
 fclose(fo);
}

void GenerateTX3(void)
{
 char *pos,*end,*s,v;
 int i=0;

 CreateFile(".tx3");
 do
   {
    ReadLine();
    if (feof(fi))
       break;
    s=bl;
    #define t(x) if (strncmp(s,"@"#x,sizeof(#x))==0) fputs("@"#x"-\n",fo); else
    #define c(x) if (strncmp(s,x,sizeof(x)-1)==0)
    c("@format")
       fputs("@exdent <pre>@*\n@format\n",fo);
    else c("@end-format")
       fputs("@end format\n@exdent </pre>@*\n@*\n",fo);
    else c("@display")
       fputs("@exdent <pre>@*\n@display\n",fo);
    else c("@end-display")
       fputs("@end display\n@exdent </pre>@*\n@*\n",fo);
    else c("@example")
       fputs("@exdent <pre>@*\n@example\n",fo);
    else c("@end-example")
       fputs("@end example\n@exdent </pre>@*\n@*\n",fo);
    else c("@smallexample")
       fputs("@exdent <pre>@*\n@smallexample\n",fo);
    else c("@end-smallexample")
       fputs("@end smallexample\n@exdent </pre>@*\n@*\n",fo);
    else c("@quotation")
       fputs("@exdent <blockquote>@*\n@quotation\n",fo);
    else c("@end-quotation")
       fputs("@end quotation\n@exdent </blockquote>@*\n@*\n",fo);
    else t(menu)
    fputs(s,fo);
    #undef t
    #undef c
   }
 while(!feof(fi));
 fclose(fo);
}

/*void GenerateTXT(void)
{
 int i,j;
 if (!cantIndices)
   {
    strcpy(bl,inf);
    strcat(bl,".tx2");
    strcpy(b2,inf);
    strcat(b2,".txt");
    fclose(fi);
    rename(bl,b2);
    return;
   }
 // Now we can't search for the indices by node
 for (i=0; i<cantIndices; i++)
    {
     for (j=0; j<cantNodes; j++)
         if (strcmp(IndexNames[i],Nodes[j].name)==0)
            break;
     if (j==cantNodes)
       {
        printf("Unresolved index: %s\n",IndexNames[i]);
        exit(7);
       }
     sprintf(bl,"%s%s",Nodes[j].Number,Nodes[j].name);
     IndexNames[i]=strdup(bl);
    }
 memset(IndexFound,0,MAX_IND*sizeof(int));
 CreateFile(".txt");
 do
   {
    ReadLine();
    if (feof(fi))
       break;
    if (strcmp
   }
 while(!feof(fi));
 fclose(fo);
} */


void GenerateTXT(void)
{
 int error;
 char s[6*PATH_MAX];
 sprintf(s,"makeinfo %s --no-split --fill-column 78 --no-headers --no-split --no-validate -Dtext -o %s%s.txt %s%s.tx1",Include,OutPath,inf,OutPath,inf);
 error=system(s);
 if (error)
   {
    printf("Failed executing:\n%s\n",s);
    exit(5);
   }
}

void DeleteFile(char *ext)
{
 char name[PATH_MAX];
 PutPathIn(name,OutPathOrig);
 strcat(name,inf);
 strcat(name,ext);
 unlink(name);
}

void DeleteFileName(char *n)
{
 char name[PATH_MAX];
 PutPathIn(name,OutPathOrig);
 strcat(name,n);
 unlink(name);
}

void GenerateTeX(void)
{
 int error;
 char s[8*PATH_MAX];
 char path[PATH_MAX];

 sprintf(s,"makeinfo %s --no-split --no-validate --no-split -E %s%s -Ddvi -o %s%s.tx2 %s%s.txi",Include,OutPath,inf,OutPath,inf,OutPath,inf);
 error=system(s);
 if (error)
   {
    printf("Failed executing:\n%s\n",s);
    exit(5);
   }
 if (OutPathOrig)
   {
    getcwd(path,PATH_MAX);
    chdir(OutPathOrig);
   }
 sprintf(s,"tex %s",inf);
 puts(s);
 system(s);
 puts(s);
 sprintf(s,"texindex %s.??",inf);
 system(s);
 puts(s);
 sprintf(s,"tex %s",inf);
 system(s);
 if (OutPathOrig)
    chdir(path);
}

void GeneratePS(void)
{
 int error;
 char s[PATH_MAX];
 sprintf(s,"dvips -o %s.ps %s.dvi",inf,inf);
 error=system(s);
 if (error)
   {
    printf("Failed executing:\n%s\n",s);
    exit(5);
   }
}

int main(int argc, char *argv[])
{
 int i;

 ParseCommandLine(argc,argv);
 Include[0]=0;
 for (i=0; i<CantSearchs; i++)
    {
     strcat(Include," -I ");
     strcat(Include,SeachPaths[i]);
    }
 if (OutPathOrig)
   {
    strcat(Include," -I ");
    strcat(Include,OutPathOrig);
   }
 fi=OpenInFile(".tx");
 // Collect the names of the chapters, sections, etc.
 ScanFile();
 // Create the numeration for each one
 Numerate();
 DumpNodes();
 // Generate a .TXI, it simply add the @node with the right prev, up,
 // down plus the insertion of menus.
 // Generate a .NUM, that's a .TXI but with numbers in the titles and ready
 // for conversion to HTML (it replaces things like @end format by @end-format)
 // These formats are ever generated
 GenerateTXI_NUM();
 if ((outFormats & F_INFO) || (outFormats & F_TXT) || (outFormats & F_HTML))
   {
    // Make the .inf from the .num using makeinfo
    GenerateINFO();
   }
 if (outFormats & F_HTML)
   {
    // Create the table of contents for the HTML
    GenerateCTX();
    // Create the indices requested
    GenerateIDX(0);
    // Makeinfo 4.7 crazyness
    fclose(fi);
    fi=OpenInFile(".num");
    GenerateTX3();
    // Make the .html from the .num using makeinfo
    GenerateHTML();
   }
 if (outFormats & F_TXT)
   {
    GenerateC_IDX();
    // Create a file with the xref as Section x
    fclose(fi);
    fi=OpenInFile(".nut");
    GenerateTX1();
    // Create the indices requested, this time in text format
    GenerateIDX(1);
    // Create the .txt
    GenerateTXT();
   }
 if (outFormats & (F_DVI | F_PS))
   {
    GenerateTeX();
    if (outFormats & F_PS)
       GeneratePS();
   }
 fclose(fi);

 if (makeClean)
   {
    int i;
    DeleteFile(".num");
    DeleteFile(".nut");
    DeleteFile(".ctx");
    DeleteFile(".txi");
    DeleteFile(".tx1");
    DeleteFile(".tx2");
    DeleteFile(".tx3");
    DeleteFile("");
    DeleteFileName("contents.idx");
    // TeX standard index files
    DeleteFile(".cp");
    DeleteFile(".cps");
    DeleteFile(".fn");
    DeleteFile(".fns");
    DeleteFile(".pg");
    DeleteFile(".pgs");
    DeleteFile(".ky");
    DeleteFile(".kys");
    DeleteFile(".vr");
    DeleteFile(".vrs");
    DeleteFile(".tp");
    DeleteFile(".tps");
    DeleteFile(".toc");
    DeleteFile(".aux");
    //DeleteFile(".log"); Let the log is very important
    for (i=0; i<cantIndices; i++)
       {
        DeleteFileName(FilesIndex[i]);
        DeleteFileName(FilesIndexTXT[i]);
       }
   }

 return 0;
}
