/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class TDskWin : public TStreamable
{
public:
 TDskWin() {};
 virtual ~TDskWin() {};
 int type;
 int CanBeSaved;
 int CanBeDeletedFromDisk;
 int ZOrder;
 TView *view;
 virtual int  GoAction(ccIndex i);
 virtual int  DeleteAction(ccIndex i, Boolean fromDiskToo);
 virtual int  Compare(void *p, int t);
 virtual char *GetText(char *dest, short maxLen) = 0;
 virtual int  GetNumber();
 char *GetText(char *dest, char *s, short maxLen);

protected:
 virtual void write(opstream& os);
 virtual void *read(ipstream& is);

private:
 virtual const char *streamableName() const;
};

inline ipstream& operator >> ( ipstream& is, TDskWin& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskWin*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskWin& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskWin* cl )
    { return os << (TStreamable *)cl; }

//   Identifiers used by SET's editor for the different classes
// derived from TDskWin, in the type member.
//   This allows to identify an object derived from this class.
const int
      dktEditor    = 1,
      dktClosed    = 2,
      dktClipboard = 3,
      dktHelp      = 4,
      dktPrj       = 5,
      dktMessage   = 6,
      dktMP3       = 7,
      dktASCII     = 8,
      dktCalendar  = 9,
      dktMan       =10,
      dktDbgMsg    =11,
      dktDbgWt     =12,
      dktDbgIns    =13,
      dktDbgDataWin=14,
      dktDbgDisasm =15;

class TDeskTop;
void InsertInOrder(TDeskTop *dsk, TDskWin *win);
void AddAndInsertDskWin(TDskWin *win);


