/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined( Uses_TCEditor_Class )

#if !defined ( __TCEditor_Class_Constants__ )
#define __TCEditor_Class_Constants__
const int maxFindStrLenEd=80,
          maxReplaceStrLenEd=maxFindStrLenEd;
#endif

#if defined( Uses_TFindCDialogRec ) && !defined( __TFindCDialogRec__ )
#define __TFindCDialogRec__

struct TFindCDialogRec
{
 TFindCDialogRec( const char *str, unsigned flgs, unsigned insel,
                  unsigned from_where )
   {
    strcpy( find, str );
    options = flgs;
    in_sel = insel;
    from = from_where;
   }
 char find[maxFindStrLenEd];
 uint32 options;
 uint32 in_sel;
 uint32 from;
};

#endif  // Uses_TFindCDialogRec


#if defined( Uses_TReplaceCDialogRec ) && !defined( __TReplaceCDialogRec__ )
#define __TReplaceCDialogRec__

struct TReplaceCDialogRec
{
 TReplaceCDialogRec( const char *str, const char *rep, unsigned flgs,
                     unsigned insel, unsigned from_where )
    {
     strcpy( find, str );
     strcpy( replace, rep );
     options = flgs;
     in_sel = insel;
     from = from_where;
    }
 char find[maxFindStrLenEd];
 char replace[maxReplaceStrLenEd];
 uint32 options;
 uint32 in_sel;
 uint32 from;
};

#endif  // Uses_TReplaceCDialogRec



#if defined( Uses_LineLengthArray ) && !defined( __LineLengthArray__ )
#define __LineLengthArray__

class LineLengthArray
{
public:
 LineLengthArray();
 ~LineLengthArray();
 uint16 operator [](unsigned pos) { return elArray[pos]; };
 uint16 safeLen(unsigned pos) { return pos>=Length ? 0 : elArray[pos]; };
 void set(unsigned pos, uint16 val);
 void insert(unsigned pos, uint16 val);
 void del(unsigned pos);
 void deleteRange(unsigned from,unsigned to);
 uint32 getAttr(unsigned line) { return elArrayAttr[line]; };
 void setAttr(unsigned line, uint32 val);
 void setAll(unsigned line, uint16 length, uint32 attr);
 unsigned Length;
 uint16 *elArray;
 uint32 *elArrayAttr;

protected:
 void Resize(unsigned size);
 unsigned MaxPos;
};

#endif // Uses_LineLengthArray


#if defined( Uses_TCEditor ) && !defined( __TCEditor__ )
#define __TCEditor__

class TRect;
class TScrollBar;
class TSIndicator;
class TIndicator;
class TEvent;
class TSubMenu;
class TSpCollection;
struct stEditorId;

const int MaxRecMacroLen=250;
const int MaxXYRingStack=12;

class XYRingStack
{
public:
 XYRingStack();
 void push(uint32 x, uint32 y);
 int  pop(uint32 &x, uint32 &y);

protected:
 uint32 Xarray[MaxXYRingStack];
 uint32 Yarray[MaxXYRingStack];
 int pushPos,basePos;
};

const unsigned lastColMarker=(unsigned)-1;

class TCEditor : public TViewPlus
{
public:

    TCEditor( const TRect&, TScrollBar *, TScrollBar *, TSIndicator *, const char *,
              Boolean openRO=False );

    virtual ~TCEditor();
    virtual void shutDown();
    char bufChar( unsigned Pos )    { return buffer[Pos]; };
    unsigned bufPtr( unsigned Pos ) { return Pos; };
    virtual void changeBounds( const TRect& );
    virtual void convertEvent( TEvent& );
    Boolean cursorVisible();
    void deleteSelect();
    virtual void doneBuffer();
    virtual void draw();
    virtual TPalette& getPalette() const;

    // Handle event is divided in sub handle
    virtual void handleEvent( TEvent& );
    void handleMouse(TEvent &event);
    void handleKey(TEvent &event);
    int  handleCommand(ushort command);

