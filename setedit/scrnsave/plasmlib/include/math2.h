/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// 64Kb
#define RandomTableSize 0x10000
extern unsigned char  RandomTable[RandomTableSize];
// short to wrap in the table
extern unsigned short RandomTablePos;
#define MA2_GetRand8()         RandomTable[RandomTablePos++]
void MA2_InitRTable(void);
