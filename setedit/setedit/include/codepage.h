/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class  TStringCollection;
extern void InitCodePages(void);
extern void DeInitCodePages(void);
extern unsigned short *GetCodePage128Translate(int id);
extern int GetCurrentOSCodePage(void);
extern TStringCollection *GetCodePagesList(void);
extern ccIndex CodePageIDToIndex(int id);
extern int     IndexToCodePageID(ccIndex index);
extern void RemapCharactersFor(int id);
uchar RemapCharCodePage(uchar c, unsigned short *map);
void  RemapStringCodePage(uchar *n, uchar *o, ushort *map);
void  RemapNStringCodePage(uchar *n, uchar *o, ushort *map, int len);
void  RemapBufferGeneric(int sourID, int destID, uchar *buffer, unsigned len, unsigned ops);
#ifdef Uses_fpstream
void SaveConvCPOptions(fpstream& s);
void LoadConvCPOptions(fpstream& s);
#endif

//const unsigned rbgDontRemapLow32=1, rbgOnlySelected=2;
