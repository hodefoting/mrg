test: mrg-amalgamation.c test.c
	gcc `pkg-config sdl gtk+-3.0 cairo --cflags --libs` test.c -o test


BUNDLED_HEADERS = \
  ../mrg-config.h \
  ../mrg.h \
  ../mrg-events.h \
  ../mrg-style.h \
  ../mrg-text.h \
  ../mrg-util.h \

BUNDLED_FILES = \
  ../mrg-list.h \
  ../mrg-internal.h \
  ../mrg-backend-gtk.c \
  ../mrg-backend-mrg.c \
  ../mrg-backend-sdl.c \
  ../mrg-backend-mem.c \
	../ufb-evsource.h \
	../ufb-evsource-kb.c \
	../ufb-evsource-mice.c \
	../ufb-evsource-ts.c \
  ../ufb.h \
  ../ufb.c \
  ../mrg-backend-ufb.c \
  ../nchanterm.c \
  ../mrg-backend-terminal.c \
  ../mrg-binding.c \
  ../mrg.c \
  ../mrg-events.c \
  ../mrg-focus.c \
  ../mrg-image.c \
  ../mrg-style-properties.c \
  ../mrg-stylesheet.c \
  ../mrg-text.c \
  ../mrg-util.c \
	../mr.h \
	../mrg-http.h \
	../mrg-string.h \
	../mrg-xml.h \
	../mrg-xml.c \
	../mrg-uri-fetcher.c \
	../mrg-xml-render.c \
  ../mrg-string.c \
	../mrg-http.c 
	

mrg-amalgamation.c: Makefile ../*.[ch]
	@echo "/* mrg-bundle / amalgation of all the sources\n" > $@
	@echo " * permitting simple .so/.dll free 'static' use  " >> $@
	@echo " *" >> $@
	@echo " * git revision generating amalgamation: " >> $@
	@echo " *   "`git log -n1|head -n1|sed s/commit//` >> $@
	@echo " * " >> $@
	@echo " */" >> $@


	@for a in $(BUNDLED_HEADERS); do \
		echo "/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌"`echo $$a|sed s:\.\./::`"▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */" >> $@;\
		cat $$a >> $@; \
	done
	echo "#ifndef MRG_HEADERS_ONLY" >> $@
	@for a in $(BUNDLED_FILES); do \
		echo "/* ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▌"`echo $$a|sed s:\.\./::`"▐▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ */" >> $@;\
		cat $$a >> $@; \
	done
	echo "#endif" >> $@
	@rpl '#include "mrg' '// bundled include of mrg' $@
	@rpl '#include "nch' '// bundled include of nch' $@
	@rpl '#include "ufb' '// bundled include of ufb' $@

clean:
	rm -f mrg-amalgamation.c test
