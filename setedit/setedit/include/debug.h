/* Copyright (C) 2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */

int DBG_SetBreakpoint(const char *source, int line);
int DBG_RemoveBreakpoint(const char *source, int line);
void DBG_SetCallBacks();
void DBG_ReleaseMemory();
void DBG_ApplyBkpts();

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
void DebugMsgUpdate(unsigned options=0);
void DebugMsgSetState();
void DebugMsgSetMode(Boolean select=False);
void DebugMsgSetError();
void DebugMsgSetStopped();
int  DebugMsgJumpToFrame(mi_frames_struct *f, char *msg, int l=0);
int  DebugMsgFillReason(mi_frames_struct *f, char *b, Boolean stop);
void DebugClearCPULine();
void DebugSetCPULine(int line, char *file);
void DebugUpdateWatches();
char *DebugMsgStateName();
void DebugSaveData(opstream &os);
void DebugReadData(ipstream &is);
int  DebugConfirmEndSession();

// Configuration dialog
void DebugOptionsEdit();
