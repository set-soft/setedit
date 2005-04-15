/* Copyright (C) 1996-2005 by Salvador E. Tropea (SET),
   see copyrigh file for details */
char *CompletionChoose(char *options, const char *delimiter, int x, int y, unsigned ops);
char *CompletionChooseFromList(TStringCollection *list, int cant, int len,
                               int xC, int yC, unsigned ops, int lPartial=0,
                               Boolean aNewStr=True);

const unsigned cmplDontAddEndChar=1;
