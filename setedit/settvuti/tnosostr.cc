/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TStreamableClass
#define Uses_TNoSortedStringCollection
#include <settvuti.h>

__link(RNoSortedStringCollection)

void TNoSortedStringCollection::writeItem( void *obj, opstream& os )
{
 os.writeString( (const char *)obj );
}

void *TNoSortedStringCollection::readItem( ipstream& is )
{
 return is.readString();
}

