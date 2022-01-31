.SILENT:

CC?=gcc
PKG_CONFIG?=pkg-config

CFLAGS?=-g -O2 -Wall
LDFLAGS?=

PREFIX?=/usr/local
DESTDIR?=

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LDFLAGS+=-Wl,--no-as-needed
endif

CFLAGS+=$(shell $(PKG_CONFIG) x11 --cflags)
LDFLAGS+=$(shell $(PKG_CONFIG) x11 --libs)

CFLAGS+=$(shell $(PKG_CONFIG) imlib2 --cflags)
LDFLAGS+=$(shell $(PKG_CONFIG) imlib2 --libs)

CFLAGS+=$(shell $(PKG_CONFIG) xinerama --cflags)
LDFLAGS+=$(shell $(PKG_CONFIG) xinerama --libs)

SRC = main.c
OBJ = ${SRC:.c=.o}

.c.o:
	${CC} -c ${CFLAGS} $<

all: sbs

sbs: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

install: all

indent:
	indent --blank-lines-after-procedures --brace-indent0 --indent-level4 \
		--no-space-after-casts --no-space-after-function-call-names \
		--dont-break-procedure-type --format-all-comments \
		--line-length100 --comment-line-length100 --tab-size4 *.c

check-indentation:
	$(eval SOURCES := $(shell ls *.c))
	for i in $(SOURCES); do \
		export DIFFS=$$(diff $$i <(indent -st -bap -bli0 -i4 -ncs -npcs -npsl -fca -l100 -lc100 -ts4 $$i)); \
		if [ -z "$$DIFFS" ]; then echo -e "\033[0;32mValid indentation format -> $$i\033[0m"; else echo -e "\033[0;31mInvalid indentation format -> $$i\033[0m"; fi \
	done

check:
	@echo Checking indentation standards
	$(MAKE) check-indentation


valgrind:
	valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=.valgrind-report \
		./sbs

clean:
	rm -f sbs ${OBJ}

.PHONY: all sbs install valgrind clean indent check-indentation check


