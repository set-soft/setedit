/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class TDskWinMan : public TDskWin
{
public:

 TDskWinMan(const char *file, const char *sections, const char *extraOps);
 ~TDskWinMan();

 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktMan) && p==(void *)view; };

 void write(opstream& os);
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:

 TDskWinMan( StreamableInit ) { type=dktMan; CanBeSaved=1; CanBeDeletedFromDisk=0; };

public:

 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskWinMan& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWinMan*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWinMan& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWinMan* cl )
    { return os << (TStreamable *)cl; }

