/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/***************************************************************

 Filename -       inf.h

 Functions
                  Member functions of following classes
                      THisCollection
                      TStrListBox
                      THelpControlDialog
                      TInfViewer
                      TInfWindow
                      TFindInfDialogRec

***************************************************************/

#if !defined( __Inf_H )
#define __Inf_H

#include <infbase.h>

#define MAX_HIST_DEEP 40
#define MAX_NODE_NAME 90

const int
      hcInfView   = 0x2100,
      hcInfSearch = 0x2101,
      hcInfControl= 0x2102,
      hcInfGoto   = 0x2103,
      hcInfChoose = 0x2104,
      hcInfConfig = 0x2105,
      hcInfBookMark = 0x2106;
const int
      cmInfHelp    = 0x2100,
      cmInfControl = 0x2101,
      cmInfBack    = 0x2102,
      cmInfPasteIn = 0x2103,
      cmInfBookM   = 0x2104,
      cmInfNodes   = 0x2105,
      cmInfGoto    = 0x2106,
      cmInfOpen    = 0x2107,
      cmInfDir     = 0x2108,
      cmInfTop     = 0x2109,
      cmInfLastLink= 0x210A,
      cmInfLink1   = 0x2140,
      cmInfLink2   = 0x2141,
      cmInfLink3   = 0x2142,
      cmInfLink4   = 0x2143,
      cmInfLink5   = 0x2144,
      cmInfLink6   = 0x2145,
      cmInfLink7   = 0x2146,
      cmInfLink8   = 0x2147,
      cmInfLink9   = 0x2148;
const int // first 16 bits are for selBestMatch
      jmpXRSubStr  = 0x10000;
      
typedef struct
{
 char Name[MAX_NODE_NAME];
 TPoint HisPos;
 int HisSel;
} stHisto;


/***************************************************************

 class THisCollection

 Una TCollection de stHisto.
 It's a TCollection of stHisto.

***************************************************************/

class THisCollection : public TCollection
{

public:

    THisCollection( short aLimit, short aDelta ) :
         TCollection(aLimit, aDelta) {};

private:

    virtual void freeItem( void *item )
        { delete (stHisto *)item; }
    virtual const char *streamableName() const
        { return name; }
    virtual void *readItem( ipstream& is );
    virtual void writeItem( void *obj, opstream& os );

protected:

    THisCollection( StreamableInit ) : TCollection ( streamableInit ) {};

public:

    static const char * const name;
    static TStreamable *build() {return new THisCollection( streamableInit );};

};

inline ipstream& operator >> ( ipstream& is, THisCollection& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, THisCollection*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, THisCollection& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, THisCollection* cl )
    { return os << (TStreamable *)cl; }


// TStrListBox

class TStrListBox : public TListBox
{
 public:
 TStrListBox(const TRect& r,ushort l,TScrollBar *a) : TListBox(r,l,a) {};
 void handleEvent(TEvent& event);
};

const int cmStrChoose=0x2110;



// THelpControlDialog

const int
      cmhNext=0x2111,
      cmhPrev=0x2112,
      cmhUp=0x2113,
      cmhPrevH=0x2114,
      cmhHide=0x2115,
      cmhNodeList=0x2116,
      cmhBookMarks=0x2117,
      cmhConfigDia=0x2118,
      cmhOpenInfo=0x2119,
      cmhHistSel=0x2130;

class THelpControlDialog : public TDialog
{
 public:
 THelpControlDialog(THisCollection *oCol);
 void handleEvent(TEvent& event);
 void getData(void *rec) { *(int *)rec=Comando; };
 void setData(void *rec) { Comando=*(int *)rec; };

 private:
 int Comando;
};

// TBookMarkDialog

const int
      cmbmAddToBookMark=0x2118,
      cmbmDeleteFromBookMark=0x2117,
      cmbmGoTo=0x2119;

class TBookMarkDialog : public TDialog
{
 public:
 TBookMarkDialog(TNoCaseStringCollection *oCol);
 void handleEvent(TEvent& event);
};

class TCommandSet;

// TInfViewer

class TInfViewer : public TScroller
{
public:

    TInfViewer( const TRect&, TScrollBar*, TScrollBar*, TInfFile*, char *,
                char *match );
    ~TInfViewer();

