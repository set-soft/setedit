/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSCheckBoxesArray) && !defined(__TSCheckBoxesArray_Defined__)
#define __TSCheckBoxesArray_Defined__

class TSCheckBoxesArray : public TSView
{
public:
 TSCheckBoxesArray(int xNum, int yNum);
 TSCheckBoxesArray(int xNum, int yNum, int (*pressCB)(int));
 TCheckBoxesArray *fill(int xNum, int yNum);
};

#endif
