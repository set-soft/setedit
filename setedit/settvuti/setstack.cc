/**[txh]*********************************************************************

  Class: SOStack
  Comments:
  Simple Object Stack (SOS), copyright (c) 1997 by Salvador E. Tropea (SET)
@p
  If you want to use this code contact me first.
@p
@<subtitle>{Purpose:}

  The SOS was designed to reduce the number of memory allocations needed
for string lists operations and to reduce the wasted space.
@p
  The overhead of each allocation is sizeof(size_t) plus the bytes needed
to align the object to a Dowble Word boundary, so all the obejcts are
32 bits aligned if size_t is 32 bits or 16 bits if size_t is 16 bits.
@p
  As the routines were designed with the DJGPP malloc scheme in mind the
program takes memory in cuasi-2-power chunks, that's for example 976 bytes,
1952, 3904, etc.
@p
  The class have only one chunk of memory that is managed as an stack, so
the number of elements in the global heap is reduced. As this chunk of
memory can be resized when all is used and that's involves a call to
realloc the class can return pointers to the objects, instead returns
handlers type stkHandler. To convert this handler to a pointer 2 members
are provided: GetPointerOf and GetStrOf, basically are just the same
but the second avoids a cast in string operations. The both are inline
and are just an addittion so the overhead is minimal.
@p
@<subtitle>{Notes of application:}

  I used it for lists of strings in dialog boxes, so I was forced to
overwrite some classes:
@p
TNoCaseSOSStringCollection from TStringCollection:@*
  Is a collection, no case sensitive that uses an SOStack for the strings,
the class ISN'T the owner of the SOStack, so you must put the strings in
the SOStack and insert the stkHandler in the collection. When the
collection is destroyed the SOStack ISN'T destroyed.
@p
TSOSSortedListBox from public TSortedListBox:@*
  Is a sorted listbox that support a TNoCaseSOSStringCollection as
collection, that's needed because the original can't handle the stkHandler.
The both are in tnocastc.cc and tnocastc.h
@p

@<pre>
Contact me at:
E-Mail: salvador@@inti.edu.ar
Telephone: (+5411) 4759-0013
Postal Address:
Salvador E. Tropea
CurapaligÅe 2124
(1678) Caseros - 3 de Febrero
Prov: Buenos Aires
Argentina
@</pre>

****************************************************************************/

#define Uses_string
#define Uses_SOStack
#include <settvuti.h>

/**[txh]**********************************************************************

 Include: settvuti.h
 Module:  SET TV Utils
 Description:
 That's the constructor for the SOStack, it just initialize the vars and
doesn't allocate any memory.

*****************************************************************************/

SOStack::SOStack()
{
 Buffer=NULL;
 MemPool=0;
 Size=0;
 Cant=0;
}

/**[txh]**********************************************************************

 Description:
 It cleans the SOStack without liberating the memory. The new objects added
will overwrite the old ones so the old references loose sense.

*****************************************************************************/

void SOStack::Clean(void)
{
 Size=0;
 Cant=0;
}

/**[txh]**********************************************************************

  Description:
  Is the destructor. It frees the allocated memory.

*****************************************************************************/

SOStack::~SOStack()
{
 free(Buffer); // Allocated with realloc
 Buffer=NULL;
}

/**[txh]**********************************************************************

  Description:
  That's a private member. It makes enough space in the stack to hold
"bytes" bytes. It calls to realloc so the old references can be
invalidated.

  Return:
    A size_t that's the new top of the stack.

*****************************************************************************/

size_t SOStack::MakeRoomFor(size_t bytes)
{
 size_t rest=4-(bytes & 3);
 size_t requested;

 if (rest==4)
    rest=0;
 bytes+=rest+sizeof(size_t);
 requested=Size+bytes;
 if (requested>MemPool)
   {
    if (!MemPool)
       MemPool=stkInitialMemPool;
    while (MemPool<requested)
       MemPool*=2;
    Buffer=(char *)realloc(Buffer,MemPool);
   }
 return requested;
}

/**[txh]**********************************************************************

  Description:
  It's a private member. It copies the memory pointed by "p" and with size
"size" to the offset "req" in the buffer of the stack. It updates the
LastChunk, Cant and Size members.  The space must be allocated by MakeRoomFor
first.

  Return:
  stkNULL if the Buffer is NULL.
  The stkHandler of the added item if all Ok.

*****************************************************************************/

stkHandler SOStack::AddItem(void *p, size_t size, size_t req)
{
 if (Buffer)
   {
    stkHandler ret=Size;
    memcpy(&Buffer[ret],p,size);
    LastChunk=*((size_t *)(&Buffer[req-sizeof(size_t)]))=req-Size;
    Size=req;
    Cant++;
    return ret;
   }
 return stkNULL;
}

