/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TDialogAID) && !defined(__TDialogAID__)
#define __TDialogAID__

class TDialogAID : public TGrowDialog
{
public:
 TDialogAID(const TRect& bounds, const char *aTitle,
            TStringableListBox *slb);
 virtual void handleEvent(TEvent& event);
 int (*AddAction)(void);
 int (*InsAction)(int);
 int (*DelAction)(int);
 int (*OkAction)(void);
 int (*CancelAction)(void);
 int (*InfoAction)(int);
 int (*BrowseAction)(void);
 TStringableListBox *List;
 unsigned flags;
};

const int
      cmAddKey     =0x2240,
      cmDeleteKey  =0x2241,
      cmInsertKey  =0x2242,
      cmAddCommand =0x2243,
      cmAddMacro   =0x2244,
      cmOKApply    =0x2245,
      cmCancelApply=0x2246,
      cmInfoAID    =0x2247,
      cmBrowseAID  =0x2248;
const int aidInsert=1, aidComMac=2, aidOKEnabled=4, aidInfo=8, aidBrowse=16;
// For CreateChooseDialog
const int aidStringable=0x1000,aidHzScroll=0x2000,aidNoCancel=0x4000;

#if 1
// Ugh! nasty, no? Fix me ... some day
#define Uses_SETAppConst
#include <../../setedit/include/setapp.h>
#else
#define cmeZoom cmZoom
#endif

// Used to create the dialogs
TDialogAID *CreateAddInsDelDialog(int x, int y, const char *name, int h, int w,
                                  int flags);
TDialog *CreateChooseDialog(int x, int y, const char *name, int h, int w,
                            unsigned options=0);

// Just handles cmeZoom
class TGrowDialogZ : public TGrowDialog
{
public:
 TGrowDialogZ( TRect r, const char *name, int extraOptions=0 ) :
      TWindowInit( &TGrowDialogZ::initFrame ),
      TGrowDialog(r,name,extraOptions) {};
 void handleEvent(TEvent& event);
};

#endif
