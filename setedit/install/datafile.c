/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *      By Shawn Hargreaves
 *      shawn@talula.demon.co.uk
 *      http://www.talula.demon.co.uk/allegro/
 *
 *      Datafile reading routines.
 *
 *      See readme.txt for copyright information.
 */

 /* This file was modified to save around 140 Kb of code in the installer,
    if you need to use another version of Allegro you can remove this file
    from the project and use the version provided with the library */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef DJGPP
#include <sys/segments.h>
#endif

//#include "internal.h"
#include <allegro.h>

#if ALLEGRO_VERSION<4 && !defined(ALLEGRO_WIP_VERSION)
/* data file ID's for compatibility with the old datafile format */
#define V1_DAT_MAGIC             0x616C6C2EL

#define V1_DAT_DATA              0
#define V1_DAT_FONT              1
#define V1_DAT_BITMAP_16         2 
#define V1_DAT_BITMAP_256        3
#define V1_DAT_SPRITE_16         4
#define V1_DAT_SPRITE_256        5
#define V1_DAT_PALLETE_16        6
#define V1_DAT_PALLETE_256       7
#define V1_DAT_FONT_8x8          8
#define V1_DAT_FONT_PROP         9
#define V1_DAT_BITMAP            10
#define V1_DAT_PALLETE           11
#define V1_DAT_SAMPLE            12
#define V1_DAT_MIDI              13
#define V1_DAT_RLE_SPRITE        14
#define V1_DAT_FLI               15
#define V1_DAT_C_SPRITE          16
#define V1_DAT_XC_SPRITE         17

#define OLD_FONT_SIZE            95

/* information about a datafile object */
typedef struct DATAFILE_TYPE
{
   int type;
   void *(*load)(PACKFILE *f, long size);
   void (*destroy)();
} DATAFILE_TYPE;


#define MAX_DATAFILE_TYPES    32
extern DATAFILE_TYPE datafile_type[MAX_DATAFILE_TYPES];


/* read_block:
 *  Reads a block of size bytes from a file, allocating memory to store it.
 */
static void *read_block(PACKFILE *f, int size, int alloc_size)
{
   void *p;

   p = malloc(MAX(size, alloc_size));
   if (!p) {
      errno = ENOMEM;
      return NULL;
   }

   pack_fread(p, size, f);

   if (pack_ferror(f)) {
      free(p);
      return NULL;
   }

   return p;
}

/* read_old_datafile:
 *  Helper for reading old-format datafiles (only needed for backward
 *  compatibility).
 */
static DATAFILE *read_old_datafile(PACKFILE *f)
{
   DATAFILE *dat;
   int size;
   int type;
   int c;

   size = pack_mgetw(f);
   if (errno) {
      pack_fclose(f);
      return NULL;
   }

   dat = malloc(sizeof(DATAFILE)*(size+1));
   if (!dat) {
      pack_fclose(f);
      errno = ENOMEM;
      return NULL;
   }

   for (c=0; c<=size; c++) {
      dat[c].type = DAT_END;
      dat[c].dat = NULL;
      dat[c].size = 0;
      dat[c].prop = NULL;
   }

   for (c=0; c<size; c++) {

      type = pack_mgetw(f);

      switch (type) {

	 case V1_DAT_FONT: 
	 case V1_DAT_FONT_8x8: 
	    dat[c].type = DAT_FONT;
	    //dat[c].dat = read_font_fixed(f, 8, OLD_FONT_SIZE);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_FONT_PROP:
	    dat[c].type = DAT_FONT;
	    //dat[c].dat = read_font_prop(f, OLD_FONT_SIZE);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_BITMAP:
	 case V1_DAT_BITMAP_256:
	    dat[c].type = DAT_BITMAP;
	    //dat[c].dat = read_bitmap(f, 8, TRUE);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_BITMAP_16:
	    dat[c].type = DAT_BITMAP;
	    //dat[c].dat = read_bitmap(f, 4, FALSE);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_SPRITE_256:
	    dat[c].type = DAT_BITMAP;
	    pack_mgetw(f);
	    //dat[c].dat = read_bitmap(f, 8, TRUE);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_SPRITE_16:
	    dat[c].type = DAT_BITMAP;
	    pack_mgetw(f);
	    //dat[c].dat = read_bitmap(f, 4, FALSE);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_PALLETE:
	 case V1_DAT_PALLETE_256:
	    dat[c].type = DAT_PALLETE;
	    //dat[c].dat = read_pallete(f, PAL_SIZE);
	    dat[c].size = sizeof(PALLETE);
	    break;

	 case V1_DAT_PALLETE_16:
	    dat[c].type = DAT_PALLETE;
	    //dat[c].dat = read_pallete(f, 16);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_SAMPLE:
	    dat[c].type = DAT_SAMPLE;
	    //dat[c].dat = read_sample(f);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_MIDI:
	    dat[c].type = DAT_MIDI;
	    //dat[c].dat = read_midi(f);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_RLE_SPRITE:
	    dat[c].type = DAT_RLE_SPRITE;
	    //dat[c].dat = read_rle_sprite(f, 8);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_FLI:
	    dat[c].type = DAT_FLI;
	    dat[c].size = pack_mgetl(f);
	    dat[c].dat = read_block(f, dat[c].size, 0);
	    break;

	 case V1_DAT_C_SPRITE:
	    dat[c].type = DAT_C_SPRITE;
	    //dat[c].dat = read_compiled_sprite(f, FALSE, 8);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_XC_SPRITE:
	    dat[c].type = DAT_XC_SPRITE;
	    //dat[c].dat = read_compiled_sprite(f, TRUE, 8);
	    dat[c].size = 0;
	    break;

	 default:
	    dat[c].type = DAT_DATA;
	    dat[c].size = pack_mgetl(f);
	    dat[c].dat = read_block(f, dat[c].size, 0);
	    break;
      }

      if (errno) {
	 if (!dat[c].dat)
	    dat[c].type = DAT_END;
	 unload_datafile(dat);
	 pack_fclose(f);
	 return NULL;
      }
   }

   return dat;
}



