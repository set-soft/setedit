/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#define Uses_stdio
#define Uses_getline
#define Uses_string
#define Uses_ctype
#define Uses_AllocLocal
#define Uses_TCEditor_Internal
#define Uses_TCEditor_External
#define Uses_TCEditor
#define Uses_TVCodePage
#include <ceditor.h>
#include <loadshl.h>
#include <edspecs.h>

#include <ced_pcre.h>
#include <dyncat.h>

static char *nameSHLFile;
static PCREData shlPCRE={0,0};

ccIndex *ConvTable=0;

static void ReplaceCRby0(char *s)
{
 for (; *s && *s!='\n' && *s!='\r'; s++);
 *s=0;
}

static char *MoveAfterEqual(char *s)
{
 for (; *s && *s!='='; s++);
 if (*s) s++;
 for (; *s && ucisspace(*s); s++);
 return s;
}

static void GetUpTo(int len, char *s, char *d, int &l)
{
 if (*s=='"')
   {
    for (s++,l=0; *s && *s!='\n' && *s!='\r' && *s!='"' && l<len; s++,l++)
       {
        if (*s=='\\' && *s && *s!='\n' && *s!='\r')
           s++;
        d[l]=*s;
       }
    return;
   }
 for (l=0; *s && *s!='\n' && *s!='\r' && l<len; s++,l++) d[l]=*s;
}

static void FillTableWith(unsigned char *s, ushort *t, int mask)
{
 for (; *s && *s!='\n' && *s!='\r'; s++) t[*s]|=mask;
}

/**[txh]********************************************************************

  Description:
  Converts a syntax value (index in the syntax hl array) into an index in
the alphabetic collection.

***************************************************************************/

ccIndex SHLConvValToPos(int a)
{
 if (!ConvTable) return a;
 return ConvTable[a];
}

/**[txh]********************************************************************

  Description:
  Converts an index in the sorted collection of syntax hl names in a syntax
value (index in the syntax hl array).

***************************************************************************/

int SHLConvPosToVal(ccIndex a)
{
 if (!ConvTable) return a;

 int i=0;
 while (ConvTable[i]!=a) i++;
 return i;
}

static
void PutInTables(int len, char *s, unsigned flag, ushort *Table, int Case)
{
 int i;

 if (len)
   {
    if (!Case)
      {
       for (i=0; i<len; i++)
           s[i]=TVCodePage::toUpper(s[i]);
       Table[TVCodePage::toLower(*s)]|=flag;
      }
    Table[*s]|=flag;
   }
}

static
void CreateSearchTables(strSHL &hl)
{
 if (hl.Keywords)
    SETSECreateTables(hl.Search,hl.Flags1 & FG1_CaseSensitive,hl.Keywords);
}

int LoadSyntaxHighLightKeywords(strSHL &hl)
{
 int loadIt=0,isCase=hl.Flags1 & FG1_CaseSensitive;
 FILE *f;
 ssize_t len;
 size_t  lenLine=0;
 char *b=0;
 char *pos,*s;

 hl.Keywords=new TStringCollection(48,12);

 if ((f=fopen(nameSHLFile,"rb"))==NULL)
    return 1;

 do
   {
    do
      {
       do
         {
          len=CLY_getline(&b,&lenLine,f);
         }
       while (len!=-1 && !feof(f) && (*b=='#' || ucisspace(*b)));

       if (strncasecmp(b,"Name",4)==0)
         {
          pos=MoveAfterEqual(b);
          ReplaceCRby0(pos);
          if (strcasecmp(pos,hl.Name)==0)
             loadIt=1;
         }
       else
       if (strncasecmp(b,"Keywords",8)==0)
         {// Only if that's the one to load
          if (loadIt)
            {
             pos=MoveAfterEqual(b);
             int end=0;
             while (!end)
               {
                for (s=pos; *s && *s!='\n' && *s!='\r' && *s!=','; s++);
                if (*s!=',')
                   end=1;
                if (s!=pos)
                  {
                   *s=0;
                   char *t=newStr(pos);
                   if (!isCase)
                      strlwr(t); // Make sure they are lower case if the language is not
                                 // case sensitive, the seachs assumes it
                   ccIndex curPos;
                   if (hl.Keywords->search(t,curPos))
                      delete[] t;
                   else
                      hl.Keywords->insert(t);
                  }
                pos=s+1;
               }
            } // loadIt
         } // "Keywords"
      }
    while (!feof(f) && strncasecmp(b,"End",3)!=0);
   }
 while (!loadIt && !feof(f));
 fclose(f);
 free(b);
 CreateSearchTables(hl);

 return 0;
}