    virtual void initBuffer();
    Boolean insertBuffer( char *, unsigned, unsigned, Boolean, Boolean, Boolean moveToEnd=True );
    virtual Boolean insertFrom( TCEditor * );
    Boolean insertText( const void *, unsigned, Boolean );
    unsigned CopySelToBuffer(char *b, unsigned l);
    void scrollTo( int, int );
    Boolean search( const char *, unsigned );
    virtual Boolean setBufSize( unsigned );
    void setCmdState( uint16 command, Boolean enable );
    void setSelect( unsigned, unsigned, Boolean);
    virtual void setState( uint16 aState, Boolean enable );
    void trackCursor( Boolean );
    void undo();
    void BlockUndoInfoStartFill(UndoCell &un, UndoState type, char *start, char *end);
    void BlockUndoInfoEndFill(UndoCell &un);
    virtual void updateCommands(int full=0);
    virtual void updateRectCommands();

    Boolean clipCopy();
    Boolean clipWinCopy(int id);
    Boolean clipFileCopy();
    void clipCut();
    void clipPaste();
    void clipWinPaste(int id);
    void clipFilePaste();
    void deleteRange( unsigned , unsigned, Boolean );
    void doUpdate();
    Boolean doSearchReplace();
    void drawLines( int, int, unsigned );
    void find();
    Boolean hasSelection() { return Boolean(selStart<selEnd); };
    Boolean hasVisibleSelection() { return Boolean(!selHided && selStart<selEnd); };
//{ return Boolean(IslineInEdition ? selLineStart<selLineEnd : selStart<selEnd); };
    void hideSelect();
    Boolean isClipboard() { return Boolean(clipboard == this); };
    void lock() { lockCount++; };
    void newLine();
    unsigned nextWordC( unsigned );
    int goEndWord();
    char *WordUnderCursor(uint32 maxLength=256, unsigned options=0);
    char *sLispUnderCursor(uint32 maxLength);
    void RunSLispAsk();
    Boolean StringUnderCursor(uint32 &word_start, uint32 &word_end);
    void replace();
    void setBufLen( unsigned );
    void startSelect();
    void toggleInsMode( Boolean allowUndo );
    void unlock();
    void update( uchar );
    void checkScrollBar( const TEvent&, TScrollBar *, int& );
    unsigned LenWithoutCRLF(unsigned yInFile,char *lineStart);
    void ExpandAllTabs(Boolean interactive);
    void CompactBuffer(Boolean interactive);
    void SourceToHTML(FILE *f, unsigned *pal, unsigned flags);
    void SourceToHTML_Old(FILE *f, unsigned *pal, unsigned flags);
    void SourceToHTML_CSS(FILE *f, unsigned *pal, unsigned flags);
    uint32 CompactFlags(void);
    void ExpandFlags(uint32 t,Boolean allowUndo=True);
    Boolean selRectCopy(Boolean allowUndo=True);
    Boolean selRectPaste(struct selRecSt *st, int X, int Y, Boolean allowUndo=True);
    Boolean selRectDelete(int X1, int Y1, int X2, int Y2, Boolean allowUndo=True);
    void selRectToUpper();
    void selRectToLower();
    void UndoRectangularPasteClear(UndoCell &un);
    Boolean FillUndoForRectangularPasteClear(int Height,struct UndoCell &un,UndoState st);
    Boolean FillUndoForRectangularStartEnd(UndoState st);
    void undoOneAction();
    void RevertModifFlagInUndo();
    void RemapCodePageBuffer(int sourID, int destID, unsigned ops, Boolean allowUndo=True);
    //   Global options managment, global to allow setting while there isn't any editor
    // opened.
    static void SetGlobalOptions(void);
    static void ExpandGlobalOptions(GlobalOptionsRect *temp);
    static void CompactGlobalOptions(GlobalOptionsRect *temp);
    void ExpandGlobalOptionsLocally(GlobalOptionsRect *temp);

