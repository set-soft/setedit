/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class  TStringCollection;
extern void RemapCharactersFor(int id);
void  RemapStringCodePage(uchar *n, uchar *o, ushort *map);
void  RemapNStringCodePage(uchar *n, uchar *o, ushort *map, int len);
#ifdef Uses_fpstream
void SaveConvCPOptions(fpstream& s);
void LoadConvCPOptions(fpstream& s);
#endif

//const unsigned rbgDontRemapLow32=1, rbgOnlySelected=2;
