/* Copyright (C) 1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
#include <stdlib.h>
#include <stdio.h>
#include <fakealle.h>
#include <math1.h>

void AllegroInit(void)
{
 MA1_CreateLookUp();
 allegro_init();

 set_gfx_mode(GFX_VGA,320,200,0,0);
}

void AllegroSetMode(void)
{
 set_gfx_mode(GFX_VGA,320,200,0,0);
}

void CalcuInit(void)
{
 printf("Calculating ...");
 printf("Please wait a little ");
}

void CalcuCallBack(void)
{
 printf(".");
}

void CalcuDeInit(void)
{
}
