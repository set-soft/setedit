/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define TCEDITOR_VERSION 0x000504UL
#define TCEDITOR_VERSION_STR "v0.5.4"
#define TCEDITOR_C_YEAR  "1996-2004"

#if defined( Uses_TCEditor_Internal ) && !defined( TCEditor_Internal_H )
#define TCEditor_Internal_H

// *************** Maximun line length supported
const int
  DeltaLineLen = 200,    // Increment when the actual limit is reached
  MaxLineLen   = 30000,  // Maximun maximorum
  MinLineLen   = 80;     // Minimun

const uint32
  sfSearchFailed = UINT_MAX;

const int
  splEndOfList  = -1000;

// *************** Syntax Highlight constants
#define ComInside      0x00001 // C++ comment in this line
#define InsideCom      0x00002 // All commented
#define Prepro         0x00004 // Starts with #
#define ExtPrepro      0x00008 // Prepro+ends with \ <= be careful
#define StartCom       0x00010 // /*
#define EndCom         0x00020 // */
#define ExtCom         0x00040 // more /* than */
#define StartInCom     0x00080 // The first char is commented
#define InsideCom2     0x00100 // All commented with { }
#define StartCom2      0x00200 // {
#define EndCom2        0x00400 // }
#define ExtCom2        0x00800 // more { than }
#define StartInCom2    0x01000 // The first char is commented
#define ExtString      0x02000 // a String ends with a \ and is continued on the next line
#define StartString    0x04000 // the line starts within a string (see ExtString)
#define ExtOneLineCom  0x08000
#define StartString2   0x10000
#define ExtString2     0x20000
#define StartString3   0x40000
#define ExtString3     0x80000
// Extra flags not useful for a line start
#define InString       0x00001
#define InString2      0x00002
#define InString3      0x00004
#define InComment      0x00008
#define InPrepro       0x00010

#define Not_ComInside      (~ComInside)
#define Not_InsideCom      (~InsideCom)
#define Not_Prepro         (~Prepro)
#define Not_ExtPrepro      (~ExtPrepro)
#define Not_StartCom       (~StartCom)
#define Not_EndCom         (~EndCom)
#define Not_ExtCom         (~ExtCom)
#define Not_StartInCom     (~StartInCom)
#define Not_InsideCom2     (~InsideCom2)
#define Not_StartCom2      (~StartCom2)
#define Not_EndCom2        (~EndCom2)
#define Not_ExtCom2        (~ExtCom2)
#define Not_StartInCom2    (~StartInCom2)
#define Not_ExtString      (~ExtString)
#define Not_StartString    (~StartString)
#define Not_ExtOneLineCom  (~ExtOneLineCom)
#define Not_ExtString2     (~ExtString2)
#define Not_StartString2   (~StartString2)
#define Not_ExtString3     (~ExtString3)
#define Not_StartString3   (~StartString3)
#define Not_InString       (~InString)
#define Not_InString2      (~InString2)
#define Not_InString3      (~InString3)
#define FilterHere         (Not_ComInside & Not_StartCom & Not_EndCom & \
                            Not_ExtPrepro & Not_StartCom2 & Not_EndCom2 & \
                            Not_ExtString & Not_StartString & Not_ExtString2 & \
                            Not_StartString2 & Not_ExtString3 & Not_StartString3)
#define FilterProp         (ExtCom | ExtPrepro | ExtCom2 | ExtString | ExtString2 \
                            | ExtString3 | ExtOneLineCom)
#define PartialCom         (StartCom | ComInside | EndCom | StartCom2 | EndCom2)
#define InsidePascalCom    (InsideCom | InsideCom2)
#define InsideGenericCom   (InsideCom | InsideCom2)
#define StartInPascalCom   (StartInCom | StartInCom2)
#define StartInGenericCom  (StartInCom | StartInCom2)
#define IsInsideCom        (ComInside | InsideCom | ExtCom | InsideCom2 | ExtCom2)
#define IsInsideStr        (InString | InString2 | InString3)

// *************** Keyboard binding tables declared in kbfun.h included by
//                 kbfun.cc and used in the editor class.
extern unsigned short kbFunNormal[128];
extern unsigned short kbFunShift[128];
extern unsigned short kbFunCtrl[128];
extern unsigned short kbFunShiftCtrl[128];
extern unsigned short kbFunCtrlQNorm[128];
extern unsigned short kbFunCtrlKNorm[128];
extern unsigned short kbFunCtrlKShift[128];

