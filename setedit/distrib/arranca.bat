@Rem Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
@Rem see copyrigh file for details
@
@Echo Off
setedit %1 %2 %3 %4 %5 %6 %7 %8 %9
if not errorlevel 255 goto end
@Echo Error!, please look in the file %SET_FILES%/erXXXXXX,
@Echo you can recover unsaved data from this file. Please send the file
@Echo to salvador@inti.gov.ar
@Echo Additionally try to explain me what actions triggered this error.
@Echo .
@Echo Note: The files aren't deleted and XXXXXX are letters to create a
@Echo unique file.
pause
:end
