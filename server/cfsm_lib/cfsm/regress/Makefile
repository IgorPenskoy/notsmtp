# Public domain - Damien Miller <djm@mindrot.org> 2007-03-26

# $Id: Makefile,v 1.12 2007/11/18 09:51:19 djm Exp $

CFSM=../cfsm 
CFSM_FLAGS=-t.. -d
TARGETS=t1 t2 t3 t_ex0
CDBG=-w

all: $(TARGETS)
	@echo -n "Running tests: "
	@set -e ; for x in $(TARGETS) ; do \
		test "x$(VERBOSE)" = "x" || echo -n $${x} ; \
		./$$x && echo -n . || echo -n '!' ; \
		test "x$(VERBOSE)" = "x" || echo -n ' ' ; \
	done
	@echo ""

# Make sure the examples work!
t_ex0_fsm.c: ../example.fsm
	$(CFSM) $(CFSM_FLAGS) -o t_ex0_fsm.c ../example.fsm

t_ex0: t_ex0_fsm.c t_ex0_fsm.o t_ex0.o
	$(CC) -o $@ t_ex0.o t_ex0_fsm.o

t1_fsm.c: t1_fsm.fsm
	$(CFSM) $(CFSM_FLAGS) -o t1_fsm.c t1_fsm.fsm

t1: t1_fsm.c t1_fsm.o t1.o
	$(CC) -o $@ t1.o t1_fsm.o

t2_fsm.c: t2_fsm.fsm
	$(CFSM) $(CFSM_FLAGS) -o t2_fsm.c t2_fsm.fsm

t2: t2_fsm.c t2_fsm.o t2.o
	$(CC) -o $@ t2.o t2_fsm.o

t3_fsm.c: t3_fsm.fsm
	$(CFSM) $(CFSM_FLAGS) -o t3_fsm.c t3_fsm.fsm

t3: t3_fsm.c t3_fsm.o t3.o
	$(CC) -o $@ t3.o t3_fsm.o

clean:
	rm -f *.o *_fsm.[ch] $(TARGETS) *.core core
