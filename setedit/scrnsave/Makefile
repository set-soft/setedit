srcdir=.
VPATH=$(srcdir)

# To avoid conflicts with the values provided by .mak files
CFLAGS=
CXXFLAGS=
export

ifneq ($(strip $(DJDIR)),)
export editor_OS:=DJGPP
endif

# If not DOS then just Linux, no other supported by now
ifeq ($(strip $(editor_OS)),)
export editor_OS:=Linux
endif

prjs=fakealle/fakealle plasmlib/plasmlib plasmlib/mixsfake \
		plasma1/ps1 plasma2/ps2 plasma3/ps3 plasma4/ps4

%.mak: %.gpr
	gpr2mak $<

plasmas: makes ps1.exe ps2.exe ps3.exe ps4.exe

libraries:
	$(MAKE) -C plasmlib -f plasmlib.mak
	$(MAKE) -C plasmlib -f mixsfake.mak
	$(MAKE) -C fakealle -f fakealle.mak

ps1.exe: libraries
	$(MAKE) -C plasma1 -f ps1.mak
	-upx -9 plasma1/ps1.exe

ps2.exe: libraries
	$(MAKE) -C plasma2 -f ps2.mak
	-upx -9 plasma2/ps2.exe

ps3.exe: libraries
	$(MAKE) -C plasma3 -f ps3.mak
	-upx -9 plasma3/ps3.exe

ps4.exe: libraries
	$(MAKE) -C plasma4 -f ps4.mak
	-upx -9 plasma4/ps4.exe

makes: $(addsuffix .mak,$(prjs))

clean:
	rm -f */*.mak *.bkp */*.bkp */*.o */*/*.o */*.a */*.exe

clean-o:
	rm -f */*.o */*/*.o */*.a */*.exe