int LoadSyntaxHighLightFile(char *name, strSHL *&hl, TStringCollection *list,int &Cant)
{
 FILE *f;
 ssize_t len;
 size_t  lenLine=0;
 char *b=0;
 int  defs,def,end,i,isCase,preLoad;
 char *pos,*s;

 nameSHLFile=newStr(name);
 Cant=0;
 if ((f=fopen(name,"rb"))==NULL)
    return 1;

 // Meassure the number of definitions
 for (defs=0; !feof(f); )
    {
     len=CLY_getline(&b,&lenLine,f);
     if (len!=-1 && !feof(f) && strncasecmp(b,"End",3)==0)
        defs++;
    }

 Cant=defs;
 if (!defs)
   {
    free(b);
    return 2;
   }

 // Allocate enough memory
 hl=new strSHL[defs];
 if (!hl)
   {
    free(b);
    return 3;
   }
 memset(hl,0,sizeof(strSHL)*defs);
 ConvTable=new ccIndex[defs];
 if (!ConvTable)
   {
    free(b);
    return 3;
   }

 PCREInitCompiler(shlPCRE);
 // Load and parse all
 rewind(f);
 for (def=0; def<defs && !feof(f); def++)
    {
     hl[def].Keywords=0;
     isCase=preLoad=0;
     for (i=0; i<256; i++)
         if (TVCodePage::isAlNum(i) || i=='_')
            hl[def].SymbolT[i]|=shl_INSNAME | shl_BEGNAME;
     do
       {
        do
          {
           len=CLY_getline(&b,&lenLine,f);
          }
        while (len!=-1 && *b=='#' || ucisspace(*b));

        if (strncasecmp(b,"NameMatch",9)==0)
          {
           pos=MoveAfterEqual(b);
           ReplaceCRby0(pos);
           if (*pos)
              hl[def].NameMatch=PCRECompileRegEx(pos,shlPCRE);
          }
        else
        if (strncasecmp(b,"Preload",7)==0)
          { // Preload the keywords
           preLoad=1;
          }
        else
        if (strncasecmp(b,"FullNameMatch",13)==0)
          {
           pos=MoveAfterEqual(b);
           ReplaceCRby0(pos);
           if (*pos)
              hl[def].PathMatch=PCRECompileRegEx(pos,shlPCRE);
          }
        else
        if (strncasecmp(b,"Name",4)==0)
          {
           pos=MoveAfterEqual(b);
           ReplaceCRby0(pos);
           char *s=newStr(pos);
           hl[def].Name=s;
           list->insert(s);
          }
        else
        if (strncasecmp(b,"Files",5)==0)
          {
           pos=MoveAfterEqual(b);
           ReplaceCRby0(pos);
           hl[def].Extensions=newStr(pos);
          }
        else
        if (strncasecmp(b,"EmacsModes",10)==0)
          {
           pos=MoveAfterEqual(b);
           ReplaceCRby0(pos);
           hl[def].EmacsModes=newStr(pos);
          }
        else
        if (strncasecmp(b,"ShellScript",11)==0)
          {
           pos=MoveAfterEqual(b);
           ReplaceCRby0(pos);
           hl[def].ShellScript=newStr(pos);
          }
        else
        if (strncasecmp(b,"OpenComment1",12)==0)
          {
           pos=MoveAfterEqual(b);
           GetUpTo(4,pos,hl[def].OpenCom1,hl[def].lOpenCom1);
           PutInTables(hl[def].lOpenCom1,hl[def].OpenCom1,shl_OPC1,hl[def].SymbolT,isCase);
          }
        else
        if (strncasecmp(b,"CloseComment1",13)==0)
          {
           pos=MoveAfterEqual(b);
           GetUpTo(4,pos,hl[def].CloseCom1,hl[def].lCloseCom1);
           PutInTables(hl[def].lCloseCom1,hl[def].CloseCom1,shl_CLOSE1,hl[def].SymbolT,isCase);
          }
        else
        if (strncasecmp(b,"OpenComment2",12)==0)
          {
           pos=MoveAfterEqual(b);
           GetUpTo(4,pos,hl[def].OpenCom2,hl[def].lOpenCom2);
           PutInTables(hl[def].lOpenCom2,hl[def].OpenCom2,shl_OPC2,hl[def].SymbolT,isCase);
          }
        else
        if (strncasecmp(b,"CloseComment2",13)==0)
          {
           pos=MoveAfterEqual(b);
           GetUpTo(4,pos,hl[def].CloseCom2,hl[def].lCloseCom2);
           PutInTables(hl[def].lCloseCom2,hl[def].CloseCom2,shl_CLOSE2,hl[def].SymbolT,isCase);
          }
        else
        if (strncasecmp(b,"EOLComment1",11)==0)
          {
           pos=MoveAfterEqual(b);
           GetUpTo(4,pos,hl[def].EOLCom1,hl[def].lEOLCom1);
           PutInTables(hl[def].lEOLCom1,hl[def].EOLCom1,shl_EOL,hl[def].SymbolT,isCase);
          }
        else
        if (strncasecmp(b,"EOLComment2",11)==0)
          {
           pos=MoveAfterEqual(b);
           GetUpTo(4,pos,hl[def].EOLCom2,hl[def].lEOLCom2);
           PutInTables(hl[def].lEOLCom2,hl[def].EOLCom2,shl_EOL,hl[def].SymbolT,isCase);
          }
        else
        if (strncasecmp(b,"HexMarker",9)==0)
          {
           int i_l;
           pos=MoveAfterEqual(b);
           GetUpTo(4,pos,hl[def].HexStart,hl[def].lHexStart);
           if (!isCase)
              for (i_l=0; i_l<4; i_l++)
                  hl[def].HexStart[i_l]=TVCodePage::toUpper(hl[def].HexStart[i_l]);
          }
        else
        if (strncasecmp(b,"Symbols1",8)==0)
          {
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_SYM1);
          }
        else
        if (strncasecmp(b,"Symbols2",8)==0)
          {
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_SYM2);
          }
        else
        if (strncasecmp(b,"String1",7)==0)
          {
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_STR1);
          }
        else
        if (strncasecmp(b,"String2",7)==0)
          {
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_STR2);
          }
        else
        if (strncasecmp(b,"String3",7)==0)
          {
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_STR3);
          }
        else
        if (strncasecmp(b,"ShortString",11)==0)
          {
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_CHAR);
          }
        else
        if (strncasecmp(b,"EscapeAnywhere",14)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
              hl[def].Flags2|=FG2_EscapeAnywhere;
          }
        else
        if (strncasecmp(b,"Escape",6)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos && *pos!='\r' && *pos!='\n')
              hl[def].Escape=*pos;
          }
        else
        if (strncasecmp(b,"Preprocessor",12)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos && *pos!='\r' && *pos!='\n')
              hl[def].Preprocessor=*pos;
          }
        else
        if (strncasecmp(b,"Preprocessor2",12)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos && *pos!='\r' && *pos!='\n')
              hl[def].Preprocessor2=*pos;
          }
        else
        if (strncasecmp(b,"Case",4)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
             {
              isCase=1;
              hl[def].Flags1|=FG1_CaseSensitive;
             }
          }
        else
        if (strncasecmp(b,"PartialKeywords",15)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
             {
              isCase=1;
              hl[def].Flags1|=FG1_PartialKeyword;
             }
          }
        else
        if (strncasecmp(b,"RelaxNumberCheck",16)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
              hl[def].Flags1|=FG1_RelaxNumbers;
          }
        else
        if (strncasecmp(b,"UseInternal",11)==0)
          {
           int i=0;
           pos=MoveAfterEqual(b);
           i=atoi(pos); // Another bug in egcs 1.1.2, if I declare int i; before pos=.. thinks it can be used unitialized
           if (i<0 || i>3)
              i=0;
           TCEditor::SHLTableUse[i]=def;
           hl[def].UseInternal=i;
          }
        else
        if (strncasecmp(b,"Keywords",8)==0)
          {// If not preload just ignore them
           if (preLoad)
             {
              if (!hl[def].Keywords)
                 hl[def].Keywords=new TStringCollection(48,12);
              pos=MoveAfterEqual(b);
              end=0;
              while (!end)
                {
                 for (s=pos; *s && *s!='\n' && *s!='\r' && *s!=','; s++);
                 if (*s!=',')
                    end=1;
                 if (s!=pos)
                   {
                    *s=0;
                    char *t=newStr(pos);
                    if (!isCase)
                       strlwr(t); // Make sure they are lower case if the language is not
                                  // case sensitive, the seachs assumes it
                    hl[def].Keywords->insert(t);
                   }
                 pos=s+1;
                }
             }
          }
        else
        if (strncasecmp(b,"AllowedInsideNames",18)==0)
          {
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_INSNAME);
          }
        else
        if (strncasecmp(b,"CanStartAName",13)==0)
          {
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_BEGNAME);
          }
        else
        if (strncasecmp(b,"SpecialSymbolCont",17)==0)
          {//Before SpecialSymbol or we will confuse it ;-)
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_SPC);
          }
        else
        if (strncasecmp(b,"SpecialSymbol",13)==0)
          {
           pos=MoveAfterEqual(b);
           FillTableWith((uchar *)pos,hl[def].SymbolT,shl_SP);
          }
        else
        if (strncasecmp(b,"PMacros",7)==0)
          {
           pos=MoveAfterEqual(b);
           ReplaceCRby0(pos);
           hl[def].PMacros=newStr(pos);
          }
        else
        if (strncasecmp(b,"NoCheckNumbers",14)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
              hl[def].Flags1|=FG1_NoNumbers;
          }
        else
        if (strncasecmp(b,"EOLCInFirstCol1",15)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
              hl[def].Flags1|=FG1_EOLCInFirstCol1;
          }
        else
        if (strncasecmp(b,"EOLCInFirstCol2",15)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
              hl[def].Flags1|=FG1_EOLCInFirstCol2;
          }
        else
        if (strncasecmp(b,"EOLCInFirstCol",14)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
              hl[def].Flags1|=FG1_EOLCInFirstCol1 |  FG1_EOLCInFirstCol2;
          }
        else
        if (strncasecmp(b,"EOLCInFirstUse1",15)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
              hl[def].Flags1|=FG1_EOLCInFirstUse1;
          }
        else
        if (strncasecmp(b,"EOLCInFirstUse2",15)==0)
          {
           pos=MoveAfterEqual(b);
           if (*pos=='1')
              hl[def].Flags2|=FG2_EOLCInFirstUse2;
          }
       }
     while (!feof(f) && strncasecmp(b,"End",3)!=0);
    }
 fclose(f);
 free(b);

 PCREStopCompiler(shlPCRE);
 for (def=0; def<defs; def++)
    {
     CreateSearchTables(hl[def]);
     list->search(hl[def].Name,ConvTable[def]);
    }
 LoadDefaultOpts(hl,Cant);

 return 0;
}

