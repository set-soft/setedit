/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#define Uses_stdlib
#define Uses_alloca
#define Uses_stdio
#define Uses_string
#define Uses_unistd
#define Uses_ctype
#define Uses_dirent
#define Uses_sys_stat
#define Uses_getcwd
#define Uses_chdir

#define Uses_TDialog
#define Uses_TDeskTop
#define Uses_TProgram
#define Uses_TApplication
#define Uses_TObject
#define Uses_TInputLine
#define Uses_TLabel
#define Uses_THistory
#define Uses_TRect
#define Uses_TCheckBoxes
#define Uses_TRadioButtons
#define Uses_TButton
#define Uses_TButton
#define Uses_MsgBox
#define Uses_TSItem
#define Uses_TCEditor
#define Uses_TInputLinePiped
#define Uses_TInputLinePipedConst
#define Uses_TListBox
#define Uses_TSOSListBox
#define Uses_fpstream
#define Uses_TVCodePage
#include <ceditor.h>
#define Uses_SETAppHelper
#define Uses_SETAppConst
#include <setapp.h>

// Prototypes for the message win.
#include <edmsg.h>
#include <splinman.h>
#include <rhutils.h>
#include <edhists.h>
//#if !(defined(SEOS_Win32) || defined(SEOSf_FreeBSD))
//#include <ftw.h>
//#endif
#include <dyncat.h>
#include <pathtool.h>

#if TV_MAJOR_VERSION>=2
#define TV_System TScreen::System
#define Uses_TScreen
#include <tv/screen.h>
#endif

// From edprj.cc generates the list of project items
extern int WriteNamesOfProjectTo(FILE *f);
// From editmain.cc generates the list of opened editors
extern int WriteNamesOfOpenedTo(FILE *f);
extern ushort execDialog( TDialog *d, void *data );

//const int tilpNoPipe=1,tilpNoCopy=2,tilpNoPaste=4;

// This collection is a list of the visited directories to avoid loops when
// a link points back in the tree.
static TStringCollection *Visited=0;
// Static to avoid consuming 1Kb by recursion in the stack
static char TempDirName[PATH_MAX];

void FullInputString(int x, int y, int w, int size,int histID, char *name,
                     unsigned flags, TDialog *d)
{
 TInputLine *inp=new TInputLinePiped(TRect(x+1,y+1,x+w,y+2),size,flags);
 d->insert(inp);
 d->insert(new TLabel(TRect(x,y,x+w,y+1),name,inp));
 d->insert(new THistory(TRect(x+w,y+1,x+w+3,y+2),inp,histID));
}

const int maxDirLen=80;

typedef struct
{
 char pattern[200];
 char files[80];
 char dirs[maxDirLen];
 uint16 sourP;
 uint16 typeP;
 uint16 where;
 uint16 recurse;
 uint16 ops;
} GrepBoxOld;

typedef struct
{
 char pattern[200];
 char files[80];
 char dirs[maxDirLen];
 uint32 sourP;
 uint32 typeP;
 uint32 where;
 uint32 recurse;
 uint32 ops;
} GrepBox;
// empty,empty,Pattern box,Basic reg.,search in project,ignore case
static GrepBox box={"","*.[ch]*",".",0,0,2,0,1};

const int Col1=1,Col2=40,End1=39,End2=72;

static
char *SaveClipToTemp(void)
{
 char *name=unique_name("cl");
 if (!name || !TCEditor::clipboard || !TCEditor::clipboard->hasSelection())
   {
    messageBox(_("The clipboard is empty"),mfOKButton);
    return 0;
   }
 FILE *f=fopen(name,"wt");
 fwrite(TCEditor::clipboard->buffer+TCEditor::clipboard->selStart,
        TCEditor::clipboard->selEnd-TCEditor::clipboard->selStart,1,f);
 fclose(f);
 return name;
}

static char ActualPath[PATH_MAX];

