/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_stdio
#define Uses_TEvent
#define Uses_TKeys
#define Uses_TSOSSortedListBox
#define Uses_TNoCaseStringCollection
#define Uses_TStringCollectionW
#define Uses_TNoCaseSOSStringCollection
#define Uses_TSOSStringCollection
#define Uses_TNCSAssociative
#define Uses_TNSSOSCol
#define Uses_TSOSCol
#define Uses_TSOSListBox
#define Uses_ctype
#define Uses_TVCodePage
#include <settvuti.h>

#define shiftKeys (char)(event.keyDown.shiftState & 0xFF)

void *TStringCollectionW::readItem( ipstream& is )
{
 return (void *)is.readString();
}

void TStringCollectionW::writeItem( void *obj, opstream& os )
{
 os.writeString((char *)obj);
}

void *TNoCaseStringCollection::readItem( ipstream& is )
{
 return (void *)is.readString();
}

void TNoCaseStringCollection::writeItem( void *obj, opstream& os )
{
 os.writeString((char *)obj);
}


void TSOSSortedListBox::getText(char *dest, ccIndex item, short maxChars)
{
 TNoCaseSOSStringCollection *i=(TNoCaseSOSStringCollection *)list();

 if ( i != 0 )
   {
    strncpy( dest, (const char *)(i->atStr(item)), maxChars );
    dest[maxChars] = '\0';
   }
 else
   *dest = EOS;
}

void TSOSListBox::getText(char *dest, ccIndex item, short maxChars)
{
 TSOSCol *i=(TSOSCol*)list();

 if ( i != 0 )
   {
    strncpy( dest, (const char *)(i->atStr(item)), maxChars );
    dest[maxChars] = '\0';
   }
 else
   *dest = EOS;
}

int TNoCaseSOSStringCollection::compare(void *s1,void *s2)
{
 return strcasecmp(GetString(s1),GetString(s2));
}

int TSOSStringCollection::compare(void *s1,void *s2)
{
 return strcmp(GetString(s1),GetString(s2));
}

char *TNoCaseSOSStringCollection::GetString( void *h )
{
 return stkL->GetStrOf((stkHandler)h);
}

Boolean TNoCaseSOSStringCollection::Search( char *key, ccIndex& index )
{
 ccIndex l = 0;
 ccIndex h = count - 1;
 Boolean res = False;
 while( l <= h )
     {
     ccIndex i = (l +  h) >> 1;
     int c = strcasecmp( GetString(items[i]), key );
     if( c < 0 )
         l = i + 1;
     else
         {
         h = i - 1;
         if( c == 0 )
             {
             res = True;
             if( !duplicates )
                 l = i;
             }
         }
     }
 index = l;
 return res;
}

#if 1
Boolean TSOSStringCollection::SearchCase( char *key, ccIndex& index )
{
 ccIndex l = 0;
 ccIndex h = count - 1;
 Boolean res = False;
 while( l <= h )
     {
     ccIndex i = (l +  h) >> 1;
     int c = strcmp( GetString(items[i]), key );
     if( c < 0 )
         l = i + 1;
     else
         {
         h = i - 1;
         if( c == 0 )
             {
             res = True;
             if( !duplicates )
                 l = i;
             }
         }
     }
 index = l;
 return res;
}
#endif

int StrCmp(char *s1,char *s2)
{
 int ret=strcmp(s1,s2),ret2;
 if (ret==0)
    return ret;
 ret2=strcasecmp(s1,s2);
 if (ret2!=0)
    return ret2;
 return -512;
}

Boolean TNoCaseSOSStringCollection::SearchCase( char *key, ccIndex& index )
{
 ccIndex l = 0;
 ccIndex h = count - 1;
 Boolean res = False;
 while( l <= h )
     {
     ccIndex i = (l +  h) >> 1;
     int c = StrCmp( GetString(items[i]), key );
     if (c==-512)
       {
        while (i && strcasecmp(GetString(items[i-1]),key)==0)
          {
           i--;
           if (strcmp(GetString(items[i]),key)==0)
             {
              index=i;
              return True;
             }
          }
        i++;
        while (i<count && strcasecmp(GetString(items[i]),key)==0)
          {
           if (strcmp(GetString(items[i]),key)==0)
             {
              index=i;
              return True;
             }
           i++;
          }
        return False;
       }
     if( c < 0 )
         l = i + 1;
     else
         {
         h = i - 1;
         if( c == 0 )
             {
             res = True;
             if( !duplicates )
                 l = i;
             }
         }
     }
 index = l;
 return res;
}


