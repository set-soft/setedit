/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// A static class to join the variables
class MP3Player
{
public:
 MP3Player() {};

 static void SelectNewFile(char *fileName);
 static int  PlaySelectedFile(void);
 static int  ConvertSelectedFile(char *name);
 static long GetMP3Info(void);
 static void Stop();
 static int  Poll();
 static int  Init();
 static void deInit();
 static int  GetTime();
 static void FFwd();
 static void Rew();
 static void TogglePause();
 static void Pause();
 static void SeekAbs(int seconds);
 static void StartPlayList(char *(*aGetNext)(), void (*aEndOfList)());
 static void SetupPlayList(char *(*aGetNext)(), void (*aEndOfList)());
 static void StopPlayList();

 static char Name[PATH_MAX];
 static char Title[31];
 static char Author[31];
 static char Album[35];
 static char Comment[31];
 static const char *Genre;

 static char Stoped;
 static char Selected;
 static char SelectedIsLoaded;
 static char Paused;
 static char PlayingList;
 static char Converting;

 static int  SampleRate;
 static int  BitRate;
 static const char *Mode;
 static int  MPEGVer;
 static int  Layer;
 static int  TotalLen;
 static int  PausedPos;

 static int  Strategy;

 static void PlayNext();

 static char FileToPlay[PATH_MAX];
 static int  PlayFileToPlay(char *out=0);

 static char butRew[];
 static char butStop[];
 static char butPlay[];
 static char butPause[];
 static char butFfw[];
 static char obutRew[];
 static char obutStop[];
 static char obutPlay[];
 static char obutPause[];
 static char obutFfw[];
 
protected:
 static char *(*GetNext)();
 static void  (*EndOfList)();
};

extern MP3Player mp3;

const int mp3Buffered=1, mp3UnBuffered=0;