// That's just to deallocate all the memory allocated and owned by this module
void UnLoadSyntaxHighLightFile(strSHL *&hl, TStringCollection *list, int &Cant)
{
 int i;

 for (i=0; i<Cant; i++)
    {// Collections
     CLY_destroy(hl[i].PM);
     CLY_destroy(hl[i].Keywords);
     CLY_destroy(hl[i].UserWords);
     // Various strings
     delete[] hl[i].Extensions;
     delete[] hl[i].EmacsModes;
     delete[] hl[i].ShellScript;
     delete[] hl[i].PMacros;
     // Search tables
     SETSEDeleteTables(hl[i].Search);
     SETSEDeleteTables(hl[i].SearchUserWords);
     // PCREs
     free(hl[i].NameMatch);
     free(hl[i].PathMatch);
    }
 delete[] hl;
 CLY_destroy(list);
 Cant=0;
 delete[] ConvTable;
 ConvTable=0;
 // PCRE compiler memory
 PCREInitCompiler(shlPCRE);
 delete[] nameSHLFile;
}

extern char *strncpyZ(char *dest, const char *orig, int size);

static
int TakeExtension(char *file, char *ext)
{
 char *s=file,*e=0,*e2=0;
 int wasExt=0;

 for (; *s; s++)
     if (*s=='/' || *s=='\\')
        wasExt=0;
     else
        if (*s=='.')
          {
           wasExt++;
           e2=e;
           e=s;
          }

 if (!wasExt)
    return 0;

 // If the extension contains gz and we have 2 extensions use the other
 // it works nice with LFNs, SFNs have 1 extension
 if (wasExt>1 && strstr(e,"gz"))
    strncpyZ(ext,e2+1,min(MaxExtension,e-e2));
 else
    strncpyZ(ext,e+1,MaxExtension);
 return 1;
}

