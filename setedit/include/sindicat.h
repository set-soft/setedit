/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined( Uses_TSIndicator ) && !defined( __TSIndicator )
#define __TSIndicator

class TRect;
class TPoint;
class TCEditor;

class TSIndicator : public TIndicator
{
public:
 TSIndicator(const TRect &bounds) :
   TIndicator(bounds) {};

 virtual void draw();
 TCEditor *editor;

private:
 virtual const char *streamableName() const
     { return name; }

protected:
 TSIndicator( StreamableInit );

public:
 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TSIndicator& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TSIndicator*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TSIndicator& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TSIndicator* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TSIndicator


