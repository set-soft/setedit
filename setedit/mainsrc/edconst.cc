/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdlib.h>
#define Uses_TCEditor_Commands
#define Uses_TNoCaseNoOwnerStringCollection
#define Uses_TCEditor_Internal
#define Uses_TInputLinePipedConst
#define Uses_MsgBox
// InfView requests
#include <infr.h>

#include <ceditor.h>
#include <setapp.h>

#include <inf.h>
#include <editcoma.h>

typedef struct
{
 char *name;
 int   command;
} edCList;

static edCList List[] =
{
{ "AltCase", 146 },
{ "ArbitraryIndent", 139 },
{ "BackSpace", 49 },
{ "BlockTrigger", 255 },
{ "CharLeft", 50 },
{ "CharRight", 51 },
{ "ChooseMacro", 116 },
{ "ChoosePMacrosList", 132 },
{ "Clear", 77 },
{ "ColorsChanged", 107 },
{ "CommentIndent", 143 },
{ "CommentUnIndent", 144 },
{ "CompactBuffer", 88 },
{ "Copy", 73 },
{ "CopyBlock", 94 },
{ "CopyClipFile", 150 },
{ "CopyClipWin", 110 },
{ "Cut", 72 },
{ "CutClipWin", 137 },
{ "DelChar", 62 },
{ "DelCharClear", 113 },
{ "DelEnd", 65 },
{ "DelLine", 66 },
{ "DelPrevWord", 12 },
{ "DelStart", 64 },
{ "DelWord", 63 },
{ "EndSelect", 13 },
{ "ExpandAllTabs", 87 },
{ "ExpandCode", 11 },
{ "Find", 79 },
{ "FirstLineInScreen", 3 },
{ "ForceMatchPairHL", 131 },
{ "GenCodeForMacro", 118 },
{ "GoBeginBlock", 18 },
{ "GoEndBlock", 19 },
{ "GoEndOfWord", 130 },
{ "GotoEditorLine", 17 },
{ "GotoMark0", 35 },
{ "GotoMark1", 36 },
{ "GotoMark2", 37 },
{ "GotoMark3", 38 },
{ "GotoMark4", 39 },
{ "GotoMark5", 40 },
{ "GotoMark6", 41 },
{ "GotoMark7", 42 },
{ "GotoMark8", 43 },
{ "GotoMark9", 44 },
{ "HideSelect", 69 },
{ "IndentBlk", 7 },
{ "IndentBlkOne", 5 },
{ "IndentMode", 70 },
{ "InsMode", 67 },
{ "InsertKeyName", 147 },
{ "InsertText", 45 },
{ "IntelIndentMode", 112 },
{ "InvertCase", 145 },
{ "JumpToFunction", 105 },
{ "JumpToPrototype", 136 },
{ "LastColInScreen", 140 },
{ "LastLineInScreen", 4 },
{ "LastPosCur", 103 },
{ "LineDown", 57 },
{ "LineEnd", 55 },
{ "LineOrScrEnd", 141 },
{ "LineStart", 54 },
{ "LineUp", 56 },
{ "LoadFileUnderCur", 108 },
{ "MarkLine", 20 },
{ "MarkWord", 21 },
{ "MouseToggle", 126 },
{ "MoveBlock", 22 },
{ "NewLine", 48 },
{ "NoCommand", 0 },
{ "PageDown", 59 },
{ "PageUp", 58 },
{ "Paste", 74 },
{ "PasteClipFile", 151 },
{ "PasteClipWin", 111 },
{ "PasteEmacsMode", 129 },
{ "PlayMacro", 91 },
{ "PopCursorPos", 134 },
{ "ProfileEditor", 106 },
{ "PushCursorPos", 133 },
{ "PutMark0", 25 },
{ "PutMark1", 26 },
{ "PutMark2", 27 },
{ "PutMark3", 28 },
{ "PutMark4", 29 },
{ "PutMark5", 30 },
{ "PutMark6", 31 },
{ "PutMark7", 32 },
{ "PutMark8", 33 },
{ "PutMark9", 34 },
{ "QuickTrigger", 254 },
{ "QuotedPrintDecode", 138 },
{ "ReadBlock", 46 },
{ "RecordMacro", 89 },
{ "Redo", 85 },
{ "RepeatMacro", 117 },
{ "Replace", 78 },
{ "ReplaceSelect", 16 },
{ "RunEnter_sLisp", 149 },
{ "RunSel_sLisp", 148 },
{ "Save", 114 },
{ "SaveAs", 115 },
{ "SaveAsDOS", 93 },
{ "SaveAsUNIX", 93 },
{ "SaveSameTime", 121 },
{ "ScrollDown", 23 },
{ "ScrollUp", 24 },
{ "SearchAgain", 76 },
{ "SearchClCor", 83 },
{ "SearchClPar", 81 },
{ "SearchComplement", 109 },
{ "SearchEnd", 10 },
{ "SearchOpCor", 82 },
{ "SearchOpPar", 80 },
{ "SearchStart", 9 },
{ "SelLength", 2 },
{ "SelRectCopy", 97 },
{ "SelRectCut", 100 },
{ "SelRectDel", 99 },
{ "SelRectEnd", 96 },
{ "SelRectHide", 102 },
{ "SelRectMove", 101 },
{ "SelRectPaste", 98 },
{ "SelRectStart", 95 },
{ "SelRectToLower", 128 },
{ "SelRectToUpper", 127 },
{ "SelectOff", 120 },
{ "SelectOn", 119 },
{ "SetGlobalOptions", 92 },
{ "SetLocalOptions", 86 },
{ "SmartIndent", 15 },
{ "SmartUnIndent", 14 },
{ "StartSelect", 68 },
{ "StopMacro", 90 },
{ "TextEnd", 61 },
{ "TextStart", 60 },
{ "ToLower", 1 },
{ "ToUpper", 84 },
{ "ToggleCharCase", 135 },
{ "ToggleMoveOnPaste", 104 },
{ "UnIndentBlk", 8 },
{ "UnIndentBlkOne", 6 },
{ "Undo", 75 },
{ "UpdateCodePage", 125 },
{ "UpdateTitle", 71 },
{ "WhichFunctionIs", 142 },
{ "WordLeft", 52 },
{ "WordRight", 53 },
{ "WrapOff", 123 },
{ "WrapOn", 122 },
{ "WrapToggle", 124 },
{ "WriteBlock", 47 }
};

