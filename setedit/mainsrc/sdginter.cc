/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#ifndef SUP_SDG
#define Uses_fpstream
#include <tv.h>
void SDGInterfaceRun(void) {}
void SDGInterfaceDialog(void) {}
void SDGInterfaceSaveData(fpstream *) {}
void SDGInterfaceReadData(fpstream *) {}
#else
#include <stdio.h>
#define Uses_string
#include <txhgen.h>
#define Uses_TMLISDGDefs
#define Uses_MsgBox
#define Uses_TDialog
#define Uses_TInputLine
#define Uses_TCheckBoxes
#define Uses_TRect
#define Uses_TButton
#define Uses_TLabel
#define Uses_TSItem
#define Uses_fpstream
#define INCL_TV_H
#include <mli.h>
class TSOSListBox;
#include <edmsg.h>
#define Uses_SETAppDialogs
#define Uses_SETAppVarious
#define Uses_SETAppConst
#include <setapp.h>

extern char *DskPrjGetNextFile(int &l, int &MustBeDeleted, char *FileName);
extern int DskPrjSDGInit(void);
extern const char *GetVariable(const char *variable);

const int maxStrs=120;

typedef struct
{
 char format[maxStrs];
 char temp[maxStrs];
 char out[maxStrs];
 char files_dir[maxStrs];
 uint32 options;
} TSDGDiagRec;

static TSDGDiagRec SDGOps = {"html.frt","Generated.txh","out","",1};

static void PrintMessage(char *s)
{
 //messageBox(s, mfOKButton | mfError);
 EdShowMessage(s);
}

static
void InitOps(void)
{
 if (SDGOps.files_dir[0]==0)
   {
    const char *v;
    if ((v=GetVariable("SET_FILES"))!=NULL)
       strcpy(SDGOps.files_dir,v);
    else
       strcpy(SDGOps.files_dir,".");
   }
}

void SDGInterfaceRun(void)
{
 InitOps();

 // These variables controls SDG
 TXHGetNextFile=DskPrjGetNextFile;
 TXHKeepTemporal=SDGOps.options & 1;
 TXHFormatFile=SDGOps.format;
 TXHTempGenFile=SDGOps.temp;
 TXHOutBaseName=SDGOps.out;
 TXHFilesDir=SDGOps.files_dir;
 TXHPrintMessage=PrintMessage;

 if (DskPrjSDGInit())
    return;
 // Init the Message Window
 EdShowMessage(_("Starting SDG:"),True);

 int error;
 if ((error=TXHGenerateAll())!=0)
   {
    char buf[256];
    sprintf(buf,_("Error while %s:"),TXHGetErrorSection(error));
    EdShowMessage(buf);
    switch (error)
      {
       case 1:
            sprintf(buf,_("(%d) %s in line %d"),TXHError,TXHGetErrorMessage(),TXHLine);
            EdShowMessage(buf);
            break;
       case 0:
       case 2:
            sprintf(buf,"(%d) %s",TXHError,TXHGetErrorMessage());
            EdShowMessage(buf);
            break;
       case 3:
            sprintf(buf,_("In section %s"),TXHGetGenSect());
            EdShowMessage(buf);
            sprintf(buf,_("Error of type: %s"),MLISDGTypeError);
            EdShowMessage(buf);
            EdShowMessage(MLISDGErrorName);
            sprintf(buf,_("Code: ...%s..."),MLISDGErrorCode);
            EdShowMessage(buf);
            break;
      }
   }
 EdShowMessage(_("End of SDG"));
 ReLoadModifEditors();
}

#define AnDiag 46
#define XIns 2
#define AnIns (AnDiag-2)
#define DeltaIn 3
// Format input
#define XFRT  XIns
#define AnFRT AnIns
#define YFRT  3
// Intermediate input
#define XINT  XIns
#define AnINT AnIns
#define YINT  (YFRT+DeltaIn)
// Output file
#define XOUT  XIns
#define AnOUT AnIns
#define YOUT  (YINT+DeltaIn)
// Files directory
#define XDIR  XIns
#define AnDIR AnIns
#define YDIR  (YOUT+DeltaIn)
// Options
#define nOPS  1
#define XOPS  XIns
#define AnOPS AnIns
#define YOPS  (YDIR+2)
// Buttons
#define AnOK 10
#define AnCan 12
#define XOK  ((AnDiag-AnOK-AnCan-2)/2)
#define YOK  (YOPS+2)
#define XCan (XOK+AnOK+2)
#define YCan YOK

#define AlDiag YOK+3

static TDialog *SDGCreateDialog(void)
{
 TDialog *d=new TDialog(TRect(0,0,AnDiag,AlDiag),_("SDG Parameters"));

 d->options|=ofCentered;
 d->helpCtx =cmeSDGDialog;

 TInputLine *format=new TInputLine(TRect(XFRT,YFRT,AnFRT,YFRT+1),maxStrs);
 d->insert(format);
 d->insert(new TLabel(TRect(XFRT,YFRT-1,AnFRT,YFRT),_("~F~ormat file (.frt)"),format));

 TInputLine *intermediate=new TInputLine(TRect(XINT,YINT,AnINT,YINT+1),maxStrs);
 d->insert(intermediate);
 d->insert(new TLabel(TRect(XINT,YINT-1,AnINT,YINT),_("~I~ntermediate file"),intermediate));

 TInputLine *outfile=new TInputLine(TRect(XOUT,YOUT,AnOUT,YOUT+1),maxStrs);
 d->insert(outfile);
 d->insert(new TLabel(TRect(XOUT,YOUT-1,AnOUT,YOUT),_("~B~ase output (no extentions)"),outfile));

 TInputLine *dir=new TInputLine(TRect(XDIR,YDIR,AnDIR,YDIR+1),maxStrs);
 d->insert(dir);
 d->insert(new TLabel(TRect(XDIR,YDIR-1,AnDIR,YDIR),_("~D~irectory of formats"),dir));

 d->insert(new TCheckBoxes32(TRect(XOPS,YOPS,AnOPS,YOPS+nOPS),
     new TSItem( __("K~e~ep intermediate"), 0 )));

 d->insert(new TButton(TRect(XOK,YOK,XOK+AnOK,YOK+2),_("O~K~"),cmOK,bfDefault));
 d->insert(new TButton(TRect(XCan,YCan,XCan+AnCan,YCan+2),_("Cancel"),cmCancel,bfNormal));

 d->selectNext( False );
 return d;
}

void SDGInterfaceDialog(void)
{
 TSDGDiagRec ops;

 InitOps();
 memcpy(&ops,&SDGOps,sizeof(SDGOps));
 if (execDialog(SDGCreateDialog(),&ops)!=cmCancel)
    memcpy(&SDGOps,&ops,sizeof(SDGOps));
}

void SDGInterfaceSaveData(fpstream *f)
{
 f->writeBytes(&SDGOps,sizeof(TSDGDiagRec));
}

void SDGInterfaceReadData(fpstream *f)
{
 f->readBytes(&SDGOps,sizeof(TSDGDiagRec));
}
#endif
