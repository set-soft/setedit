/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSCheckBoxes) && !defined(__TSCheckBoxes_Defined__)
#define __TSCheckBoxes_Defined__

class TSCheckBoxes : public TSView
{
public:
 TSCheckBoxes(TSItem *aStrings, int wForced=-1, int Columns=1);
};

#endif
