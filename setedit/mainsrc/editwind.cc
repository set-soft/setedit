/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// That's the first include because is used to configure the editor.
#include "ceditint.h"

#define Uses_TFrame
#define Uses_TWindow
#define Uses_TStreamableClass
#define Uses_TStreamable
#define Uses_opstream
#define Uses_ipstream
#define Uses_TStringCollection
#define Uses_TPalette
#define Uses_TScreen

#define Uses_TCEditWindow
#define Uses_TCEditor_Commands
#include <ceditor.h>

//  Palette management added by Robert to allow the customization of the
// palette.
#define cpBlueEditWindow "\x08\x09\x0A\x0B\x0C\x40\x41\x42\x43\x44\x45\x46"\
                         "\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x76"\
                         "\x77\x78\x79\x7A\x7B\x7C"
#define cpCyanEditWindow "\x10\x11\x12\x13\x14\x52\x53\x54\x55\x56\x57\x58"\
                         "\x59\x5A\x5B\x5C\x5D\x5E\x5F\x60\x61\x62\x63\x76"\
                         "\x77\x78\x79\x7A\x7B\x7C"
#define cpGrayEditWindow "\x18\x19\x1A\x1B\x1C\x64\x65\x66\x67\x68\x69\x6A"\
                         "\x6B\x6C\x6D\x6E\x6F\x70\x71\x72\x73\x74\x75\x76"\
                         "\x77\x78\x79\x7A\x7B\x7C"

const int   TCEditWindow::ResumeVersion=6;
const char *TCEditWindow::clipboardTitle=__("Clipboard");
const char *TCEditWindow::untitled=__("Untitled");
stTVIntl   *TCEditWindow::iClipboardTitle=NULL;
stTVIntl   *TCEditWindow::iUntitled=NULL;

TPalette & TCEditWindow::getPalette() const
{
  static TPalette blue(cpBlueEditWindow,sizeof(cpBlueEditWindow)-1);
  static TPalette cyan(cpCyanEditWindow,sizeof(cpCyanEditWindow)-1);
  static TPalette gray(cpGrayEditWindow,sizeof(cpGrayEditWindow)-1);
  static TPalette * palettes[] =
  {
    &blue,
    &cyan,
    &gray
  };
  return *(palettes[palette]);
}

const TPoint minEditWinSize = {24, 6};

TCEditWindow::TCEditWindow( const TRect& bounds,
                            const char *fileName,
                            int aNumber,
                            Boolean openRO ) :
    TWindowInit( &TCEditWindow::initFrame ),
    TWindow( bounds, 0, aNumber )
{
    options |= ofTileable;

    TScrollBar *hScrollBar =
        new TScrollBar( TRect( 18, size.y - 1, size.x - 2, size.y ) );
    hScrollBar->hide();
    insert(hScrollBar);

    TScrollBar *vScrollBar =
        new TScrollBar( TRect( size.x - 1, 1, size.x, size.y - 1 ) );
    vScrollBar->hide();
    insert(vScrollBar);

    TSIndicator *indicator =
        new TSIndicator( TRect( 2, size.y - 1, 16, size.y ) );
    indicator->hide();
    insert(indicator);


    TRect r( getExtent() );
    r.grow(-1, -1);
    editor=new TCEditor(r,hScrollBar,vScrollBar,indicator,fileName,openRO);
    insert(editor);
}


void TCEditWindow::close()
{
 if (editor->isClipboard())
    hide();
 else
    TWindow::close();
}

const char *TCEditWindow::getTitle( short )
{
    if( editor->isClipboard() )
        return TVIntl::getText(clipboardTitle,iClipboardTitle);
    else if( *(editor->fileName) == EOS )
        return TVIntl::getText(untitled,iUntitled);
    else
        return editor->fileName;
}

void TCEditWindow::handleEvent( TEvent& event )
{
    TWindow::handleEvent(event);
    if( event.what == evBroadcast && event.message.command == cmcUpdateTitle )
        {
        if( frame != 0 )
            frame->drawView();
        clearEvent(event);
        }
}

void TCEditWindow::sizeLimits( TPoint& min, TPoint& max )
{
    TWindow::sizeLimits(min, max);
    min = minEditWinSize;
}

