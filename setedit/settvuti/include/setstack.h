/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

  (c) 1997 by SET
  Header for setstack.cc, read the other file for information.

*****************************************************************************/

#if defined(Uses_SOStack)

#ifndef __SET_SOStack_H__
#define __SET_SOStack_H__

const int stkInitialMemPool=976;

typedef size_t stkHandler;
const   size_t stkNULL=0xFFFFFFFF;

class SOStack
{
 public:

 SOStack();
 ~SOStack();

 void Clean(void);
 stkHandler add(void *p, size_t size) { return AddItem(p,size,MakeRoomFor(size)); };
 stkHandler alloc(size_t size) { return AddEmptyItem(MakeRoomFor(size)); };
 stkHandler addStr(char *s);
 void *GetTop();
 stkHandler GetTopHandle();
 stkHandler GetPreviousOf(stkHandler pos);
 void *GetItemNumber(unsigned index);
 char *GetStrNumber(unsigned index) { return (char *)GetItemNumber(index); };
 void DestroyTop(int shrink=0);
 void *GetPointerOf(stkHandler h) { return (void *)(Buffer+h); };
 char *GetStrOf(stkHandler h) { return (char *)(Buffer+h); };

 char *Buffer;

 private:
 size_t MemPool;
 size_t Size;
 size_t LastChunk;
 unsigned Cant;

 size_t MakeRoomFor(size_t bytes);
 stkHandler AddItem(void *p, size_t size, size_t req);
 stkHandler AddEmptyItem(size_t req);
};

#endif // __SET_SOStack_H__

#endif // Uses_SOStack