// **************** Color codes originally defined by Robert
#define cNormal   1
#define cMarked   2
#define cComment  3
#define cReserved 4
#define cIdent    5
#define cSymbol   6
#define cString   7
#define cInteger  8
#define cFloat    9
#define cOctal    10
#define cHex      11
#define cChar     12
#define cPre      13
#define cIllegal  14
#define cUser     15
#define cCPU      16
#define cBreak    17
#define cSymbol2  18
#define cCrossCur 19
#define cStatusLi 20
#define cMPHighL  21
#define cRectSel  22
#define cOddTab   23
#define cEvenTab  24
#define cColMark  25

#define cNumColors 26


/************************* Rectangular Selection defs *****************/
// Notes:
// The Xr1 col is included, but not the Xr2, so the width is: Xr2-Xr1
// The Yr1 row is included AND the Yr2 row too, so the heigth is: Yr2-Yr1+1

struct selRecSt {
 int Xr1,Yr1;     // Top left corner of the original sel.
 int Xr2,Yr2;     // Botton right of the original sel.
 Boolean selHide; // Only used in the undo blocks
 int Ycur;        // Where the cursor is
 char s[0];       // Here is where the data start
};
/*********************** End of Rectangular Selection defs *****************/

// ******************* String Collections for reserved words
// The editor can use a non-sorted string collection
#ifdef USE_TSTRCOL
class TStrCol : public TStringCollection
{
 public:
 TStrCol(short aLimit, short aDelta) :  TStringCollection(aLimit,aDelta) {};
 int getCount(void) { return count; };
};

extern TStrCol *ReservedWords;
extern TStrCol *UserWords;
extern TStrCol *PascalRWords;
extern TStrCol *ClipperRWords;
#else
class  TStringCollection;
extern TStringCollection *ReservedWords;
extern TStringCollection *UserWords;
extern TStringCollection *PascalRWords;
extern TStringCollection *ClipperRWords;
#define TStrCol TStringCollection
#endif

// ****************** Functions
class TPMCollection;
extern Boolean LoadPseudoMacroFile(char *name, TPMCollection &coll);
extern Boolean CreateSHShortCutTables(void);
extern void DestroySHShortCutTables(void);
extern Boolean LoadFileUnderCursor(char *lineStart,char *cursor,unsigned l,
                                   void (*Message)(const char *msg, void *data),
                                   void *data, const char *reference);
extern char *ExpandFileNameToThePointWhereTheProgramWasLoaded(const char *s);
extern int CheckForSequence(char *s, int len, int available, char *d);
extern int CheckForSequenceNotFirst(char *s, int len, int available, char *d);
extern ccIndex SHLConvValToPos(int a);
extern int SHLConvPosToVal(ccIndex a);
class TSOSStringCollection;
struct strSETSE;
extern Boolean SETSECreateTables(strSETSE &st, int CaseSens, TStringCollection *TSC);
extern void SETSEDeleteTables(strSETSE &st);

// isalnum() + _ + all the accents
const unsigned char ttedIsWordChar=1;
// () [] {} ,;.
const unsigned char ttedIsCsymb1  =2;
// !%&*+-/:<=>?^|~
const unsigned char ttedIsCsymb2  =4;
const unsigned char ttedIsCsymb   =6;
// () [] {} ,;.
const unsigned char ttedIsPasSymb1=8;
// !#%&*+-/:<=>?@^|
const unsigned char ttedIsPasSymb2=16;
const unsigned char ttedIsPasSymb =24;
// !#%&$*+-/:<=>?@^|() [] {} ,;.
const unsigned char ttedIsClipSymb=32;

extern unsigned char TableTypesEditor[256];

inline int isWordChar(unsigned char ch)
{
 return TableTypesEditor[ch] & ttedIsWordChar ? 1 : 0;
}

inline int isWordCharColon(unsigned char ch)
{
 return (TableTypesEditor[ch] & ttedIsWordChar) || ch==':';
}

