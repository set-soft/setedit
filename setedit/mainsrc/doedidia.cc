/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  This module contains the default definitions for the editor dialogs. To
overwrite the behavior you must:@p

1) Assign TCEditor::editorDialog to the function to handle your dialogs.@*
2) Overwrite the needed dialogs.@*
3) Call to doEditDialog for the rest.@p

The routines needs ushort execDialog( TDialog *d, void *data ); to be
somewhere in your code to execute the dialogs.


***************************************************************************/

#include <ceditint.h>

#define Uses_stdio
#define Uses_string
#define Uses_limits
#define Uses_snprintf
#define Uses_MsgBox
#define Uses_fpstream
#define Uses_TPoint
#define Uses_TView
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TCEditor_Commands
#define Uses_TCEditor_External
#include <ceditor.h>
#include <bufun.h>
#define Uses_FileOpenAid
#include <settvuti.h>
#include <setconst.h>
#include <errno.h>

typedef char *_charPtr;
typedef TPoint *PPoint;

void ApplyBroadcast(TView *p, void *e)
{
 p->handleEvent(*(TEvent *)e);
}

#ifdef SEOSf_Solaris
 #define StrError(a) strerror(a)
#else
 #if defined(SEOSf_Linux) && ((__GLIBC__==2 && __GLIBC_MINOR__>1) || __GLIBC__>2)
  #define StrError(a) strerror(a)
 #else
  #define StrError(a) sys_errlist[a]
 #endif
#endif

