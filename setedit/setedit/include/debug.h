/* Copyright (C) 2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */

int DBG_SetBreakpoint(const char *source, int line);
int DBG_RemoveBreakpoint(const char *source, int line);
void DBG_SetCallBacks();
void DBG_ReleaseMemory();
void DBG_ApplyBkpts();
void DBG_ApplyWpts();

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

