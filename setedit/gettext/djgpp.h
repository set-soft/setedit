/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */
/* Special definitions, processed by autoheader.
   Copyright (C) 1995-1998, 2001 Free Software Foundation.
   Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

/* Default value for alignment of strings in .mo file.  */
#define DEFAULT_OUTPUT_ALIGNMENT 1

#ifndef PARAMS
# if __STDC__
#  define PARAMS(args) args
# else
#  define PARAMS(args) ()
# endif
#endif


/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
/* #undef HAVE_ALLOCA_H */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have a working `mmap' system call.  */
/* #undef HAVE_MMAP */

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you have the __argz_count function.  */
/* #undef HAVE___ARGZ_COUNT */

/* Define if you have the __argz_next function.  */
/* #undef HAVE___ARGZ_NEXT */

/* Define if you have the __argz_stringify function.  */
/* #undef HAVE___ARGZ_STRINGIFY */

/* Define if you have the dcgettext function.  */
/* #undef HAVE_DCGETTEXT */

/* Define if you have the feof_unlocked function.  */
/* #undef HAVE_FEOF_UNLOCKED */

/* Define if you have the fgets_unlocked function.  */
/* #undef HAVE_FGETS_UNLOCKED */

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the getdelim function.  */
/* #undef HAVE_GETDELIM */

/* Define if you have the getegid function.  */
#define HAVE_GETEGID 1

/* Define if you have the geteuid function.  */
#define HAVE_GETEUID 1

/* Define if you have the getgid function.  */
#define HAVE_GETGID 1

/* Define if you have the getpagesize function.  */
#define HAVE_GETPAGESIZE 1

/* Define if you have the getuid function.  */
#define HAVE_GETUID 1

/* Define if you have the isascii function.  */
#define HAVE_ISASCII 1

/* Define if you have the iswcntrl function.  */
/* #undef HAVE_ISWCNTRL */

/* Define if you have the iswprint function.  */
/* #undef HAVE_ISWPRINT */

/* Define if you have the mblen function.  */
#define HAVE_MBLEN 1

/* Define if you have the mbsinit function.  */
/* #undef HAVE_MBSINIT */

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the mempcpy function.  */
/* #undef HAVE_MEMPCPY */

/* Define if you have the memset function.  */
#define HAVE_MEMSET 1

/* Define if you have the munmap function.  */
/* #undef HAVE_MUNMAP */

/* Define if you have the putenv function.  */
#define HAVE_PUTENV 1

/* Define if you have the setenv function.  */
#define HAVE_SETENV 1

/* Define if you have the setlocale function.  */
#define HAVE_SETLOCALE 1

/* Define if you have the stpcpy function.  */
#define HAVE_STPCPY 1

/* Define if you have the stpncpy function.  */
/* #undef HAVE_STPNCPY */

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strcspn function.  */
#define HAVE_STRCSPN 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strncasecmp function.  */
#define HAVE_STRNCASECMP 1

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the strtoul function.  */
#define HAVE_STRTOUL 1

/* Define if you have the tsearch function.  */
/* #undef HAVE_TSEARCH */

/* Define if you have the uname function.  */
#define HAVE_UNAME 1

/* Define if you have the vasprintf function.  */
/* #undef HAVE_VASPRINTF */

/* Define if you have the wcwidth function.  */
/* #undef HAVE_WCWIDTH */

/* Define if you have the <argz.h> header file.  */
/* #undef HAVE_ARGZ_H */

/* Define if you have the <dlfcn.h> header file.  */
/* #undef HAVE_DLFCN_H */

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
#define HAVE_LOCALE_H 1

/* Define if you have the <malloc.h> header file.  */
#define HAVE_MALLOC_H 1

/* Define if you have the <nl_types.h> header file.  */
/* #undef HAVE_NL_TYPES_H */

/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <values.h> header file.  */
#define HAVE_VALUES_H 1

/* Define if you have the <wchar.h> header file.  */
#define HAVE_WCHAR_H 1

/* Define if you have the <wctype.h> header file.  */
#define HAVE_WCTYPE_H 1

/* Name of package */
#define PACKAGE "gettext"

/* Version number of package */
#define VERSION "0.10.39"

/* Make sure we see all GNU extensions. */
#define _GNU_SOURCE 1

/* Define to empty if the C compiler doesn't support this keyword. */
/* #undef signed */

/* Define if backslash-a works in C strings. */
#define HAVE_C_BACKSLASH_A 1

/* Define if you have the unsigned long long type. */
#define HAVE_UNSIGNED_LONG_LONG 1

/* Define if system has ptrdiff_t type */
#define HAVE_PTRDIFF_T 1

/* Define if <inttypes.h> exists, doesn't clash with <sys/types.h>,
   and declares uintmax_t.  */
/* #undef HAVE_INTTYPES_H */

/* Define to unsigned long or unsigned long long
   if <inttypes.h> doesn't define. */
#define uintmax_t unsigned long long

/* Define to 1 if mbrtowc and mbstate_t are properly declared. */
/* #undef HAVE_MBRTOWC */

/* Define to 1 if you have the declaration of wcwidth(), and to 0 otherwise. */
#define HAVE_DECL_WCWIDTH 0

/* Define to a type if <wchar.h> does not define. */
/* #undef mbstate_t */

/* Define if you have the parse_printf_format() function. */
/* #undef HAVE_PARSE_PRINTF_FORMAT */

/* Define as const if the declaration of setlocale() needs const. */
#define SETLOCALE_CONST const

/* Define if you have the iconv() function. */
/* #define HAVE_ICONV 1 */

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST const

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
/* #undef HAVE_LANGINFO_CODESET */

/* Define if your <locale.h> file defines LC_MESSAGES. */
/* #undef HAVE_LC_MESSAGES */

/* Define to 1 if translation of program messages to the user's native language
   is requested. */
#define ENABLE_NLS 1

/* Define if the GNU gettext() function is already present or preinstalled. */
/* #undef HAVE_GETTEXT */


/* We don't test for the basename function but still want to use the
   version in the libc when compiling for a system using glibc.  */
#ifdef __GNU_LIBRARY__
# define HAVE_BASENAME	1
#endif


/* A file name cannot consist of any character possible.  INVALID_PATH_CHAR
   contains the characters not allowed.  */
#ifndef MSDOS
# define	INVALID_PATH_CHAR "\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37 \177/"
#else
/* Something like this for MSDOG.  */
# define	INVALID_PATH_CHAR "\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37 \177\\:."
#endif

/* This is the page width for the message_print function.  It should
   not be set to more than 79 characters (Emacs users will appreciate
   it).  It is used to wrap the msgid and msgstr strings, and also to
   wrap the file position (#:) comments.  */
#define PAGE_WIDTH 79
