/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#include <ctype.h>
#define Uses_string
#include <limits.h>

#define Uses_TStringCollection
#define Uses_TSortedListBox
#define Uses_TScrollBar
#define Uses_TPalette
#define Uses_TKeys
#define Uses_TEvent
#define Uses_TProgram
#define Uses_TDeskTop

#define Uses_TCEditor_Internal // isSymbol, etc.
#define Uses_TCEditor          // SHL cache
#include <ceditor.h>
#include <completi.h>

/**[txh]********************************************************************

  Description:
  Special TSortedListBox class that handles key events so they can be used
for word completion.

***************************************************************************/

class TCompletionBox : public TSortedListBox
{
public:
 TCompletionBox(const TRect& bounds, TScrollBar *aScrollBar) :
   TSortedListBox(bounds,1,aScrollBar) { moveIt=0; };

 virtual void draw();
 void setSearchPos(ushort val) { searchPos=val-1; moveIt=1; };
 virtual void handleEvent(TEvent& event);

 char endChar;

protected:
 char moveIt;
};

void TCompletionBox::draw()
{
 TSortedListBox::draw();
 if (moveIt && (state & (sfSelected | sfActive))==(sfSelected | sfActive) && range>0)
   {
    moveIt=0;
    setCursor(cursor.x+searchPos,cursor.y);
   }
}

void TCompletionBox::handleEvent(TEvent& event)
{
 if (event.what==evKeyDown)
   {
    unsigned code=event.keyDown.keyCode;
    unsigned char v=event.keyDown.charScan.charCode;
    // Valid character for move
    if (code==kbUp || code==kbDown || code==kbPgUp || code==kbPgDn || code==kbBack)
       TSortedListBox::handleEvent(event);
    else
    // Accepted selection
    if (code==kbEnter)
      {
       endChar=0;
       endModal(cmOK);
       clearEvent(event);
      }
    else
    if (isSymbol(v) || v==' ')
      {
       endChar=v;
       endModal(cmOK);
       clearEvent(event);
      }
    else
    // Incrementar search
    if ((searchPos==USHRT_MAX && isWordCharBe(v)) || isWordCharIn(v))
       TSortedListBox::handleEvent(event);
    else
      {
       // End rejecting
       endModal(cmCancel);
       clearEvent(event);
      }
   }
 else
    TSortedListBox::handleEvent(event);
}

/**[txh]********************************************************************

  Description:
  Simple TGroup with the TDialog palette, used to create groups without a
frame.

***************************************************************************/

class TNoFrame : public TGroup
{
public:
 TNoFrame(const TRect &r) : TGroup(r) {};

 virtual TPalette& getPalette() const;
 virtual void setCursor(int x, int y);
};

// Same as a TDialog
#define cpNoFrame "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F"\
                  "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F"

TPalette& TNoFrame::getPalette() const
{
 static TPalette palette(cpNoFrame,sizeof(cpNoFrame)-1);
 return palette;
}

void TNoFrame::setCursor(int x, int y)
{
 TGroup::setCursor(x+origin.x,y+origin.y);
}


char *CompletionChooseFromList(TStringCollection *list, int cant, int len,
                               int xC, int yC, unsigned ops, int lPartial)
{
 // Don't pop-up if the answer is obvious
 if (cant<1)
    return 0;
 if (cant==1)
    return newStr((char *)list->at(0));

 // Heavy stuff: position the list in a good place
 TRect dktR=TProgram::deskTop->getExtent();
 int dktW=dktR.b.x-dktR.a.x;
 int dktH=dktR.b.y-dktR.a.y;
 int x,y,h,w;
 len+=3; // 1 at left, 1 at right and 1 for scroll bar

 if (yC+cant>=dktH && yC>dktH/2)
   {// Do it in the reverse direction
    h=min(yC,cant);
    y=yC-h;
   }
 else
   {
    y=yC+1;
    h=min(dktH-yC-TProgram::deskTop->origin.y,cant);
   }

 w=min(dktW-2,len);
 if (xC+w>=dktW)
    x=dktW-1-w;
 else
    x=xC;

 TNoFrame *group=new TNoFrame(TRect(x,y,x+w,y+h));
 TListBoxRec r={list,0};
 TScrollBar *ts=h<cant ? new TScrollBar(TRect(w-1,0,w,h)) : NULL;
 TCompletionBox *b=new TCompletionBox(TRect(0,0,w-(ts ? 1 : 0),h),ts);
 b->setData(&r);
 if (lPartial)
    b->setSearchPos(lPartial);
 group->insert(b);
 b->setState(sfSelected | sfActive,True);
 if (ts)
   {
    group->insert(ts);
    ts->show();
   }

 char *ret;
 if (TProgram::deskTop->execView(group)==cmOK)
   {
    b->getData(&r);
    char *s=(char *)list->at(r.selection);
    int l=strlen(s)+1;
    ret=new char[l+1];
    strcpy(ret,s);
    if ((ops & cmplDontAddEndChar)==0 && b->endChar)
      {
       ret[l-1]=b->endChar;
       ret[l]=0;
      }
   }
 else
   ret=0;
 CLY_destroy(group);

 return ret;
}

char *CompletionChoose(char *options, const char *delimiter, int x, int y,
                       unsigned ops)
{
 if (!options || !delimiter) return 0;

 int numOps=0,lenDelim=strlen(delimiter),maxLen=0,thisLen;
 char *curPos=options,*posAnt=options;
 TStringCollection *list=new TStringCollection(10,4);
 list->setOwnerShip(False);

 // Split the string
 do
   {
    if ((curPos=strstr(curPos,delimiter))!=0)
      {
       memset(curPos,0,lenDelim);
       curPos+=lenDelim;
      }
    numOps++;
    if (*posAnt)
      {
       thisLen=strlen(posAnt);
       if (thisLen>maxLen)
          maxLen=thisLen;
       list->insert(posAnt);
       posAnt=curPos;
      }
   }
 while (curPos);

 char *ret=CompletionChooseFromList(list,list->getCount(),maxLen,x,y,ops);

 // Restore it
 curPos=options;
 numOps--;
 for (int i=0; i<numOps; i++)
    {
     for (; *curPos; curPos++);
     memcpy(curPos,delimiter,lenDelim);
     curPos+=lenDelim;
    }

 return ret;
}