static
int TakeCommentLowLev(char *buffer, int l, char *ext, int *tab_width,
                      int *startCom, int *endCom, int *modeFound, int offset)
{
 int i;
 char buf[MaxExtension];

 if (l<7)
    return 0;
 l-=2;

 char *s=buffer+offset;
 int p1=-1,p2=-1;

 for (i=0; i<l; i++)
    {
     if (s[i]=='-' && s[i+1]=='*' && s[i+2]=='-')
       {
        if (p1==-1)
          {
           p1=i;
           i+=2;
          }
        else
          if (p2==-1)
            {
             p2=i;
             break;
            }
       }
    }
 if (p2==-1)
    return 0;

 if (startCom)
    *startCom=p1+offset;
 if (endCom)
    *endCom=p2+offset;
 for (p1+=3; ucisspace(s[p1]); p1++);
 for (p2--;  ucisspace(s[p2]); p2--);

 l=p2-p1+2;
 if (l<2)
    return 1;

 if (l>MaxExtension)
    l=MaxExtension;

 strncpyZ(ext,&s[p1],l);
 strcpy(buf,ext);
 //printf("Parsing comment: -*- %s -*-\n",buf);
 // Make a little bit of parsing here
 s=strstr(buf,"mode:");
 if (s)
   {
    s+=5;
    for (;*s && ucisspace(*s); s++);
    char *end=s;
    for (;*end && (TVCodePage::isAlNum(*end) || *end=='-'); end++);
    *end=0;
    strcpy(ext,s);
    *end=';';
    *modeFound=1;
   }
 if (tab_width)
   {
    s=strstr(buf,"tab-width:");
    if (s)
      {
       s+=10;
       for (;*s && ucisspace(*s); s++);
       char *end;
       int temp=strtol(s,&end,0);
       if (temp>0 && temp<32) // Tabs>32 looks like an error, no?
          *tab_width=temp;
      }
   }
 return 1;
}