    TScrollBar  *hScrollBar;
    TScrollBar  *vScrollBar;
    TSIndicator *indicator;
    TIndicator  *OldIndicator;
    char *buffer;
    unsigned bufSize;
    unsigned bufLen;
    unsigned gapLen;
    unsigned selStart;
    unsigned selEnd;
    unsigned curPtr;
    TPoint delta;
    TPoint limit;
    unsigned drawPtr;
    unsigned delCount;
    unsigned insCount;
    Boolean isValid;
    Boolean canUndo;
    Boolean modified;
    unsigned modifiedCounter; // Changes each time the file is modified
    void MarkAsModified() { modified=True; modifiedCounter++; };
    Boolean selecting;
    Boolean overwrite;
    Boolean isReadOnly;

    Boolean autoIndent;               // Indent when ENTER is pressed
    static Boolean staticAutoIndent;  // the static version
    Boolean intelIndent;              // try to be smart in the autoindent
    static Boolean staticIntelIndent; // the static version
    unsigned tabSize;                 // The size of the tabulator
    static unsigned staticTabSize;    // the static version
    Boolean UseTabs;                  // True if when we press Tab an ascii 9 is inserted
    static Boolean staticUseTabs;     // the static version
    Boolean PersistentBlocks;         // True if the blocks isn't deleted after a move
    static Boolean staticPersistentBlocks;  // the static version
    Boolean ShowMatchPair;            // True if the editor highlights the match pair
    static Boolean staticShowMatchPair;     // the static version
    Boolean ShowMatchPairFly;
    static Boolean staticShowMatchPairFly;
    static TCEditor *showMatchPairFlyCache;
    Boolean ShowMatchPairNow;
    static Boolean staticShowMatchPairNow;
    static Boolean staticNoMoveToEndPaste;  //  True if the cursor is moved to the end of
                                            // the pasted area.
    Boolean TransparentSel;           // True if you can see through the slelection
    static Boolean staticTransparentSel; // the static version
    Boolean OptimalFill;             // When True the editor tries to fill all the gaps
                                     // with tabs instead of spaces
    static Boolean staticOptimalFill;// the static version
    Boolean WrapLine;                // If True wraps the line after WrapCol columns
    static Boolean staticWrapLine;   // static version
    int WrapCol;                     // Column to wrap
    static int staticWrapCol;        // Default column
    Boolean DontPurgeSpaces;              // Don't purge spaces at the end of lines
    static Boolean staticDontPurgeSpaces; // the static version
    Boolean SeeTabs;                 // If true the tabs are painted to be visible
    static  Boolean staticSeeTabs;   // the static version
    static  char    oTabChar;        // When selected the tabs are drawed as a character
                                     // that's the PC437 code for it.
    static  char    TabChar;         // That's the value for the current codepage
    Boolean NoInsideTabs;               // When True the cursor can't be inside a tab
    static  Boolean staticNoInsideTabs; // the static version
    Boolean TabIndents;              // Tab indents and doesn't move to the next tabulator
    static  Boolean staticTabIndents;// the static version
    Boolean BackSpUnindents;              // Backspace unindents looking for a hole in prev. line
    static  Boolean staticBackSpUnindents;// the static version
    Boolean UseIndentSize;               // Use the indent size instead of the tab size
    static  Boolean staticUseIndentSize; // the static version
    unsigned indentSize;                 // The ammount to indent when the above is true
    static unsigned staticIndentSize;    // the static version
    Boolean ColumnMarkers;               // Are column markers enabled?
    static Boolean staticColumnMarkers;  // the static version

