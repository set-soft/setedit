/* Copyright (C) 2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */

int DBG_SetBreakpoint(const char *source, int line);
int DBG_RemoveBreakpoint(const char *source, int line);
void DBG_SetCallBacks();
void DBG_ReleaseMemory();
void DBG_ApplyBkpts();
void DBG_RefreshIgnoreBkpts();
void DBG_ApplyWpts();
void DBG_AddPathForSources(const char *path);

class TDebugMsgDialog;
class TWatchesDialog;
struct mi_frames_struct;

// Debug Status and Messages Window
TDebugMsgDialog *DebugMsgInit(Boolean hide=True, int ZOrder=-1);
TWatchesDialog  *WatchesInit(Boolean hide=False, int ZOrder=-1);
void DebugMsgDeInit();
void WatchesDeInit();
void DebugMsgClose();
void WatchesClose();
void DebugMsgAdd(char *msg);
void DebugMsgClear();
void DebugMsgUpdate(unsigned options=0);
void DebugMsgSetState();
void DebugMsgSetMode(Boolean select=False);
void DebugMsgSetError();
void DebugMsgSetStopped();
int  DebugMsgJumpToFrame(mi_frames_struct *f, char *msg, int l=0);
int  DebugMsgFillReason(mi_frames_struct *f, char *b, Boolean stop);
int  DebugGetErrorSt();
void DebugClearCPULine();
void DebugSetCPULine(int line, char *file);
void DebugUpdateWatches();
const char *DebugMsgStateName();
void DebugSaveData(opstream &os);
void DebugReadData(ipstream &is);
int  DebugConfirmEndSession();

// Configuration dialog
void DebugOptionsEdit();

#if defined(Uses_TBreakpoints) && !defined(TBreakpoints_Defined)
#define TBreakpoints_Defined
class TBreakpoints : public TStringable
{
public:
 TBreakpoints() :
   TStringable() {}
 ~TBreakpoints();

 virtual unsigned GetCount() { return count; };
 virtual void getText(char *dest, unsigned item, int maxLen);

 static void add(mi_bkpt *);
 static void remove(mi_bkpt *b);
 static int  set(const char *source, int line);
 static int  unset(const char *source, int line);
 static mi_bkpt *search(const char *source, int line);
 static void apply();
 static void refreshIgnore();
 static void refreshBinRef();
 static mi_bkpt *getItem(int num);
 static void releaseAll();
 static void replace(mi_bkpt *old, mi_bkpt *n);

 static void save(opstream &os);
 static void load(ipstream &is);

protected:
 static mi_bkpt *first, *last;
 static int count;
 static stTVIntl *icNone;

 static void updateAbs(mi_bkpt *b);

 friend class TDiagBrk;
};
#endif // Uses_TBreakpoints

#if defined(Uses_TWatchpoints) && !defined(TWatchpoints_Defined)
#define TWatchpoints_Defined
class TWatchpoints : public TStringable
{
public:
 TWatchpoints() :
   TStringable() {}
 ~TWatchpoints();

 virtual unsigned GetCount() { return count; };
 virtual void getText(char *dest, unsigned item, int maxLen);

 static void add(mi_wp *);
 static void remove(mi_wp *b);
 static void apply();
 static mi_wp *getItem(int num);
 static void releaseAll();
 static void replace(mi_wp *old, mi_wp *n);
 static int  unset(int num);
 static mi_wp *search(int num);

 static void save(opstream &os);
 static void load(ipstream &is, char version);

protected:
 static mi_wp *first, *last;
 static int count;

 friend class TDiagWp;
};
#endif // Uses_TWatchpoints

#if defined(Uses_TDisAsmWin) && !defined(TDisAsmWin_Defined)
#define TDisAsmWin_Defined
struct stAdd2Line
{
 void *addr;
 int line;
 mi_asm_insns *sourceL;
 mi_asm_insn *asmL;
};

class TAdd2Line : public TNSSortedCollection
{
public:
 TAdd2Line() : TNSSortedCollection(50,50) { byLine=0; };