unsigned doEditDialog(int dialog, va_list arg)
{
 char *s;
 switch(dialog)
   {
    case edOutOfMemory:
         return messageBox(__("Not enough memory for this operation"),
                           mfError | mfOKButton);
    case edReadError:
        {
         s=va_arg(arg,_charPtr);
         va_end(arg);
         return messageBox(mfError | mfOKButton,__("Error reading file %s. %s (%d)"),
                           s,StrError(errno),errno);
        }
    case edWriteError:
        {
         s=va_arg(arg,_charPtr);
         va_end(arg);
         return messageBox(mfError | mfOKButton,__("Error writing file %s. %s (%d)"),
                           s,StrError(errno),errno);
        }
    case edCreateError:
        {
         s=va_arg(arg,_charPtr);
         va_end(arg);
         return messageBox(mfError | mfOKButton,__("Error creating file %s. %s (%d)"),
                           s,StrError(errno),errno);
        }
    case edCreateTMPError:
         return messageBox(__("Error creating temporal file, operation aborted"),mfError | mfOKButton);
    case edSaveModify:
        {
         s=va_arg(arg,_charPtr);
         va_end(arg);
         return messageBox(mfInformation | mfYesNoCancel,
                           __("%s has been modified. Save?"),s);
        }
    case edSaveUntitled:
        return messageBox( __("Save untitled file?"),
                           mfInformation | mfYesNoCancel );
    case edSaveAs:
         return GenericFileDialog(__("Save file as"),va_arg(arg,_charPtr),"*",
                                  hID_FileSave,fdDialogForSave);
         /*return execDialog( new TFileDialog( "*.*",
                                             __("Save file as"),
                                             __("~N~ame"),
                                             fdOKButton,
                                             101 ), va_arg(arg,_charPtr) );*/
    case edSaveBlock:
         return GenericFileDialog(__("Write to file"),va_arg(arg,_charPtr),0,
                                  hID_SaveBlock,fdDialogForSave);
    case edReadBlock:
         return GenericFileDialog(__("Read from file"),va_arg(arg,_charPtr),0,
                                  hID_SaveBlock);
 
    case edFind:
        {
         void *box1=va_arg(arg,void *);
         void *box2=va_arg(arg,void *);
         return execDialog(createFindDialog(box2),box1);
        }
 
    case edSearchFailed:
        return messageBox( __("Search string not found."),
                           mfError | mfOKButton );
    case edReplace:
        {
         void *box1=va_arg(arg,void *);
         void *box2=va_arg(arg,void *);
         return execDialog(createReplaceDialog(box2),box1);
        }
 
    case edReplacePrompt:
        {
        //  Avoid placing the dialog on the same line as the cursor
        TPoint *pt=va_arg(arg,PPoint);
        return execDialog(createYesNoAllCancel(&TProgram::deskTop->size,pt),NULL);
        }
 
    case edLineLenght:
        {
         uint32 bytes=va_arg(arg,uint32);
         uint32 lines=va_arg(arg,uint32);
         return messageBox(mfInformation | mfOKButton,
                           __("%d bytes selected, in %d"),bytes,lines);
        }
 
    case edGotoLine:
        {
         int *p;
         char buf[64];
 
         p=va_arg(arg,int *);
         CLY_snprintf(buf,64,"%d",*p);
 
         if (execDialog(createGotoLineDialog(),(void *)buf)==cmOK)
           {
            sscanf(buf,"%d",p);
            return 1;
           }
         return 0;
        }

    case edJumpToPrototype:
    case edJumpToFunction:
        {
         int *p;
         char *bu,*word,*file,*shl;
         unsigned l;
 
         p=va_arg(arg,int *);
         bu=va_arg(arg,char *);
         l=va_arg(arg,unsigned);
         word=va_arg(arg,char *);
         file=va_arg(arg,char *);
         shl=va_arg(arg,char *);
 
         *p=SelectFunctionToJump(bu,l,word,dialog==edJumpToFunction ? modeBFFunctions :
                                 modeBFPrototypes,file,shl);
 
         return (*p!=-1);
        }
 
    case edSetLocalOptions:
        {
         void *box1=va_arg(arg,void *);
         ShlDiagBox *box2=va_arg(arg,ShlDiagBox *);
         return (execDialog(createSetLocalOptions(box2),box1)==cmOK);
        }
 
    case edSetGlobalOptions:
        {
         void *p;
         p=va_arg(arg,void *);
         ushort ret=execDialog( createSetGlobalOptions(), p );
         if (ret==cmApplyAll)
           {
            TEvent event;
            event.what = evBroadcast;
            event.message.command = cmcSetGlobalOptions;
            event.message.infoPtr = p;
            TProgram::deskTop->forEach(ApplyBroadcast,&event);
           }
         return (ret==cmApplyAll) || (ret==cmOK);
        }
 
    case edLineOverflow:
         return messageBox(__("Line too long, cut it?"),
                           mfError | mfYesButton | mfNoButton);

    case edLineTooLong:
         return messageBox(__("Line extremely long. Can't handle it reliably, sorry"),
                           mfError | mfOKButton);
 
    case edNotFromDisk:
         return messageBox(__("This file isn't on the disk!"),
                           mfInformation | mfOKButton);
 
    case edIsReadOnly:
        {
         char *file=va_arg(arg,_charPtr);
         return LimitedFileNameDialog(mfWarning | mfYesButton | mfNoButton,
                __("The file %s is read-only, try to revert it?"),file);
        }
    case edStillReadOnly:
         return messageBox(__("Failed to revert the read-only status, you won't be able to overwrite this file"),
                           mfError | mfOKButton);

    case edActionWOUndo:
         return messageBox(__("This action can't be undone. Are you sure?"),
                           mfWarning | mfYesButton | mfNoButton);

    case edFileExists:
         s=va_arg(arg,_charPtr);
         va_end(arg);
         return messageBox(mfYesButton | mfNoButton | mfWarning,
                           __("%s already exists. Overwrite?"),s);

    case edFileNoFile:
         s=va_arg(arg,_charPtr);
         va_end(arg);
         return messageBox(mfYesButton | mfNoButton | mfWarning,
                           __("%s isn't a file, probably a device, go ahead?"),s);

    case edCantBkp:
         return messageBox(__("Can't create a backup. Continue saving?"),mfYesButton | mfNoButton | mfError);

    case edRectOutside:
         messageBox(__("You can't paste a rectangle crossing the end of the file"),mfError | mfOKButton);
         break;

    case edExportHTMLOps:
        {
         void *p;
         p=va_arg(arg,void *);
         return (execDialog(createHTMLExportOps(),p)==cmOK);
        }

    case edNoPMacrosAvail:
         return messageBox(__("No pseudo macros available"),mfError | mfOKButton);

    case edChoosePMacro:
        {
         void *p;
         p=va_arg(arg,void *);
         return execDialog(createPMChoose(),p);
        }

    case edArbitraryIndent:
        {
         char *Buf=va_arg(arg,char *);
         int len=va_arg(arg,int);
         return execDialog(createArbitraryIndent(len),Buf);
        }

    case edFileCompMant:
         return messageBox(__("The file is compressed. Do you want to save with compression?"),mfInformation | mfYesNoCancel);
   }
 return 0;
}


