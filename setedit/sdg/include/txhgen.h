/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
typedef char *(*TXHGetNextFileType)(int &l, int &MustBeDeleted, char *FileName);
extern TXHGetNextFileType TXHGetNextFile;
extern void (*TXHPrintMessage)(const char *s);

#define TXHGetErrorSection(a) TXHErrorSection[a]
#define TXHGetGenSect()       TXHSections[TXHGenSection]
#define TXHGetErrorMessage()  TXHErrors[TXHError]
extern int   TXHGenerateAll(void);
extern const char *TXHErrorSection[];
extern int   TXHGenSection;
extern const char *TXHSections[];
extern int   TXHError;
extern int   TXHLine;
extern const char *TXHErrors[];
extern int   TXHKeepTemporal;
extern char *TXHFormatFile;    // Configuration file
extern char *TXHTempGenFile;   // Name of the temporal for nodes
extern char *TXHOutBaseName;   // No extentions here!
extern char *TXHFilesDir;      // Base for format files

