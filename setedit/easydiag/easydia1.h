/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */

#ifndef Dont_Use_32Bits_Clusters
#define Clusters32Bits
#endif

#ifdef Uses_TSSlider
#define INCL_TSSlider
#define Uses_TScrollBar
#define Uses_TStaticText
#define Uses_TSView
#endif

#ifdef Uses_TSLabelCheck
#define INCL_TSLabelCheck
#define Uses_TSLabel
#endif

#ifdef Uses_TSLabelRadio
#define INCL_TSLabelRadio
#define Uses_TSLabel
#endif

#ifdef Uses_TSCheckBoxesArray
#define Uses_TSView
#define Uses_TCheckBoxesArray
#define INCL_TSCHKARR
#endif

#ifdef Uses_TCheckBoxesArray
#define Uses_TClusterArray
#define INCL_TCheckBoxesArray
#endif

#ifdef Uses_TClusterArray
#define Uses_TView
#define INCL_TClusterArray
#endif

#ifdef Uses_TSTextScroller
#define Uses_TTextScroller
#define Uses_TSViewCol
#define INCL_TSTextScroller
#endif

#ifdef Uses_TTextScroller
#define Uses_TNSCollection
#define Uses_TScroller
#define Uses_TScrollBar
#define Uses_TPalette
#define Uses_TEvent
#define INCL_TTextScroller
#endif

#ifdef Uses_TSHzLabel
#define Uses_TSLabel
#endif

#ifdef Uses_TSHzGroup
#define INCL_TSHzGroup
#define Uses_TSViewCol
#endif

#ifdef Uses_TSVeGroup
#define INCL_TSVeGroup
#define Uses_TSViewCol
#endif

#ifdef Uses_TSLabel
#define INCL_TSLabel
#define Uses_TSViewCol
#define Uses_TLabel
#endif

#ifdef Uses_TSCheckBoxes
#define INCL_TSCheckBoxes
#define Uses_TSViewCol
#define Uses_TCheckBoxes
#define Uses_TSItem
#endif

#ifdef Uses_TSRadioButtons
#define INCL_TSRadioButtons
#define Uses_TSViewCol
#define Uses_TRadioButtons
#define Uses_TSItem
#endif

#ifdef Uses_TSInputLine
#define INCL_TSInputLine
#define Uses_TSViewCol
#define Uses_TInputLine
#define Uses_THistory
#endif

#ifdef Uses_TSInputLinePiped
#define INCL_TSInputLinePiped
#define Uses_TSViewCol
#define Uses_TInputLinePiped
#define Uses_THistory
#endif

#ifdef Uses_TSButton
#define INCL_TSButton
#define Uses_TSViewCol
#define Uses_TButton
#endif


#ifdef Uses_TSStaticText
#define INCL_TSStaticText
#define Uses_TSViewCol
#define Uses_TStaticText
#endif

#ifdef Uses_TSSortedListBox
#define INCL_TSSortedListBox
#define Uses_TSListBox
#define Uses_TSortedListBox
#endif

#ifdef Uses_TSStringableListBox
#define INCL_TSStringableListBox
#define Uses_TSListBox
#define Uses_TStringableListBox
#endif

#ifdef Uses_TSSOSSortedListBox
#define INCL_TSSOSSortedListBox
#define Uses_TSListBox
#define Uses_TSOSSortedListBox
#endif

#ifdef Uses_TSListBox
#define INCL_TSListBox
#define Uses_TSSortedListBox_Var
#define Uses_TSViewCol
#define Uses_TListBox
#define Uses_TScrollBar
#endif

#ifdef Uses_TSSortedListBox_Var
#define INCL_TSSortedListBox_Var
#endif

#ifdef Uses_TSNoStaticText
#define INCL_SNOSTATE
#define Uses_TSViewCol
#define Uses_TNoStaticText
#endif

#ifdef Uses_TSHScrollBar
#define INCL_TSHSCROL
#define Uses_TSViewCol
#define Uses_TScrollBar
#endif

#ifdef Uses_TSViewCol
#define INCL_TSViewCol
#define Uses_TNSCollection
#define Uses_TSView
#define Uses_TDeskTop
#endif

#ifdef Uses_TSView
#define INCL_TSView
#define Uses_TDialog
#endif