/* load_data_object:
 *  Loads a binary data object from a datafile.
 */
void *load_data_object(PACKFILE *f, long size)
{
   return read_block(f, size, 0);
}


/* load_font_object:
 *  Loads a font object from a datafile.
 */
void *load_font_object(PACKFILE *f, long size)
{
 return 0;
}

/* load_sample_object:
 *  Loads a sample object from a datafile.
 */
void *load_sample_object(PACKFILE *f, long size)
{
   return 0;
}

/* load_midi_object:
 *  Loads a midifile object from a datafile.
 */
void *load_midi_object(PACKFILE *f, long size)
{
   return 0;
}

/* load_bitmap_object:
 *  Loads a bitmap object from a datafile.
 */
void *load_bitmap_object(PACKFILE *f, long size)
{
   return 0;
}

/* load_rle_sprite_object:
 *  Loads an RLE sprite object from a datafile.
 */
void *load_rle_sprite_object(PACKFILE *f, long size)
{
   return 0;
}

/* load_compiled_sprite_object:
 *  Loads a compiled sprite object from a datafile.
 */
void *load_compiled_sprite_object(PACKFILE *f, long size)
{
   return 0;
}

/* load_xcompiled_sprite_object:
 *  Loads a mode-X compiled object from a datafile.
 */
void *load_xcompiled_sprite_object(PACKFILE *f, long size)
{
   return 0;
}

/* unload_sample: 
 *  Destroys a sample object.
 */
void unload_sample(SAMPLE *s)
{
}



/* unload_midi: 
 *  Destroys a MIDI object.
 */
void unload_midi(MIDI *m)
{
}


/* load_object:
 *  Helper to load an object from a datafile.
 */
static void *load_object(PACKFILE *f, int type, long size)
{
   int i;

   /* look for a load function */
   for (i=0; i<MAX_DATAFILE_TYPES; i++)
      if (datafile_type[i].type == type)
	 return datafile_type[i].load(f, size);

   /* if not found, load binary data */
   return load_data_object(f, size);
}



/* load_file_object:
 *  Loads a datafile object.
 */
