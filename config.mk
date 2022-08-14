PACKAGE ?= sbs
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
