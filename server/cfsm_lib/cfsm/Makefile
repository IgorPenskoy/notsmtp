BINDIR=/usr/local/bin
TEMPLATEDIR=/usr/local/share/cfsm

CDBG=-w
CFLAGS+=    -g -std=gnu99 -D_GNU_SOURCE
CFLAGS+=    -I.
CFLAGS+=    -DTEMPLATE_DIR=\"$(TEMPLATEDIR)\"

LDFLAGS+= -L../mtemplate
CFLAGS+= -I../mtemplate -DYYDEBUG=1
LIBS+= -lmtemplate -ly -ll

RANLIB=ranlib
LEX=lex
YACC=yacc

CFSM_OBJS=cfsm.o cfsm_parse.o cfsm_lex.o
COMPAT_OBJS=strlcat.o strlcpy.o

all: cfsm

cfsm: $(CFSM_OBJS) $(COMPAT_OBJS)
	$(CC) -o $@ $(CFSM_OBJS) $(COMPAT_OBJS) $(LDFLAGS) $(LIBS)

cfsm_lex.o: cfsm_parse.h

cfsm_lex.c: cfsm_lex.l
	$(LEX) -o$@ cfsm_lex.l

cfsm_parse.c: cfsm_parse.y
	$(YACC) -d -o$@ cfsm_parse.y

clean:
	rm -f *.o cfsm cfsm_lex.[ch] cfsm_parse.[ch]
	rm -f lex.yy.[ch] y.tab.[ch] core *.core fsm.c fsm.h fsm.dot
	cd regress && make clean

test: all
	cd regress && make
