/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
typedef struct
{
 int cont; // starting point
 int cant; // number of points
 unsigned *Tray; // trajectory
 char *s1,*s2;   // Surfaces
} MVS_2Astruct;

extern MVS_2Astruct MVS_2A;
void MVS_2SurfA(void);