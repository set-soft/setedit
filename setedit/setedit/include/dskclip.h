/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class TDskWinClipboard : public TDskWin
{
public:

 TDskWinClipboard(TCEditWindow *edw);
 ~TDskWinClipboard();

 int  DeleteAction(ccIndex i, Boolean fromDiskToo=False);
 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktClipboard) && p==(void *)edw; };

 TCEditWindow *edw;

 void write( opstream& os );
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:

 TDskWinClipboard( StreamableInit ) { type=dktClipboard; CanBeSaved=1; CanBeDeletedFromDisk=0; };

public:

 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskWinClipboard& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWinClipboard*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWinClipboard& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWinClipboard* cl )
    { return os << (TStreamable *)cl; }

