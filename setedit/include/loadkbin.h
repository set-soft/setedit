/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  This header is shared by keytrans.cc and edkeys.cc.
  The name comes from the old loadkbin.cc no longer used.

***************************************************************************/

int  KeyBindEdit(void);
int  SaveKeyBind(char *name);
void ShowKeyBindError(void);
int  KeyBackToDefault(Boolean ask=True);
void SeeScanCodes(void);
int  AltKeysSetUp(void);
int  KeyPadSetUp(void);