// cmeBase
static edCList SetApp[] =
{
{ "ASCIIChart"     , 59 },
{ "AboutBox"       , 41 },
{ "AnotherInfView" , 10 },
{ "BoardMixer"     , 70 },
{ "Calculator"     , 4  },
{ "Calendar"       , 60 },
{ "Cascade"        , 21 },
{ "ChangeDrct"     , 2  },
{ "Close"          , 24 },
{ "ClosePrj"       , 12 },
{ "ColorTheme"     , 74 },
{ "ConfRunCommand" , 39 },
{ "DeleteBkps"     , 55 },
{ "DosShell"       , 3  },
{ "EdGralOptions"  , 37 },
{ "EditDeflOpts"   , 58 },
{ "EditKeyBind"    , 25 },
{ "EditNoBkp"      , 72 },
{ "EditPalette"    , 36 },
{ "EditUserWords"  , 57 },
{ "Encodings"      , 76 },
{ "ExportAsHTML"   , 64 },
{ "FileOpenOptions", 61 },
{ "Fonts"          , 77 },
{ "GrepDialog"     , 32 },
{ "HTMLAccents"    , 53 },
{ "HTMLTag2Accent" , 68 },
{ "IncludeList"    , 73 },
{ "InfView"        , 6  },
{ "KbBackDefault"  , 28 },
{ "KeyPadBehavior" , 49 },
{ "KeyboardSetUp"  , 66 },
{ "LastHelp"       , 8  },
{ "ListWin"        , 7  },
{ "LoadKeyScans"   , 26 },
{ "MP3Convert"     , 43 },
{ "MP3EditPlayList", 44 },
{ "MP3Open"        , 42 },
{ "MP3PlayList"    , 47 },
{ "MP3Start"       , 45 },
{ "MP3Stop"        , 46 },
{ "MP3StopList"    , 48 },
{ "ManPageView"    , 63 },
{ "New"            , 1  },
{ "Next"           , 22 },
{ "NextMessage"    , 33 },
{ "Open"           , 0  },
{ "OpenPrj"        , 11 },
{ "OpenROCopy"     , 56 },
{ "PocketCalc"     , 54 },
{ "Prev"           , 23 },
{ "PrevMessage"    , 34 },
{ "PrintEditor"    , 30 },
{ "Quit"           , 17 },
{ "QuitDelete"     , 16 },
{ "ReDraw"         , 67 },
{ "RemapCodePage"  , 65 },
{ "Resize"         , 18 },
{ "RunCommand"     , 38 },
{ "SDG"            , 13 },
{ "SDGDialog"      , 14 },
{ "SaveAll"        , 78 },
{ "SaveDesktop"    , 75 },
{ "SavePrj"        , 69 },
{ "ScreenSaverOpts", 62 },
{ "SearchTag"      , 80 },
{ "SeeScanCodes"   , 29 },
{ "SetColors"      , 15 },
{ "SetScreenOps"   , 35 },
{ "SetUpAltKeys"   , 27 },
{ "SetUpPrinter"   , 31 },
{ "ShowClip"       , 5  },
{ "StopChild"      , 71 },
{ "SyntaxHelp"     , 50 },
{ "SyntaxHelpFiles", 52 },
{ "SyntaxHelpOps"  , 51 },
{ "TagFiles"       , 79 },
{ "Tile"           , 20 },
{ "TipOfTheDay"    , 40 },
{ "UserScreen"     , 9  },
{ "Zoom"           , 19 }
};