void *load_file_object(PACKFILE *f, long size)
{
   #define MAX_PROPERTIES  64

   DATAFILE_PROPERTY prop[MAX_PROPERTIES];
   DATAFILE *dat;
   int prop_count, count, type, c, d;

   count = pack_mgetl(f);

   dat = malloc(sizeof(DATAFILE)*(count+1));
   if (!dat) {
      errno = ENOMEM;
      return NULL;
   }

   for (c=0; c<=count; c++) {
      dat[c].type = DAT_END;
      dat[c].dat = NULL;
      dat[c].size = 0;
      dat[c].prop = NULL;
   }

   for (c=0; c<MAX_PROPERTIES; c++)
      prop[c].dat = NULL;

   c = 0;
   prop_count = 0;

   while (c < count) {
      type = pack_mgetl(f);

      if (type == DAT_PROPERTY) {
	 /* load an object property */
	 type = pack_mgetl(f);
	 d = pack_mgetl(f);
	 if (prop_count < MAX_PROPERTIES) {
	    prop[prop_count].type = type;
	    prop[prop_count].dat = malloc(d+1);
	    if (prop[prop_count].dat != NULL) {
	       pack_fread(prop[prop_count].dat, d, f);
	       prop[prop_count].dat[d] = 0;
	       prop_count++;
	       d = 0;
	    }
	 }
	 while (d-- > 0)
	    pack_getc(f);
      }
      else {
	 /* load actual data */
	 f = pack_fopen_chunk(f, FALSE);
	 d = f->todo;

	 dat[c].dat = load_object(f, type, d);

	 if (dat[c].dat) {
	    dat[c].type = type;
	    dat[c].size = d;

	    if (prop_count > 0) {
	       dat[c].prop = malloc(sizeof(DATAFILE_PROPERTY)*(prop_count+1));
	       if (dat[c].prop != NULL) {
		  for (d=0; d<prop_count; d++) {
		     dat[c].prop[d].dat = prop[d].dat;
		     dat[c].prop[d].type = prop[d].type;
		     prop[d].dat = NULL;
		  }
		  dat[c].prop[d].dat = NULL;
		  dat[c].prop[d].type = DAT_END;
	       }
	       else {
		  for (d=0; d<prop_count; d++) {
		     free(prop[d].dat);
		     prop[d].dat = NULL;
		  }
	       }
	       prop_count = 0;
	    }
	    else
	       dat[c].prop = NULL;
	 }

	 f = pack_fclose_chunk(f);
	 c++;
      }

      if (errno) {
	 unload_datafile(dat);

	 for (c=0; c<MAX_PROPERTIES; c++)
	    if (prop[c].dat)
	       free(prop[c].dat);

	 return NULL;
      }
   }

   for (c=0; c<MAX_PROPERTIES; c++)
      if (prop[c].dat)
	 free(prop[c].dat);

   return dat;
}



/* load_datafile:
 *  Loads an entire data file into memory, and returns a pointer to it. 
 *  On error, sets errno and returns NULL.
 */
DATAFILE *load_datafile(char *filename)
{
   PACKFILE *f;
   DATAFILE *dat;
   int type;

   f = pack_fopen(filename, F_READ_PACKED);
   if (!f)
      return NULL;

   type = pack_mgetl(f);

   if (type == V1_DAT_MAGIC) {
      dat = read_old_datafile(f);
   }
   else {
      if (type == DAT_MAGIC)
	 dat = load_file_object(f, 0);
      else
	 dat = NULL;
   }

   pack_fclose(f);
   return dat; 
}



/* load_datafile_object:
 *  Loads a single object from a datafile.
 */
DATAFILE *load_datafile_object(char *filename, char *objectname)
{
   PACKFILE *f;
   DATAFILE *dat = NULL;
   void *object;
   int type, size;
   int use_next = FALSE;
   char buf[256];

   f = pack_fopen(filename, F_READ_PACKED);
   if (!f)
      return NULL;

   type = pack_mgetl(f);

   if (type != DAT_MAGIC)
      goto getout;

   pack_mgetl(f);

   while (!pack_feof(f)) {
      type = pack_mgetl(f);

      if (type == DAT_PROPERTY) {
	 type = pack_mgetl(f);
	 size = pack_mgetl(f);
	 if (type == DAT_ID('N','A','M','E')) {
	    /* examine name property */
	    if (size > (int)(sizeof(buf)-1)) {
	       pack_fread(buf, sizeof(buf)-1, f);
	       buf[sizeof(buf)-1] = 0;
	       pack_fseek(f, size-(sizeof(buf)-1));
	    }
	    else {
	       pack_fread(buf, size, f);
	       buf[size] = 0;
	    }
	    if (stricmp(buf, objectname) == 0)
	       use_next = TRUE;
	 }
	 else {
	    /* skip property */
	    pack_fseek(f, size);
	 }
      }
      else {
	 if (use_next) {
	    /* load actual data */
	    f = pack_fopen_chunk(f, FALSE);
	    size = f->todo;
	    object = load_object(f, type, size);
	    f = pack_fclose_chunk(f);
	    if (object) {
	       dat = malloc(sizeof(DATAFILE));
	       if (dat != NULL) {
		  dat->dat = object;
		  dat->type = type;
		  dat->size = size;
		  dat->prop = NULL;
	       }
	    }
	    goto getout;
	 }
	 else {
	    /* skip unwanted object */
	    size = pack_mgetl(f);
	    pack_fseek(f, size+4);
	 }
      }
   }

   getout:

   pack_fclose(f);

   return dat; 
}



