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
extern const char *TXHFormatFile;    // Configuration file
extern const char *TXHTempGenFile;   // Name of the temporal for nodes
extern const char *TXHOutBaseName;   // No extentions here!
extern const char *TXHFilesDir;      // Base for format files

typedef struct
{
 int PCREMaxMatchs;
 int *PCREMatchs;
 int PCREHits;
} PCREData;

void  PCREInitCompiler(PCREData &p);
void  PCREStopCompiler(PCREData &p);
pcre *PCRECompileRegEx(char *text, PCREData &p);
int   PCREDoSearch(char *search, int len, pcre *CompiledPCRE, PCREData &p);
#define PCREDataDestroy(p) PCREInitCompiler(p)
void  PCREGetMatch(int match, int &offset, int &len, PCREData &p);

