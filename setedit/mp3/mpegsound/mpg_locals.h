/* MPEG/WAVE Sound library

   (C) 1997 by Jung woo-jae */

// Mpegsound_locals.h
// It is used for compiling library

#ifndef _L__SOUND_LOCALS__
#define _L__SOUND_LOCALS__

// Inline functions
inline int Mpegtoraw::getbyte(void)
{
  int r=(unsigned char)buffer[bitindex>>3];

  bitindex+=8;
  return r;
};

inline int Mpegtoraw::getbits9(int bits)
{
  unsigned a;
#ifndef WORDS_BIGENDIAN
  {
    int offset=bitindex>>3;

    a=(((unsigned char)buffer[offset])<<8) | ((unsigned char)buffer[offset+1]);
  }
#else
  {
    int offset=bitindex>>3;

    a=(((unsigned char)buffer[offset+1])<<8) | ((unsigned char)buffer[offset]);
    //a=*((unsigned short *)(buffer+offset));
  }
#endif

  a<<=(bitindex&7);
  bitindex+=bits;
  return (int)((a & 0xFFFF)>>(16-bits));
};

inline int Mpegtoraw::getbits8(void)
{
  unsigned a;

#ifndef WORDS_BIGENDIAN
  {
    int offset=bitindex>>3;

    a=(((unsigned char)buffer[offset])<<8) | ((unsigned char)buffer[offset+1]);
  }
#else
  {
    int offset=bitindex>>3;

    a=(((unsigned char)buffer[offset+1])<<8) | ((unsigned char)buffer[offset]);
    //a=*((unsigned short *)(buffer+offset));
  }
#endif

  a<<=(bitindex&7);
  bitindex+=8;
  return (int)((a & 0xFFFF)>>8);
};

inline int Mpegtoraw::getbit(void)
{
  register int r=(buffer[bitindex>>3]>>(7-(bitindex&7)))&1;

  bitindex++;
  return r;
};

#endif
