@Rem Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
@Rem see copyrigh file for details
@
redir -e error.txt editor --no-redirect --no-signal
gsymify -i error.txt -o error.rep editor.exe
gsymify -i yamd.log -o yamd.sym editor.exe