/**[txh]**********************************************************************

  Description:
  It's a private member. It just updates the LastChunk, Cant and Size members
to keep space for a new element in the stack. The space must be allocated by
MakeRoomFor first.

  Return:
  stkNULL if the Buffer is NULL.
  The stkHandler of the added item if all Ok.

*****************************************************************************/

stkHandler SOStack::AddEmptyItem(size_t req)
{
 if (Buffer)
   {
    stkHandler ret=Size;
    LastChunk=*((size_t *)(&Buffer[req-sizeof(size_t)]))=req-Size;
    Size=req;
    Cant++;
    return ret;
   }
 return stkNULL;
}

/**[txh]**********************************************************************

  Description:
  It adds an string to the stack.

  Return: stkHandler to access later to it.

*****************************************************************************/

stkHandler SOStack::addStr(char *s)
{
 size_t l;

 for (l=0; s[l]; l++);
 return add(s,l+1);
}

/**[txh]**********************************************************************

  Description:
  This member is used to deallocate the last allocated element. If "shrink"
is !=0 the class will deallocate the memory too, if not it will let the
space for another element.

*****************************************************************************/

void SOStack::DestroyTop(int shrink)
{
 if (Buffer && Size)
   {
    Size-=LastChunk;
    LastChunk=*((size_t *)(&Buffer[Size-sizeof(size_t)]));
    Cant--;
    if (shrink && Size*2<MemPool)
      {
       size_t newSize=MemPool;
       size_t testSize=MemPool;

       while (testSize>Size)
         {
          newSize=testSize;
          testSize>>=1;
         }
       Buffer=(char *)realloc(Buffer,newSize);
       MemPool=newSize;
      }
   }
}

/**[txh]**********************************************************************

  Description:
  Is used to get the pointer of the last element added.

  Return: A void ponter to it or NULL if fails.

*****************************************************************************/

void *SOStack::GetTop()
{
 if (Buffer && Size)
    return (void *)(&Buffer[Size-LastChunk]);
 return NULL;
}

/**[txh]**********************************************************************

  Description:
  Is used to find the stkHandler of the last added element.

  Return: stkHandler of it or stkNULL if fails

*****************************************************************************/

stkHandler SOStack::GetTopHandle()
{
 if (Buffer && Size)
    return Size-LastChunk;
 return stkNULL;
}

/**[txh]**********************************************************************

  Description:
  Is used to go though the stack from the last added element to the first one.

  Return: The handler of the previous element of pos or stkNULL.

*****************************************************************************/

stkHandler SOStack::GetPreviousOf(stkHandler pos)
{
 if (Buffer && Size && pos)
    return pos-*((stkHandler *)(&Buffer[pos-sizeof(size_t)]));
 return stkNULL;
}

/**[txh]**********************************************************************

  Description:
  Is used to get the pointer to the element number "index". The first element
is the 0.

  Return: a void pointer to it or NULL.

*****************************************************************************/

void *SOStack::GetItemNumber(unsigned index)
{
 if (Buffer && Size && index<Cant)
   {
    unsigned i=Cant;
    size_t *Len=(size_t *)(Buffer+Size-sizeof(size_t));

    do
      {
       Len=(size_t *)(((char *)Len)-*Len);
       i--;
      }
    while (index!=i);
    return (void *)(((char *)Len)+sizeof(size_t));
   }
 return NULL;
}

// Here is the description for the inline members:
/**[txh]**********************************************************************

 Function: add
 Prototype: stkHandler add(void *p, size_t size)
 Description:
 Adds the data pointed by "p" with size "size" to the stack. Uses @x{::AddItem}.
Is an inline member.

 Return: The stkHandler of the data added or stkNULL if fails.

**********/
/**[txh]***

 Function: alloc
 Prototype: stkHandler alloc(size_t size)
 Description:
 Reserves "size" bytes in the stack to hold data. Uses @x{::AddEmptyItem}. Is an
inline member.

 Return: The stkHandler of the data added or stkNULL if fails.

**********/
/**[txh]***

 Function: GetStrNumber
 Prototype: char *GetStrNumber(unsigned index)
 Description:
  Is used to get the pointer to the string number "index". The first element
is the 0. Is an inline cast of @x{SOStack::GetItemNumber}.

 Return: a char pointer to it or NULL.

**********/
/**[txh]***

 Function: GetPointerOf
 Prototype: void *GetPointerOf(stkHandler h)
 Description:
 Is used to convert a handler into a void pointer. Is an inline addition.

 Return: The void pointer to this element. No checks are made!

**********/
/**[txh]***

 Function: GetStrOf
 Prototype: char *GetStrOf(stkHandler h)
 Description:
 Is used to convert a handler into a char pointer. Is an inline addition.

 Return: The char pointer to this element. No checks are made!

*****************************************************************************/






