/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifdef __cplusplus
extern "C" {
#endif
typedef struct
{
 unsigned *s1,*s2,*s3,*s4;
 unsigned char *dest;
 unsigned sWw,sH,mWw,mW;
} MS_4struct;

extern MS_4struct MS_4;

void MS_4add(void);
void MS_4addC(void);
void MS_4addW(void);
void MS_3add(void);
void MS_3addC(void);
void MS_2addC(void);
void MS_2add(void);
void MS_InvertDWordScreen(int w, int h, unsigned char *b);
#ifndef NO_BITMAP
void MS_AddBmpsMem(BITMAP *source, BITMAP *dest, int source_x,
                   int source_y, int dest_x, int dest_y, int width,
                   int height);
void MS_CutBMPColorsAt(BITMAP *bmp, unsigned char cut, unsigned char val);
void MS_CutBMPColorsAtInv(BITMAP *bmp, unsigned char cut, unsigned char val);
void MS_AddWithMaskMem(BITMAP *source, BITMAP *dest, int source_x,
                   int source_y, int dest_x, int dest_y, int width,
                   int height,int val);
void MS_AddWithMaskMemC(char *source, BITMAP *dest, int dest_x,
                        int dest_y, int width, int height,int val);
void MS_CompactMask(BITMAP *bmp);
void MS_DumpCompacted(BITMAP *bmp, char *name, char *file);
#endif // NO_BITMAP
#ifdef __cplusplus
}
#endif

