/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSButton) && !defined(__TSButton_Defined__)
#define __TSButton_Defined__

class TSButtonBase : public TSView
{
public:
 void fill(int type, const char *aTitle, ushort aCommand, ushort aFlags,
           int wForced, TButtonCallBack cb, void *cbData);
 virtual void insert(TDialog *d);
};

class TSButton : public TSButtonBase
{
public:
 TSButton(const char *aTitle, ushort aCommand=cmOK, ushort aFlags=bfNormal);
 TSButton(const char *aTitle, ushort aCommand, ushort aFlags, int wForced);
 TSButton(const char *aTitle, ushort aCommand, ushort aFlags,
          TButtonCallBack cb, void *cbData=NULL);
 TSButton(const char *aTitle, ushort aCommand, ushort aFlags, int wForced,
          TButtonCallBack cb, void *cbData=NULL);
};

class TSButtonRef : public TSButtonBase
{
public:
 TSButtonRef(const char *aTitle, ushort aCommand=cmOK, ushort aFlags=bfNormal);
 TSButtonRef(const char *aTitle, ushort aCommand, ushort aFlags, int wForced);
 TSButtonRef(const char *aTitle, ushort aCommand, ushort aFlags,
             TButtonCallBack cb, void *cbData=NULL);
 TSButtonRef(const char *aTitle, ushort aCommand, ushort aFlags, int wForced,
             TButtonCallBack cb, void *cbData=NULL);
};

class TSViewCol;
void EasyInsertOKCancel(TSViewCol *col, int sep=1);
void EasyInsertOKCancelSp(TSViewCol *col, int sep=1);
#endif
