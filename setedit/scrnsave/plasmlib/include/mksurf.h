/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
void MKS_DistCentS(int w, int h, unsigned char *s, int sw, int sh,
                   void (*CallBack)());
void MKS_SinDisCentS(int w, int h, unsigned char *s, int sw, int sh,
                     double Bsize, void (*CallBack)());

void MKS_SinDisCentSf(int w, int h, unsigned char *s, int sw, int sh,
                      double Bsize, void (*CallBack)());
void MKS_C2ySySiCx(int w, int h, unsigned char *s, void (*CallBack)());
void MKS_SxCxSyCySxyCh(int w, int h, unsigned char *s, void (*CallBack)(void));
void MKS_SxCxSyCySxy(int w, int h, unsigned char *s, void (*CallBack)(void));