const int searchFromStart=1000, searchAtEnd=3000;

/**[txh]********************************************************************

  Description:
  This function looks for the Emacs style variables in the provided
@var{buffer}. The search is done upto the @var{lenBuf} offset. The content
of the "mode" variable is returned in @var{ext} argument. If the
@var{tab_width} argument is provided the content of the "tab-width" variable
is stored there. Providing @var{startCom} and @var{endCom} you can get
pointers to the beggining and the end of the comment.@*
  Note that only the first 1 KB and last 3 KB of the buffer are analyzed.@*
  This function have some high-level behavior and that's why is documented.
If you don't pass the @var{startCom} and @var{endCom} they are NULL and in
this case the function does a search looking for all instances of variable
definitions, it doesn't stop after finding one definition. If the pointers
are provided the function looks for a definition where at least "mode" is
defined.

  Return: 0 if not found.
  
***************************************************************************/

int TakeCommentEmacs(char *buffer, int lenBuf, char *ext, int *tab_width,
                     int *startCom, int *endCom)
{
 int len,start;

 /* Don't borther with empty buffers */
 if (!buffer || lenBuf<7)
    return 0;

 /* First 1Kb or available */
 start=0;
 len=searchFromStart;
 if (lenBuf<len)
    len=lenBuf;

 int searchAll=startCom==NULL;
 int found, lStart, lEnd, totalFound=0, modeFound=0;
 if (searchAll)
   {
    startCom=&lStart;
    endCom=&lEnd;
   }

 found=TakeCommentLowLev(buffer,len,ext,tab_width,startCom,endCom,&modeFound,0);
 if (found)
   {
    if (!searchAll && modeFound)
       return 1;
    while (found && *endCom+3<len)
      {
       totalFound++;
       *endCom+=3;
       found=TakeCommentLowLev(buffer,len-*endCom,ext,tab_width,startCom,endCom,
                               &modeFound,*endCom);
       if (found && !searchAll && modeFound)
          return 1;
      }
   }

 /* If we searched in all the buffer give up */
 if (lenBuf<=searchFromStart)
    return 0;

 /* Last 3Kb or available */
 start=lenBuf-searchAtEnd;
 if (start<searchFromStart)
    start=searchFromStart;
 len=lenBuf-start;

 found=TakeCommentLowLev(buffer,len,ext,tab_width,startCom,endCom,&modeFound,start);
 if (found)
   {
    if (!searchAll && modeFound)
       return 1;
    while (found && *endCom+3<len)
      {
       totalFound++;
       *endCom+=3;
       found=TakeCommentLowLev(buffer,len-*endCom,ext,tab_width,startCom,endCom,
                               &modeFound,*endCom);
       if (found && !searchAll && modeFound)
          return 1;
      }
   }
 if (!searchAll)
    return 0;
 return totalFound;
}


