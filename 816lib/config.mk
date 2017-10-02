CC      := gcc
CCFLAGS := -c -O2 -fomit-frame-pointer $(CCOPTS) -g
LD      := gcc
LDFLAGS := -g

PREFIX  := /Programs/Kestrel/1
LIBPATH := $(PREFIX)/lib
INCPATH := $(PREFIX)/include

DELETE  := rm -rf

