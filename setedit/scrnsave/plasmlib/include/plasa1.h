/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// Plasma 1 using sin(X)+cos(Y) as base. Really simple and fast
void PLA1_Step320x200_1(unsigned char *screen_buffer);
void PLA1_Step320x200_1b(unsigned char *screen_buffer);
void PLA1_Step320x200_2(unsigned char *screen_buffer);
void PLA1_Step320x200_2b(unsigned char *screen_buffer);
void PLA1_Step320x200_3(unsigned char *screen_buffer);
void PLA1_Step320x200_3b(unsigned char *screen_buffer);
void PLA1_Step320x200_4(unsigned char *screen_buffer);
void PLA1_Step320x200_5(unsigned char *screen_buffer);
void PLA1_Step320x200_6(unsigned char *screen_buffer);
void PLA1_Step320x200_7(unsigned char *screen_buffer);
void PLA1_Step_1(int w, int h, unsigned char *screen_buffer);
void PLA1_Step_2(int w, int h, unsigned char *screen_buffer);
void PLA1_Step_3(int w, int h, unsigned char *screen_buffer);
void PLA1_Step_4(int w, int h, unsigned char *screen_buffer);
void PLA1_Step_7(int w, int h, unsigned char *screen_buffer);

// Plasma 2 uses circles (3D cones)
int  PLA2_InitPlasmaTables(void);
void PLA2_DeInit(void);
void PLA2_StepBody(unsigned char *buf);
void PLA2_Step1(unsigned char *buf);
void PLA2_Step2(unsigned char *buf);
void PLA2_Step3(unsigned char *buf);
int  PLA2G_InitPlasmaTables(int w,int h, unsigned char *buf,double Bsize,
                            int Surf1, int mW, int mH,
                            void (*CallBack)(void));
void PLA2G_DeInit(void);
void PLA2G_Step();

int  PLA3_InitPlasmaTables(int Surface,int w,int h,unsigned char *buf,int w2,
                           int h2,unsigned char *extSurface,
                           void (*CallBack)(void));
void PLA3_DeInit(void);
//void PLA3_StepMap(unsigned char *buf);
void PLA3_StepMap(void);
void PLA3_StepPal(void);
