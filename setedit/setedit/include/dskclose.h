/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class TDskWinClosed : public TDskWin
{
public:

 TDskWinClosed(TCEditWindow *edw);
 ~TDskWinClosed();

 int  GoAction(ccIndex i);
 int  DeleteAction(ccIndex i, Boolean fromDiskToo=False);
 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktClosed) && (strcmp((char *)p,Name)==0); };

 char *Name;
 EditorResume resume;

 void write( opstream& os );
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:

 TDskWinClosed( StreamableInit ) { type=dktClosed; CanBeSaved=1; CanBeDeletedFromDisk=1; };

public:

 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskWinClosed& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWinClosed*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWinClosed& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWinClosed* cl )
    { return os << (TStreamable *)cl; }


