/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
typedef struct BITMAP            /* a bitmap structure */
{
   int w, h;                     /* width and height in pixels */
//   int clip;                     /* flag if clipping is turned on */
//   int cl, cr, ct, cb;           /* clip left, right, top and bottom values */
   void *dat;                    /* the memory we allocated for the bitmap */
//   int bitmap_id;                /* for identifying sub-bitmaps */
//   void *extra;                  /* points to a structure with more info */
//   int x_ofs;                    /* horizontal offset (for sub-bitmaps) */
//   int y_ofs;                    /* vertical offset (for sub-bitmaps) */
//   int seg;                      /* bitmap segment */
   unsigned char *line[0];       /* pointers to the start of each line */
} BITMAP;

void allegro_exit();
void allegro_init();
void vsync();
void set_gfx_mode(int driver,int xres, int yres, int x, int y);
BITMAP *create_bitmap(int x, int y);
void blit(BITMAP *bmp);
void clear(BITMAP *bmp);

#define GFX_VGA 0

extern int mouse_x,mouse_y,mouse_b,key_pressed;
#define MOUSE_EVENT 1
#define KEY_EVENT   2
int poll_input();

#define keypressed poll_input
#define screen 0

extern inline
void _putpixel(BITMAP *Bmp, int x, int y, unsigned char val)
{
 Bmp->line[y][x]=val;
}

extern inline
unsigned char _getpixel(BITMAP *Bmp, int x, int y)
{
 return Bmp->line[y][x];
}

#define is_linear_bitmap(a) (1)

