/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Class: TSArray<T>
  Comments:
  I'm just playing with templates, that's my array. I think the way GCC
implements templates sucks (sorry for the language). They are just a kind
of macros wothout any kind of intelligence. GCC can't detect automatically
where to create an instance of the template and ones he created one is
unable to inline things.@p
  This .cc file is an include for the instances of the class so is in the
include dir.

***************************************************************************/

#include <stdlib.h>

// All here and not in the class because GCC can't inline template members,
// that's a real pain!

template <class T>
TSArray<T>::TSArray(int initSize, int aDelta) :
   curSize(initSize),
   delta(aDelta)
{
 array=(T *)malloc(sizeof(T)*initSize);
}

template <class T>
TSArray<T>::~TSArray()
{
 free(array);
}

template <class T>
void TSArray<T>::resize(int i)
{
 if (i<curSize)
    return;
 do
   {
    curSize+=delta;
   }
 while (curSize<=i);
 array=(T *)realloc(array,sizeof(T)*curSize);
}

// GCC can't inline it!
template <class T>
T& TSArray<T>::operator()(int i)
{
 return array[i];
}

template <class T>
T& TSArray<T>::operator[](int i)
{
 if (i>=curSize)
    resize(i);
 return array[i];
}

