/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// Calculator and Debug Eval/Modif
const int
      cmEval          =0x2200,
      cmCaCopy        =0x2201,
      cmCaPaste       =0x2202,
      cmChange        =0x2203,
// Find & Replace dialog
      cmRegExOptions  =0x2218,
// Local options dialog
      cmSHLOptions    =0x2219,
// Calendar broadcast
      cmCalendarPlugIn=0x221A,
// List of windows (ALT+0) also project Window
      cmGo            =0x2290,
      cmDelete        =0x2291,
      cmInsert        =0x2292,
      cmDelFile       =0x2293,
      cmChangeSort    =0x2294,
      cmReEnumerate   =0x2295,
// Debug Watches & other debug
      cmEditWatch     =0x22A0,
      cmInsertWScope  =0x22A1,
      cmGDBCommand    =0x22A2,
      cmBkModify      =0x22A3,
      cmBkAdd         =0x22A4,
      cmBkDel         =0x22A5,
      cmBkEnable      =0x22A6,
      cmBkDisable     =0x22A7,
      cmBkGo          =0x22A8,
      cmDbgInspect    =0x22A9,
      cmExpand        =0x22AA,
      cmCollapse      =0x22AB,
      cmVarChanged    =0x22AC, // Broadcast
      cmDbgChgState   =0x22AD, // To current or Broadcast for dis/connected
      cmRecycle       =0x22AE,
      cmModifyIns     =0x22AF,
      cmFormatIns     =0x22B0,
      cmModifyReg     =0x22B1,
      //----------------------------
      // Data Window commands.
      //----------------------------
      cmDWFirstCommand =0x2300,
      //----------------------------
      cmDWUp           =0x2300,
      cmDWDown         =0x2301,
      cmDWRight        =0x2302,
      cmDWBaseIncrement=0x2303,
      cmDWLeft         =0x2304,
      cmDWBaseDecrement=0x2305,
      cmDWPgDn         =0x2306,
      cmDWPgUp         =0x2307,
      cmDWFirstColumn  =0x2308,
      cmDWFirstRow     =0x2309,
      cmDWLastRow      =0x230A,
      cmDWLastColumn   =0x230B,
      cmDWBaseAddress  =0x230C,
      cmDWGotoAddress  =0x230D,
      cmDWFollowPointer=0x230E,
      cmDWFollowPtnNew =0x230F,
      cmDWRecompute    =0x2310,
      cmDWTogAutoF     =0x2311,
      cmDWDispMode     =0x2312,
      cmDWTogEndian    =0x2313,
      cmDWRadix        =0x2314,
      cmDWFill         =0x2315,
      cmDWClear        =0x2316,
      cmDWMove         =0x2317,
      cmDWLessLines    =0x2318,
      cmDWMoreLines    =0x2319,
      cmDWUpdateMemory =0x231A,
      cmDWRead         =0x231B,
      cmDWWrite        =0x231C,
      //----------------------------
      cmDWLastCommand  =0x231D;
      //----------------------------
      // End of Data Window commands.
      //----------------------------

const int
      hcListWin            =0x2110,
      hcEditorProjectWindow=0x2115,
      hcMessageWindow      =0x2130,
      hcRegExOptions       =0x2150,
      hcDebugMsgWin        =0x2151,
      hcWatchesWin         =0x2152,
      hcWatchesWinEdit     =0x2153,
      hcEditBkpt           =0x2154,
      hcEditWp             =0x2155,
      hcBkptDialog         =0x2156,
      hcWpDialog           =0x2157,
      hcInspector          =0x2158,
      hcDataViewer         =0x2159,
      hcDisassembler       =0x215A,
      hcSourceLoc          =0x215B,
      hcDebugAdvOps        =0x215C,
      hcDebugOps           =0x215D,
      hcCalculator         =0x2200,
      hcEditKeys           =0x2240,
      hcEditKeysSeq        =0x2241,
      hcEditKeysMac        =0x2242,
      hcEditKeysCom        =0x2243,
      hcEditKeysLisp       =0x2244;