static
char *ParseFun(char *buf, FileInfo &fI, char *&fileName)
{
 char *endOfName,*endOfLine=0;
 int offset=0;

 // Look for file name and line number
 // It fails if: The file is absolute and starts with a number
 if (TVCodePage::isAlpha(buf[0]) && buf[1]==':' && (!ucisdigit(buf[2])))
    offset=2;
 endOfName=strchr(buf+offset,':');
 if (endOfName)
    endOfLine=strchr(endOfName+1,':');
 if (!endOfName || !endOfLine || !ucisdigit(endOfName[1]))
    return 0;

 char *ret;
 fI.len=strlen(endOfLine+1);
 fI.offset=endOfLine-buf+1;
 ret=strdup(buf);
 char bFile[PATH_MAX];
 if (offset || buf[0]=='/' || buf[0]=='\\')
   { // Absolute path
    *endOfName=0;
    strcpy(bFile,buf);
   }
 else
   { // Relative path
    // Put the actual directory
    strcpy(bFile,ActualPath);
    // Now the name
    *endOfName=0;
    strcat(bFile,buf);
   }
 // Fix it to avoid things like /dir/../dir/file
 CLY_fexpand(bFile);
 fileName=strdup(bFile);

 *endOfLine=0;
 fI.Line=atoi(endOfName+1);
 fI.Column=1;

 return ret;
}

/*
  This routine checks if grep is there. If we can't find it we must put a warning
*/
static
int CheckForGREP(void)
{
 static int isGREPInstalled=0;

 if (!isGREPInstalled)
   {
    // We must rediret the error to avoid getting it in the stderr file
    char *err=open_stderr_out();
    TV_System("grep -V");
    close_stderr_out();
    // Check what we got
    FILE *f=fopen(err,"r");
    int ok=0;
    if (f)
      {
       char resp[80];
       fgets(resp,80,f);
       fclose(f);
       ok=strstr(resp,"GNU grep")!=0;
      }
    unlink(err);

    if (ok)
       isGREPInstalled=1;
    else
       messageBox(_("You must install GNU grep to use it!"), mfError | mfOKButton);
   }

 return isGREPInstalled;
}

static
void RunGrep(char *command)
{
 char *out;
 char *err;
 if (!CheckForGREP())
    return;
 char b[PATH_MAX+60];
 out=open_stdout();
 err=open_stderr();
 getcwd(ActualPath,PATH_MAX);
 sprintf(b,_("Running grep in: %s"),ActualPath);
 if (ActualPath[strlen(ActualPath)-1]!='/')
    strcat(ActualPath,"/");
 EdShowMessage(b);
 #ifdef SECompf_djgpp
 // Force the command.com because bash fails
 // Is this use of putenv correct? I think not.
 char *envShell=getenv("SHELL");
 char *envShellCopy=0;
 if (envShell)
   {
    envShellCopy=(char *)alloca(strlen(envShell)+1+6);
    sprintf(envShellCopy,"SHELL=%s",envShell);
    putenv("SHELL=");
   }
 // Call it
 TV_System(command);
 // Now restore the environment
 if (envShell)
    // IMPORTANT! we can do it with djgpp because djgpp makes a copy
    // of the string we pass to putenv. Other implementations (like
    // GNU libc) doesn't copy the string and this is invalid.
    putenv(envShellCopy);
 #else
 // Call it
 TV_System(command);
 #endif
 close_stdout();
 close_stderr();
 DumpFileToMessage(err,__("From stderr:"),edsmEverScroll);
 DumpFileToMessage(out,__("From stdout:"),edsmEverScroll,ParseFun);
}

// That's similar to ftw, I didn't used ftw because isn't in the libc.info of my
// Linux so I don't know if that's so common.
static
void look_in(char *command)
{
 DIR *d;
 struct dirent *f;
 char *name,*pwdHere;
 ccIndex pos;

 // Keep record of our current location
 getcwd(TempDirName,PATH_MAX);
 pwdHere=strdup(TempDirName);
 if (Visited->search(pwdHere,pos))
   { // Hey we already scanned it!
    delete pwdHere;
    return;
   }
 Visited->atInsert(pos,pwdHere);
 RunGrep(command);
 
 d=opendir(".");
 if (d)
   {
    while ((f=readdir(d))!=0)
      {
       name=f->d_name;
       // Skip . and .. they aren't useful
       if (name[0]=='.')
         {
          if (name[1]==0 || (name[1]=='.' && name[2]==0))
             continue;
         }
       if (IsADirectory(name)) // It also checks valid access
         { // Recurse
          chdir(name);
          look_in(command);
          // We must go back to the previous location, .. isn't useful
          // when we taked a link
          chdir(pwdHere);
         }
      }
    closedir(d);
   }
}