// Editor constants used by sLisp (edf*)
static edCList SetFlags[] =
{
{"ComInside"     ,ComInside     }, // C++ comment in this line
{"EndCom"        ,EndCom        }, // */
{"EndCom2"       ,EndCom2       }, // }
{"ExtCom"        ,ExtCom        }, // more /* than */
{"ExtCom2"       ,ExtCom2       }, // more { than }
{"ExtOneLineCom" ,ExtOneLineCom },
{"ExtPrepro"     ,ExtPrepro     }, // Prepro+ends with \ <= be careful
{"ExtString"     ,ExtString     }, // a String ends with a \ and is continued on the next line
{"ExtString2"    ,ExtString2    },
{"ExtString3"    ,ExtString3    },
{"InsideCom"     ,InsideCom     }, // All commented
{"InsideCom2"    ,InsideCom2    }, // All commented with { }

// Message Box
{"MBCancelButton",mfCancelButton},
{"MBConfirmation",mfConfirmation},
{"MBError"       ,mfError       },
{"MBInformation" ,mfInformation },
{"MBNoButton"    ,mfNoButton    },
{"MBOKButton"    ,mfOKButton    },
{"MBOKCancel"    ,mfOKCancel    },
{"MBWarning"     ,mfWarning     },
{"MBYesButton"   ,mfYesButton   },
{"MBYesNoCancel" ,mfYesNoCancel },

{"Prepro"        ,Prepro        }, // Starts with #
{"StartCom"      ,StartCom      }, // /*
{"StartCom2"     ,StartCom2     }, // {
{"StartInCom"    ,StartInCom    }, // The first char is commented
{"StartInCom2"   ,StartInCom2   }, // The first char is commented
{"StartString"   ,StartString   }, // the line starts within a string (see ExtString)
{"StartString2"  ,StartString2  },
{"StartString3"  ,StartString3  },
{"WEDisk"        ,3             }, // WhichEditor constants
{"WEExtension"   ,4             },
{"WEFull"        ,0             },
{"WEFullNoExt"   ,1             },
{"WENameNoExt"   ,5             },
{"WEPath"        ,2             }
};

