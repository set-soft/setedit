/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSArray_Def) && !defined(__Uses_TSArray_Def__)
#define __Uses_TSArray_Def__
template <class T>
class TSArray
{
public:
 TSArray(int initSize, int aDelta);
 ~TSArray();

 void resize(int i);
 T& operator[](int i);
 T& operator()(int i);

private:
 T *array;
 int curSize;
 int delta;
};
#endif
