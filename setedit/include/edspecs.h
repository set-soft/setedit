/* Copyright (C) 2001-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef EDSPECS_H_INCLUDED
#define EDSPECS_H_INCLUDED

extern void InsertEnvironmentVar(const char *variable, const char *contents);
extern void InitEnvirVariables(void);
extern void DeInitEnvirVariables(void);
extern const char *GetVariable(const char *variable, const char *def=NULL);
extern int  EnvirVariablesIsOldVersion();
extern unsigned EnvirGetIntVar(const char *name, unsigned aDefault=0);
extern void     EnvirSetIntVar(const char *name, unsigned Value);
extern unsigned EnvirSetBits(const char *name, unsigned Value);
extern unsigned EnvirResetBits(const char *name, unsigned Value);
extern unsigned EnvirGetBits(const char *name, unsigned Value, unsigned aDefault=0);

// Values for SET_VARIOUS1
const unsigned svr1DontShowAbout=1;
// Values for SET_CREATE_DST
// Options for the .dst creation. Configured in edprefs.cc
extern unsigned GetDSTOptions();
const unsigned dstCreate=1,dstHide=2,dstNoCursorPos=4,dstRemmeberFK=8;
// Are editors restored?
const unsigned dstEdEver=0,dstEdOnlyIfNoCL=0x10,dstEdNever=0x20,dstEdMask=0x30;
// Are other windows restored?
const unsigned dstOwEver=0,dstOwOnlyIfNoCL=0x40,dstOwNever=0x80,dstOwMask=0xC0;
// Are closed files restored?
const unsigned dstCfEver=0,dstCfOnlyIfNoCL=0x100,dstCfNever=0x200,dstCfMask=0x300;

#endif // EDSPECS_H_INCLUDED