 void insert(void *addr, int line, mi_asm_insns *sl, mi_asm_insn *al);
 stAdd2Line *At(ccIndex pos) { return (stAdd2Line *)at(pos); };
 Boolean searchL(int line, ccIndex &pos);

 virtual void *keyOf(void *item);
 virtual int compare(void *s1, void *s2);
 virtual void freeItem(void *s);

protected:
 int byLine;
};

class TRegisters : public TStringable
{
public:
 TRegisters(mi_chg_reg *aRegs, int cRegs);
 ~TRegisters();

 virtual void getText(char *dest, unsigned item, int maxLen);
 mi_chg_reg *getItem(int index);
 int update();

protected:
 mi_chg_reg *regs;
};

#define cpDisAsmEd \
  "\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C"\
  "\x2D\x2E\x2F\x30\x31\x32\x33\x34\x35\x36\x37\x38"\
  "\x39\x3A\x3B\x3C\x3D\x3E"

class TDisAsmEdWin : public TCEditWindow
{
public:
 TDisAsmEdWin(const TRect &aR);
 ~TDisAsmEdWin();

 virtual TPalette &getPalette() const;
 virtual const char *getTitle(short);

 int jumpToFrame(mi_frames *f);
 char *getFileLine(int &line)
 {
  if (!curLine)
     return NULL;
  line=curLine->line;
  return curLine->file;
 }
 char *getCodeInfo(char *b, int l);
 void *getFrom() { return from; }
 void *getTo() { return to; }
 int   runToCursor();

protected:
 mi_asm_insns *lines;
 TAdd2Line *a2l;
 void *from, *to;
 TSpCollection *spLine;
 mi_asm_insns *curLine;

 void setCode(mi_asm_insns *aLines);
 int  dissasembleFrame(mi_frames *f);
};

// Hack: An editor inside a dialog needs a special palette:
#define cpDisAsmWin \
  /* TDialog palette */ \
  "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F"\
  "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F"\
  /* TCEditWindow palette */ \
  "\x08\x09\x0A\x0B\x0C\x40\x41\x42\x43\x44\x45\x46"\
  "\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x76"\
  "\x77\x78\x79\x7A\x7B\x7C"

class TDisAsmWin : public TDialog
{
public:
 TDisAsmWin(const TRect &aR);
 ~TDisAsmWin();

 virtual TPalette &getPalette() const;
 virtual void close(void);
 virtual void handleEvent(TEvent &);

 static int  windowCreated() { return theDisAsmWin!=NULL; }
 static void beSelected() { theDisAsmWin->select(); }
 static int  jumpToFrame(mi_frames *f)
 {
  int ret=theDisAsmWin->edw->jumpToFrame(f);
  if (theDisAsmWin->regs && theDisAsmWin->regs->update())
     theDisAsmWin->bRegs->drawView();
  return ret;
 }
 static int  isDisAsmWinCurrent()
  { return theDisAsmWin && TProgram::deskTop->current==theDisAsmWin; }
 static int  isDisAsmWinAvailable()
  { return theDisAsmWin!=NULL; }
 static char *getFileLine(int &line)
  { return theDisAsmWin->edw->getFileLine(line); }
 static void updateCodeInfo()
  { if (theDisAsmWin) theDisAsmWin->upCodeInfo(); }
 static void *getFrom()
  { return theDisAsmWin ? theDisAsmWin->edw->getFrom() : NULL; }
 static void *getTo()
  { return theDisAsmWin ? theDisAsmWin->edw->getTo() : NULL; }
 static int   runToCursor()
  { return theDisAsmWin->edw->runToCursor(); }

protected:
 static TDisAsmWin *theDisAsmWin;
 TDisAsmEdWin *edw;
 TNoStaticText *codeInfo;
 TStringableListBox *bRegs;
 TRegisters *regs;

 int codeInfoLine;
 void upCodeInfo();
};

TDisAsmWin *TDisAsmWin::theDisAsmWin=NULL;
#endif // Uses_TDisAsmWin

