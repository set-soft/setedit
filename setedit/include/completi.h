/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
char *CompletionChoose(char *options, const char *delimiter, int x, int y, unsigned ops);
char *CompletionChooseFromList(TStringCollection *list, int cant, int len,
                               int xC, int yC, unsigned ops, int lPartial=0);

const unsigned cmplDontAddEndChar=1;
