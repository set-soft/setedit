/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
const int
gadvUserScreen  =0,
gadvAbsolutePath=1,
gadvDeleteFile0 =2,
gadvFOShiftCase =3,
gadvFOShFuzzy   =4,
gadvFOShiftDirs =5,
gadvTabsOps     =6,
gadvTagsOld     =7,
gadvNoTags      =8,
gadvDiffModOpts =9,
gadvDbgNoPrj    =10,
gadvDbgDestSes  =11,
gadvDbgSesActive=12,
gadvDbgKillPrg  =13,
gadvDbgLinuxTTY =14;

int GiveAdvice(int number);
void AdviceManager();
