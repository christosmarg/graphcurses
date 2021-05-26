# See LICENSE file for copyright and license details.
# graphcurses - curses(3) 2D graph generator
.POSIX:

include config.mk

BIN = graphcurses
DIST = ${BIN}-${VERSION}
MAN1 = ${BIN}.1

SRC = graphcurses.c
OBJ = ${SRC:.c=.o}

all: options ${BIN}

options:
	@echo ${BIN} build options:
	@echo "CFLAGS	= ${CFLAGS}"
	@echo "LDFLAGS	= ${LDFLAGS}"
	@echo "CC	= ${CC}"

${BIN}: ${OBJ}
	${CC} ${LDFLAGS} ${OBJ} -o $@

.c.o:
	${CC} -c ${CFLAGS} $<

dist: clean
	mkdir -p ${DIST}
	cp -R config.mk graphcurses.1 graphcurses.c LICENSE Makefile \
		README.md ${DIST}
	tar -cf ${DIST}.tar ${DIST}
	gzip ${DIST}.tar
	rm -rf ${DIST}

run:
	./${BIN}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin ${DESTDIR}${MANPREFIX}/man1
	cp -f ${BIN} ${DESTDIR}${PREFIX}/bin
	cp -f ${MAN1} ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < ${MAN1} > ${DESTDIR}${MANPREFIX}/man1/${MAN1}
	chmod 755 ${DESTDIR}${PREFIX}/bin/${BIN}
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/${MAN1}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${BIN} \
		${DESTDIR}${MANPREFIX}/man1/${MAN1}

clean:
	rm -f ${BIN} ${OBJ} ${DIST}.tar.gz *.core

.PHONY: all options clean dist install uninstall run