static
int TakeLocalVarLowLev(char *buffer, int lenBuf, char *ext, int *tab_width)
{
 unsigned lv,mode,end,i,tabWidth;
 char *s=buffer;

 if (!buffer || lenBuf<25)
    return 0;

 // Search these magic words, quit if one isn't there, ensure they are in the right
 // order
 lv=TCEditor_iScan(s,lenBuf,"Local Variables:");
 if (lv==sfSearchFailed)
    return 0;

 lenBuf-=lv;
 s+=lv;
 if (lenBuf<5)
    return 0;
 end=TCEditor_iScan(s,lenBuf,"End");
 if (end==sfSearchFailed)
    return 0;

 if (tab_width)
   {
    tabWidth=TCEditor_iScan(s,end,"tab-width:");
    if (tabWidth!=sfSearchFailed)
      {
       unsigned offset=tabWidth+10;
       for (; offset<end && isspace(s[offset]); offset++);
       if (offset<end && isdigit(s[offset]))
          *tab_width=atoi(s+offset);
      }
   }

 mode=TCEditor_iScan(s,end,"mode:");
 if (mode==sfSearchFailed)
    return 0;
 s=buffer+lv+mode+5;
 for (i=0; i<MaxExtension-1 && s[i]!='\n' && s[i]!='\r'; i++)
     ext[i]=s[i];
 ext[i]=0;
 return 1;
}

static
int TakeCommentLocalVars(char *buffer, int lenBuf, char *ext, int *tab_width)
{
 int start;

 /* Last 3Kb or available */
 start=lenBuf-searchAtEnd;
 if (start<0)
    start=0;

 return TakeLocalVarLowLev(buffer+start,lenBuf-start,ext,tab_width);
}

static
int TakeComment(char *buffer, int lenBuf, char *ext, int &tab_width)
{
 if (TakeCommentEmacs(buffer,lenBuf,ext,&tab_width))
    return 1;
 return TakeCommentLocalVars(buffer,lenBuf,ext,&tab_width);
}

static
int TakeScriptPrg(char *buffer, int lenBuf, char *ext)
{
 // I assume it must start with #!
 if (lenBuf<3 || *buffer!='#' || buffer[1]!='!')
    return 0;
 char *end=buffer+lenBuf;
 *ext=0;
 // Kill any space before the path (also /)
 for (buffer+=2; buffer<end && (ucisspace(*buffer) || *buffer=='/'); buffer++);
 int i;
 do
   {
    if (*buffer=='/')
       buffer++;
    for (i=0; buffer<end && *buffer!='/' && *buffer!=' ' && CLY_IsntEOL(*buffer); buffer++)
        if (i<MaxExtension-1)
           ext[i++]=*buffer;
    ext[i]=0;
    if (strcmp(ext,"env")==0 && *buffer==' ')
      {
       for (; buffer<end && ucisspace(*buffer); buffer++);
       i=-1;
      }
   }
 while (*buffer=='/' || i==-1);
 return *ext;
}

static
int IsInList(char *list, char *val, int noCase=0)
{
 int l=strlen(list)+1;

 AllocLocalStr(buf,l);
 strcpy(buf,list); // Because strtok fucks the string

 char *tok=strtok(buf,",");

 if (noCase)
   {
    while (tok)
      {
       if (strcasecmp(tok,val)==0)
          return 1;
       tok=strtok(NULL,",");
      }
   }
 else
   {
    while (tok)
      {
       if (strcmp(tok,val)==0)
          return 1;
       tok=strtok(NULL,",");
      }
   }
 return 0;
}

const int extExtension=1, extEmacsModes=2, extShellScript=3;

static
int SearchSHLForIt(TCEditor &e, char *Extension, int &index, int type)
{
 char EnvName[MaxExtension+10];
 const char *v;
 int i;

 if (Extension[0]==0)
    return 0;
 else
   {
    if (TCEditor::SHLGenList)
      {// Search if the user overwritten the roules with an enviro. var.
       strcpy(EnvName,"SET_SHL.");
       strcat(EnvName,Extension);
       if ((v=GetVariable(EnvName))!=NULL)
         {
          ccIndex posAlpha;
          if (TCEditor::SHLGenList->search((void *)v,posAlpha))
            {
             index=SHLConvPosToVal(posAlpha);
             return 1;
            }
         }
      }
    // User defined
    for (i=0; i<e.SHLCant; i++)
       {
        switch (type)
          {
           case extExtension:
                if (e.SHLArray[i].Extensions &&
                    IsInList(e.SHLArray[i].Extensions,Extension))
                  {
                   index=i;
                   return 1;
                  }
                break;
           case extEmacsModes:
                if (e.SHLArray[i].EmacsModes &&
                    IsInList(e.SHLArray[i].EmacsModes,Extension,1))
                  {
                   index=i;
                   return 1;
                  }
                break;
           case extShellScript:
                if (e.SHLArray[i].ShellScript &&
                    IsInList(e.SHLArray[i].ShellScript,Extension))
                  {
                   index=i;
                   return 1;
                  }
                break;
          }
       }
   }
 return 0;
}

