/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifdef INCL_TClusterArray
#include <tmiclust.h>
#endif

#ifdef INCL_TCheckBoxesArray
#include <tcheck2.h>
#endif

#ifdef INCL_TSView
#include <tsview.h>
#endif

#ifdef INCL_TSViewCol
#include <tsviewco.h>
#endif

#ifdef INCL_SNOSTATE
#include <snostate.h>
#endif

#ifdef INCL_TSHSCROL
#include <tshscrol.h>
#endif

#ifdef INCL_TTextScroller
#include <ttextscr.h>
#endif

#ifdef INCL_TSTextScroller
#include <tstextsc.h>
#endif

#ifdef INCL_TSSortedListBox_Var
#include <tssortva.h>
#endif

// It before the rest of the List Boxes because it's the base
#ifdef INCL_TSListBox
#include <tslistbo.h>
#endif

#ifdef INCL_TSStringableListBox
#include <tstsorlb.h>
#endif

#ifdef INCL_TSSortedListBox
#include <tssortlb.h>
#endif

#ifdef INCL_TSSOSSortedListBox
#include <tssosslb.h>
#endif

#ifdef INCL_TSStaticText
#include <tstatext.h>
#endif

#ifdef INCL_TSButton
#include <tsbutton.h>
#endif

#ifdef INCL_TSCHKARR
#include <tschkarr.h>
#endif

#ifdef INCL_TSHzGroup
#include <tshzgrp.h>
#endif

#ifdef INCL_TSVeGroup
#include <tsvegrp.h>
#endif

#ifdef INCL_TSInputLine
#include <tsinplin.h>
#endif

#ifdef INCL_TSInputLinePiped
#include <tsinplpi.h>
#endif

#ifdef INCL_TSLabel
#include <tslabel.h>
#endif

#ifdef INCL_TSCheckBoxes
#include <tschkbox.h>
#endif

#ifdef INCL_TSRadioButtons
#include <tsradbot.h>
#endif

#ifdef INCL_TSSlider
#include <tslider.h>
#endif

#ifdef INCL_TSLabelCheck
TSLabel *TSLabelCheck(const char *name, ...);
TSLabel *TSLabelCheck(int columns, const char *name, ...);
#endif

#ifdef INCL_TSLabelRadio
TSLabel *TSLabelRadio(const char *name, ...);
TSLabel *TSLabelRadio(int columns, const char *name, ...);
#endif

