/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Class:
  TInputScanKey

  Comments:
  That's a simple class to watch the scan codes and your ASCII. Is useful
to configure non-US keyboards.@p
  The class is basically TInputLine but super-simplified for this silly
purppose.

***************************************************************************/
#define Uses_string
#include <stdio.h>

#define Uses_TInputScanKey
#define Uses_TDrawBuffer
#define Uses_TEvent
//#define Uses_opstream
//#define Uses_ipstream
//#define Uses_TStreamableClass
#define Uses_TPalette
#include <settvuti.h>

#define cpInputScanKey "\x13\x13\x14\x15"

TInputScanKey::TInputScanKey( const TPoint &p ) :
    TView(TRect(p.x,p.y,p.x+20,p.y+1)),
    Scan(0),
    Ascii(0)
{
 options |= ofSelectable | ofFirstClick;
}

TInputScanKey::~TInputScanKey()
{
}

uint32 TInputScanKey::dataSize()
{
 return sizeof(unsigned short);
}

void TInputScanKey::draw()
{
 TDrawBuffer b;
 
 uchar color=(state & sfFocused) ? getColor(2) : getColor(1);
 
 char buf[256];
 sprintf(buf," Scan: %02X ASCII: %02X ",Scan,Ascii);
 b.moveStr(0,buf,color);

 writeLine( 0, 0, size.x, size.y, b );
}

void TInputScanKey::getData( void *rec )
{
 unsigned short *p=(unsigned short *)rec;
 *p=(Scan<<8) | Ascii;
}

TPalette& TInputScanKey::getPalette() const
{
 static TPalette palette( cpInputScanKey, sizeof( cpInputScanKey )-1 );
 return palette;
}


void  TInputScanKey::handleEvent( TEvent& event )
{
 TView::handleEvent(event);
 
 if (event.what==evKeyDown)
   {
    Scan=event.keyDown.raw_scanCode;
    Ascii=event.keyDown.charScan.charCode;
    drawView();
    clearEvent(event);
   }
}

void TInputScanKey::setData(void *rec)
{
 unsigned short *p=(unsigned short *)rec;
 Ascii=*p & 0xFF;
 Scan=*p>>8;
}

/*void TInputScanKey::setState( ushort aState, Boolean enable )
{
 TView::setState( aState, enable );
 if( aState == sfSelected ||
     ( aState == sfActive && (state & sfSelected) != 0 )
   )
     selectAll( enable );
}*/

#if !defined( NO_STREAM )
void TInputScanKey::write( opstream& os )
{
 TView::write(os);
 os << Scan << Ascii;
}

void *TInputScanKey::read( ipstream& is )
{
 TView::read(is);
 is >> Scan >> Ascii;
 return this;
}

TStreamable *TInputScanKey::build()
{
 return new TInputScanKey( streamableInit );
}

TInputScanKey::TInputScanKey( StreamableInit ) : TView( streamableInit )
{
 Scan=Ascii=0;
}
#endif // NO_STREAM

/*Boolean TInputScanKey::valid(ushort )
{
  return True;
}*/


