/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]**********************************************************************

  Class: TInputLinePiped
  Comments:
  This class is used to connect a TInputLine object to some text buffer. The
typical application is to connect the "Find" dialog input line with the
editor's buffer.

  To use it you must provide a function to make this connection. See the
@x{::TInputLinePiped}, constructor for details.

*****************************************************************************/

#define Uses_string
#define Uses_TInputLinePiped
#define Uses_TEvent
#define Uses_TKeys
#define Uses_TCEditWindow
#include <ceditor.h>


/**[txh]**********************************************************************

 Include: tinppipe
 Module: SET TV Utils
 Description:
 This is the constructor. The "bounds" and "aMaxLen" parameters are the same
used in TInpLine. "aPipe" is the pointer to a function that makes as nexus
between the input line and the text buffer, see example.@p

 Example:
 Here is the function used in the editor:

@<pre>
static unsigned PipeOrigin;
static char    *PipeBuf;
static unsigned PipeBufLen;

int PipeTCEditor(unsigned PosRel)
{
 if (PosRel+PipeOrigin<PipeBufLen)
    return PipeBuf[PosRel+PipeOrigin];
 return -1;
}
@</pre>

  Before creating the dialog the editor sets the values of these vars,
PipeBuf is a pointer to the editors buffer, PipeOrigin is the offset of
the start of the connected text and PipeBufLen is the length of the buffer.
The input line calls to this function each time the cursor is moved to
the right passing the end of the input buffer. The PosRel parameter is the
relative offset.

*****************************************************************************/

TInputLinePiped::TInputLinePiped( const TRect& bounds, int aMaxLen,
                                  unsigned flags ) :
TInputLine(bounds,aMaxLen),
mFlags(flags)
{
 if (!TCEditor::clipboard)
    mFlags|=tilpNoCopy | tilpNoPaste;
}

/**[txh]**********************************************************************

  Description:
  The handleEvent was modified to ask for text using the provided function.

*****************************************************************************/

void TInputLinePiped::handleEvent( TEvent& event )
{
 int Pos,Ret;
 switch (event.what)
   {
    case evKeyDown:
         switch (ctrlToArrow(event.keyDown.keyCode))
           {
            case kbRight:
                 Pos=strlen(data);
                 if (curPos>=Pos)
                    if ((Ret=PipeLine(Pos))!=-1)
                       if (Ret!='\r' && Ret!='\n' && Ret!='\t')
                         {
                          event.keyDown.keyCode=0; // No key
                          event.keyDown.charScan.charCode=Ret; // ASCII
                          selStart = 0;
                          selEnd = 0;
                         }
                 break;
            case kbCtrlIns:
                 CopyToClip();
                 clearEvent(event);
                 break;
            case kbShiftIns:
                 CopyFromClip();
                 clearEvent(event);
                 break;
           }
         break;

    case evCommand:
         switch (event.message.command)
           {
            case cmtilCopy:
                 CopyToClip();
                 clearEvent(event);
                 break;
            case cmtilPaste:
                 CopyFromClip();
                 clearEvent(event);
                 break;
           }
         break;
   }
 TInputLine::handleEvent(event);
}

int  TInputLinePiped::PipeLine(unsigned pos)
{
 if (mFlags & tilpNoPipe)
    return -1;
 return PipeTCEditor(pos);
}

void TInputLinePiped::CopyToClip(void)
{
 if (mFlags & tilpNoCopy)
    return;
 unsigned Len=strlen(data);
 if (Len)
    TCEditor::clipboard->insertBuffer(data,0,Len,False,True);
}

void TInputLinePiped::CopyFromClip(void)
{
 if (mFlags & tilpNoPaste)
    return;
 TCEditor::clipboard->CopySelToBuffer(data,maxLen);
 selStart = 0;
 curPos = selEnd = strlen(data);
 drawView();
}

void TInputLinePiped::setState( uint16 aState, Boolean enable )
{
 TInputLine::setState(aState,enable);
 if (aState==sfSelected)
   {
    if (enable)
      {
       if (mFlags & tilpNoPaste)
          disableCommand(cmtilPaste);
       else
          enableCommand(cmtilPaste);
       if (mFlags & tilpNoCopy)
          disableCommand(cmtilCopy);
       else
          enableCommand(cmtilCopy);
      }
    else
      {
       disableCommand(cmtilPaste);
       disableCommand(cmtilCopy);
      }
   }
}

