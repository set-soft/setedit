/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#define Uses_string

#define Uses_TSIndicator
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TView
#define Uses_opstream
#define Uses_ipstream
#define Uses_TStreamableClass
#define Uses_TCEditor
#define Uses_TScreen
#include <ceditor.h>

void TSIndicator::draw()
{
 uchar color, frame;
 TDrawBuffer b;
 char s[15];

 if ((state & sfDragging)==0)
   {
    color = getColor(1);
    frame = dragFrame;
   }
 else
   {
    color = getColor(2);
    frame = normalFrame;
   }

 b.moveChar(0,frame,color,size.x);
 
 if (editor->isReadOnly)
    b.putChar(0,'R');
 else
   if (editor->modified)
      b.putChar(0,modifiedStar);

 if (!TScreen::cursorShapes())
    b.putChar(1,editor->overwrite ? 'O' : 'I');
      
 sprintf(s," %d:%d ",editor->curPos.y+1,editor->curPos.x+1);
 b.moveCStr(8-int(strchr(s,':')-s),s,color);
 
 writeBuf(0,0,size.x,1,b);
}

TStreamable *TSIndicator::build()
{
 return new TSIndicator(streamableInit);
}

TSIndicator::TSIndicator(StreamableInit) : TIndicator( streamableInit )
{
}

