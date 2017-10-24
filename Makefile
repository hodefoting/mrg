CC=gcc
LIB_PKGMODULES=cairo mmm
PROJECT_NAME=mrg
PROJECT_DESCRIPTION=Microraptor GUI
SYMBOL_PREFIX=
CFLAGS += -Wall -Wextra -g -D_FILE_OFFSET_BITS=64

LIB_MAJOR_VERSION=0
LIB_MINOR_VERSION=1
LIB_MICRO_VERSION=2

LD= gcc
LIB_LD_FLAGS=-lutil -lm 

LIB_CFILES=$(wildcard lib/*.c)
BIN_CFILES=$(wildcard bin/*.c)

INSTALLED_HEADERS=lib/mrg-config.h \
lib/mrg.h \
lib/mrg-events.h \
lib/mrg-style.h \
lib/mrg-text.h \
lib/mrg-list.h \
lib/mrg-audio.h \
lib/mrg-string.h \
lib/mrg-utf8.h \
lib/mrg-host.h \
lib/mrg-util.h


include .mm/magic
include .mm/lib
include .mm/bin
include .mm/pkgconfig


data/index: data/* Makefile
	echo "<html><body><ul>" > data/index
	for a in data/*;do b=`echo $$a|sed s:data/::`;\
		echo "<li><a href=\"$$b\">$$b</a></li>" >> data/index;\
  done
	echo "</ul></body></html> " >> data/index

data.inc: data/* Makefile data/index
	echo "typedef struct MrgData{const char *path;" > data.inc
	echo "int length; " >> data.inc
	echo "const char *data;}MrgData;" >> data.inc
	echo "static MrgData mrg_data[]={" >> data.inc
	for a in data/*;do b=`echo $$a|sed s:data/::`;\
		echo "{\"$$b\", `wc -c $$a|cut -f 1 -d ' '`," >> data.inc ;\
		cat $$a | \
	  sed 's/\\/\\\\/g' | \
	  sed 's/\r/a/' | \
		sed 's/"/\\"/g' | \
		sed 's/^/"/' | \
		sed 's/$$/\\n"/' >> data.inc;\
		echo "}," >> data.inc;\
		done ;
	echo "{\"\", 6, \"fnord\"}," >> data.inc;
	echo "{NULL,0,NULL}};" >> data.inc;

lib/mrg-uri-fetcher.o: lib/mrg-uri-fetcher.c data.inc

clean: extra-clean

install: install-extra
install-extra:
	install mrg-host mrg-terminal mrg-edit mrg-browser $(DESTDIR)$(PREFIX)/bin/

luajit/mrg_h.lua: lib/*.h Makefile
	echo "local ffi = require'ffi'" > $@
	echo "ffi.cdef[[" >> $@
	cat lib/*.h >> $@
	echo "]]" >> $@

extra-clean:
	rm -f mrg.static data.inc
	rm -f tests/output/*

mrg.static: bin/*.c libmrg.a
	$(CC) \
	   -static \
	bin/*.c libmrg.a -Ilib -Ibin \
	  `pkg-config mmm cairo --cflags --libs --static` \
	   -lutil \
	   -o $@
	strip $@

dist:
	(cd ..;tar cvzf mrg.tar.gz --exclude='.git*'\
		 --exclude='mrg/mrg' \
		 --exclude='mrg/old-os' \
		 --exclude='todo' \
		 --exclude='*.o' \
		 --exclude='*.a' \
		 --exclude='*.so' mrg; ls -la mrg.tar.gz; cp mrg.tar.gz /tar)

sentry:
	sentry */*.[ch] -- sh -c 'make && sudo make install'
