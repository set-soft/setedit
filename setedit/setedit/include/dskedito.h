/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class TDskWinEditor : public TDskWin
{
public:

 TDskWinEditor(TCEditWindow *EdW,int Num) : edw(EdW), number(Num)
        { type=dktEditor; CanBeSaved=1; CanBeDeletedFromDisk=1; view=EdW; };
 ~TDskWinEditor();

 int  DeleteAction(ccIndex i, Boolean fromDiskToo=False);
 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktEditor) && (p==edw); };
 int  GetNumber();
 void SetNumber(int number);

 TCEditWindow *edw;
 int number;

 void write( opstream& );
 void *read(ipstream& );

 const char *streamableName() const
     { return name; }

protected:

 TDskWinEditor( StreamableInit ) { type=dktEditor; CanBeSaved=1; CanBeDeletedFromDisk=1; };

public:

 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskWinEditor& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWinEditor*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWinEditor& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWinEditor* cl )
    { return os << (TStreamable *)cl; }