const int typeFullName=0,typeName=1;

static
int SearchByName(TCEditor &e, char *fileName, int &index, int type)
{
 char *name=fileName;
 if (type==typeName)
   {
    char *name=strrchr(fileName,'/');
    if (!name)
       name=fileName;
    else
       name++;
   }
 int len=strlen(name),i;
 for (i=0; i<e.SHLCant; i++)
    {
     pcre *regex=type==typeFullName ? e.SHLArray[i].PathMatch : e.SHLArray[i].NameMatch;
     if (regex && PCREDoSearch(name,len,regex,shlPCRE))
       {
        index=i;
        return 1;
       }
    }
 return 0;
}

int SHLSelect(TCEditor &e, char *buffer, int lenBuf)
{
 int i,found=0,tab_width=-1;
 char Extension[MaxExtension];
 Extension[0] = 0; // this must be initialized !! (RH)

 // In case the editor isn't locked is better to lock it to avoid multiple
 // redraws, for example because a SHL change and a tab size change
 e.lock();
 int oldTab=e.tabSize;
 e.SHLValueSelected=-1;
 // First try using Emacs style syntax highlight methode
 if (TakeComment(buffer,lenBuf,Extension,tab_width))
    found=SearchSHLForIt(e,Extension,i,extEmacsModes);

 // If not present then check if the file is a UNIX script
 if (!found && TakeScriptPrg(buffer,lenBuf,Extension))
   {
    found=SearchSHLForIt(e,Extension,i,extShellScript);
    if (!found)
      {
       char *s=Extension;
       // Try without the numbers. Example: python2.2 => try python
       for (; *s && !ucisdigit(*s); s++);
       if (*s)
         {
          *s=0;
          found=SearchSHLForIt(e,Extension,i,extShellScript);
         }
      }
   }

 // Try RegEx with the full name
 if (!found)
    found=SearchByName(e,e.fileName,i,typeFullName);

 // Try RegEx with the name
 if (!found)
    found=SearchByName(e,e.fileName,i,typeName);

 // If not present then try with the extension
 if (!found)
   {
    TakeExtension(e.fileName,Extension);
    found=SearchSHLForIt(e,Extension,i,extExtension);
   }

 dflOptions *dflOps;
 if (found)
   {
    if (e.SHLArray[i].UseInternal)
       e.SetHighlightTo((shlState)e.SHLArray[i].UseInternal);
    else
       e.SetHighlightTo(shlGenericSyntax,i);
    e.SHLValueSelected=i;
    dflOps=&e.SHLArray[i].df;
   }
 else
   {
    e.SetHighlightTo(shlNoSyntax);
    dflOps=&e.dflOps;
   }
 // Transfer the default options
 uint32 lo=e.CompactFlags();
 lo&=dflOps->resetOpts;
 lo|=dflOps->setOpts;
 e.ExpandFlags(lo);
 if (dflOps->tabSize)
   {
    e.tabSize=dflOps->tabSize;
    e.update(ufView);
   }
 if (dflOps->indentSize)
    e.indentSize=dflOps->indentSize;
 if (dflOps->wrapCol)
    e.WrapCol=dflOps->wrapCol;
 if (dflOps->colMarkers)
   {
    delete[] e.colMarkers;
    e.colMarkers=TCEditor::CopyColMarkers(dflOps->colMarkers);
   }

 if (tab_width>0 && tab_width!=oldTab)
   {
    e.tabSize=tab_width;
    e.update(ufView);
   }
 e.unlock();

 return found;
}

void SHLTransferDefaultsNewFile(TCEditor &e)
{
 dflOptions *dflOps=&e.dflOps;
 uint32 lo=e.CompactFlags();
 lo&=dflOps->resetOpts;
 lo|=dflOps->setOpts;
 e.ExpandFlags(lo);
 if (dflOps->tabSize)
   {
    e.tabSize=dflOps->tabSize;
    e.update(ufView);
   }
 if (dflOps->indentSize)
    e.indentSize=dflOps->indentSize;
 if (dflOps->wrapCol)
    e.WrapCol=dflOps->wrapCol;
}

