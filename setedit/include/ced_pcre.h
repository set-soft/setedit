/* Copyright (C) 2004-2015 by Salvador E. Tropea (SET),
   see copyrigh file for details */

// PCRE support
#ifndef SUP_PCRE
 #if defined(HAVE_PCRE_LIB)
  #define SUP_PCRE 1
 #else
  #define SUP_PCRE 0
 #endif
#endif

#if !SUP_PCRE
 // Just to avoid errors
 typedef void pcre;
 extern void *(*pcre_malloc)(size_t a);
 extern void  (*pcre_free)(void *p);
#else
 #define STATIC
 #include PCRE_HEADER_NAME
 #undef STATIC
#endif

#if PCRE_MAJOR>=8
 #define PCRE_MATCHES(count,compiled,ret_fail)  \
         if (pcre_fullinfo(compiled,NULL,PCRE_INFO_CAPTURECOUNT,&count)) \
            return ret_fail; \
         else \
            count=(count+1)*3
#else
 #define PCRE_MATCHES(count,compiled,ret_fail)  count=(pcre_info(compiled,0,0)+1)*3
#endif

