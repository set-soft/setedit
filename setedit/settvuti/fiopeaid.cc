/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  This module is a very particular case. It belongs to setedit, but is
used by InfView and the editor class itself, so I created it in the
library. Why? because IfView needs SetTVUti and the editor class needs
it too and the standalone editor is in the same situation. So I think
that's the neutral point.@*
  The bad thing is that these definitions are dependent of the three at
the same time. To avoid overheads the buffers are dynamically allocated.

***************************************************************************/

#include <limits.h>
#define Uses_string
#include <stdlib.h>

#define Uses_FileOpenAid
#define Uses_TFileDialog  // For the buttons' constants
#define Uses_fpstream     // To save/load the dirs
#include <settvuti.h>

static
AsoID Convert[]={
// Used by the standalone editor to open the files to edit
{hID_FileOpen,0,0,hcFileOpen},
// Used by the editmenu.cc to store the files under edition
{hID_FileSave,0,0,hcFileSave},
// Used by InfView to open a .info file
{hID_OpenInfo,0,0,hcOpenInfo},
// Used by the editor class to store/retreive blocks
{hID_SaveBlock,0,0,hcSaveBlock},
// Used by the standalone editor to store/retreive configuration files
{hID_ConfigFiles,0,0,hcConfigFiles},
// Used by the standalone editor to store/retreive project files
{hID_ProjectFiles,0,0,hcProjectFiles},
// Used by the standalone editor to load MP3 files
{hID_OpenMP3,0,0,hcOpenMP3},
// Used by the standalone editor to store WAV converted MP3 files
{hID_SaveMP3,0,0,hcSaveMP3},
// Used by the standalone editor to select TAG files
{hID_SelectTagFile,0,0,hcSelTagFile},
{0,0}
};


/**[txh]********************************************************************

  Description:
  It just deallocates all atexit time.

***************************************************************************/

void DeInitialize(void)
{
 int i=0;
 while (Convert[i].id)
   {
    delete[] Convert[i].dir;
    Convert[i].dir=0;
    delete[] Convert[i].mask;
    Convert[i].mask=0;
    i++;
   }
}

/**[txh]********************************************************************

  Description:
  Allocates the directory buffers for each history id used for files.

***************************************************************************/

static
int Initialize()
{
 if (Convert[0].dir==0)
   {
    int i=0;
    while (Convert[i].id)
      {
       Convert[i].dir=new char[PATH_MAX];
       if (!Convert[i].dir)
         {
          delete Convert[0].dir;
          Convert[0].dir=0;
          break;
         }
       else
          Convert[i].dir[0]=0;
       i++;
      }
    if (Convert[0].dir)
       atexit(DeInitialize);
   }
 return Convert[0].dir==0;
}

/**[txh]********************************************************************

  Description:
  Returns the associated directory buffer. It contains the current
directory for this ID. In case of fail it could return 0.

***************************************************************************/

AsoID *GetFileIDDirBuffer(int id)
{
 int i=0;

 Initialize();
 while (Convert[i].id)
   {
    if (Convert[i].id==id)
       return &Convert[i];
    i++;
   }
 return 0;
}

/**[txh]********************************************************************

  Description:
  Allows to set the current directory for a desired id.

***************************************************************************/

void SetFileIDDirValue(int id, char *dir, char *mask)
{
 AsoID *s=GetFileIDDirBuffer(id);
 if (s)
   {
    if (s->dir)
       strncpy(s->dir,dir,PATH_MAX-1);
    if (mask)
      {
       delete[] s->mask;
       s->mask=newStr(mask);
      }
   }
}


/**[txh]********************************************************************

  Description:
  Calls to the GenericFileDialog located in fileopen.cc with the directory
buffer used by the provided id and with the buttons according to the flags.

***************************************************************************/

int GenericFileDialog(const char *title, char *file, char *mask, int id, unsigned flags)
{
 char *dir=0;
 int buttons=0,ctx;
 AsoID *aso;

 aso=GetFileIDDirBuffer(id);
 if (!aso)
    return cmCancel;
 if (aso)
   {
    dir=aso->dir;
    ctx=aso->helpCtx;
    if (aso->mask)
       mask=aso->mask;
   }
 if (Initialize() || !dir)
    return cmCancel;

 if (flags & fdSelectButton)
    buttons|=fdSelectButton;
 else
 if (flags & fdAddButton)
    buttons|=fdAddButton;
 else
 if (flags & (fdDialogForSave | fdMultipleSel))
    buttons|=fdOKButton;
 else
    buttons|=fdOpenButton;

 if (flags & fdMultipleSel)
    buttons|=fdDoneButton;

 if (!mask)
    mask=file;

 int ret=GenericFileDialog(title,file,mask,id,buttons,dir,flags,ctx);

 delete[] aso->mask;
 aso->mask=GetLastMaskUsed();

 return ret;
}

const int Version=2;

void SaveFileIDDirs(fpstream &s)
{
 int i=0;
 char *p;

 s << -Version; // #@#!! I did it again :-(
 while (Convert[i].id)
   {
    s << Convert[i].id;
    p=Convert[i].dir;
    if (p)
       s.writeString(p);
    else
       s.writeString("");
    p=Convert[i].mask;
    if (p)
       s.writeString(p);
    else
       s.writeString("");
    i++;
   }
 s << 0;
}

void LoadFileIDDirs(fpstream &s, Boolean isLocal)
{
 int i,version=1;
 char aux[PATH_MAX];

 s >> i;
 if (i<0)
   {
    version=-i;
    s >> i;
   }
 do
   {
    if (i)
      {
       AsoID *p=GetFileIDDirBuffer(i);
       // isLocal means the desktop file belongs to a project or the
       // current directory, in this case we can use the directories
       // but if that's a default desktop file we can't.
       if (p && p->dir && isLocal)
         {
          s.readString(p->dir,PATH_MAX);
          delete[] p->mask;
          if (version>1)
             p->mask=s.readString();
          else
             p->mask=0;
         }
       else
         {
          s.readString(aux,PATH_MAX);
          if (version>1)
             s.readString(aux,PATH_MAX);
         }
      }
    s >> i;
   }
 while (i);
}
