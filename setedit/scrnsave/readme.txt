  This directory contains instruction on how to create an external screen saver
(read the how-to.txt file) and some DOS examples.
  This feature isn't tested under Linux.
  The idea was introduced in 0.4.34 suggested by "Grzegorz Adam Hankiewicz"
<gah@jet.es>.
  All the files here are distributed under the GPL license as the rest of the
editor.
  The directory plasmlib contains a reduced version of my plasma library, it
have various interesting routines. The code is optimized for Cyrix 5x86 and
gcc 2.7.2, I didn't have time to tune it for Pentium or other gcc versions.
  The directory fakealle contains some routines that are called like the
Allegro library function but that just works for 320x200 video modes. I used
it to make the screen savers really small (35 to 40Kb compressed with UPX)
but the code was original written for Allegro 3.