    virtual void changeBounds( const TRect& );
    virtual void draw();
    virtual void scrollDraw();
    virtual void resetCursor();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& );
    virtual void setState(uint16 aState, Boolean enable);
    void setCmdState(uint16 command, Boolean enable);
    void updateCommands(int full=0);
    void makeSelectVisible();
    void makeVisible(TPoint&,int largo);
    void GoEndOfLine(void);
    void UpdateSelection(int X, int Y);
    void MoveToMouse( TPoint m, uchar selMode );
    void lock() { lockCount++; };
    void unlock();
    void ChooseNode();
    void ConfigDialog();
    void OpenInfo();
    void NextWord(int selectMode, int x, int y);
    void PrevWord(int selectMode, int x, int y);

    void gotoInteractive();
    void switchToTopic( char *, TPoint );
    void switchToTopic( char * );
    void switchToTopic( stHisto *h );
    int  jumpXRefPartial(char *name, unsigned opts=0);

    TInfFile *hFile;
    TInfTopic *topic;
    int selected;

    // Search stuff
    void find();
    void findInTopic(char *s);
    void makeSearch(int beVerbose=1);
    int  searchInCurrentTopic(int Linea,int Col,int largo);
    int searchInCurrentFile(int Linea,int Col,int largo);
    int LineStartSelect;
    int LineEndSelect;
    int ColStartSelect;
    int ColEndSelect;
    char *lastSearch;
    char findStr[MAX_NODE_NAME];
    ushort findType;
    ushort findWhere;
    int    TopicInSearch;
    int    FileInSearch;
    Boolean TextSelected;
    stHisto OriginalPlace;
    int OpenVerbose;       // 1 normally, 0 if no warning on open (search)
    int SearchArmed;

    int selRowStart,selRowEnd,selRowStartPoint;
    int selColStart,selColEnd,selColStartPoint;
    Boolean selecting;
    static void (*InsertRoutine)(char *b, long l);
    //static void (*InsertRoutineSecondary)(char *b, long l); // i.e. GUI clipboard
    static void OSInsertRoutine0(char *b, long l);
    static void OSInsertRoutine1(char *b, long l);
    static void OSInsertRoutine(int clip, char *b, long l);
    void PasteToClipboard(void (*ir)(char *b, long l));
    void PasteInclude(void);
    int modeFlags;

    static TNoCaseStringCollection *BookMark;
    void BookMarksDialog(void);

    static void DisableAllCommands();
    static void InitCommandSet();
    static TCommandSet *ts;
    static int version;

protected:
    static int TranslateName;

private:
    THisCollection *History;
    int HistPoint;
    void  AddToHistory(char *);
    char *TakeFromHistory(TPoint& Pos);
    void SetTitle(char *File, char *Node);
    char QuickSearch[MAX_NODE_NAME];
    int  QuickLen;
    int  QuickVisPos;
    int  lockCount;
    Boolean mustBeRedrawed;

private:

    virtual const char *streamableName() const
	{ return name; }

protected:

    TInfViewer( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();

    friend class TInfWindow;
};

inline ipstream& operator >> ( ipstream& is, TInfViewer& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TInfViewer*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TInfViewer& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TInfViewer* cl )
    { return os << (TStreamable *)cl; }


const int fitWWord=2, fitCaseSens=1;
const int fitInTopic=0, fitInFile=1, fitInDir=2;
const int fitNotInTopic=-1;


// TInfWindow

class TInfWindow : public TWindow
{
public:

 TInfWindow( TInfFile*, char *context, char *match=0,
             void (*ir)(char *b, long l)=NULL, Boolean IsTheOne=False );

 virtual TPalette& getPalette() const;
 virtual void close();
 virtual TInfWindow::~TInfWindow();
 TInfViewer *viewer;
 Boolean isTheOne;
 static void SetMagicEditorInfoTranslation() { TInfViewer::TranslateName=1; };
 static void ResetMagicEditorInfoTranslation() { TInfViewer::TranslateName=0; };

private:

    virtual const char *streamableName() const
	{ return name; }

protected:

    TInfWindow( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TInfWindow& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TInfWindow*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TInfWindow& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TInfWindow* cl )
    { return os << (TStreamable *)cl; }


// Structure to hold the info returned by the dialog
#pragma pack(1)
struct TFindInfDialogRec
{
 TFindInfDialogRec(char *findStr, ushort flags1, ushort flags2)
  {
   strcpy(find,findStr);
   options=flags1;
   ops2=flags2;
  }
 char find[MAX_NODE_NAME];  // for TInputLine of MAX_NODE_NAME
 uint32 options; // for TCheckBoxes32   Type
 uint32 ops2;    // for TRadioButtons32 Where
};
#pragma pack()

TDialog *createGoToDialog();

void LoadInfoEnviroment(void);

#endif  // __Inf_H
