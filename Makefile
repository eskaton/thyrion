VERSIONM= 1
VERSION = 1.0.0
#CFLAGS  = -Wall -Wextra -g -O2 -I/usr/local/include/misc
CFLAGS  = -Wall -Wextra -g -I/usr/local/include/misc 
LDFLAGS = -L/usr/local/lib -L. -lmisc -lm
ARFLAGS = -rc
CC      = gcc 
LD      = gcc 
AR      = ar

STATICLIB=libthyrion.a
SHAREDLIB=libthyrion.so
SHAREDLIBV=libthyrion.so.$(VERSION)
SHAREDLIBVM=libthyrion.so.$(VERSIONM)

prefix =/usr/local
bindir =${prefix}/bin
libdir =${prefix}/lib
includedir =${prefix}/include

SRC = thyrion.c elf_util.c
OBJ = ${SRC:.c=.o}
PIC_OBJ = ${SRC:.c=.lo}

all: $(STATICLIB) $(SHAREDLIBV) dwarfdump line2addr

.c.o:
	${CC} -c $< ${CFLAGS}

.SUFFIXES: .lo

.c.lo:
	${CC} -fPIC -c $< -o $@ ${CFLAGS}

$(SHAREDLIBV): $(PIC_OBJ)
	${LD} -shared -fPIC ${LDFLAGS} *.lo -o $@
	ln -fs $@ $(SHAREDLIB)
	ln -fs $@ $(SHAREDLIBVM)

$(STATICLIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ ${OBJ}

dwarfdump: dwarfdump.o $(SHAREDLIBV)
	${CC} dwarfdump.o -o $@ ${CFLAGS} -L. -lthyrion ${LDFLAGS} 

line2addr: line2addr.o $(SHAREDLIBV)
	${CC} line2addr.o -o $@ ${CFLAGS} -L. -lthyrion ${LDFLAGS} 

install: $(STATICLIB) $(SHAREDLIBV) dwarfdump line2addr
	cp thyrion.h $(includedir)
	chmod 644 $(includedir)/thyrion.h
	cp $(STATICLIB) $(libdir)
	chmod 644 $(libdir)/$(STATICLIB)
	cp $(SHAREDLIBV) $(libdir)
	chmod 755 $(libdir)/$(SHAREDLIBV)
	rm -f $(libdir)/$(SHAREDLIB) $(libdir)/$(SHAREDLIBVM)
	ln -fs $(SHAREDLIBV) $(libdir)/$(SHAREDLIB)
	ln -fs $(SHAREDLIBV) $(libdir)/$(SHAREDLIBVM)
	(ldconfig -m || true) >/dev/null 2>&1
	cp dwarfdump $(bindir)
	chmod 755 $(bindir)/dwarfdump 
	cp line2addr $(bindir)
	chmod 755 $(bindir)/line2addr

clean:
	@rm -f *.o *.lo $(SHAREDLIB) $(SHAREDLIBV) $(SHAREDLIBVM) $(STATICLIB) ${OBJ} \
	${PIC_OBJ} dwarfdump line2addr
