/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

 Command Line Errors

 This module handles the parsing of errors reported by external programs.

*****************************************************************************/

#ifndef SUP_PCRE
// Just to avoid errors
typedef void pcre;
#else
#define STATIC
#include <pcre.h>
#undef STATIC
#endif

struct strCLE
{
 // Name of the program
 char *Name;
 // Pattern to match
 pcre *Pattern;
 // Subexpressions:
 uchar File;
 uchar Line;
 uchar Severity;
 uchar Description;
 // Pattern to indicate the makefile changed the path
 pcre *EnterDirPat;
 uchar EnterDirDir;
 // Flag to indicate this structure was loaded
 char Loaded;
 // This falg indicates we must use the internal parser. That's for the GNU parser
 char UseInternal;
 // Pattern to indicate the makefile went back one dir level
 pcre *LeaveDir;
};

extern strCLE *CLEValues;
const int maxCLEFileWidth=256;

class TStringCollection;

extern void CLESetFileName(const char *name);
extern void UnloadCLEFile();
extern TStringCollection *CLEGetList();
extern int CLEGetIndexOf(const char *name);
extern int CLEGetIndexOfLoad(const char *name);
extern int CLEDoSearch(char *search, int len, pcre *CompiledPCRE);
extern void CLEGetMatch(int match, char *buf, int maxLen);
extern void CLEGetMatch(int match, int &offset, int &len);
