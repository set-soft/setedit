/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TStreamableClass
#define Uses_TSIndicator
#include <ceditor.h>
__link( RView )

TStreamableClass RSIndicator( TSIndicator::name,
                              TSIndicator::build,
                              __DELTA(TSIndicator)
                            );

