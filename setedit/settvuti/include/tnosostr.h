/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TNoSortedStringCollection) && !defined(__TNoSortedStringCollection__)
#define __TNoSortedStringCollection__

class TNoSortedStringCollection : public TCollection
{
public:
  TNoSortedStringCollection(ccIndex aLimit, ccIndex aDelta) :
    TCollection(aLimit,aDelta) {};

 //**************** Stream stuff
 virtual const char *streamableName() const { return name; }
 virtual void writeItem( void *obj, opstream& os );
 virtual void *readItem( ipstream& is );

protected:
 TNoSortedStringCollection(StreamableInit) :
   TCollection(streamableInit) {};

public:
 static const char * const name;
 static TStreamable *build()
  { return new TNoSortedStringCollection(streamableInit);};
};

inline ipstream& operator >> ( ipstream& is, TNoSortedStringCollection& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TNoSortedStringCollection*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TNoSortedStringCollection& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TNoSortedStringCollection* cl )
    { return os << (TStreamable *)cl; }

#endif
