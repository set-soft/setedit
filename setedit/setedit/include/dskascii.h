/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */

class TDskWinASCII : public TDskWin
{
public:

 TDskWinASCII();
 ~TDskWinASCII();

 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktASCII) && p==(void *)view; };

 void write(opstream& os);
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:

 TDskWinASCII( StreamableInit ) { type=dktASCII; CanBeSaved=1; CanBeDeletedFromDisk=0; };

public:

 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskWinASCII& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWinASCII*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWinASCII& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWinASCII* cl )
    { return os << (TStreamable *)cl; }