static
void RunRecurseGrep(char *command, int recurse)
{
 char dirTemp[maxDirLen];
 // A copy to use strtok
 strcpy(dirTemp,box.dirs);

 char StartPoint[PATH_MAX];
 getcwd(StartPoint,PATH_MAX);

 destroy(Visited); // Just in case
 Visited=new TStringCollection(10,5);

 char *s=strtok(dirTemp,";, ");
 while (s)
   {
    if (chdir(s))
      {
       char b[PATH_MAX+60];
       sprintf(b,_("chdir error: %s"),s);
       EdShowMessage(b);
      }
    else
      {
       if (recurse)
          look_in(command);
       else
          RunGrep(command);
      }

    s=strtok(NULL,";, ");
    chdir(StartPoint);
   }

 destroy(Visited);
 Visited=0;
}

static
void ArrangeGrepCommand(char *command, char *param)
{
 #if defined(SEOS_DOS) || defined(SEOS_Win32)
 #define InitParametersFile()
 #define SetAccessToParameters(a)
 // In DOS/Win32 I use a response file to pass a very long list of parameters
 sprintf(command,"grep @%s",param);
 #endif

 #ifdef SEOS_UNIX
 #define InitParametersFile() fputs("grep ",f)
 #define SetAccessToParameters(a) chmod(a,S_IXUSR | S_IRUSR)
 // Response files aren't supported in UNIX :-(, so I just use a file that's
 // in fact a script
 strcpy(command,param);
 #endif
}

