/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TCheckBoxesArray
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

const char *TCheckBoxesArray::button=" [ ] ";

void TCheckBoxesArray::draw()
{
    drawBox( button, 'X' );
}

Boolean TCheckBoxesArray::mark(int item)
{
    return Boolean( (value & (uint64(1)<<item)) != 0 );
}

void TCheckBoxesArray::press(int item)
{
 if (!pressCallBack || (pressCallBack && pressCallBack(item)))
    value=value^(uint64(1)<<item);
}

#define NO_STREAM
#if !defined( NO_STREAM )

TStreamable *TCheckBoxesArray::build()
{
    return new TCheckBoxesArray( streamableInit );
}

TCheckBoxesArray::TCheckBoxesArray( StreamableInit ) : TCluster( streamableInit )
{
}

#endif // NO_STREAM
