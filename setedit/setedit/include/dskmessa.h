/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class TEdMsgDialog;

class TDskWinMessage : public TDskWin
{
public:

 TDskWinMessage(TEdMsgDialog *w);
 ~TDskWinMessage();

 int  DeleteAction(ccIndex i, Boolean fromDiskToo=False);
 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktMessage) && p==(void *)view; };

 void write( opstream& os );
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:

 TDskWinMessage( StreamableInit ) { type=dktMessage; CanBeSaved=1; CanBeDeletedFromDisk=0; };

public:

 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskWinMessage& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWinMessage*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWinMessage& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWinMessage* cl )
    { return os << (TStreamable *)cl; }

