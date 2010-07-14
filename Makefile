# Makefile for src/mod/megahal.mod/

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../megahal.o

modules: ../../../megahal.$(MOD_EXT)

../megahal.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/megahal.c
	@rm -f ../megahal.o
	mv megahal.o ../

../../../megahal.$(MOD_EXT): ../megahal.o
	$(LD) -o ../../../megahal.$(MOD_EXT) ../megahal.o $(XLIBS)
	$(STRIP) ../../../megahal.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/megahal.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~
distclean: clean

#safety hash
../megahal.o: .././megahal.mod/megahal.c ../../../src/mod/module.h \
 ../../../src/main.h ../../../src/lang.h ../../../src/eggdrop.h \
 ../../../src/flags.h ../../../src/proto.h ../../../lush.h \
 ../../../src/misc_file.h ../../../src/cmdt.h ../../../src/tclegg.h \
 ../../../src/tclhash.h ../../../src/chan.h ../../../src/users.h \
 ../../../src/compat/compat.h ../../../src/compat/inet_aton.h \
 ../../../src/compat/snprintf.h ../../../src/compat/memset.h \
 ../../../src/compat/memcpy.h ../../../src/compat/strcasecmp.h \
 ../../../src/mod/modvals.h ../../../src/tandem.h
