/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TStringableListBox
#define Uses_TEvent
#define Uses_TKeys
#include <settvuti.h>

/* List Box used in the keybinding, I think that Borland was wrong in your
   design, using it the list box isn't limited to collections and can be
   applied to arrays, lists, etc. */
TStringableListBox::TStringableListBox( const TRect& bounds,
                    ushort aNumCols,
                    TScrollBar *aScrollBar ) :
    TListViewer(bounds, aNumCols, 0, aScrollBar),
    items(0)
{
 setRange(0);
 center=False;
}

TStringableListBox::TStringableListBox( const TRect& bounds,
                    ushort aNumCols,
                    TScrollBar *aHScrollBar,
                    TScrollBar *aVScrollBar,
                    Boolean aCenterOps ) :
    TListViewer(bounds, aNumCols, aHScrollBar, aVScrollBar),
    items(0)
{
 setRange(0);
 center=aCenterOps;
}

TStringableListBox::~TStringableListBox()
{
}

uint32 TStringableListBox::dataSize()
{
 return sizeof(TStringableListBoxRec);
}

void TStringableListBox::getData( void * rec )
{
 TStringableListBoxRec *p = (TStringableListBoxRec *)rec;
 p->items = items;
 p->selection = focused;
}

void TStringableListBox::getText( char *dest, ccIndex item, short maxChars )
{
 if (items)
   {
    items->getText(dest,item,maxChars);
    dest[maxChars]=EOS;
   }
 else
   *dest=EOS;
}

void TStringableListBox::newList( TStringable *aList )
{
 delete items;
 items=aList;
 if (aList!=0)
    setRange(aList->GetCount());
 else
    setRange(0);
 if (range>0)
    focusItem(0);
 drawView();
}

void TStringableListBox::setData( void *rec )
{
 TStringableListBoxRec *p = (TStringableListBoxRec *)rec;
 newList(p->items);
 if (center)
    focusItemCentered(p->selection);
 else
    focusItem(p->selection);
 drawView();
}

void TStringableListBox::Update(void)
{
 setRange(items->GetCount());
 drawView();
}

void TStringableListBox::handleEvent(TEvent& event)
{
 if (event.what==evKeyDown && event.keyDown.keyCode==kbSpace &&
     items->taggingSupported())
   {
    items->setTag(focused,items->isTagged(focused) ? False : True);
    drawView();
    clearEvent(event);
    return;
   }
 TListViewer::handleEvent(event);
}

