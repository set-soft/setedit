/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef CEDITINT_H_SET
#define CEDITINT_H_SET

// Editor configuration (can modify the next test), must be first.
#include <configed.h>

// International support from TV
#ifdef FORCE_INTL_SUPPORT
 #define HAVE_INTL_SUPPORT 1
#else
 #include <tv/configtv.h>
#endif

#ifndef HAVE_INTL_SUPPORT
 #define NO_INTL_SUP
#endif

// Configuration for the Editor:
// Note: Starting with 0.4.23 I'm changing the mechanism.
//   There are two main configurations, one is for the standalone editor and
// is the default. The other is used for RHIDE to create libset.a to enable
// it you must define FOR_LIBSET *externally*. So the libset.gpr/mak have
// this define and only includes other projects not affected by ceditint.h.
//   Projects like easydiag, settvuti and extra shouldn't include ceditint.h
// or at least don't rely in features that are related to the FOR_LIBSET
// definition.

// ** Shared options
// Gzip support is inside the editor using libz. The support for external
// gzip is broken and I don't think is a good idea to fix it.
#define SUP_GZ

#ifndef FOR_LIBSET
 #define STANDALONE
 #define SUP_SDG
#endif

// Starting with 0.4.36 I have a small configure script to determine the
// following:
// MP3 support only for standalone
#if defined(WITH_MP3) && defined(STANDALONE)
 #define SUP_MP3
#endif

// PCRE support
#ifndef SUP_PCRE
 #if defined(HAVE_PCRE_LIB)
  #define SUP_PCRE 1
 #else
  #define SUP_PCRE 0
 #endif
#endif

#ifdef HAVE_PCRE206
 #define PCRE206 0,
#else
 #define PCRE206
#endif

// Tabs setings

#ifdef Tabs8
 #define AdvanceWithTab(a,b)     if (a==9) b|=7; b++
 #define NextTabPos(x)           (((x/8)+1)*8)
 #define MoveWithTab(b)          b|=7; b++
 #define IsATabPos(a)            (a%8)==0
#else
 #define AdvanceWithTab(a,b)     if (a==9) b+=tabSize-(b%tabSize); else b++
 #define NextTabPos(x)           (((x/tabSize)+1)*tabSize)
 #define MoveWithTab(b)          b+=tabSize-(b%tabSize)
 #define IsATabPos(a)            (a%tabSize)==0
#endif
#define NextIndentPos(x)        (((x/indentSize)+1)*indentSize)

// Non standard libc stuff

#if defined(SECompf_djgpp) || defined(SEOS_Win32)
 #define IDLE_SLEEP 0
#else
 #define IDLE_SLEEP 1000
#endif

//
// Debug printf. Defining DEBUG the program prints to stderr.
//
#ifdef SEComp_GCC
 #ifndef DEBUG
  #define dbprintf(a...)
 #else // DEBUG
  #ifndef TO_STDOUT
   #define dbprintf(a...) fprintf(stderr,a)
  #else
   #define dbprintf(a...) printf(a)
  #endif // else TO_STDOUT
 #endif // else DEBUG
#else // GCC
 inline void dbprintf(...) {};
#endif // else GCC

#ifdef TVComp_BCPP
 #define popen(command, mode) _popen(command, mode)
 #define pclose(stream) _pclose(stream)
#endif

#ifdef Uses_BestWrite
 #if defined(SECompf_djgpp)
  #define BestWrite(a,b) _write(STDERR_FILENO,a,b)
  #define Uses_io
 #elif defined(SEOS_UNIX)
  #define BestWrite(a,b) write(STDERR_FILENO,a,b)
  #define Uses_stdio
 #elif defined(SEOS_Win32)
  #define BestWrite(a,b) write(2,a,b)
  #define Uses_io
 #endif
#endif

#ifdef SEOS_Win32
 // 0 padded
 #define CLK24_FORMAT   "%H:%M  "
 #define CLKAMPM_FORMAT "%I:%M%p  "
#else
 // DJGPP and Linux have a space padded option
 #define CLK24_FORMAT   "%k:%M  "
 #define CLKAMPM_FORMAT "%l:%M%p  "
#endif

#if defined(SEOS_DOS) || (defined(SEOS_Win32) && !defined(SECompf_Cygwin))
 // This definition controls how the editor behaves when dealing with
 // file locations. In DOS/Win32 you don't log as an user with a home
 // directory.
 #define NoHomeOrientedOS
#endif

#endif // CEDITINT_H_SET