    // Column markers
    uint32 *colMarkers;              // Columns to be highlighted, an array terminated by (unsigned)-1
    static uint32 *staticColMarkers; // the static version
    // Helper functions
    static int     LenColMarkers(uint32 *markers);
    static uint32 *CopyColMarkers(uint32 *markers);
    static uint32 *LoadColMarkers(ipstream& is);
    static void    SaveColMarkers(opstream& os, uint32 *markers);
    static void    ColMarkers2Str(uint32 *markers, char *str, unsigned maxLen);
    static uint32 *Str2ColMarkers(char *str);

    Boolean NoNativeEOL;             // True if we are in DOS an the file was UNIX
    int IsaCompressedFile;           // !=0 if we loaded a .gz or .bz2 file and hence we must
                                     // save a compressed file.
    Boolean FailedToLoad;            // Used to indicate when we couldn't load the file and
                                     // instead created an empty editor.

    static TEditorDialog editorDialog;
    static unsigned editorFlags;
    static unsigned fromFlags;
    static unsigned whereFlags;
    static char findStr[maxFindStrLenEd];
    static char replaceStr[maxReplaceStrLenEd];
    static TCEditor *clipboard;
    static TCommandSet cmdsAux;     // This is used to quickly enable/disable all the
                                    // editor commands when it get/releases the focus
    uchar lockCount;
    uchar updateFlags;
    uchar isDisassemblerEditor;
    int keyState;
    unsigned WantedCol;
    void formatLine(void *, unsigned, int, unsigned short, unsigned, uint32,unsigned,uint32 *);
    void formatLineHighLight(void *, unsigned, int, unsigned short, unsigned, uint32,unsigned,uint32 *);
    void formatLineHighLightPascal(void *,unsigned,int,unsigned short,unsigned,uint32,unsigned,uint32 *);
    void formatLineHighLightClipper(void *,unsigned,int,unsigned short,unsigned,uint32,unsigned,uint32 *);
    void formatLineHighLightGeneric( void *,unsigned,int,unsigned short,unsigned,uint32,unsigned,uint32 *);
    unsigned lineMove( unsigned p, int count );
    void TurnOffHighLight(void) {  formatLinePtr=&TCEditor::formatLine; SyntaxHL=shlNoSyntax; SHLValueSelected=-1; };
    void TurnOnCHighLight(void);
    void TurnOnPascalHighLight(void);
    void TurnOnClipperHighLight(void);
    void TurnOnGenericHighLight(void);
    void SetHighlightTo(shlState sHL, int subSHL=0);
    void CacheSyntaxHLData(int id);
    uint32 SyntaxHighlightForOffset(unsigned offset);
    uint32 SyntaxHighlightExtraFor(char *lineStart, char *posTarget, int line);
    void *SearchPMTrigger(char *trg);
    void ChoosePMacroFromList(void);
    void RecalculateLineAttributes(void);
    void ProfileEditor(void);
    void CacheColors(void);
    void ColorsCacheToIDs(void);

