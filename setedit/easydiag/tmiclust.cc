/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string

#define Uses_TKeys
#define Uses_TMiCluster
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TPoint
#define Uses_TSItem
#define Uses_TStringCollection
#define Uses_TGroup
#define Uses_opstream
#define Uses_ipstream
#define Uses_TPalette

#define Uses_TClusterArray

#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

#include <ctype.h>

#define cpCluster "\x10\x11\x12\x12"

TClusterArray::TClusterArray( const TRect& bounds, int number ) :
    TView(bounds),
    value( 0 ),
    sel( 0 )
{
    options |= ofSelectable | ofFirstClick | ofPreProcess | ofPostProcess;

    numItems=number;

    setCursor( 2, 0 );
    showCursor();
}

TClusterArray::~TClusterArray()
{
}

uint32  TClusterArray::dataSize()
{
    return sizeof(uint64);
}

void TClusterArray::drawBox( const char *icon, char marker)
{
    TDrawBuffer b;
    ushort color;

    ushort cNorm = getColor( 0x0301 );
    ushort cSel = getColor( 0x0402 );
    for( int i = 0; i <= size.y; i++ )
        {
        for( int j = 0; j <= (numItems-1)/size.y + 1; j++ )
            {
            int cur = j * size.y + i;
            if( cur < numItems )
                {
                int col = column( cur );
                if( (cur == sel) && (state & sfSelected) != 0 )
                    color = cSel;
                else
                    color = cNorm;
                b.moveChar( col, ' ', color, size.x - col );
                b.moveCStr( col, icon, color );
                if( mark(cur) )
                    b.putChar( col+2, marker );
                if( showMarkers && (state & sfSelected) != 0 && cur == sel )
                    {
                    b.putChar( col, specialChars[0] );
                    b.putChar( column(cur+size.y)-1, specialChars[1] );
                    }
                }
            }
        writeBuf( 0, i, size.x, 1, b );
        }
    setCursor( column(sel)+2, row(sel) );
}

void TClusterArray::getData(void * rec)
{
    memcpy(rec,&value,dataSize());
#if 0 // Why this
    drawView();
#endif
}

ushort TClusterArray::getHelpCtx()
{
    if( helpCtx == hcNoContext )
        return hcNoContext;
    else
        return helpCtx + sel;
}

TPalette& TClusterArray::getPalette() const
{
    static TPalette palette( cpCluster, sizeof( cpCluster )-1 );
    return palette;
}

void TClusterArray::handleEvent( TEvent& event )
{
    TView::handleEvent(event);
    if( event.what == evMouseDown )
        {
        TPoint mouse = makeLocal( event.mouse.where );
        int i = findSel(mouse);
        if( i != -1 )
            sel = i;
        drawView();
        do  {
            mouse = makeLocal( event.mouse.where );
            if( findSel(mouse) == sel )
                showCursor();
            else
                hideCursor();
            } while( mouseEvent(event,evMouseMove) );
        showCursor();
        mouse = makeLocal( event.mouse.where );
        if( findSel(mouse) == sel )
            {
            press(sel);
            drawView();
            }
        clearEvent(event);
        }
    else if( event.what == evKeyDown )
        switch (ctrlToArrow(event.keyDown.keyCode))
            {
            case kbUp:
                if( (state & sfFocused) != 0 )
                    {
                    if( --sel < 0 )
                        sel = numItems-1;
                    movedTo(sel);
                    drawView();
                    clearEvent(event);
                    }
                break;

            case kbDown:
                if( (state & sfFocused) != 0 )
                    {
                    if( ++sel >= numItems )
                        sel = 0;
                    movedTo(sel);
                    drawView();
                    clearEvent(event);
                    }
                break;
            case kbRight:
                if( (state & sfFocused) != 0 )
                    {
                    sel += size.y;
                    if( sel >= numItems )
                        {
                        sel = (sel +  1) % size.y;
                        if( sel >= numItems )
                            sel =  0;
                        }
                    movedTo(sel);
                    drawView();
                    clearEvent(event);
                    }
                break;
            case kbLeft:
                if( (state & sfFocused) != 0 )
                    {
                    if( sel > 0 )
                        {
                        sel -= size.y;
                        if( sel < 0 )
                            {
                            sel = ((numItems+size.y-1) /size.y)*size.y + sel - 1;
                            if( sel >= numItems )
                                sel = numItems-1;
                            }
                        }
                    else
                        sel = numItems-1;
                    movedTo(sel);
                    drawView();
                    clearEvent(event);
                    }
                break;
            default:
                if( event.keyDown.charScan.charCode == ' ' &&
                    (state & sfFocused) != 0
                  )
                    {
                    press(sel);
                    drawView();
                    clearEvent(event);
                    }
            }
}

void TClusterArray::setData(void * rec)
{
    memcpy(&value,rec,dataSize());
    drawView();
}

void TClusterArray::setState( ushort aState, Boolean enable )
{
    TView::setState( aState, enable );
    if( aState == sfSelected )
        drawView();
}

Boolean TClusterArray::mark( int )
{
    return False;
}

void TClusterArray::movedTo( int )
{
}

void TClusterArray::press( int )
{
}

int TClusterArray::column( int item )
{
    if( item < size.y )
        return 0;
    else
        {
        int width = 0;
        int col = -4;
        int l = 0;
        for( int i = 0; i <= item; i++ )
            {
            if( i % size.y == 0 )
                {
                col += width + 4;
                width = 0;
                }

            if( i < numItems )
                l = 0;
            if( l > width )
                width = l;
            }
        return col;
        }
}

int TClusterArray::findSel( TPoint p )
{
    TRect r = getExtent();
    if( !r.contains(p) )
        return -1;
    else
        {
        int i = 0;
        while( p.x >= column( i + size.y ) )
            i += size.y;
        int s = i + p.y;
        if( s >= numItems )
            return -1;
        else
            return s;
        }
}

int TClusterArray::row( int item )
{
    return item % size.y;
}

#define NO_STREAM
#if !defined( NO_STREAM )
void TClusterArray::write( opstream& os )
{
    TView::write( os );
    os << value << sel << numItems;
}

void *TClusterArray::read( ipstream& is )
{
    TView::read( is );
    is >> value >> sel >> numItems;
    setCursor( 2, 0 );
    showCursor();
    return this;
}

TStreamable *TClusterArray::build()
{
    return new TClusterArray( streamableInit );
}

TClusterArray::TClusterArray( StreamableInit ) : TView( streamableInit )
{
}
#endif // NO_STREAM

