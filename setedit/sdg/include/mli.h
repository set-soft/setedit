/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
//char *MLIGetTypeError(int error);
//char *MLIGetError(int Error);

#ifdef Uses_TLispSDGstring
#define INCL_MLISDG_H
#define Uses_TMLISDGDefs
#define Uses_TLispVar
#define Uses_TLispVarDefs
#endif

#ifdef Uses_TMLISDG
#define INCL_MLICSDG_H
#define Uses_TMLISDGDefs
#define Uses_TMLIBase
#endif

#ifdef Uses_TMLIEditor
#define INCL_MLIEDITO_H
#define Uses_TLispVarDefs
#define Uses_TMLIEditorDefs
#define Uses_TMLIBase
#endif

#ifdef Uses_TMLISDGDefs
#define INCL_MLISDGI_H
#endif

#ifdef Uses_TMLIEditorDefs
#define INCL_MLIEDITD_H
#endif

#ifdef Uses_TMLIArraySimple
#define INCL_MLIASIMP_H
#define Uses_TMLIArrayBase
#endif

#ifdef Uses_TMLIArrayBase
#define INCL_MLIABASE_H
#endif

#ifdef Uses_TMLIBase
#define INCL_MLIBASE_H
#endif

#ifdef Uses_TLispVariableCol
#define INCL_TV_H
#define INCL_MLIVAR_H
#define Uses_TNSSortedCollection
#endif

#ifdef Uses_TLispVarDefs
#define INCL_MLIVAR_H
#endif

#ifdef Uses_TLispVar
#define INCL_MLIVAR_H
#endif

#ifdef Uses_TLispBaseVars
#define INCL_MLIVAR_H
#define Uses_TLispVar
#endif

#ifdef INCL_TV_H
#include <tv.h>
#endif

#ifdef INCL_MLIVAR_H
#include "mlivar.h"
#endif

#ifdef INCL_MLIABASE_H
#include "mliabase.h"
#endif

#ifdef INCL_MLIBASE_H
#include "mlibase.h"
#endif

#ifdef INCL_MLIASIMP_H
#include "mliasimp.h"
#endif

#ifdef INCL_MLISDG_H
#include "mlisdg.h"
#endif

#ifdef INCL_MLISDGI_H
#include "mlisdgi.h"
#endif

#ifdef INCL_MLIEDITD_H
#include "mlieditd.h"
#endif

#ifdef INCL_MLICSDG_H
#include "mlicsdg.h"
#endif

#ifdef INCL_MLIEDITO_H
#include "mliedito.h"
#endif