    void InsertCharInLine(char cVal, Boolean allowUndo=True);
    void MakeEfectiveLineInEdition();
    char *ColToPointer();
    char *ColToPointer(int &Dif);
    char *ColToPointerPost();
    int  PosLeftChar();
    int  FixPosCharLeft();
    int  nextWord();
    int  prevWord(Boolean moveCursor=True);
    void MoveCursor(char *ori,char *dest);
    void EditLine();
    int  ComputeXLineInEdition(Boolean alsoRestChars=False);
    void deleteRange(char *from,char *to,Boolean allowUndo=True);
    void deleteRangeLineInEdition(char *from,char *to,int x);
    int  LineWidth();
    int  LineWidth(char *s, char *d);
    void insertSpaces( unsigned length, int X, Boolean canUseTabs=True );
    void MoveLinesUp(int i);
    void MoveLinesDown(int i);
    void ScrollLinesUp(int lines);
    void ScrollLinesDown(int lines);
    void SetStartOfSelecting(unsigned startOffSet);
    void UpdateSelecting(void);
    void MoveToMouse( TPoint m, uchar selMode );
    void MoveCursorTo(unsigned x, unsigned y, Boolean undo=False);
    void GoAndSelectLine(int line, Boolean selectLine=True);
    int  IsFirstCharInLine(void);
    void GotoOffSet(unsigned o);
    void JumpEndOfText();
    void ResetCursorPosition(void);
    void IndentBlock(char *Fill, Boolean allowUndo=True);
    void UnIndentBlock(unsigned Amount, Boolean allowUndo=True);
    void ArbitraryIndent();
    void CommentIndent();
    void CommentUnIndent();
    void BackSpace(Boolean allowUndo=True);
    void ExpandMacro(void);
    void ExpandPMacro(void *pm, char *s=0);
    void MacroGenerateCode(void);
    int TestPropagation(uint32 OldAttr,uint32 NewAttr,char *proxLine, uint32 proxLineNum);
    int SearchCloseSymbol(char open, char close);
    int SearchCloseSymbolXY(char open, char close, int &X, int &Y, char *pos=0);
    int SearchOpenSymbol(char open, char close);
    int SearchOpenSymbolXY(char open, char close, int &X, int &Y, char *pos=0);
    Boolean SearchMatchOnTheFly();
    Boolean clipReplace(void);
    int GoFirstCharInLine(void);
    void SelWordUnderCursor(void);
    int nextCWord();
    void addToUndo(UndoState st, void *p=NULL);
    void freeUndoCell(int Index);
    void flushUndoInfo();
    void redo(void);
    void UndoSaveFinalState(UndoCell &un);
    void UndoSaveStartState(UndoCell &un);
    void BlockToUpper(Boolean allowUndo=True);
    void BlockToLower(Boolean allowUndo=True);
    void BlockInvertCase(Boolean allowUndo=True);
    void BlockAltCase(Boolean allowUndo=True);
    void BlockCaseChange(UndoState action, Boolean allowUndo=True);
    void SetCharCase(int option);
    int  AdjustBufEditFor(int lar);
    void Beep(void);
    void AdjustLineSel(uint32 pos,int dif,Boolean IncludeStart=False,Boolean toLeft=True);
    void AdjustDrawPtr(void);
    void updateCrossCur(void);
    void setStatusLine(char *s);
    void RecalculateXofLineInEdit(void);
    Boolean hasRectSel(void) { return Boolean(!selRectHided && Xr1<Xr2 && Yr1<=Yr2); };
    unsigned GetOffSetOffLine(int y);
    unsigned GetOffsetGeneric(int y, int yRef, unsigned lOff);
    int FindLineForOffSet(unsigned offset, unsigned &rest);
    int EnsureXDontTab(char *s,int x,int w,char **stop);
    void lockUndo(void) { undoLockCount++; };
    void unlockUndo(void);
    void SetSpecialLines(TSpCollection *aSpCol);
    void PasteEmacsMode();
    void QuotedPrintDecode();
    void InsertKeyName();
    void ShowWhichFunInStatus();
    unsigned JumpLineStartSelection();
    void UpdateSyntaxHLBlock(unsigned firstLine, char *firstTouchedP, unsigned lastLine);
    // File handle members
    Boolean loadFile(Boolean setSHL=False);
    Boolean reLoadFile();
    Boolean save();
    Boolean saveAs(Boolean ConvertEOL=False, Boolean AvoidAutoConvert=False);
    Boolean saveAsConvertEOL() { return saveAs(True); };
    Boolean saveAsNoConvertEOL() { return saveAs(False,True); };
    Boolean saveSameTime();
    Boolean saveFile(Boolean ConvertEOL=False, Boolean AvoidAutoConvert=False,
                     Boolean noChangeTime=False);
    char   *saveToTemp();
    Boolean checkDiskCopyChanged(Boolean force=False);
    virtual Boolean valid( ushort );
    char fileName[PATH_MAX];

