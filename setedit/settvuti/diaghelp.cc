/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TDialog
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TRect
#define Uses_TButton
#define Uses_MsgBox
#include <tv.h>

#include <edspecs.h>

ushort execDialogNoDestroy(TDialog *d, void *data, char &lSet)
{
 TView *p=TProgram::application->validView(d);
 if (p==0)
    return cmCancel;
 else
   {
    if (lSet && data!=0)
       p->setData(data);
    lSet=0;
    ushort result=TProgram::deskTop->execView(p);
    if (result!=cmCancel && data!=0)
        p->getData(data);
    return result;
   }
}

const int lBotOK=10,lBotCan=11;

void AddOKCancel(TDialog *d)
{
 TRect r=d->getExtent();

 int sep=(r.b.x-lBotOK-lBotCan)/3;
 TRect r2(sep,r.b.y-3,sep+lBotOK,r.b.y-1);
 d->insert(new TButton(r2,__("~O~K"),cmOK,bfDefault));
 r2.a.x=r2.b.x+sep;
 r2.b.x=r2.a.x+lBotCan;
 d->insert(new TButton(r2,__("Cancel"),cmCancel,bfNormal));
}

ushort messageBoxDSA(const char *msg, ushort aOptions, char *var,
                     ushort defComm)
{
 char *s=(char *)GetVariable(var);
 if (!s || *s!='1')
   {
    ushort ret=messageBox(msg,aOptions | mfDontShowAgain);
    if (ret & 0x8000)
      {
       ret&=0x7FFF;
       InsertEnvironmentVar(var,"1");
      }
    return ret;
   }
 return defComm;
}

ushort messageBoxDSA(const char *msg, ushort aOptions, char *var,
                     unsigned mask, ushort defComm)
{
 unsigned val=EnvirGetBits(var,mask);
 if (!val)
   {
    ushort ret=messageBox(msg,aOptions | mfDontShowAgain);
    if (ret & 0x8000)
      {
       ret&=0x7FFF;
       EnvirSetBits(var,mask);
      }
    return ret;
   }
 return defComm;
}

inline
void MoveRectBut(TRect &r,int len,int sep)
{
 r.a.x=r.b.x+sep;
 r.b.x=r.a.x+len;
}

int GetDeskTopCols()
{
 TRect r=TProgram::deskTop->getExtent();
 return r.b.x-r.a.x;
}

int GetDeskTopRows()
{
 TRect r=TProgram::deskTop->getExtent();
 return r.b.y-r.a.y;
}

TRect GetDeskTopSize()
{
 return TProgram::deskTop->getExtent();
}
