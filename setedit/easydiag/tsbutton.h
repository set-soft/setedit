/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSButton) && !defined(__TSButton_Defined__)
#define __TSButton_Defined__

class TSButton : public TSView
{
public:
 TSButton(const char *aTitle, ushort aCommand=cmOK, ushort aFlags=bfNormal);
 TSButton(const char *aTitle, ushort aCommand, ushort aFlags, int wForced);
 TSButton(const char *aTitle, ushort aCommand, ushort aFlags,
          TButtonCallBack cb);
 TSButton(const char *aTitle, ushort aCommand, ushort aFlags, int wForced,
          TButtonCallBack cb);
 void fill(const char *aTitle, ushort aCommand, ushort aFlags, int wForced,
           TButtonCallBack cb);
 virtual void insert(TDialog *d);
};

class TSViewCol;
void EasyInsertOKCancel(TSViewCol *col, int sep=1);
void EasyInsertOKCancelSp(TSViewCol *col, int sep=1);
#endif