static int ptf(const void *ckey, const void *celem)
{
 return strcmp((char *)ckey,((edCList *)celem)->name);
}

const int NumCommands=sizeof(List)/sizeof(edCList);
const int NumCommandsEdit=sizeof(SetApp)/sizeof(edCList);
const int NumFlags=sizeof(SetFlags)/sizeof(edCList);

int SearchEdCommand(char *name)
{
 void *elem=bsearch(name,List,NumCommands,sizeof(edCList),ptf);
 if (elem==NULL)
    return -1;
 return ((edCList *)elem)->command;
}

int SearchEditCommand(char *name)
{
 void *elem=bsearch(name,SetApp,NumCommandsEdit,sizeof(edCList),ptf);
 if (elem==NULL)
    return -1;
 return ((edCList *)elem)->command;
}

int SearchEditFlag(char *name, unsigned &val)
{
 void *elem=bsearch(name,SetFlags,NumFlags,sizeof(edCList),ptf);
 if (elem==NULL)
    return 0;
 val=((edCList *)elem)->command;
 return 1;
}

char *TranslateEdCommand(int cmc_Com)
{
 int i;
 // Turbo Vision commands are 16 bits long. For some technical reasons some
 // times I use unsigned or int to handle them but in some special cases the
 // high 16 bits have crap. Old keybind.dat files can produce it.
 cmc_Com&=0xFFFF;
 cmc_Com-=cmbBaseNumber;
 for (i=0; i<NumCommands; i++)
     if (List[i].command==cmc_Com)
        return List[i].name;
 return "Unknown";
}

TNoCaseNoOwnerStringCollection *CreateEdCommandCol(void)
{
 TNoCaseNoOwnerStringCollection *p=new TNoCaseNoOwnerStringCollection(NumCommands,1);
 int i;
 for (i=0; i<NumCommands; i++)
     p->insert(List[i].name);
 return p;
}

/*****************************************************************************

 Miscellaneous constants used from the status line.
 All start with 'cm' which is omitted in the list.
 They must be alphabetically sorted because I use a binary search.

*****************************************************************************/

static edCList Simple[] =
{
#define C(a) { #a, cm##a }
 C(Close),
 C(DelFile),
 C(Delete),
 C(Eval),
 C(InfBack),
 C(InfBookM),
 C(InfControl),
 C(InfDir),
 C(InfGoto),
 C(InfHelp),
 C(InfNodes),
 C(InfOpen),
 C(InfPasteIn),
 C(InfTop),
 C(Insert),
 C(Menu),
 C(Zoom),
#undef C
#define C(a) { "h" #a, cmh##a }
 C(BookMarks),
 C(ConfigDia),
 C(Hide),
 C(HistSel),
 C(Next),
 C(NodeList),
 C(OpenInfo),
 C(Prev),
 C(PrevH),
 C(Up),
#undef  C
#define C(a) { "til" #a, cmtil##a }
 C(Copy),
 C(Paste)
#undef C
};

static
int compare(const void *s1, const void *s2)
{
 return strcmp((char *)s1,((edCList *)s2)->name);
}

int SearchSimpleCommand(char *name)
{
 edCList *res=(edCList *)bsearch(name,Simple,sizeof(Simple)/sizeof(edCList),
                                 sizeof(edCList),compare);
 return res ? res->command : -1;
}

#define C(a) { #a, hc##a }
static edCList HelpCtx[] =
{
 C(Calculator),
 C(EditKeys),
 C(EditKeysCom),
 C(EditKeysMac),
 C(EditKeysSeq),
 C(EditorProjectWindow),
 C(InfBookMark),
 C(InfChoose),
 C(InfConfig),
 C(InfControl),
 C(InfGoto),
 C(InfSearch),
 C(InfView),
 C(ListWin),
 C(MessageWindow),
 C(RegExOptions)
};
#undef C

int SearchHelpCtxCommand(char *name)
{
 edCList *res=(edCList *)bsearch(name,HelpCtx,sizeof(HelpCtx)/sizeof(edCList),
                                 sizeof(edCList),compare);
 return res ? res->command : -1;
}


