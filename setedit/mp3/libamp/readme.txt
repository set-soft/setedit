LibAmp 0.2

Based on Amp 0.7.6 by Tomislav Uzelac

Ported to DJGPP/Allegro and turned into library form by Ove Kaaven
 <ovek@arcticnet.no>
(the direct Amp port can be made with: make -f makefile.amp)

To use, #include "libamp.h" into your program and link with -lamp
(Beware of name clashes between routines in this library and your program,
 there is potentially some serious name space pollution here.)

See the demo MP3 player libdemo.c for an example.
(You're welcome to write a better player if you wish.)

New in LibAmp 0.2
 amp_reverse_phase, enables the classic 3D effect
 volume slider in libdemo

Notes:
 #defines in audioalg.c

  TRACK_FRAME is useful for players that want to show real-time scopes
   (like libdemo), but is otherwise unnecessary and just wastes CPU time
   and RAM. You may want to undefine it if you don't need it.

  STREAM_SAMP is how big the Allegro stream buffer size should be.
   Increase it if the playback becomes jerky due to run_amp() or poll_amp()
   not being called often enough to supply the stream with data often enough.

  BUF_SAMP is how big the internal audio buffer size should be.
   It *must* be a multiple of 2*576 *and* a multiple of STREAM_SAMP for
   proper operation. run_amp() will try to fill it up completely, so if
   the buffer is big, it may spend some time on first call after load or
   seek.

  UPD_SAMP is with what resolution amp_play_left and amp_play_right is
   updated to provide real-time scope functionality. amp_play_len will
   be set to this value. STREAM_SAMP must be a multiple of this value.
   It is only meaningful if TRACK_FRAME is defined.