    LineLengthArray lenLines; // pseudo-Array with all the line-lengths
    uint32 totalLines;        // Total number of lines
    uint32 drawLine;          // first displayed line
    uint32 lineInEdition;     // Line number in edition process
    char  *curLinePtr;        // Pointer to the start of the line where the
                              // cursor is.
    char  *bufEdit;           // Buffer to edit a line
    uint16 bufEditLen;        // Allocated memory for bufEdit
    char  *inEditPtr;         // Pointer to the cursor inside the line edition
                              // buffer
    Boolean IslineInEdition;  // There is a line under edition?
    static int DontPurge;     // Special flag to avoid the extra spaces purge feature
                              // that's an experimental patch (ugly).
    static int DontLoadFile;  // If !=0 the read member doesn't really load the text
                              // in memory, but reads all the rest of stuff.
    TPoint curPos;            // Position of cursor inside the file
    TPoint lastCurPos;        // Last position before a move, used for ^Q-P
    int    restCharsInLine;   // Number of chars between inEditPtr and the end
                              // of the buffer.
    unsigned selLineStart;    // selStart in a line
    unsigned selLineEnd;      // selEnd in a line
    unsigned selNewStart;     // The value of selXXX when the line in edition
    unsigned selNewEnd;       // will be put in the buffer
    Boolean  selHided;        // True if the selection is invisible
    uint32 selStartOffSet;    // The start of a selecting operation
    void MarkLine(Boolean moveCursor); // Select current line and move cursor to the beggining
    void SetMarker(unsigned marker);
    unsigned Markers[10];     // 0=does't exist != It's a marker
    int MarkersInLine[10];    // -1 Isn't in this line other offset in the line

    XYRingStack CurPosStack; // Cursor position stack, works as a ring
    void PushCursorPosition();
    int  PopCursorPosition();

    uint32 attrInEdit;       // Attributes for the line in edition
    Boolean NotExpandingMacro; // False if we are expanding a pseudo macro to
                               // avoid an alteration of the selected area
    //uchar     ForceSelection;  // Force to set this selecting mode instead
                               // of use the shifts in handleEvent
    //uchar     ForcingSelection;  // Enables the other

    UndoCell UndoArray[MAX_UNDO];    // Array with the undo info
    UndoState UndoSt;                // Actual State
    int UndoBase,UndoActual,UndoTop; // Index in the array

    // a pointer to the function to format the line to be displayed.
    void (TCEditor::*formatLinePtr)(void *, unsigned, int, unsigned short, unsigned, uint32, unsigned, uint32 *);
    // a pointer to the function to format calculate the s. hl. attributes
    unsigned (*LineMeassure)(char *, char *, uint32 &, uint32 *extra);
    shlState SyntaxHL;        // Says the type of syntax highlight used
    int      GenericSHL;      // What sub-type if the type is generic
    int SHLValueSelected;     // That's the position in the array of the detected SHL
    static unsigned char SHLTableUse[4];  // A conversion table to know what generic shl uses an internal SHL
    static strSHL  *SHLArray;   // Array of configurable SHLs
    static strSHL   strC;       // Cached Syntax HL values
    static int      strCid;     // What element is cached
    static shlState strCtype;   // What type is cached (all or just user words)
    static void InvalidateSHLCache() { strCid=-2; };
    static int      SHLCant;    // Number of SHLs defined
    static const char *shlNames[]; // Compact names for the colors
    static TStringCollection *SHLGenList; // Names of the SHLs tp choose one
    static dflOptions dflOps;   // Default options settings for SHL=none

    static  Boolean Recording;
    static  TSArray<unsigned int> MacroArray;
    static  int     MacroCount;

    static unsigned LoadingVersion;   // That's the version of the desktop file opened
                             // Is here to comunicate the value between the
                             // ::read functions
    // CrossCursor
    Boolean CrossCursorInCol,CrossCursorInRow;
    static Boolean staticCrossCursorInCol;
    static Boolean staticCrossCursorInRow;
    int CrossCursorCol,CrossCursorRow;
    char CrossCursorBuf[setMaxScreenY];
    char CrossCursorBufR[setMaxScreenX];
    int CrossCursorY2,CrossCursorX2;
    Boolean CrossCurInCacheC,CrossCurInCacheR;

