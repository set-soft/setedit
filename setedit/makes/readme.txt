Hi!

  This directory contains the files needed to make the editor. You should
read the readme.txt file located in the root directory of the distribution
before reading it.
  Please read this file if you want to have less problems while compiling.

Topics:

0. I don't have time to read it.      <----- Quick instructions
1. Systems where RHIDE is installed.
2. Systems without RHIDE.
3. Other .gpr files included.
4. MP3 support under DOS.
5. Generating the distribution files.
6. Installing the compiled files.


0. I don't have time to read it:
-------------------------------


DOS ONLY: ------->
 Edit the ../include/ceditint.h and comment the line:
 #  define SUP_MP3
 Remove alleg and amp in editor.mak, the line looks like it:
 RHIDE_OS_LIBS=amp alleg intl
<-------
Windows NT ONLY: ------->
 Read the README located in ..\WinNT directory.
<-------
  
  If that's your case just run 'make' in this directory and cross your
fingers. If you have all the needed tools you'll get the editor compiled
after some minutes. Then just run 'make install' to install it. If it fails
then you'll need time to read the rest ;-)
  Note: You can just run 'make install' directly.
  Don't forget to install Turbo Vision first.



1. Systems where RHIDE is installed:
-----------------------------------

Needed tools: RHIDE: last beta release.

  The .gpr files are RHIDE projects files and they are the best methode to
make the editor. You must have the last release of RHIDE to use these files.
The files contains the dependencies. These dependencies are to internal .h
files or Turbo Vision files, they are constructed supposing you installed
Turbo Vision in ../../tvision/ for example: /usr/src/tvision and
/usr/src/setedit or c:/djgpp/contrib/tvision or c:/djgpp/contrib/setedit.
If you didn't do it the dependencies will fail.

  The .gpr files are the same for both OSs (DOS and Linux) all the OS
specific (or just configurable) stuff is specified in the rhide.env file.

  If you have the DOS distribution and you want to compile it under Linux you
can copy linux.env to rhide.env (don't worry you can overwrite it because
djgppenv.env contains what rhide.env currently contains) and that's all.

  You can easilly configure the compilation process editing rhide.env. For
example: if you want to compile the editor without debug information you can
do it editing the

RHIDE_OS_BOTH=-Wall -Werror -gstabs3 -O2

definition to delete the -gstabs3 option.

  The steps are:
0) Read the point 4 for the DOS version.
A) Edit rhide.env to match your OS and needs. djgppenv.env is what I use in
DOS and linux.env is what I use in Linux. You should choose the debug
options, optimization, C++ especific options, etc.
B) Run "rhide editor.gpr".
C) Press F9.



2. Systems without RHIDE:
------------------------

Needed tools:
make: I use 3.77 in DOS and 3.76.1 in Linux.
fileutiles: it means rm, mv, cat, etc.
perl: 5.x. Not normally needed, but some scripts are in perl.

  That's the alternative way, I tried it and works ok.

  The .mak files contains the dependencies. These dependencies are to
internal .h files or Turbo Vision files, they are constructed supposing
you installed in ../../tvision/ for example: /usr/src/tvision and
/usr/src/setedit or c:/djgpp/contrib/tvision or c:/djgpp/contrib/setedit.
If you didn't do it the dependencies will fail.

  Currently I create the makefiles (*.mak) using a Makefile that calls
gpr2mak (a RHIDE tool to convert .gpr files to make format). The .gpr files
are the same for both OSs (DOS and Linux) all the OS specific stuff is
specified in the rhide.env file.

  If you have the DOS distribution and you want to compile it under Linux you
can copy linux.env to rhide.env (don't worry you can overwrite it because
djgppenv.env contains what rhide.env currently contains), do a "touch
rhide.env" (or just delete all the .mak files) and run "make makes". It will
create the makefiles again but with the Linux settings.
  If you need to reconfigure the makefiles for an OS where you don't have
gpr2mak you can do it in another OS and then copy the .mak files, you just
need to edit rhide.env. If you don't have access to a platform with RHIDE
don't desesperate and continue reading.

  To configure the .gpr files you just need to edit the rhide.env file, this
will also configure the .imk and .mkf files derived from the .mak files.

  The steps are:
1) Read the point 4 for the DOS version.
2) Edit rhide.env to match your OS and needs. djgppenv.env is what I use in
DOS and linux.env is what I use in Linux. You should choose the debug
options, optimization, C++ especific options, etc.
3) Run "make".



