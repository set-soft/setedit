/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifdef Uses_TCEditWindow
#define _INCL_CED_CLAS_
#define Uses_TCEditor
#define Uses_TWindow
#define Uses_stdio
#endif

#if defined(Uses_TCEditor) || defined(Uses_TCFileEditor)
#define _INCL_CED_CLAS_
#define _INCL_TVSETUTI
#define Uses_TRect
#define Uses_TScrollBar
#define Uses_TSIndicator
#define Uses_TEvent
#define Uses_TCEditor_External
#define Uses_EditorId
#define Uses_LineLengthArray
#define Uses_SOStack
#define Uses_TViewPlus
#define Uses_TPMCollection
#define Uses_TSArray_Def
#define _INCL_TIME_H_
#define Uses_limits
#define Uses_CLYFileAttrs
#define Uses_stdio
#endif

#ifdef Uses_TSIndicator
#define Uses_TIndicator
#define _INCL_SINDICAT_H_
#endif

#ifdef Uses_LineLengthArray
#define _INCL_CED_CLAS_
#endif

#ifdef Uses_TFindCDialogRec
#define _INCL_CED_CLAS_
#define Uses_string
#endif

#ifdef Uses_TReplaceCDialogRec
#define _INCL_CED_CLAS_
#define Uses_string
#endif

#ifdef Uses_TCEditor_Internal
#define _INCL_CED_INTE
#define _INCL_CTYPE
#define Uses_TStringCollection
#define	Uses_TSOSStringCollection
#define	Uses_SOStack
#endif

#ifdef Uses_TCEditor_External
#define _INCL_CED_EXTE
#define	Uses_TSOSStringCollection
#endif

#ifdef Uses_TCEditor_Commands
#define _INCL_CED_COMA
#endif

#ifdef Uses_TPMCollection
#define Uses_TStringCollection
#define Uses_TStringable
#define _INCL_PMCOLL
#endif

// That's to avoid a loop with the current TV
#ifdef Uses_TCEditor
#undef Uses_TCEditor
#define _Uses_TCEditor
#endif
#ifdef Uses_TCFileEditor
#undef Uses_TCFileEditor
#define _Uses_TCFileEditor
#endif
#ifdef Uses_TCEditWindow
#undef Uses_TCEditWindow
#define _Uses_TCEditWindow
#endif

#ifdef _INCL_STRING
#define Uses_string
#endif

#ifdef _INCL_DIR
#define Uses_dir
#endif

#ifdef _INCL_CTYPE
#define Uses_ctype
#endif

#ifdef _INCL_TIME_H_
#define Uses_time
#endif

#include <settvuti.h>

#ifdef _Uses_TCEditor
#undef _Uses_TCEditor
#define Uses_TCEditor
#endif
#ifdef _Uses_TCFileEditor
#undef _Uses_TCFileEditor
#define Uses_TCFileEditor
#endif
#ifdef _Uses_TCEditWindow
#undef _Uses_TCEditWindow
#define Uses_TCEditWindow
#endif

#ifdef _INCL_SINDICAT_H_
#include <sindicat.h>
#endif

#ifdef _INCL_TVSETUTI
#include <tvsetuti.h>
#endif

#ifdef _INCL_PMCOLL
#include <pmcoll.h>
#endif

#ifdef _INCL_CED_INTE
#include <ced_inte.h>
#endif

#ifdef _INCL_CED_EXTE
#include <ced_exte.h>
#endif

#ifdef _INCL_CED_COMA
#include <ced_coma.h>
#endif

#ifdef _INCL_CED_CLAS_
#define Uses_TCEditor_Class
#include <ced_clas.h>
#endif

#ifdef Uses_TCEditor
#undef Uses_TCEditor
#endif
#ifdef Uses_TCFileEditor
#undef Uses_TCFileEditor
#endif
#ifdef Uses_TCEditWindow
#undef Uses_TCEditWindow
#endif