    // Status Line
    Boolean IsStatusLineOn;
    int     StatusLinePos;    // 1 means is at the end 0 at top
    static char StatusLine[setMaxScreenX*2];

    // New search options
    static ushort SearchInSel;
    static ushort FromWhere;
    static ushort RegExStyle;
    static ushort ReplaceStyle;
    static ushort CanOptimizeRegEx;
    unsigned StartOfSearch;
    unsigned selStartF,selEndF;
    Boolean IsFoundOn;
    static int    CompileSearch(char *searchStr, char *replaceStr=0);
    static uint32 MakeASearch(char *text, uint32 len, int &matchLen);
    static char  *GetTheReplace(int &mustDelete, uint32 &len);
    static int    CompileReplace(char *replaceStr);
    static char  *GetNormalReplace(int &mustDelete, uint32 &len);
    static char  *GetSpecialReplace(int &mustDelete, uint32 &len,char *(*GetHit)(int numHit, int &lenHit));
    static int    CompileRegEx(char *searchStr);
    static int    CompileNormal(char *searchStr);
    static uint32 MakeANormalSearch(char *block, uint32 size, int &matchLen);
    static uint32 MakeARegExSearch(char *block, uint32 size, int &matchLen);
    static int    CompilePCRE(char *searchStr);
    static uint32 MakeAPCRESearch(char *block, uint32 size, int &matchLen);
    static void   FreeRegExMemory(void);
    Boolean       SearchAndJump(char *search, unsigned flags);

    // Highligth of one char
    int XHLCO,YHLCO,XHLCC,YHLCC,XHLCo,YHLCo,XHLCc,YHLCc;
    Boolean IsHLCOn;
    char OldHLAttr,OldHLAttro;

    // Rectangular selection
    int Xr1,Yr1,Xr2,Yr2;     // Start and end point of the rect. sel
    Boolean selRectHided;    // To hide the sel
    struct selRecSt *selRectClip;

    // Groups of undo
    int undoLockCount;
    int undoGroupCount;

    // To keep track of the position of some lines
    TSpCollection *SpecialLines;

    // The modification time of the disk copy of the file
    // 0 if the file isn't in disk
    time_t DiskTime;
    // Last time we checked if the file on disk is newer.
    time_t lastTimeCheck;
    // Force a check in the next call (don't compute lastTimeCheck).
    Boolean forceNextTimeCheck;
    // Number of seconds to wait between checks
    static uint32 minDifModCheck;
    // device and starting inode of the file. 0 on new files.
    stEditorId EditorId;
    // attributes of the file from stat under Linux or _chmod in DOS
    CLY_mode_t ModeOfFile;

    // If 1 the colors are already cached
    static int colorsCached;
    // Colors cache
    static uchar cachedColors[];

    // For the pseudo macros by now only one
    static TPMCollection *PMColl;

    static const char *backupExt;
    static int (*MakeBkpForIt)(const char *name);
    // Menu that will be displayed when the right button of the mouse is pressed
    // It should be set by the TApplication, by default is 0 and the behavior is
    // the same used for the left button.
    static TSubMenu *RightClickMenu;

    void ShowLength();

    SetDefStreamMembersCommon(TCEditor,TViewPlus)
};

SetDefStreamOperators(TCEditor)

// A class to get lines from a TCEditor
class LineHandler
{
public:
 LineHandler() { ed=NULL; }
 void setEditor(TCEditor *anEd) { ed=anEd; offset=0; line=0; }
 int isReady() { return ed!=NULL; }
 char *getLine(int y, unsigned &len);

protected:
 unsigned offset;
 int line;
 TCEditor *ed;
};

#endif  // Uses_TCEditor


