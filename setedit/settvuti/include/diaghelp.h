/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class TDialog;
ushort execDialogNoDestroy(TDialog *d, void *data, char &lSet);
void AddOKCancel(TDialog *d);
ushort messageBoxDSA(const char *msg, ushort aOptions, char *var, ushort defComm);
ushort messageBoxDSA(const char *msg, ushort aOptions, char *var,
                     unsigned mask, ushort defComm);
int GetDeskTopCols();
int GetDeskTopRows();
TRect GetDeskTopSize();

extern __inline__
void MoveRectBut(TRect &r,int len,int sep)
{
 r.a.x=r.b.x+sep;
 r.b.x=r.a.x+len;
}

#define Destroy(p) { CLY_destroy(p); p=0; }