void grepWindow(char *patStart)
{
 TDialog *d=new TDialog(TRect(0,0,74,18),_("Powered Grep by SET"));
 d->options|=ofCentered;
 d->helpCtx=cmeGrepDialog;

 // ACDEFGIJKLNORPSTUWX
 // Pattern section
 FullInputString(Col1,1,End1-Col1-3,200,hID_TextSearchEditor,_("~P~attern box"),0,d);
 FullInputString(Col2,1,End2-Col2-3,80,hID_GrepFiles,_("Files to ~s~earch"),tilpNoPipe,d);
 FullInputString(Col1,3,End2-Col1-3,maxDirLen,hID_GrepPlaces,_("~D~irectories to search"),tilpNoPipe,d);

 TRadioButtons32 *sourP=new TRadioButtons32(TRect(Col1+1,7,End1,10),
     new TSItem(_("`Pattern box' is the pa~t~tern"),
     new TSItem(_("`Pattern box' is a fi~l~e name"),
     new TSItem(_("Use the ~c~lipboard selection"),0 ))));
 d->insert(sourP);
 d->insert(new TLabel(TRect(Col1,6,End1,7),_("Source of Pattern"),sourP));

 TRadioButtons32 *typeP=new TRadioButtons32(TRect(Col1+1,11,End1,14),
     new TSItem(_("Basic regular expression (-~G~)"),
     new TSItem(_("~E~xtended regular expression (-E)"),
     new TSItem(_("~F~ixed separated by CR (-F)"),0 ))));
 d->insert(typeP);
 d->insert(new TLabel(TRect(Col1,10,End1,11),_("Type of Pattern"),typeP));

 TRadioButtons32 *where=new TRadioButtons32(TRect(Col2+1,7,End2,10),
     new TSItem(_("~U~se `Files to search'"),
     new TSItem(_("Search in ope~n~ed windows"),
     new TSItem(_("Search in pro~j~ect"),0 ))));
 d->insert(where);
 d->insert(new TLabel(TRect(Col2,6,End2,7),_("Pl~a~ces to search"),where));
 d->insert(new TCheckBoxes32(TRect(Col2+1,10,End2,11),
     new TSItem(_("~R~ecurse in subdirs"),0 )));

 TCheckBoxes32 *ops=new TCheckBoxes32( TRect(Col2+1,12,End2,16),
     new TSItem(_("~I~gnore case (-i)"),
     new TSItem(_("~W~hole words only (-w)"),
     new TSItem(_("Whole line (-~x~)"),
     new TSItem(_("In~v~erse matching (-v)"), 0 )))));
 d->insert(ops);
 d->insert(new TLabel(TRect(Col2,11,End2,12),_("~O~ptions"),ops));

 d->insert(new TButton(TRect(2,15,14,17),_("O~K~"),cmOK,bfDefault));
 d->insert(new TButton(TRect(18,15,30,17),_("Cancel"),cmCancel,bfNormal));

 d->selectNext(False);

 if (patStart)
   {
    strcpy(box.pattern,patStart);
    delete patStart;
   }

 if (execDialog(d,&box)!=cmCancel)
   {
    if (box.dirs[0]==0)
      {
       messageBox(_("You must provide at least one directory to search"),mfOKButton);
       return;
      }
    char b[12];
    char *param=0;
    char command[PATH_MAX*2+80];
    char *clipTemp=0;
    int ok=1,absolute=0;

    param=unique_name("pa");
    FILE *f=fopen(param,"wb");
    if (!f)
      {
       string_free(param);
       return;
      }
    InitParametersFile();
    int i=1;
    b[0]='-';
    // Always numbers
    b[i++]='n';
    // Options from the dialog
    if (box.ops & 1)
       b[i++]='i';
    if (box.ops & 2)
       b[i++]='w';
    if (box.ops & 4)
       b[i++]='x';
    if (box.ops & 8)
       b[i++]='v';
    // Type of grep
    switch (box.typeP)
      {
       case 1:
            b[i++]='E';
            break;
       case 2:
            b[i++]='F';
            break;
       default:
            b[i++]='G';
            break;
      }
    b[i++]=' ';
    b[i]=0;
    fputs(b,f);
    // Pattern
    switch (box.sourP)
      {
       case 1:
            fputs("-f ",f);
            fputs(box.pattern,f);
            break;
       case 2:
            if ((clipTemp=SaveClipToTemp())==0)
               ok=0;
            else
              {
               fputs("-f ",f);
               fputs(clipTemp,f);
              }
            break;
       default:
            fputs("-e ",f);
            if (strchr(box.pattern,'\"')!=0)
              { // If have " let as-is
               fputs(box.pattern,f);
              }
            else
              { // Try to preserve the spaces
               fputc('\"',f);
               fputs(box.pattern,f);
               fputc('\"',f);
              }
      }
    fputc(' ',f);
    // Files
    switch (box.where)
      {
       case 1:
            if (!WriteNamesOfOpenedTo(f))
              {
               messageBox(_("There aren't opened files"),mfOKButton);
               ok=0;
              }
            absolute=1;
            break;
       case 2:
            if (!WriteNamesOfProjectTo(f))
              {
               messageBox(_("No files in project"),mfOKButton);
               ok=0;
              }
            absolute=1;
            break;
       default:
            fputs(box.files,f);
      }
    fputs(" /dev/null",f);
    fclose(f);
    SetAccessToParameters(param);

    if (ok)
      {
       SaveAllEditors();
       EdShowMessage(_("Powered grep"),True);
       if (absolute)
         {
          if (box.recurse)
            {
             sprintf(command,_("Recurse & dirs. ignored, using internal names"));
             EdShowMessage(command);
            }
          ArrangeGrepCommand(command,param);
          RunGrep(command);
         }
       else
         {
          ArrangeGrepCommand(command,param);
          RunRecurseGrep(command,box.recurse);
         }
       EdShowMessage(_("End of grep search"));
       EdJumpToMessage(0);
       SpLinesUpdate();
      }

    if (box.sourP==2)
       unlink(clipTemp);
    unlink(param);
    string_free(param);
    string_free(clipTemp);
   }
}

void SaveGrepData(fpstream &s)
{
 s << (char)2; // version
 s.writeBytes(&box,sizeof(GrepBox));
}

void LoadGrepData(fpstream &s)
{
 char version;
 s >> version;
 if (version<=1)
   {
    GrepBoxOld oldBox;
    s.readBytes(&oldBox,sizeof(GrepBoxOld));
    memcpy(&box,&oldBox,200+80+80);
    box.sourP=oldBox.sourP;
    box.typeP=oldBox.typeP;
    box.where=oldBox.where;
    box.recurse=oldBox.recurse;
    box.ops=oldBox.ops;
   }
 else
    s.readBytes(&box,sizeof(GrepBox));
}