#if (defined(Uses_TFindCDialogRec) || defined(Uses_TReplaceCDialogRec)) && \
    !defined(__TRegexDialogRec__) && defined(Uses_TCEditor)
#define __TRegexDialogRec__
struct TRegexDialogRec
{
 uint32 regStyle;
 uint32 repStyle;
 uint32 optimize;
};

inline
void regexRecCreate(TRegexDialogRec &r)
{
 r.regStyle=TCEditor::RegExStyle;
 r.repStyle=TCEditor::ReplaceStyle;
 r.optimize=TCEditor::CanOptimizeRegEx;
}

inline
void regexRecUpdate(TRegexDialogRec &r)
{
 TCEditor::RegExStyle=r.regStyle;
 TCEditor::ReplaceStyle=r.repStyle;
 TCEditor::CanOptimizeRegEx=r.optimize;
}
#endif



#if defined( Uses_TCEditWindow ) && !defined( __TCEditWindow__ )
#define __TCEditWindow__

// That's a structure to hold the most important information of an
// editor window.
// The coordinates are short because a 32768 screen is big enough ;-)
struct EditorResumeV2
{
 uchar version,dummy;
 uchar shl,subshl;
 short origin_x,origin_y;
 short size_x,size_y;
 short cursor_x,cursor_y;
 short zorigin_x,zorigin_y;
 short zsize_x,zsize_y;
 uint16 ed_flags,prj_flags;
};

struct EditorResumeV3
{
 uchar version,dummy;
 uchar shl,subshl;
 short origin_x,origin_y;
 short size_x,size_y;
 short cursor_x,cursor_y;
 short zorigin_x,zorigin_y;
 short zsize_x,zsize_y;
 uint32 ed_flags,prj_flags;
};

struct EditorResumeV5
{
 uchar version,dummy;
 uchar shl,subshl;
 short origin_x,origin_y;
 short size_x,size_y;
 short cursor_x,cursor_y;
 short zorigin_x,zorigin_y;
 short zsize_x,zsize_y;
 uint32 ed_flags,prj_flags;
 unsigned tabSize,indentSize,wrapCol;
};

struct EditorResume
{
 uchar version,dummy;
 uchar shl,subshl;
 short origin_x,origin_y;
 short size_x,size_y;
 short cursor_x,cursor_y;
 short zorigin_x,zorigin_y;
 short zsize_x,zsize_y;
 uint32 ed_flags,prj_flags;
 unsigned tabSize,indentSize,wrapCol;
 uint32   extraSize;
 time_t   dateResume;
};

#define CopyEditorResume(dst,sou) memcpy(dst,sou,sizeof(EditorResume))

class TCEditWindow : public TWindow
{
public:
 TCEditWindow( const TRect&, const char *, int, Boolean openRO=False );
 ~TCEditWindow();
 virtual void close();
 virtual const char *getTitle( short );
 virtual void handleEvent( TEvent& );
 virtual void sizeLimits( TPoint& min, TPoint& max );
 virtual TPalette& getPalette() const;

 void FillResume(EditorResume &r);
 void ApplyResume(EditorResume &r);
 static void ReadResume(EditorResume &r, ipstream& is);
 static void SaveResume(EditorResume &r, opstream& os);
 static void FillResumeWith(EditorResume &r,TPoint &origin,TPoint &size,TPoint &cursor);

 TCEditor *editor;

protected:
 static const char *clipboardTitle;
 static stTVIntl   *iClipboardTitle;
 static const char *untitled;
 static stTVIntl   *iUntitled;

 static void EnlargeSizesResume(EditorResume &r);
 static void EnlargeSizeResume(short &x, short &y, int wS, int hS);
 static void ReduceSizesResume(EditorResume &r);
 static void ReduceSizeResume(short &x, short &y, int wS, int hS);

 static const int ResumeVersion;

 SetDefStreamMembersNoConst(TCEditWindow)
};

SetDefStreamOperators(TCEditWindow)

#endif  // Uses_TCEditWindow

#endif