/* _unload_datafile_object:
 *  Helper to destroy a datafile object.
 */
void _unload_datafile_object(DATAFILE *dat)
{
   int i;

   /* free the property list */
   if (dat->prop) {
      for (i=0; dat->prop[i].type != DAT_END; i++)
	 if (dat->prop[i].dat)
	    free(dat->prop[i].dat);

      free(dat->prop);
   }

   /* look for a destructor function */
   for (i=0; i<MAX_DATAFILE_TYPES; i++) {
      if (datafile_type[i].type == dat->type) {
	 if (dat->dat) {
	    if (datafile_type[i].destroy)
	       datafile_type[i].destroy(dat->dat);
	    else
	       free(dat->dat);
	 }
	 return;
      }
   }

   /* if not found, just free the data */
   if (dat->dat)
      free(dat->dat);
}



/* unload_datafile:
 *  Frees all the objects in a datafile.
 */
void unload_datafile(DATAFILE *dat)
{
   int i;

   if (dat) {
      for (i=0; dat[i].type != DAT_END; i++)
	 _unload_datafile_object(dat+i);

      free(dat);
   }
}



/* unload_datafile_object:
 *  Unloads a single datafile object, returned by load_datafile_object().
 */
void unload_datafile_object(DATAFILE *dat)
{
   if (dat) {
      _unload_datafile_object(dat);
      free(dat);
   }
}



/* get_datafile_property:
 *  Returns the specified property string for the datafile object, or
 *  an empty string if the property does not exist.
 */
char *get_datafile_property(DATAFILE *dat, int type)
{
   DATAFILE_PROPERTY *prop = dat->prop;

   if (prop) {
      while (prop->type != DAT_END) {
	 if (prop->type == type)
	    return prop->dat;

	 prop++;
      }
   }

   return "";
}

/* _construct_datafile:
 *  Helper used by the output from dat2s.exe, for fixing up parts of
 *  the data that can't be statically initialised, such as locking
 *  samples and MIDI files, and setting the segment selectors in the 
 *  BITMAP structures.
 */
void _construct_datafile(DATAFILE *data)
{
   int c, c2;
   SAMPLE *s;
   MIDI *m;

   for (c=0; data[c].type != DAT_END; c++) {

      switch (data[c].type) {

	 case DAT_FILE:
	    _construct_datafile((DATAFILE *)data[c].dat);
	    break;

	 case DAT_BITMAP:
	    ((BITMAP *)data[c].dat)->seg = _my_ds();
	    break;

	 case DAT_FONT:
	    if (((FONT *)data[c].dat)->height < 0) {
	       for (c2=0; c2<FONT_SIZE; c2++)
		  ((FONT *)data[c].dat)->dat.dat_prop->dat[c2]->seg = _my_ds();
	    }
	    break;

	 case DAT_SAMPLE:
	    s = (SAMPLE *)data[c].dat;
	    LOCK_DATA(s, sizeof(SAMPLE));
	    LOCK_DATA(s->data, s->len * ((s->bits==8) ? 1 : sizeof(short)) * ((s->stereo) ? 2 : 1));
	    break;

	 case DAT_MIDI:
	    m = (MIDI *)data[c].dat;
	    LOCK_DATA(m, sizeof(MIDI));
	    for (c2=0; c2<MIDI_TRACKS; c2++)
	       if (m->track[c2].data)
		  LOCK_DATA(m->track[c2].data, m->track[c2].len);
	    break;
      }
   }
}



/* initialize_datafile_types:
 *  Register my loader functions with the code in dataregi.c.
 */
#ifdef __GNUC__
/* alternatively called from allegro_init */
static void initialize_datafile_types() __attribute__ ((constructor));
static
#endif
void initialize_datafile_types()
{
   register_datafile_object(DAT_FILE,         load_file_object,             (void (*)(void *data))unload_datafile        );
}
#else


