/* Copyright (C) 2001-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class fpstream;

void PathListEdit(int which, unsigned hCtx);
void PathListSave(fpstream& s);
void PathListLoad(fpstream& s);
int  PathListGetItem(ccIndex pos, char *buffer, int which);
void PathListUnLoad();
void PathListAdd(int which, const char *path);
void PathListAddPathFor(void *item, int which);

const int paliInclude=0, paliSource=1, paliLists=2;
