/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TStringableListBox) && !defined(__TStringableListBox__)
#define __TStringableListBox__
class TRect;
class TScrollBar;
class TCollection;

typedef struct
{
 TStringable *items;
 ccIndex selection;
} TStringableListBoxRec;

class TStringableListBox : public TListViewer
{
public:
 TStringableListBox(const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar);
 TStringableListBox(const TRect& bounds, ushort aNumCols, TScrollBar *aHScrollBar,
                    TScrollBar *aVScrollBar, Boolean aCenterOps=False);
 ~TStringableListBox();

 virtual uint32 dataSize();
 virtual void getData(void *rec);
 virtual void getText(char *dest, ccIndex item, short maxLen);
 virtual void newList(TStringable *aList);
 virtual void setData(void *rec);
 virtual void handleEvent(TEvent& event);
 void Update(void);

 Boolean center;

 TStringable *list();

protected: // Why if protected is enough the idiots of Borland used private?
 TStringable *items;
};

inline TStringable *TStringableListBox::list()
{
 return items;
}
#endif
