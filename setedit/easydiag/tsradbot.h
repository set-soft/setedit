/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSRadioButtons) && !defined(__TSRadioButtons_Defined__)
#define __TSRadioButtons_Defined__

class TSRadioButtons : public TSView
{
public:
 TSRadioButtons(TSItem *aStrings, int wForced=-1, int Columns=1);
 void setEnableMask(uint32 *masks, TSView **views, int cViews);
};

#endif
