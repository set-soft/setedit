/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  That's an interesting way to make an Open dialog. The routine takes as
parameter the title of the window ("Select a file", etc) and a char * with
enough space for the filename containing the file mask.
  The interesting thing is function remembers the directory where the
user opened the last file and changes the curdir to it before showing the
list. After the user selected the file the routines changes back the curdir
to the one used by the program. It avoids saving files in other directories,
that's a very annoying feature of the RHIDE 1.4 methode.
  But to complet the thing it supports a history ID and a flag to set a mode
where the dialog doesn't end when the user selects a file, instead the
dialog sends a broadcast and if the message was received by someebody moves
the selection to the next position. That's the same behavior used in the
project window of the IDE and the editor but solved in a generic way (hey
man! I'm loving these f*king messages ;-)

  Return:
  The button choosed by the user. cmCancel if aborted.

***************************************************************************/
#define Uses_unistd
#define Uses_string
#define Uses_getcwd
#define Uses_chdir

#define Uses_TFileDialog
#define Uses_TFileList
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TEvent
#define Uses_TButton
#define Uses_TInputLine
#define Uses_TChDirDialog
#include <tv.h>

#include <fileopen.h>

class TFileDialogHome : public TFileDialog
{
public:
 TFileDialogHome(const char *aWildCard, const char *aTitle,
                 const char *inputName, ushort aOptions, uchar histId,
                 const char *aHomeDir, unsigned aFlags, char *aSelDir,
                 TRect &dialogSize);
 virtual void handleEvent(TEvent& event);
 const char *homeDirectory;

 TInputLine *link;
 unsigned flags;
 char letHistoryPass;
 char *selDir;
 TRect *returnSize;
 static int (*ConfDiag)();
};

int (*TFileDialogHome::ConfDiag)()=0;

static
TButton *AddButton(TRect &r, const char *label, unsigned command)
{
 TButton *bt=new TButton(r,label,command,bfNormal);
 bt->growMode=gfGrowLoX | gfGrowHiX;
 r.a.y+=2; r.b.y+=2;
 return bt;
}

TFileDialogHome::TFileDialogHome(const char *aWildCard, const char *aTitle,
                                 const char *inputName, ushort aOptions,
                                 uchar histId, const char *aHomeDir,
                                 unsigned aFlags, char *aSelDir,
                                 TRect &dialogSize) :
   TWindowInit(&TFileDialogHome::initFrame),
   TFileDialog(aWildCard,aTitle,inputName,aOptions,histId)
{
 // ugly:
 TRect r(35,3,46,5);

 #define shift(flag) if (aOptions & flag) { r.a.y+=2; r.b.y+=2; }
 shift(fdOpenButton)
 shift(fdOKButton)
 shift(fdAddButton)
 shift(fdReplaceButton)
 shift(fdClearButton)
 shift(fdHelpButton)
 shift(fdSelectButton)
 r.a.y+=2; r.b.y+=2;
 #undef shift

 insert(AddButton(r,__("Ho~m~e"),cmHomeDir));
 insert(AddButton(r,__("~T~ree"),cmChangeDir));
 if (ConfDiag)
    insert(AddButton(r,__("O~p~tions"),cmFileOpenOptions));
 selectNext(False);
 link=(TInputLine *)current;

 homeDirectory=aHomeDir;
 flags=aFlags;
 letHistoryPass=0;
 selDir=aSelDir;

 if (dialogSize.a.x || dialogSize.b.x)
   {
    if (dialogSize.b.x-dialogSize.a.x<49 || dialogSize.b.y-dialogSize.a.y<20)
       // Avoid using a size smaller than 49x20
       dialogSize=getBounds();
    else
      {
       changeBounds(dialogSize);
       options&=~ofCentered;
      }
   }
 else
    dialogSize=getBounds();
 returnSize=&dialogSize;
}


char *ChooseDir(const char *startDir)
{
 char buf[PATH_MAX],buf2[PATH_MAX];

 if (!getcwd(buf,PATH_MAX) || (startDir && !chdir(startDir)))
    return NULL;

 TChDirDialog *d=new TChDirDialog(cdNormal | cdHelpButton,0);
 d->helpCtx=hcGenChDir;
 unsigned ret=0;
 TView *p=TProgram::application->validView(d);
 if (p)
   {
    ret=TProgram::deskTop->execView(p);
    TObject::CLY_destroy(p);
   }
 char *retVal=NULL;
 if (ret==cmOK && getcwd(buf2,PATH_MAX))
   {
    if (strcmp(buf,buf2)!=0)
      {
       int l=strlen(buf2);
       if (buf2[l-1]!='/' && l<PATH_MAX-2)
         {
          buf2[l]='/';
          buf2[l+1]=0;
         }
       retVal=newStr(buf2);
      }
   }
 chdir(buf);
 return retVal;
}

void TFileDialogHome::handleEvent(TEvent& event)
{
 char buf[PATH_MAX],buf2[PATH_MAX];
 char endIt=0,cancelIt=0,advance=1;
 char *newDir;

 if (event.what==evBroadcast)
   {
    if ((event.message.command==cmReleasedFocus && event.message.infoPtr==link)
        || (event.message.command==cmRecordHistory && !letHistoryPass))
      {
       clearEvent(event);
       return;
      }
    if (event.message.command==cmFileDoubleClicked)
      {
       event.what=evCommand;
       event.message.command=cmFileOpen;
       advance=0;
      }
   }
 if (event.what==evCommand)
    switch(event.message.command)
      {
       case cmHomeDir:
            DeleteArray(directory);
            directory=newStr(homeDirectory);
            fileList->readDirectory(homeDirectory,wildCard);
            clearEvent(event);
            return;
       case cmChangeDir:
            newDir=ChooseDir(directory);
            if (newDir)
              {
               DeleteArray(directory);
               directory=newDir;
               fileList->readDirectory(newDir,wildCard);
              }
            clearEvent(event);
            return;
       case cmFileOpen:
       case cmFileReplace:
       case cmFileClear:
       case cmFileSelect:
            if (valid(event.message.command))
              { // Valid checks the name and change the directory if that's the case
               if (flags & fdMultipleSel)
                 {// Copy the directory to remmember it
                  strcpy(selDir,directory);
                  getFileName(buf);
                  if (message(TProgram::deskTop,evBroadcast,
                      cmFileDialogFileSelected,buf))
                    {
                     if (advance)
                       {
                        int foc=fileList->focused+1;
                        if (foc<fileList->range)
                           fileList->focusItemNum(foc);
                       }
                    }
                  else
                     endIt=1;
                 }
               else
                  endIt=1;
              }
            if (endIt)
              {// Copy the directory to remmember it
               strcpy(selDir,directory);
               endModal(event.message.command);
               link->getData(buf);
               getFileName(buf2);
               link->setData(buf2);
               letHistoryPass=1;
               message(this,evBroadcast,cmRecordHistory,0);
               letHistoryPass=0;
               if (strchr(buf,'/')==0)
                 {// If the user entered something that isn't here
                  // don't add this point
                  strcpy(buf2,directory);
                  strcat(buf2,wildCard);
                  link->setData(buf2);
                  event.what=evBroadcast;
                  event.message.command=cmRecordHistory;
                 }
               else
                 {
                  clearEvent(event);
                  return;
                 }
              }
            else
              {
               clearEvent(event);
               return;
              }
            break;
       // If the user aborts also add it to the history, just in case s/he wants to
       // reuse it.
       case cmCancel:
            endModal(event.message.command);
            link->getData(buf);
            strcpy(buf2,directory);
            strcat(buf2,wildCard);
            link->setData(buf2);
            event.what=evBroadcast;
            event.message.command=cmRecordHistory;
            cancelIt=1;
            break;
       // Configuration dialog
       case cmFileOpenOptions:
            if (ConfDiag())
               fileList->readDirectory(directory,wildCard);
            clearEvent(event);
            return;
       /*case cmeZoom:
            event.message.command=cmZoom;
            TDialog::handleEvent(event);
            break;*/
      }
 TDialog::handleEvent(event);
 if (endIt)
    link->setData(buf);
 // Store the current size in the rectangle provided by the caller
 if (endIt || cancelIt)
    *returnSize=getBounds();
}

static char *LastMaskUsed=0;

int GenericFileDialog(const char *title, char *file, char *mask, int histID, int buttons,
                      char *dir, unsigned flags, int ctx, TRect &dialogSize)
{
 char curDir[PATH_MAX];
 char dirChanged=0;

 getcwd(curDir,PATH_MAX);
 strcat(curDir,"/");
 if (dir[0])
   {
    chdir(dir);
    dirChanged=1;
   }
 int fgButtons=buttons | fdHelpButton;
 // If that's a wildcard the directory will be scanned by setdata, so avoid reading
 // 2 directories.
 if ((!mask || !mask[0]) && CLY_IsWild(file))
    fgButtons|=fdNoLoadDir;
 TFileDialogHome *d=new TFileDialogHome(mask,title,__("~N~ame"),fgButtons,histID,curDir,
                                        flags,dir,dialogSize);
 d->helpCtx=ctx;

 int result=cmCancel;
 TView *p=TProgram::application->validView(d);
 if (p)
   {
    if (!mask || !mask[0])
       p->setData(file);

    result=TProgram::deskTop->execView(p);
    if (result!=cmCancel)
       p->getData(file);
    delete[] LastMaskUsed;
    LastMaskUsed=newStr(d->wildCard);

    TObject::CLY_destroy(p);
   }

 if (dirChanged)
    chdir(curDir);
 return result;
}

char *GetLastMaskUsed()
{
 char *ret=LastMaskUsed;
 LastMaskUsed=0;
 return ret;
}

void SetConfigDialogFunc(int (*func)())
{
 TFileDialogHome::ConfDiag=func;
}
/*
static char *dirForOpen=0;
static char *dirForSave=0;

int FileOpenDialog(char *title, char *file, int historyID, unsigned flags)
{
 if (!dirForOpen)
   {
    dirForOpen=new char[PATH_MAX];
    getcwd(dirForOpen,PATH_MAX);
   }      
 return GenericFileDialog(title,file,file,historyID,fdOpenButton,dirForOpen,flags,hcGenOpenFile);
}

int FileSaveDialog(char *title, char *file, int historyID, unsigned flags)
{
 if (!dirForSave)
   {
    dirForSave=new char[PATH_MAX];
    getcwd(dirForSave,PATH_MAX);
   }
 return GenericFileDialog(title,file,file,historyID,fdOKButton,dirForSave,flags,hcGenOpenFile);
}

void SetDirForOpen(char *s)
{
 if (dirForOpen)
    delete dirForOpen;
 dirForOpen=s;
}

void SetDirForSave(char *s)
{
 if (dirForSave)
    delete dirForSave;
 dirForSave=s;
}
*/

