/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/***************************************************************

 Filename -       infbase.h

 Functions
                  Member functions of following classes
                      TInfTopic
                      TInfIndex
                      TInfFile
                      TInfIndexCollection

***************************************************************/

#ifndef __InfBase_H
#define __InfBase_H

#define cInfColor      "\x37\x3F\x3A\x13\x13\x30\x3E\x1E\x4E\x2E"
#define cInfBlackWhite "\x07\x0F\x07\x70\x70\x07\x0F\x70\x0F\x70"
#define cInfMonochrome "\x07\x0F\x07\x70\x70\x07\x0F\x70\x0F\x70"
#define cInfViewer     "\x06\x07\x08\x09"
#ifdef FOR_EDITOR
#define cInfWindow     "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89"
#else
#define cInfWindow     "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49"
#endif

/*#ifndef __STDIO_H
#if defined(TVCompf_djgpp) && defined(NULL)
#undef NULL
#endif

#include <stdio.h>

#if defined(TVCompf_djgpp) && !defined(NULL)
#define NULL 0
#endif
#endif //__STDIO_H

#define Uses_TNoCaseSOSStringCollection
#include <settvuti.h>*/

// 90 because the Ralf Brown list can make that
#define MAX_NODE_NAME 90
#define BUF_SIZE (MAX_NODE_NAME*4+MAX_NODE_NAME/2)

// structure CrossRef

struct CrossRef
{
 char Name[MAX_NODE_NAME];
 char Name2[MAX_NODE_NAME];
 int offset;
 int line;
 int linebreak;
 uchar length;
};


// structure IndirectIndex

typedef struct
{
 long lPos;
 char Name[MAX_NODE_NAME];
} IndirectIndex;

const int bestMVisibleName=1;

// TInfTopic

class TInfFile;

class TInfTopic
{

public:

    TInfTopic( int mode );
    virtual ~TInfTopic();

    void getCrossRef( int i, TPoint& loc, uchar& length, char *& ref, int &pl );
    char *getCrossRef( int i ) { return crossRefs[i].Name; };
    int getLine( int line, char *buffer );
    int getNumCrossRefs() { return numRefs; };
    int numLines() { return iLines; };
    int maxWidth() { return maxLineWidth; };
    void setWidth( int aWidth ) { width = aWidth; };

    int selBestMatch(char *match, int &PerfectMatch, unsigned opts=0);

    int numRefs;
    CrossRef *crossRefs;

    Boolean Status;
    int modeFlags;

    void *Read(TInfFile& File, int offset, int &y);

    // Lo que contiene
    char *Text;
    int  iLines;
    int  maxLineWidth;
    long lSize;

    char Node[MAX_NODE_NAME];
    char Next[MAX_NODE_NAME];
    char Prev[MAX_NODE_NAME];
    char Up[MAX_NODE_NAME];

    int SearchNextWith(char *str,int len,int &selected,int &vislen);

    int ReadNodeInfo(TInfFile &File);

private:

    void ReadText(TInfFile &File, int offset, int &y);
    void ReadCrossRefs( void );
    int width;

    // Usadas por getLine
    int lastOffset;
    int lastLine;

};

// Valores para modeFlag
const int moinCutNodeWord  = 1,
          moinHideNodeLink = 2;

const unsigned tiicReference=1;

struct TIIC
{
 long pos;
 stkHandler node;
 char name[0];
};

// TInfIndexCollection
//   That's a No case sensitive, No owner and SOStack collection, the data in each
// cell is an TIIC structure.

class TInfIndexCollection : public TNoCaseSOSStringCollection
{
public:
 TInfIndexCollection(ccIndex aLimit, ccIndex aDelta, SOStack *stk) :
   TNoCaseSOSStringCollection(aLimit,aDelta,stk) {};
 stkHandler insert(long pos, stkHandler node, char *s, int len);
 virtual char *GetString( void *h );
};

// TInfIndex

typedef char NameOfNode[MAX_NODE_NAME];

class TInfIndex
{
public:

    TInfIndex( TInfFile& o, int Indirect );
    TInfIndex( TInfFile& o ); // Para archivos sin Tag Table
    ~TInfIndex();

    long position(int, char *&, int &);
    char *nameOf(int i);
    int WhatIndex(char *Nom);

    int size;
    IndirectIndex *IndOffsets;
    int indirects;
    Boolean Status;

    SOStack *stk;
    TInfIndexCollection *coll;

    int SearchFirstWith(int Key,int &selected);
};

#define InfV_UseBinaryFile
// TInfFile

class TInfFile
{

public:

    TInfFile( char *s, int Verbose=1 ) { DoAll(s,Verbose); };
    virtual ~TInfFile();
    void DoAll(char *s, int Verbose=0 );

    int seekToNode(const char *Name, int fromStart=0);
    TInfTopic *getTopic(char *, int Verbose, int modeForTopic, int &suggY);
    TInfTopic *invalidTopic();

    FILE *stream;
    Boolean Status;
    Boolean IsCompressed;
    Boolean DontRemoveCompressed;

    TInfIndex *index;
    long indexPos;
    int  iFile;

    char Buffer[BUF_SIZE];
    #ifndef InfV_UseBinaryFile
    void GetLine(void) { fgets(Buffer,SizeOfReadBuf,stream); };
    #else
    void GetLine(void);
    #endif
    void SkipNode(void);

    // Special I/O for multi-file
    int  fSeek(long Pos);
    FILE *fOpen(char *Nombre);
    int  fClose(FILE *f);
    long fTell(void);

    char NameOfFile[MAX_NODE_NAME];

    int ConvertIt(long Pos);
    static int SizeOfReadBuf;

    char NameOfTemporal[FILENAME_MAX];

    long fileLength;

private:
    int ExpandName(char *Buf, char *Nombre, int iExt);

};

extern char *InfViewGetInfoDir(void);
extern void InfViewAddInfoDir(char *dir);

#endif // __InfBase_H


