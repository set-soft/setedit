/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef __CONTROLDATA_H
#define __CONTROLDATA_H

#define MSG_CTRL         0
#define MSG_BUFFER       1
#define MSG_SONG         2
#define MSG_QUIT         3
#define MSG_NEXT         4
#define MSG_QUERY        5
#define MSG_RESPONSE     6
#define MSG_BUFAHEAD     7
#define MSG_FRAMES       8
#define MSG_POSITION     9
#define MSG_SEEK         10
#define MSG_PRIORITY     11
#define MSG_RELEASE      12
#define MSG_AUDIOFAILURE 13
#define MSG_INFO         14

/* MSG_PRIORITY */
#define PRIORITY_NORMAL   1
#define PRIORITY_REALTIME 2
#define PRIORITY_NICE     3

/* MSG_CTRL */
#define PLAY_STOP     0
#define PLAY_PAUSE    1
#define FORWARD_BEGIN 2
#define FORWARD_STEP  3
#define FORWARD_END   4
#define REWIND_BEGIN  5
#define REWIND_STEP   6
#define REWIND_END    7

/* MSG_QUERY */
#define QUERY_PLAYING 1
#define QUERY_PAUSED  2

/* MSG_RESPONSE */
#define FAILURE       0
#define SUCCESS       1

struct m_cmsghdr
{
      unsigned int  cmsg_len;
      int  cmsg_level;   
      int  cmsg_type;    
      int  fd;
};

typedef struct __controlmsg
{
      int type;
      int data;
} TControlMsg, *PControlMsg;

/*
 * The Jukebox assumes that the player reads its commands from stdin
 * and writes its replies to stdout. The connection type setup by the
 * jukebox and the player is of type socket.
 * The jukebox knows nothing more about the player than the following
 * messages. If they are properly implemented, any player should work.
 *
 * The receiving player should send the following messages when specific
 * events occurs.
 *
 ************************************************************************
 *
 * MSG_NEXT
 *   When the player is done playing a song, and the playing has not
 *   been explicitly stopped by use of MSG_CTRL/PLAY_STOP, the player
 *   need to send a MSG_NEXT to notify the Jukebox that it's done.
 *
 * MSG_INFO
 *   When the player recieves and starts decoding a new song, it can
 *   send a MSG_INFO to the Jukebox telling it about various data
 *   regarding the song. The Information is sent by first sending
 *   a MSG_INFO, directly followed by a AudioInfo struct.
 *
 ************************************************************************
 *
 * The receiving player should send the following messages at some interval
 * to notify the Jukebox regarding its progress.
 *
 ************************************************************************
 *
 * MSG_POSITION
 *  The data member of the message contains the file position, ie what
 *  position is currently being played. This message updates the progress
 *  bar.
 *
 * MSG_FRAMES
 *  The number of Mpeg frames currently played. An Mpeg frame, as far as
 *  I know is 4608 bytes. This message updates the clock.
 * 
 ************************************************************************
 * 
 * The receiving player can send the following messages to notify the
 * player regarding some kind of error.
 ************************************************************************
 *
 * MSG_AUDIOFAILURE
 *   The player could not acquire the audiodevice.
 *
 * MSG_PRIORITY
 *   If the Jukebox receives this message it will notify the user that
 *   the player does not have the permissions required to use the priority
 *   mode the user wanted.
 *
 ************************************************************************
 *
 * The receiving process has to be able to handle the following messages:
 ************************************************************************
 * MSG_CTRL
 *
 *   FORWARD_BEGIN
 *   FORWARD_STEP
 *   FORWARD_END
 *     The reason there are three messages is because you might need to lock
 *     the decoding/playing thread while jumping forward in the song.
 *     Once the player has fulfilled one request, it responds with a 
 *     MSG_RESPONSE, with data set to FORWARD_BEGIN, FORWARD_STEP, 
 *     FORWARD_END. The reason for this is that the jukebox will not send
 *     a new FORWARD_STEP or FORWARD_END request until it is certain the
 *     previous has been served.
 *     If you do not wish to implement forward, just send a MSG_RESPONSE
 *     back immediately upon receiving a forward request.
 *
 *   REWIND_BEGIN
 *   REWIND_STEP
 *   REWIND_END
 *     The same setup as with forward, and the response is MSG_RESPONSE with
 *     data set to REWIND_BEGIN, REWIND_STEP, REWIND_END.
 *
 *   PLAY_STOP
 *     This message is sent when the jukebox want the player to stop playing.
 *     The player will not reply to this message in any way, and will neither
 *     send a MSG_NEXT.
 *
 *   PLAY_PAUSE
 *     This message is sent to either release a paused player, or to pause
 *     the player. No reply is expected.
 *
 ************************************************************************
 * 
 * MSG_QUIT
 *   This message is sent by the Jukebox when the jukebox want the player
 *   to die. No reply is expected. The Jukebox will capture the SIGCHLD if
 *   its around to receive it.
 *
 ************************************************************************
 *
 * MSG_QUERY
 *   QUERY_PLAYING
 *     This message is sent by the Jukebox when it needs to know if the
 *     player is currently playing or not. It will expect a reply in the
 *     form of MSG_RESPONSE, with data set to TRUE or FALSE.
 *
 *   QUERY_PAUSED
 *     This message is sent by the Jukebox when it needs to know if the
 *     player is currently paused or not. It will expect a reply in the
 *     form of MSG_RESPONSE, with data set to TRUE or FALSE.
 *
 ************************************************************************
 *
 * MSG_SONG
 *   This is the most complicated message that the player needs to support.
 *   It's an ordinary message, followed by a message sent by sendmsg with
 *   a filedescriptor `in flight'. Once the MSG_SONG is recieved and read,
 *   the player immediately need to call recvmsg to fetch the filedescriptor.
 *   The recvmsg requries a buffer as an argument. You should have one iovec,
 *   a char buffer of two bytes, and an iovec length of 2.
 *   Basically, if you're confused, look in sajberplay how it extracts the
 *   filedescriptor from the message.
 *
 ************************************************************************
 *
 * 
 * The following are features that should be included.
 ************************************************************************
 *
 * MSG_BUFFER
 *   The data contains the number of frames that the Jukebox wants buffered
 *   ahead.
 *
 * MSG_BUFAHEAD
 *   The data contains the number of frames that the Jukebox wants decoded
 *   before actual play starts. Be careful with the relationship between
 *   frames buffered and bufahead, or you'll deadlock.
 *
 * MSG_SEEK
 *   This message tells the player that the Jukebox wants playing to continue
 *   at the file position specified by the data.
 *   Any request that can't be served should just be silently ignored,
 *   like a rewind requests on a stream or socket.
 *
 ************************************************************************
 *
 * The following are features that would be good if they are included.
 * Most of them are rather easy to do.
 ************************************************************************
 *
 * MSG_PRIORITY
 *   PRIORITY_NORMAL
 *     Tells the player that is should set its scheduling and static priorities
 *     to SCHED_OTHER and 0.
 *   PRIORITY_REALTIME
 *     Tells the player that its allowed to enter realtime scheduling and
 *     changing static priorities as it sees fit.
 *   PRIORITY_NICE
 *     Tells the player that it's to use SCHED_OTHER scheduling but may change
 *     static priorities at will.
 *   
 * MSG_RELEASE
 *   The data member of the message is either TRUE or FALSE. This message
 *   tells the player if it should release the audiodevice on an explicit
 *   MSG_STOP or when playing stops [The player might want to wait with
 *   releasing the audio device until the Jukebox has had ample time 
 *   anwering a MSG_NEXT].
 */

#endif