/*****************************************************************************

  This is the same for Allegro 3.9.x

*****************************************************************************/


#if ALLEGRO_VERSION<4
 #include "allegro/aintern.h"
#else
 #include "allegro/internal/aintern.h"
 #define datafile_type _datafile_type
#endif

/* read_block:
 *  Reads a block of size bytes from a file, allocating memory to store it.
 */
static void *read_block(PACKFILE *f, int size, int alloc_size)
{
   void *p;

   p = malloc(MAX(size, alloc_size));
   if (!p) {
      *allegro_errno = ENOMEM;
      return NULL;
   }

   pack_fread(p, size, f);

   if (pack_ferror(f)) {
      free(p);
      return NULL;
   }

   return p;
}

/* load_data_object:
 *  Loads a binary data object from a datafile.
 */
void *load_data_object(PACKFILE *f, long size)
{
   return read_block(f, size, 0);
}

/* load_object:
 *  Helper to load an object from a datafile.
 */
static void *load_object(PACKFILE *f, int type, long size)
{
   int i;

   /* look for a load function */
   for (i=0; i<MAX_DATAFILE_TYPES; i++)
      if (datafile_type[i].type == type)
	 return datafile_type[i].load(f, size);

   /* if not found, load binary data */
   return load_data_object(f, size);
}

/* load_file_object:
 *  Loads a datafile object.
 */
void *load_file_object(PACKFILE *f, long size)
{
   #define MAX_PROPERTIES  64

   DATAFILE_PROPERTY prop[MAX_PROPERTIES];
   DATAFILE *dat;
   PACKFILE *ff;
   int prop_count, count, type, c, d;
   char *p;

   count = pack_mgetl(f);

   dat = malloc(sizeof(DATAFILE)*(count+1));
   if (!dat) {
      *allegro_errno = ENOMEM;
      return NULL;
   }

   for (c=0; c<=count; c++) {
      dat[c].type = DAT_END;
      dat[c].dat = NULL;
      dat[c].size = 0;
      dat[c].prop = NULL;
   }

   for (c=0; c<MAX_PROPERTIES; c++)
      prop[c].dat = NULL;

   c = 0;
   prop_count = 0;

   while (c < count) {
      type = pack_mgetl(f);

      if (type == DAT_PROPERTY) {
	 /* load an object property */
	 type = pack_mgetl(f);
	 d = pack_mgetl(f);
	 if (prop_count < MAX_PROPERTIES) {
	    prop[prop_count].type = type;
	    prop[prop_count].dat = malloc(d+1);
	    if (prop[prop_count].dat != NULL) {
	       pack_fread(prop[prop_count].dat, d, f);
	       prop[prop_count].dat[d] = 0;
	       if (need_uconvert(prop[prop_count].dat, U_UTF8, U_CURRENT)) {
		  p = malloc(uconvert_size(prop[prop_count].dat, U_UTF8, U_CURRENT));
		  if (p)
		     do_uconvert(prop[prop_count].dat, U_UTF8, p, U_CURRENT, -1);
		  free(prop[prop_count].dat);
		  prop[prop_count].dat = p;
	       }
	       prop_count++;
	       d = 0;
	    }
	 }
	 while (d-- > 0)
	    pack_getc(f);
      }
      else {
	 /* load actual data */
	 ff = pack_fopen_chunk(f, FALSE);

	 if (ff) {
	    d = ff->todo;

	    dat[c].dat = load_object(ff, type, d);

	    if (dat[c].dat) {
	       dat[c].type = type;
	       dat[c].size = d;

	       if (prop_count > 0) {
		  dat[c].prop = malloc(sizeof(DATAFILE_PROPERTY)*(prop_count+1));
		  if (dat[c].prop != NULL) {
		     for (d=0; d<prop_count; d++) {
			dat[c].prop[d].dat = prop[d].dat;
			dat[c].prop[d].type = prop[d].type;
			prop[d].dat = NULL;
		     }
		     dat[c].prop[d].dat = NULL;
		     dat[c].prop[d].type = DAT_END;
		  }
		  else {
		     for (d=0; d<prop_count; d++) {
			free(prop[d].dat);
			prop[d].dat = NULL;
		     }
		  }
		  prop_count = 0;
	       }
	       else
		  dat[c].prop = NULL;
	    }

	    f = pack_fclose_chunk(ff);

	    c++;
	 }
      }

      if (*allegro_errno) {
	 unload_datafile(dat);

	 for (c=0; c<MAX_PROPERTIES; c++)
	    if (prop[c].dat)
	       free(prop[c].dat);

	 return NULL;
      }
   }

   for (c=0; c<MAX_PROPERTIES; c++)
      if (prop[c].dat)
	 free(prop[c].dat);

   return dat;
}

