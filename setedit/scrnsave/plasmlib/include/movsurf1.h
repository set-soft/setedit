/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
typedef struct
{
 fix8_8 c1,c2,c3,c4,c5,c6,c7,c8; // 8 angles
 fix8_8 i1,i2,i3,i4,i5,i6,i7,i8; // 8 increments
 char *s1,*s2,*s3,*s4; // 4 surfaces
} MVS_4struct;

extern MVS_4struct  MVS_4;
void MVS_4SurfSC(void);

