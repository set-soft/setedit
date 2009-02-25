/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <inf.h>

class TInfFile;
class TInfWindow;

class TDskWinHelp : public TDskWin
{
public:

 TDskWinHelp(const char *File, const char *Node, Boolean TheOne=False);
 ~TDskWinHelp();

 int  DeleteAction(ccIndex i, Boolean fromDiskToo=False);
 char *GetText(char *dest, short maxLen);
 int  Compare(void *,int t) { return (t==dktHelp); };

 TInfFile *file;
 TInfWindow *window;

 void Goto(const char *File, const char *Node, const char *word=0);
 void Create(const char *File, const char *Node, Boolean TheOne=False);
 void CreateModal(const char *File, const char *Node);
 void MakeVisible(void);

 void write( opstream& os );
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:

 TDskWinHelp( StreamableInit ) { type=dktHelp; CanBeSaved=1; CanBeDeletedFromDisk=0; };

public:

 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskWinHelp& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWinHelp*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWinHelp& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWinHelp* cl )
    { return os << (TStreamable *)cl; }