void TSOSSortedListBox::handleEvent(TEvent& event)
{
 char curString[256], newString[256];
 int value, oldPos, oldValue;

 oldValue = focused;
 TListBox::handleEvent( event );
 if( oldValue != focused )
     SearchPos = USHRT_MAX;
 if( event.what == evKeyDown )
     {
     if( event.keyDown.keyCode != kbEnter &&
         ( event.keyDown.charScan.charCode != 0 ||
           event.keyDown.keyCode == kbBack ) )
         {
         value = focused;
         if( value < range )
             getText( curString, value, 255 );
         else
             *curString = EOS;
         oldPos = SearchPos;
         if( event.keyDown.keyCode == kbBack )
             {
             if( SearchPos == USHRT_MAX )
                 return;
             curString[SearchPos--] = EOS;
             if( SearchPos == USHRT_MAX )
                 shiftState = shiftKeys;
             }
         else if( (event.keyDown.charScan.charCode == '.') )
             {
             char *loc = strchr( curString+
                                 (SearchPos==USHRT_MAX ? 0 : SearchPos), '.' );
             if( loc )
               {
                SearchPos = ushort(loc - curString);
                if (oldPos == USHRT_MAX)
                   oldPos = 0;
               }
             else
               {
                if (SearchPos == USHRT_MAX)
                  {
                   SearchPos++;
                   curString[SearchPos] = '.';
                   curString[SearchPos+1] = EOS;
                   oldPos = 0;
                  }
               }
             }
         else
             {
             SearchPos++;
             if( SearchPos == 0 )
                 {
                 ShiftState = shiftKeys;
                 oldPos=0;
                 }
             curString[SearchPos] = event.keyDown.charScan.charCode;
             curString[SearchPos+1] = EOS;
             }
         ((TNoCaseSOSStringCollection *)list())->Search( curString, value );
         if( value < range )
             {
             getText( newString, value, 255 );
             if( strncasecmp( curString, newString, SearchPos+1 )==0 )
                 {
                 if( value != oldValue )
                     {
                     focusItem(value);
                     setCursor( cursor.x+SearchPos, cursor.y );
                     }
                 else
                     setCursor(cursor.x+(SearchPos-oldPos), cursor.y );
                 }
             else
                 SearchPos = oldPos;
             }
         else
             SearchPos = oldPos;
         if( SearchPos != oldPos ||
             TVCodePage::isAlpha( event.keyDown.charScan.charCode )
           )
             clearEvent(event);
         }
     }
}

void TNCSAssociative::freeItem(void *s)
{
 // Remove the collection asociated with this item
 stNCSAssociative *st=(stNCSAssociative *)stkL->GetPointerOf((stkHandler)s);
 if (st->c)
    delete st->c;
}

void TNCSAssociative::insert(char *s, stkHandler hv)
{
 ccIndex pos;
 stNCSAssociative *st;
 stkHandler h,hs;

 if (Search(s,pos))
   { // There are already a collection for it
    // Get the collection
    h=(stkHandler)at(pos);
    st=(stNCSAssociative *)stkL->GetPointerOf(h);
    // Insert in the collection
    st->c->insert(hv);
   }
 else
   { // No yet
    // Put the name in the stack
    hs=stkL->addStr(s);
    // Get space for the structure
    h =stkL->alloc(sizeof(stNCSAssociative));
    // Fill the structure
    st=(stNCSAssociative *)stkL->GetPointerOf(h);
    st->h=hs;
    st->c=new TNoCaseSOSStringCollection(10,5,stkL);
    st->c->insert(hv);
    st->Cont=stkNULL;
    TNoCaseSOSStringCollection::insert(h);
   }
}

void TNCSAssociative::SetContent(char *s, char *cont)
{
 ccIndex pos;
 stNCSAssociative *st;
 stkHandler h,hCont,hs;

 if (Search(s,pos))
   { // Ok it exists
    // First add the string to the stack or we can invalidate the pointers
    // during the process
    hCont=stkL->addStr(cont);
    // Get the struct
    h=(stkHandler)at(pos);
    st=(stNCSAssociative *)stkL->GetPointerOf(h);
    st->Cont=hCont;
   }
 else
   { // No yet
    // Put the name in the stack
    hs=stkL->addStr(s);
    // Put the content
    hCont=stkL->addStr(cont);
    // Get space for the structure
    h =stkL->alloc(sizeof(stNCSAssociative));
    // Fill the structure
    st=(stNCSAssociative *)stkL->GetPointerOf(h);
    st->h=hs;
    st->Cont=hCont;
    st->c=new TNoCaseSOSStringCollection(10,5,stkL);
    TNoCaseSOSStringCollection::insert(h);
   }
}

char *TNCSAssociative::GetString( void *h )
{
 stNCSAssociative *st;
 st=(stNCSAssociative *)stkL->GetPointerOf((stkHandler)h);
 return stkL->GetStrOf(st->h);
}

TNoCaseSOSStringCollection *TNCSAssociative::atCol(ccIndex index)
{
 stNCSAssociative *st;
 st=(stNCSAssociative *)stkL->GetPointerOf((stkHandler)at(index));
 return st->c;
}

char *TNCSAssociative::GetContent(ccIndex index)
{
 stNCSAssociative *st;
 st=(stNCSAssociative *)stkL->GetPointerOf((stkHandler)at(index));
 if (st->Cont==stkNULL)
    return NULL;
 return stkL->GetStrOf(st->Cont);
}


char *TNSSOSCol::GetString( void *h )
{
 return stkL->GetStrOf((stkHandler)h);
}

char *TSOSCol::GetString( void *h )
{
 return stkL->GetStrOf((stkHandler)h);
}

/*
void TSOSListBox::getText( char *dest, ccIndex item, short maxChars )
{
 if (items != 0 )
   {
    strncpy( dest, (const char *)(items->atStr(item)), maxChars );
    dest[maxChars] = '\0';
   }
 else
   *dest = EOS;
}

*/
