/*----------------------------------------------------------*/
/*                                                          */
/*   Turbo Vision 1.0                                       */
/*   Copyright (c) 1991 by Borland International            */
/*                                                          */
/*   Turbo Vision TVEDIT header file                        */
/*----------------------------------------------------------*/

class TMenuBar;
class TStatusLine;
class TEditWindow;
class TDialog;

const int
  //cmCalculator = 104,
  //cmDosShell   = 105,
  cmInfView    = 106,
  cmGrep       = 107,
  cmSyntax     = 108,
  cmSyntaxOps  = 109,
  cmSyntaxList = 110,
  cmAbout      = 111,
  cmManPage    = 112,
  cmScreenConf = 113,
  cmInfMainOpen= 114;

class fpstream;

struct stScreenOptions
{
 // Encoding options
 // !=0 if the encoding is forced. App=Application, Scr=Screen, Inp=Input.
 uchar enForceApp, enForceScr, enForceInp;
 // Which encoding is used, -1 means default one.
 int   enApp, enScr, enInp;
};

class TEditorMiApp : public TApplication
{
public:
 TEditorMiApp();

 virtual void handleEvent( TEvent& event );
 static TMenuBar *initMenuBar( TRect );
 static TStatusLine *initStatusLine( TRect );
 virtual void outOfMemory();
 virtual TPalette& getPalette() const;
 void saveDesktop(const char *fName);
 void storeDesktop(fpstream &s);
 void retrieveDesktop(const char *name, int loadWindows);
 void loadDesktop(fpstream &s, int loadWindows);
 
protected:
 void dosShell();
 void tile();
 void cascade();
 //void grepWindow();
 void ManPageView();
 void SetScreenOps(void);
 void SetCodePage(int cp);
 static void createDefaultSO();
 static char *createTitle(const char *title);

 static stScreenOptions *so;
};

extern void SyntaxSearch(void);

ushort execDialog( TDialog *d, void *data );
TDialog *createFindDialog();
TDialog *createReplaceDialog();
ushort doEditDialog( int dialog, ... );