char *SHLNameOf(unsigned number)
{
 if (number<(unsigned)TCEditor::SHLCant)
    return TCEditor::SHLArray[number].Name;
 return "";
}


int SHLNumberOf(char *name)
{
 int i;

 for (i=0; i<TCEditor::SHLCant; i++)
    {
     if (strcmp(TCEditor::SHLArray[i].Name,name)==0)
        return i;
    }
 return -1;
}

char *SHLConstructEmacsModeComment(TCEditor &e, int &sizeSt, int &sizeEnd)
{
 int shl=e.SHLValueSelected;
 if (shl<0 || shl>=e.SHLCant || !e.SHLArray[shl].EmacsModes)
    return 0;

 char *list=e.SHLArray[shl].EmacsModes;
 char *tok=strtok(list,",");
 if (!tok)
    return 0;

 int useEOL=e.SHLArray[shl].lEOLCom1!=0;
 if (!useEOL && (!e.SHLArray[shl].lOpenCom1 || !e.SHLArray[shl].lCloseCom1))
    return 0;

 DynStrCatStruct cat;
 if (useEOL)
   {
    DynStrCatInit(&cat,e.SHLArray[shl].EOLCom1,e.SHLArray[shl].lEOLCom1);
    sizeSt=e.SHLArray[shl].lEOLCom1;
   }
 else
   {
    DynStrCatInit(&cat,e.SHLArray[shl].OpenCom1,e.SHLArray[shl].lOpenCom1);
    sizeSt=e.SHLArray[shl].lOpenCom1;
   }
 DynStrCat(&cat," -""*- mode:");
 DynStrCat(&cat,tok);
 char buf[32];
 int l=sprintf(buf,"; tab-width: %d -""*-",e.tabSize);
 DynStrCat(&cat,buf,l);
 if (useEOL)
    sizeEnd=0;
 else
   {
    DynStrCat(&cat," ",1);
    sizeEnd=e.SHLArray[shl].lCloseCom1+1;
    DynStrCat(&cat,e.SHLArray[shl].CloseCom1,e.SHLArray[shl].lCloseCom1);
   }
 DynStrCat(&cat,(char *)CLY_crlf);

 return cat.str;
}

PMacroStr *SHLSearchPMTrigger(char *trg);

/************************ Regular expressions file matching stuff *******************/
/**[txh]********************************************************************

  Description:
  Initialize the matchs array to 0. Called before starting to compile the
expressions.

***************************************************************************/

void PCREInitCompiler(PCREData &p)
{
 if (!SUP_PCRE)
    return;
 p.PCREMaxMatchs=0;
 DeleteArray(p.PCREMatchs);
 p.PCREMatchs=0;
}

/**[txh]********************************************************************

  Description:
  Allocates memory for the matchs array. Called after compiling all the
expressions and before executing any of them.

***************************************************************************/

void PCREStopCompiler(PCREData &p)
{
 if (!SUP_PCRE)
    return;
 p.PCREMatchs=new int[p.PCREMaxMatchs];
}

/**[txh]********************************************************************

  Description:
  Compiles a RegEx.

  Return: A pointer to the compiled RegEx or 0 if error.

***************************************************************************/

pcre *PCRECompileRegEx(char *text, PCREData &p)
{
 if (!SUP_PCRE)
    return NULL;
 const char *error;
 int   errorOffset;
 pcre *ret=pcre_compile(text,0,&error,&errorOffset,0);
 if (!ret)
    return NULL;

 int matchs=(pcre_info(ret,0,0)+1)*3;
 if (matchs>p.PCREMaxMatchs)
    p.PCREMaxMatchs=matchs;

 return ret;
}

int PCREDoSearch(char *search, int len, pcre *CompiledPCRE, PCREData &p)
{
 if (!SUP_PCRE)
    return 0;
 p.PCREHits=pcre_exec(CompiledPCRE,0,search,len,PCRE206 0,p.PCREMatchs,p.PCREMaxMatchs);

 return p.PCREHits>0;
}

void PCREGetMatch(int match, int &offset, int &len, PCREData &p)
{
 if (!SUP_PCRE || match<0 || match>=p.PCREHits)
   {
    offset=-1; len=0;
    return;
   }
 offset=p.PCREMatchs[match*2];
 int end=p.PCREMatchs[match*2+1];
 len=end-offset;
}
/********************** End Regular expressions file matching stuff *****************/