void TCEditWindow::write( opstream& os )
{
    TWindow::write( os );
    os << editor;
}

void *TCEditWindow::read( ipstream& is )
{
 TWindow::read( is );
 is >> editor;

 /* It can be done only here because I must remove the old version from the
    Group and that's inserted after the read */
 if (editor->OldIndicator)
   {
    remove(editor->OldIndicator);
    editor->indicator->hide();
    editor->indicator->editor=editor;
    insert(editor->indicator);
   }
 return this;
}

TCEditWindow::TCEditWindow( StreamableInit ) :
    TWindowInit( NULL ),
    TWindow( streamableInit )
{
}

void TCEditWindow::EnlargeSizesResume(EditorResume &r)
{
 int wS=TScreen::getCols(), hS=TScreen::getRows();
 EnlargeSizeResume(r.origin_x,r.origin_y,wS,hS);
 EnlargeSizeResume(r.size_x,r.size_y,wS,hS);
 EnlargeSizeResume(r.zorigin_x,r.zorigin_y,wS,hS);
 EnlargeSizeResume(r.zsize_x,r.zsize_y,wS,hS);
}

void TCEditWindow::ReduceSizesResume(EditorResume &r)
{
 int wS=TScreen::getCols(), hS=TScreen::getRows();
 ReduceSizeResume(r.origin_x,r.origin_y,wS,hS);
 ReduceSizeResume(r.size_x,r.size_y,wS,hS);
 ReduceSizeResume(r.zorigin_x,r.zorigin_y,wS,hS);
 ReduceSizeResume(r.zsize_x,r.zsize_y,wS,hS);
}

void TCEditWindow::EnlargeSizeResume(short &x, short &y, int wS, int hS)
{
 int max=2*wS-1;
 if (x>max) x=max;
 x=(x*0x4000+wS/2)/wS;
 max=2*hS-1;
 if (y>max) y=max;
 y=(y*0x4000+hS/2)/hS;
}

void TCEditWindow::ReduceSizeResume(short &x, short &y, int wS, int hS)
{
 x=(x*wS+0x2000)/0x4000;
 y=(y*hS+0x2000)/0x4000;
}

void TCEditWindow::FillResume(EditorResume &r)
{
 r.version=ResumeVersion;
 r.shl=editor->SyntaxHL;
 r.subshl=editor->GenericSHL;

 r.origin_x=origin.x; r.origin_y=origin.y;
 r.size_x=size.x; r.size_y=size.y;
 r.cursor_x=editor->curPos.x; r.cursor_y=editor->curPos.y;
 r.zorigin_x=zoomRect.a.x; r.zorigin_y=zoomRect.a.y;
 r.zsize_x=zoomRect.b.x; r.zsize_y=zoomRect.b.y;
 r.ed_flags=editor->CompactFlags();
 r.tabSize=editor->tabSize;
 r.indentSize=editor->indentSize;
 r.wrapCol=editor->WrapCol;

 EnlargeSizesResume(r);

 r.extraSize=sizeof(EditorResume)-(sizeof(EditorResumeV5)+sizeof(uint32));
 time(&r.dateResume);
}

void TCEditWindow::ApplyResume(EditorResume &r)
{
 ReduceSizesResume(r);
 moveTo(r.origin_x,r.origin_y);
 growTo(r.size_x,r.size_y);
 zoomRect.a.x=r.zorigin_x;
 zoomRect.a.y=r.zorigin_y;
 zoomRect.b.x=r.zsize_x;
 zoomRect.b.y=r.zsize_y;
 editor->tabSize=r.tabSize;
 editor->indentSize=r.indentSize;
 editor->WrapCol=r.wrapCol;
 editor->MoveCursorTo(r.cursor_x,r.cursor_y);
 editor->trackCursor(True);
 editor->update(ufView); // needed if trackCursor didn't do it
 if (r.shl!=255)
   {
    editor->ExpandFlags(r.ed_flags,False);
    editor->SetHighlightTo((shlState)r.shl,r.subshl);
   }
}