3. Other .gpr files included:
----------------------------

  I'm including some extra project files that you could want to use. They are
only in RHIDE format by now because I think they won't be used by much
people.
  testeasy.gpr: Creates an easydiag test program. This program is only to test
some easydiag features and debug it. The program will probably end with a
SIGSEGV if you close the test dialog, that's fine because I'm not
initializing some important things. The test is just to see how well the
widgets are aligned and show how to use the library.
  sdgcline.gpr: Creates a command line version of SDG (SET's Documentation
Generator). It could be outdated because I don't normally use the command
line version.
  libset.gpr: Creates a libset.a file containing all the modules. That's used
by Robert Hoehne to incorporate the editor in RHIDE.



4. MP3 support under DOS:
------------------------

  The DOS version supports MP3 songs, yes you can play your favorite song
while reading/writing even under plain DOS (no multitasker).
  By default the editor enables the MP3 support, you can disable it editing
the include/ceditint.h file and commenting the #define SUP_MP3 line.
Additionally you should edit rhide.env and remove "amp alleg" from the
definition of the RHIDE_OS_LIBS variable.
  If you want the support enabled (the default) you'll need libamp and
liballeg in your system. Allegro can be downloaded from any Simtelnet mirror
(normally is in the /pub/simtelnet/gnu/djgpp/v2tk/allegro). Only sources are
available for download so you'll need to compile it. The version needed is
the one named in readme.src (3.1 for v0.4.23 for example). I use a patched
version of libamp so I included the sources. As the code is just a disaster I
didn't include the libamp project in the main project. The code uses tabs
and a very bad indentation methode. I guess that's the most common but when
the code have 5 or 6 levels of if/else the code is totally impossible to
track, in fact gcc gives tons of warnings about ambiguos else statements and
I think at least 3 of them are plain bugs! Anyways, the code works. To create
the library go to the mp3/libamp directory and start RHIDE or do "make -f
libamp.mak".



5. Generating the distribution files:
------------------------------------

Needed tools: make, perl and makeinfo.

  To generated the distribution packages you just need to run 'make distrib'.
It will generate the distribution in the djgpp/result or linux/result
directory. Under DOS the output files are in ZIP format and in Linux .tar.gz.
To do it you need Perl 5.x and fileutils for DOS.
  Notes for DOS:
  The files are placed in djgpp/result in .ZIP format. An uncompressed tree
is left in djgpp/distrib. You can change the target directory defining an
environment variable called prefix or PREFIX or calling the compress.pl
script by hand and using '--prefix PATH' as parameter. It will generate the
.ZIP files in PATH/result and the tree in PATH/distrib. The files are *ever*
packed to be installed in a djgpp tree, no matters what the prefix is.
  Notes for Linux:
  The files are placed in linux/result in .tar.gz format. An uncompressed
tree is left in linux/distrib. The user rights and destination directory are
configured in the INSTALL.MAK file. If you define an environment variable
called prefix or PREFIX the script will modify the INSTALL.MAK makefile to
match this definition. The same can be done using the '--prefix PATH'
parameter of the script. By default the prefix is /usr.



6. Installing the compiled files:
--------------------------------

  The recommended methode is first generate the distribution package and then
just install the file, just like if you downloaded it. If you are a make
fanatic or need something very automatic you can just try 'make install'. I
don't think that's the best because you don't keep a package and because you
can't see what exactly will be installed until the files are installed.
You'll need the same tools needed for 'Generating the distribution files'.
  Notes for DOS:
  Using 'make install' the files are simply copied from the sources to the
%DJDIR% tree. If you want to install the files in other directory use the
prefix mechanism descripted in 'Generating the distribution files'.
  Notes for Linux:
  The make install process is relative 'fake', what I do is first create the
distribution tree (not the .tar.gz) and then just run the INSTALL.LINUX
script. So if you want to configure the user rights and other stuff just edit
the ../distrib/INSTALL.MAK file. The only thing you can configure without
editing this file is the prefix. By default the package is generated to be
installed in /usr but as some people uses /usr/local you can fix it defining
an environment variable like this (bash):

export PREFIX="/usr/local"

  And then calling 'make install'. In this way the files will be installed in
/usr/local/bin, /usr/local/share/setedit, etc.
