/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */

class TDskWinCalendar : public TDskWin
{
public:

 TDskWinCalendar();
 ~TDskWinCalendar();

 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktCalendar) && p==(void *)view; };

 void write(opstream& os);
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:

 TDskWinCalendar( StreamableInit ) { type=dktCalendar; CanBeSaved=1; CanBeDeletedFromDisk=0; };

public:

 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskWinCalendar& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWinCalendar*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWinCalendar& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWinCalendar* cl )
    { return os << (TStreamable *)cl; }