/* load_datafile:
 *  Loads an entire data file into memory, and returns a pointer to it. 
 *  On error, sets errno and returns NULL.
 */
DATAFILE *load_datafile(AL_CONST char *filename)
{
   return load_datafile_callback(filename, NULL);
}

/* load_datafile_callback:
 *  Loads an entire data file into memory, and returns a pointer to it. 
 *  On error, sets errno and returns NULL.
 */
DATAFILE *load_datafile_callback(AL_CONST char *filename, void (*callback)(DATAFILE *))
{
   PACKFILE *f;
   DATAFILE *dat;
   int type;

   f = pack_fopen(filename, F_READ_PACKED);
   if (!f)
      return NULL;

   if ((f->flags & PACKFILE_FLAG_CHUNK) && (!(f->flags & PACKFILE_FLAG_EXEDAT)))
      type = (_packfile_type == DAT_FILE) ? DAT_MAGIC : 0;
   else
      type = pack_mgetl(f);

   if (type == DAT_MAGIC) {
      dat = load_file_object(f, 0);
   }
   else
      dat = NULL;

   pack_fclose(f);
   return dat; 
}

/* load_datafile_object:
 *  Loads a single object from a datafile.
 */
DATAFILE *load_datafile_object(AL_CONST char *filename, AL_CONST char *objectname)
{
   PACKFILE *f;
   DATAFILE *dat;
   void *object;
   char buf[512];
   int size;

   ustrzcpy(buf, sizeof(buf), filename);

   if (ustrcmp(buf, uconvert_ascii("#", NULL)) != 0)
      ustrzcat(buf, sizeof(buf), uconvert_ascii("#", NULL));

   ustrzcat(buf, sizeof(buf), objectname);

   f = pack_fopen(buf, F_READ_PACKED);
   if (!f)
      return NULL;

   size = f->todo;

   dat = malloc(sizeof(DATAFILE));
   if (!dat) {
      pack_fclose(f);
      return NULL;
   }

   object = load_object(f, _packfile_type, size);

   pack_fclose(f);

   if (!object) {
      free(dat);
      return NULL;
   }

   dat->dat = object;
   dat->type = _packfile_type;
   dat->size = size;
   dat->prop = NULL;

   return dat; 
}

/* _unload_datafile_object:
 *  Helper to destroy a datafile object.
 */
void _unload_datafile_object(DATAFILE *dat)
{
   int i;

   /* free the property list */
   if (dat->prop) {
      for (i=0; dat->prop[i].type != DAT_END; i++)
	 if (dat->prop[i].dat)
	    free(dat->prop[i].dat);

      free(dat->prop);
   }

   /* look for a destructor function */
   for (i=0; i<MAX_DATAFILE_TYPES; i++) {
      if (datafile_type[i].type == dat->type) {
	 if (dat->dat) {
	    if (datafile_type[i].destroy)
	       datafile_type[i].destroy(dat->dat);
	    else
	       free(dat->dat);
	 }
	 return;
      }
   }

   /* if not found, just free the data */
   if (dat->dat)
      free(dat->dat);
}

/* unload_datafile:
 *  Frees all the objects in a datafile.
 */
void unload_datafile(DATAFILE *dat)
{
   int i;

   if (dat) {
      for (i=0; dat[i].type != DAT_END; i++)
	 _unload_datafile_object(dat+i);

      free(dat);
   }
}

/* unload_datafile_object:
 *  Unloads a single datafile object, returned by load_datafile_object().
 */
void unload_datafile_object(DATAFILE *dat)
{
   if (dat) {
      _unload_datafile_object(dat);
      free(dat);
   }
}

/* _initialize_datafile_types:
 *  Register my loader functions with the code in dataregi.c.
 */
#ifdef CONSTRUCTOR_FUNCTION
   CONSTRUCTOR_FUNCTION(void _initialize_datafile_types());
#endif

void _initialize_datafile_types()
{
   register_datafile_object(DAT_FILE,         load_file_object,             (void (*)(void *data))unload_datafile        );
}

#endif

