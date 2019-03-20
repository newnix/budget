CC = clang-devel
DBG ?= -ggdb
LDFLAGS = -z relro -z now -z combreloc -s
CFLAGS = -Oz -std=c99 -fpic -fpie -fPIC -fPIE -Wl${LDFLAGS}
