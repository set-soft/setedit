/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class TView;

class TDskWinMP3 : public TDskWin
{
public:

 TDskWinMP3(TView *w);
 ~TDskWinMP3();

 int  GoAction(ccIndex i);
 int  DeleteAction(ccIndex i, Boolean fromDiskToo=False);
 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktMP3) && p==(void *)view; };

 void write( opstream& os );
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:

 TDskWinMP3( StreamableInit ) { type=dktMP3; CanBeSaved=1; CanBeDeletedFromDisk=0; };

public:

 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskWinMP3& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWinMP3*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWinMP3& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWinMP3* cl )
    { return os << (TStreamable *)cl; }

