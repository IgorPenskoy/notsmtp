EXE = notsmtp
UNIT_EXE = test_notsmtp

SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include
TEST_DIR = test

AUTOOPTS_DIR = src/shell_opt
AUTOOPTS_CFLAGS = # -DTEST_NOTSMTP_OPTS `autoopts-config cflags`
AUTOOPTS_LDFLAGS = $(shell autoopts-config ldflags)

#AUTOFSM_DIR = src / fsm
FSM_DIR = src/fsm
CFSM_DIR = cfsm_lib
CFSM_CFLAGS = -std=gnu99

SRC = $(wildcard $(SRC_DIR)/*.c)

TEST_FILES =\
$(wildcard $(TEST_DIR)/*.c)\
$(SRC_DIR)/client.c\
$(SRC_DIR)/config.c\
$(SRC_DIR)/logging.c\
$(SRC_DIR)/server.c\
$(SRC_DIR)/smtp_cmd_utils.c\
$(SRC_DIR)/smtp_cmd.c\
$(SRC_DIR)/smtp.c\
$(SRC_DIR)/socket_utils.c\
$(SRC_DIR)/socket.c\
$(SRC_DIR)/sds.c\
$(FSM_DIR)/fsm.o

OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC = gcc

CPPFLAGS += -I$(INCLUDE_DIR)
CFLAGS += -std=gnu99 -Wall -Werror
LDFLAGS +=
LDLIBS += -lconfig -lrt -lpcre -lfiredns $(AUTOOPTS_LDFLAGS)

.PHONY: all clean

all: clean $(EXE)

$(EXE): $(AUTOOPTS_DIR)/shell_opt.o $(FSM_DIR)/fsm.o $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# AUTOOPTS #

$(AUTOOPTS_DIR)/shell_opt.c: $(AUTOOPTS_DIR)/shell_opt.def
	cd $(AUTOOPTS_DIR) && SHELL=/bin/sh autogen shell_opt.def
	mv -f $(AUTOOPTS_DIR)/shell_opt.h $(INCLUDE_DIR)

$(AUTOOPTS_DIR)/shell_opt.o: $(AUTOOPTS_DIR)/shell_opt.c
	$(CC) -c -I$(INCLUDE_DIR) $(AUTOOPTS_CFLAGS) -o $@ $<

# FSM #

$(FSM_DIR)/fsm.o: $(FSM_DIR)/fsm.c
	$(CC) -c -I$(INCLUDE_DIR) $(CFSM_CFLAGS) -o $@ $<

$(FSM_DIR)/fsm.c: $(FSM_DIR)/fsm.fsm
	cd $(CFSM_DIR) && make --silent
	$(CFSM_DIR)/cfsm/cfsm -t $(CFSM_DIR)/cfsm/ -d $(FSM_DIR)/fsm.fsm -o fsm.c
	mv -f fsm.c $(FSM_DIR)
	mv -f fsm.h $(INCLUDE_DIR)
	$(CFSM_DIR)/cfsm/cfsm -t $(CFSM_DIR)/cfsm/ -g $(FSM_DIR)/fsm.fsm -o $(FSM_DIR)/fsm.dot

####################################################################################

# AUTOFSM #

# $(AUTOFSM_DIR)/server_fsm-fsm.c: $(AUTOFSM_DIR)/server_fsm.def
# 	cd $(AUTOFSM_DIR) && autogen -L$(AUTOFSM_DIR) server_fsm.def
# 	mv -f $(AUTOFSM_DIR)/server_fsm-fsm.h $(INCLUDE_DIR)

# $(AUTOFSM_DIR)/server_fsm-fsm.o: $(AUTOFSM_DIR)/server_fsm-fsm.c
# 	$(CC) -c -I$(INCLUDE_DIR) -o $@ $<

####################################################################################

test_units: $(EXE) $(UNIT_EXE)
	./$(UNIT_EXE)

$(UNIT_EXE): $(TEST_FILES)
	$(CC) -I$(INCLUDE_DIR) $(CFLAGS) $^ -lconfig -lrt -lpcre -lcunit -o $@

test_memory: test_units
	valgrind ./$(UNIT_EXE)

test_system: $(EXE)
	python3 $(TEST_DIR)/sys_test.py

test_style:
	clang-format -i -style='{IndentWidth: 4}' $(SRC_DIR)/*.c
	clang-format -i -style='{IndentWidth: 4}' $(INCLUDE_DIR)/*.h

tests: test_units test_memory test_system test_style

clean:
	$(RM) $(OBJ)
	$(RM) $(AUTOOPTS_DIR)/*.o $(AUTOOPTS_DIR)/*.c $(AUTOOPTS_DIR)/*.h
	$(RM) $(FSM_DIR)/*.o $(FSM_DIR)/*.c $(FSM_DIR)/*.h $(FSM_DIR)/*.dot
	$(RM) $(INCLUDE_DIR)/fsm.h $(INCLUDE_DIR)/shell_opt.h
	cd $(CFSM_DIR) && make clean --silent
	$(RM) $(UNIT_EXE)