#ifdef DEBUG
# define MyAssert(p) ((p) ? (void)0 : (void) printf( \
                    "Assertion failed: %s, file %s, line %d\n", \
                    #p, __FILE__, __LINE__ ),getch(),abort() )
#else
# define MyAssert(p) ((void)0)
#endif


#define CutIfNotPersistent()    if (!PersistentBlocks && hasSelection()) \
                                   clipCut()
#define ClearSelIfNonPers()     { if (!PersistentBlocks && hasSelection()) \
                                    { \
                                     selEnd=selStart=0; \
                                     update(ufView); \
                                    } \
                                }

/*#define CheckForShiftSelection() { \
		    if (selectMode==smStartSel && NotExpandingMacro &&           \
			event.message.command!=cmcExpandCode)                    \
		      {                                                          \
		       if (IslineInEdition)                                      \
			  MakeEfectiveLineInEdition();                           \
		       SetStartOfSelecting((uint32)(ColToPointerPost()-buffer)); \
		       selectMode = smExtend;                                    \
		      }    }
*/
#define CheckForShiftSelection()

// New Palette based on Robert's palette
#define cpEditor "\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10"\
                 "\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E"
/* colors for the editor
  1 : normal text
  2 : marked text
  3 : comment
  4 : reserved word
  5 : identifier
  6 : symbol
  7 : string
  8 : integer
  9 : float
 10 : octal
 11 : hex
 12 : character
 13 : preprocessor
 14 : illegal char
 15 : user defined word
 16 : CPU line ( in RHIDE )
 17 : Breakpoint ( in RHIDE )
 18 : binary operators
 19 : cross cursor
 20 : status line
 21 : match-pair highlight
 22 : rectangular selection
 23 : odd tabs
 24 : even tabs
 25 : column markers
*/

// ************ Generic Syntax Highlight flags
#define shl_SYM1       1
#define shl_SYM2       2
#define shl_STR1       4
#define shl_CHAR       8
#define shl_EOL       16
#define shl_OPC1      32
#define shl_OPC2      64
#define shl_CLOSE1   128
#define shl_CLOSE2   256
#define shl_INSNAME  512
#define shl_BEGNAME 1024
#define shl_STR2    2048
#define shl_STR3    4096
#define shl_SP      8192
#define shl_SPC    16384

#define SHLFile "syntaxhl.shl"

#define isSymbol1(a)     (TCEditor::strC.SymbolT[(uchar)a] & shl_SYM1)
#define isSymbol2(a)     (TCEditor::strC.SymbolT[(uchar)a] & shl_SYM2)
#define isSymbol(a)      (TCEditor::strC.SymbolT[(uchar)a] & (shl_SYM1 | shl_SYM2))
#define isString(a)      (TCEditor::strC.SymbolT[(uchar)a] & shl_STR1)
#define isString2(a)     (TCEditor::strC.SymbolT[(uchar)a] & shl_STR2)
#define isString3(a)     (TCEditor::strC.SymbolT[(uchar)a] & shl_STR3)
#define isCharacter(a)   (TCEditor::strC.SymbolT[(uchar)a] & shl_CHAR)
#define isEOLComment(a)  (TCEditor::strC.SymbolT[(uchar)a] & shl_EOL)
#define isComment1(a)    (TCEditor::strC.SymbolT[(uchar)a] & shl_OPC1)
#define isComment2(a)    (TCEditor::strC.SymbolT[(uchar)a] & shl_OPC2)
#define isCloseComm1(a)  (TCEditor::strC.SymbolT[(uchar)a] & shl_CLOSE1)
#define isCloseComm2(a)  (TCEditor::strC.SymbolT[(uchar)a] & shl_CLOSE2)
#define isWordCharIn(a)  (TCEditor::strC.SymbolT[(uchar)a] & shl_INSNAME)
#define isWordCharBe(a)  (TCEditor::strC.SymbolT[(uchar)a] & shl_BEGNAME)
#define isSpecialSymb(a) (TCEditor::strC.SymbolT[(uchar)a] & shl_SP)
#define isSpecialSCon(a) (TCEditor::strC.SymbolT[(uchar)a] & shl_SPC)

extern char CheckSeqCase;

unsigned TCEditor_scan(const void *block, unsigned size, const char *str);
unsigned TCEditor_iScan(const void *block, unsigned size, const char *str);
#endif
