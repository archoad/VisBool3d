# définition des cibles particulières
.PHONY: clean, mrproper
  
# désactivation des règles implicites
.SUFFIXES:

UNAME_S:=$(shell uname -s)

CC=gcc
CL=clang
CFLAGS= -O3 -Wall -W -Wstrict-prototypes -Werror
ifeq ($(UNAME_S),Linux)
	IFLAGSDIR= -I/usr/include
	LFLAGSDIR= -L/usr/lib
	COMPIL=$(CC)
endif
ifeq ($(UNAME_S),Darwin)
	IFLAGSDIR= -I/opt/local/include
	LFLAGSDIR= -L/opt/local/lib
	COMPIL=$(CL)
endif
GL_FLAGS= -lGL -lGLU -lglut
NET_FLAGS= -lnet -lpcap
MATH_FLAGS= -lm
AES_FLAGS= -lcrypto
PNG_FLAGS= -lpng

all: dest_sys visBool3d

visBool3d: visBool3d.c
	$(COMPIL) $(CFLAGS) $(IFLAGSDIR) $(LFLAGSDIR) $(GL_FLAGS) $< -o $@

dest_sys:
	@echo "Destination system:" $(UNAME_S)

clean:
	@rm -f visBool3d

