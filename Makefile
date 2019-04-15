.POSIX:

PREFIX ?= ${HOME}
DESTDIR = /bin
TARGET = budget
SRCS = budget.c
INCS = -I/usr/local/include
LIBS = -L/usr/local/lib

CC = clang-devel
DBG ?= -ggdb
LDFLAGS = --gc-sections,-icf=all,-zrelro,-zcombreloc,-znow
CFLAGS = -Oz -std=c99 -fpic -fpie -fPIC -fPIE -Wl,${LDFLAGS} -Werror -Wall -Wextra -pedantic -march=native -mtune=native ${INCS} #${LIBS} -lsqlite3
HELP = -h

## Run clang's static analyzer
check: ${SRCS}
	@clang-tidy-devel -checks=* ${SRCS}

## Build with debugging symbols
debug: ${SRCS}
	$(CC) ${DBG} ${CFLAGS} ${SRCS} -o ${TARGET}
	@install -v -m 1755 ${TARGET} ${PREFIX}${DESTDIR}
	${PREFIX}${DESTDIR}/${TARGET} ${HELP}

## Build with debug symbols stripped
install: ${SRCS}
	$(CC) ${CFLAGS} ${SRCS} -o ${TARGET}
	@strip -s ${TARGET}
	@install -v -m 1755 ${TARGET} ${PREFIX}${DESTDIR}
	${PREFIX}${DESTDIR}/${TARGET} ${HELP}

## Display the current settings
help:
	@printf "Current settings for %s\n" ${TARGET}
	@printf "\tPREFIX=\t\t%s\n" ${PREFIX}
	@printf "\tDESTDIR=\t%s\n" ${DESTDIR}
	@printf "\tINSTALLPATH=\t%s\n" "${PREFIX}${DESTDIR}/${TARGET}"
	@printf "\tINSTALLMODE=\t%d\n" 1755
	@printf "\tCC=\t\t%s\n" ${CC}
	@printf "\tCFLAGS=\t\t%s\n" "${CFLAGS}"
	@printf "\tDBGFLAGS=\t%s\n" ${DBG}
	@printf "To change these settings run 'make config' or use ${EDITOR} Makefile\n"

config:
	@$(EDITOR) ${PWD}/Makefile

diff:
	@fossil diff

commit:
	@fossil commit

tests:
	@printf "No tests are currently defined for this project\n"

test: tests