static inline
unsigned MoveFlags(unsigned flags, unsigned mask, unsigned pos)
{
 return (flags & mask) | ((flags & (~mask))<<pos);
}

void TCEditWindow::ReadResume(EditorResume &r, ipstream& is)
{
 uchar version;

 is >> version;
 if (version>ResumeVersion) version=2;
 r.version=version;
 if (version<3)
   { // v1 and v2
    EditorResumeV2 r2;
    is.readBytes(((char *)&r2)+1,sizeof(EditorResumeV2)-1);
    if (r.version==1)
      { // In v0.4.31 I moved some flags so now resumes changed version
       r.ed_flags=MoveFlags(r.ed_flags,0xFF,1);
       r.version=2;
      }
    // Translate it into the v3 structure.
    r.version=3;
    r.shl=r2.shl;
    r.subshl=r2.subshl;
    r.origin_x=r2.origin_x;
    r.origin_y=r2.origin_y;
    r.size_x=r2.size_x;
    r.size_y=r2.size_y;
    r.cursor_x=r2.cursor_x;
    r.cursor_y=r2.cursor_y;
    r.zorigin_x=r2.zorigin_x;
    r.zorigin_y=r2.zorigin_y;
    r.zsize_x=r2.zsize_x;
    r.zsize_y=r2.zsize_y;
    r.ed_flags=r2.ed_flags;
    r.prj_flags=r2.prj_flags;
    // exits as v3
   }
 else
   {
    if (version==3)
       // v3
       is.readBytes(((char *)&r)+1,sizeof(EditorResumeV3)-1);
    else
      {// v4 & v5
       is.readBytes(((char *)&r)+1,sizeof(EditorResumeV5)-1);
       if (version>=6)
         {
          is >> r.extraSize;
          if (r.extraSize==0) r.extraSize=4; // Workaround
          is.readBytes(&r.dateResume,r.extraSize);
         }
      }
   }
 if (r.version==3)
   {
    r.version=4;
    r.tabSize=TCEditor::staticTabSize;
    r.indentSize=TCEditor::staticIndentSize;
    r.wrapCol=TCEditor::staticWrapCol;
   }
 if (r.version==4)
   {
    r.version=5;
    EnlargeSizesResume(r);
   }
 if (r.version==5)
   {
    r.version=6;
    r.extraSize=sizeof(EditorResume)-(sizeof(EditorResumeV5)+sizeof(uint32));
    time(&r.dateResume);
   }
}

void TCEditWindow::SaveResume(EditorResume &r, opstream& os)
{
 // If the file wasn't closed the resume is empty, even the version!
 r.version=ResumeVersion;
 // It must be filled properly.
 // PrjItem *TPrjItemColl::createNewElement(char *name, int flags) doesn't
 // fill it for new (and hence empty) items.
 r.extraSize=sizeof(EditorResume)-(sizeof(EditorResumeV5)+sizeof(uint32));
 os.writeBytes(&r,sizeof(EditorResume));
}

/**[txh]********************************************************************

  Description:
  Used when reading desktop versions older than 0.3.6.
  
***************************************************************************/

void TCEditWindow::FillResumeWith(EditorResume &r, TPoint &origin,
                                  TPoint &size, TPoint &cursor)
{
 r.version=ResumeVersion;
 r.shl=255; // Unknown, is applied to the flags too
 r.zorigin_x=r.origin_x=origin.x;
 r.zorigin_y=r.origin_y=origin.y;
 r.zsize_x=r.size_x=size.x;
 r.zsize_y=r.size_y=size.y;
 r.cursor_x=cursor.x; r.cursor_y=cursor.y;
 r.tabSize=TCEditor::staticTabSize;
 r.indentSize=TCEditor::staticIndentSize;
 r.wrapCol=TCEditor::staticWrapCol;
 r.ed_flags=0;
 EnlargeSizesResume(r);
 r.extraSize=sizeof(EditorResume)-(sizeof(EditorResumeV5)+sizeof(uint32));
 time(&r.dateResume);
}

TCEditWindow::~TCEditWindow()
{
 TVIntl::freeSt(iClipboardTitle);
 TVIntl::freeSt(iUntitled);
}
