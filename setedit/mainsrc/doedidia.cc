/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
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
#define Uses_MsgBox
#define Uses_fpstream
#define Uses_TPoint
#define Uses_TView
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TCEditor_Commands
#define Uses_TCEditor_External
#define Uses_StrStream
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
 #define StrError(a) sys_errlist[a]
#endif

unsigned doEditDialog(int dialog, va_list arg)
{
 CreateStrStream(os,buf,PATH_MAX+80);
 switch(dialog)
   {
    case edOutOfMemory:
         return messageBox(_("Not enough memory for this operation"),
                           mfError | mfOKButton);
    case edReadError:
        {
        os << _("Error reading file ") << va_arg(arg,_charPtr)
           << ". " << StrError(errno) << " (" << errno << ")"
           << CLY_std(ends);
        va_end(arg);
        return messageBox(GetStrStream(buf),mfError | mfOKButton);
        }
    case edWriteError:
        {
        os << _("Error writing file ") << va_arg(arg,_charPtr)
           << "." << CLY_std(ends);
        va_end(arg);
        return messageBox(GetStrStream(buf),mfError | mfOKButton);
        }
    case edCreateError:
        {
        os << _("Error creating file ") << va_arg(arg,_charPtr)
           << ". " << StrError(errno) << " (" << errno << ")"
           << CLY_std(ends);
        va_end(arg);
        return messageBox(GetStrStream(buf),mfError | mfOKButton);
        }
    case edCreateTMPError:
         return messageBox(_("Error creating temporal file, operation aborted"),mfError | mfOKButton);
    case edSaveModify:
        {
        os << va_arg(arg,_charPtr)
           << _(" has been modified. Save?") << CLY_std(ends);
        va_end(arg);
        return messageBox(GetStrStream(buf),mfInformation | mfYesNoCancel);
        }
    case edSaveUntitled:
        return messageBox( _("Save untitled file?"),
                           mfInformation | mfYesNoCancel );
    case edSaveAs:
         return GenericFileDialog(_("Save file as"),va_arg(arg,_charPtr),"*",
                                  hID_FileSave,fdDialogForSave);
         /*return execDialog( new TFileDialog( "*.*",
                                             _("Save file as"),
                                             _("~N~ame"),
                                             fdOKButton,
                                             101 ), va_arg(arg,_charPtr) );*/
    case edSaveBlock:
         return GenericFileDialog(_("Write to file"),va_arg(arg,_charPtr),0,
                                  hID_SaveBlock,fdDialogForSave);
    case edReadBlock:
         return GenericFileDialog(_("Read from file"),va_arg(arg,_charPtr),0,
                                  hID_SaveBlock);
 
    case edFind:
        {
         void *box1=va_arg(arg,void *);
         void *box2=va_arg(arg,void *);
         return execDialog(createFindDialog(box2),box1);
        }
 
    case edSearchFailed:
        return messageBox( _("Search string not found."),
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
         os << bytes << _(" bytes selected, in ") << lines << _(" lines")
            << CLY_std(ends);
         return messageBox(GetStrStream(buf),mfInformation | mfOKButton);
        }
 
    case edGotoLine:
        {
         int *p;
 
         p=va_arg(arg,int *);
         os << *p << CLY_std(ends);
 
         if (execDialog(createGotoLineDialog(),(void *)GetStrStream(buf))==cmOK)
           {
            sscanf(GetStrStream(buf),"%d",p);
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
         return messageBox(_("Line too long, cut it?"),
                           mfError | mfYesButton | mfNoButton);

    case edLineTooLong:
         return messageBox(_("Line extremely long, can't handle it reliably, sorry"),
                           mfError | mfOKButton);
 
    case edNotFromDisk:
         return messageBox(_("This file isn't in disk!"),
                           mfInformation | mfOKButton);
 
    case edIsReadOnly:
         return messageBox(_("The file is read-only, try to revert it?"),
                           mfWarning | mfYesButton | mfNoButton);
    case edStillReadOnly:
         return messageBox(_("Failed to revert the read-only status, you won't be able to overwrite this file"),
                           mfError | mfOKButton);

    case edActionWOUndo:
         return messageBox(_("This action doesn't have undo, are you sure?"),
                           mfWarning | mfYesButton | mfNoButton);

    case edFileExists:
         os << va_arg(arg,_charPtr) << _(" already exist, overwrite?")
            << CLY_std(ends);
         va_end(arg);
         return messageBox(GetStrStream(buf),mfYesButton | mfNoButton | mfWarning);

    case edFileNoFile:
         os << va_arg(arg,_charPtr) << _(" isn't a file, probably a device, go ahead?")
            << CLY_std(ends);
         va_end(arg);
         return messageBox(GetStrStream(buf),mfYesButton | mfNoButton | mfWarning);

    case edCantBkp:
         return messageBox(_("Can't make a back up file, continue saving?"),mfYesButton | mfNoButton | mfError);

    case edRectOutside:
         messageBox(_("You can't paste a rectangle crossing the end of the file"),mfError | mfOKButton);
         break;

    case edExportHTMLOps:
        {
         void *p;
         p=va_arg(arg,void *);
         return (execDialog(createHTMLExportOps(),p)==cmOK);
        }

    case edNoPMacrosAvail:
         return messageBox(_("No pseudo macros available"),mfError | mfOKButton);

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
         return messageBox(_("The file is compressed. Do you want to save compressing?"),mfInformation | mfYesNoCancel);
   }
 return 0;
}


