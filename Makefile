#################################
# Makefile for RPI Engine       #
# Copyright (C) 2006            #
# Shadows of Isildur            #


#################################
# CONFIGURATION VARIABLES       #
SRV_TYPE = tp


#################################
# INTERNAL VARIABLES            #
#    STANDARD DEFS              #
CC=g++ -Wno-write-strings 
CFLAGS = -ggdb -DLINUX -Wall -w
LIBS = -lcrypt -lmysqlclient -lpthread -lz -lm -lrt -ldl
INCLUDE = -I/usr/include/mysql/ -I/usr/include/openssl/
#    DIRECTORY TREE             #
BASE_DIR = /home/LRPI/$(SRV_TYPE)
BIN_DIR = $(BASE_DIR)/bin
SOURCE_DIR = $(BASE_DIR)/src
OBJ_DIR = .objects
LIB_DIR = -L/usr/lib/i386-linux-gnu
#    ACTIVE OBJECT LIST         #
HEADERS = $(wildcard *.h)
SOURCE = account act.comm act.informative act.movement act.offensive act.other arena auctions clan \
comm commands commerce constants control crafts create_mobile creation db destruction dwellings edit \
employment fight group guest guides handler hash item_card item_tossable limits magic mobact mobprogs \
mysql nanny net_link object_damage objects olc perception psionics roles roomprogs save server somatics \
staff transformation utility weather wounds turf traps electronic variables firearms BroadwaveClient


#################################
# BUILD TARGETS                 #
#    BINARY                     #
/home/LRPI/tp/bin/server: $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(SOURCE)))
	@echo Rebuilding Binary
	@$(CC) $(INCLUDE) $(LIB_DIR) -o $@ $(CFLAGS) $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(SOURCE))) $(LIBS)
#    COMPILED OBJECTS           #
$(OBJ_DIR)/%.o: %.cpp $(HEADERS)
	@echo "    Compiling $<"
	@$(CC) -c $(CFLAGS) $< -o $@
#    RECLAIMATION              #
clean:
	@echo Cleaning...
	@echo "    all binaries"
	@echo "    intermediary objects"
	@rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/server
#    INITIALIZATION            #
claim:
	chmod 771 $(BIN_DIR)
	chmod 771 $(BIN_DIR)/server
	chown LRPI:LRPI $(BIN_DIR)
	chown LRPI:LRPI $(BIN_DIR)/server
.PHONY: clean

################################
# REQUSITE COMMANDS            #
#    DIRECTORY INITIALIZATION  #
$(shell mkdir -p $(OBJDIR) $(BIN_DIR))
