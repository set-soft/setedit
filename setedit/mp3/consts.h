/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
const int cmaBase=0x22A0;

#define h(a,b) const int cma##a=cmaBase+b,hc##a=cmaBase+b;

h(UpdateTime,0)
h(IsAmpDiagThere,1)
h(UpdateFile,2)
h(MP3Play,3)
h(MP3Stop,4)
h(MP3Pause,5)
h(MP3Ffw,6)
h(MP3Rew,7)
h(ReflectStatus,8)
h(AddMP3,9)
h(InsertMP3,10)
h(DeleteMP3,11)
h(SaveMP3List,12)
h(LoadMP3List,13)
h(MP3Prev,14)
h(MP3Next,15)

#undef h

