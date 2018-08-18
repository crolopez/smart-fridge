####################################
#
#  ******* MAKEFILE OPTIONS *******
#
# SQLITE -> RUN SQLITE INTERFACE
# DEBUG=YES -> COMPILE WITH DEBUG FLAGS
# TEST=YES 	-> COMPILE WITH TESTS BINARIES
# PARALLEL_DEPS=<THREADS> -> COMPILES DEPENDENCIES IN PARALLEL
# ENABLE_LDAP=YES -> ENABLE OPEN LDAP INTEGRATION (TBD)
# DB -> LAUNCHES THE DATABASE INTERFACE
# ENABLE_CAM=YES -> COMPILE WITH CAMERA SUPPORT
#
####################################

PROJECT_NAME="Smart Fridge"

# Print colours
CGREEN=\033[1;32m
CRED=\033[1;31m
CCYAN=\033[1;36m
CYELLOW=\033[1;33m
CEXTERN=\033[1;35m
CEND=\033[0m

# Messages
P_SUCCESS=@printf '$(CGREEN)%s$(CEND)\n'
P_COMP=@printf '$(CRED)%s$(CEND)\n'
P_OBJ=@printf '$(CCYAN)%s$(CEND)\n'
P_TOOL=@printf '$(CYELLOW)%s$(CEND)\n'
P_EXT=@printf '$(CEXTERN)'
P_END=@printf '$(CEND)'

# Compiler
CC=gcc

# Directories
SRC=src
OBJ=obj
BIN=bin
BIN_T=$(BIN)/tests
INC=include
EXT=external
LIBS=libs
#DBF=db
CONFIGF=config
TOOLS=$(SRC)/tools
TESTS=$(SRC)/tests
OBJ_TO=$(OBJ)/tools
OBJ_TE=$(OBJ)/tests
OBJ_S=$(OBJ)/servers

# Programs
SAPP=sf_app
SDB=sf_db
SREADER=sf_reader

# External libs
EXT_L=curl yaml openssl zlib openldap cjson sqlite
LCJSON=$(OBJ_TO)/cjson.o
LCURL=$(LIBS)/libcurl.a
LOPENSSL=$(LIBS)/libssl.a
LSQLITE=$(LIBS)/libsqlite3.a
LCRYPTO=$(LIBS)/libcrypto.a
LLBER=$(LIBS)/liblber.a
LZ=$(LIBS)/libz.a
LYAML=$(LIBS)/libyaml.a
ifeq (${ENABLE_LDAP},YES)
	LDAP=$(LIBS)/libldap.a
	LDAP_FLAG="--enable-ldap"
	LDAP_LIBS="-lldap -llber"
else
	LDAP_FLAG="--disable-ldap"
endif
ifeq (${ENABLE_CAM},YES)
	ZBAR=$(LIBS)/libzbar.a
	EXT_L+= zbar
	EXT_VAR=-DCAMERA_ENABLED
	CAMERA_CFLAG=-I$(EXT)/zbar/include
	ZBAR_LIB=-lzbar -lX11 -lXv -ljpeg#-lGL -lGLU
endif
EXT_D := $(EXT_L:%=$(EXT)/%)

# Flags
SF_CFLAGS= -I$(INC) -I$(EXT)/yaml/include -I$(EXT)/curl/include/curl -I$(EXT)/cjson -I$(EXT)/sqlite $(CAMERA_CFLAG)
SF_LDFLAGS=-L$(LIBS) -lyaml -lpthread -ldl
CURL_LDFLAGS=-lcurl -lz $(LDAP_LIBS) -lssl -lcrypto $(ZBAR_LIB)

# Auxiliary variables
PROJECT_PATH := $(shell pwd)
OPENSSL_PATH=$(PROJECT_PATH)/$(EXT)/openssl
SCHEMA_LOCATION=./database.sql
DB_LOCATION="./sf.db"
EXT_VAR+= -DSCHEMA_LOCATION=\"$(SCHEMA_LOCATION)\" -DDB_LOCATION=\"$(DB_LOCATION)\"

BINARIES=$(BIN)/$(SAPP) $(BIN)/$(SDB) $(BIN)/$(SREADER) #$(EXT_L)

ifeq (${TEST},YES)
	BINARIES+=$(BIN_T)/log_test
	BINARIES+=$(BIN_T)/yaml_test
	BINARIES+=$(BIN_T)/bar_test
	BINARIES+=$(BIN_T)/curl_test
	#BINARIES+=$(BIN_T)/multi_curl
endif

ifeq (${DEBUG},YES)
	SF_CFLAGS+=-g
endif

ifdef PARALLEL_DEPS
	P_DEPS=-j$(PARALLEL_DEPS)
endif

default: install $(EXT) $(BINARIES)
	$(P_SUCCESS) "Everything has been compiled correctly."

$(EXT): $(EXT_D) $(LCURL) $(LYAML) $(LSQLITE) $(ZBAR)
	$(P_END)

$(EXT)/%:
	$(P_COMP) "############ Deploying $@ ############"
	$(P_EXT)
	cd $(EXT) && tar -zxf $(patsubst $(EXT)/%,%,$@.tar.gz)

########### Bin
$(BIN)/$(SAPP): $(OBJ_S)/$(SAPP).o
	$(P_COMP) "############ Compiling the application server ############"
	$(CC) -o $@ $^ $(SF_LDFLAGS)

$(BIN)/$(SDB): $(OBJ_S)/$(SDB).o $(OBJ_TO)/log.o $(OBJ_TO)/config.o $(OBJ_TO)/log.o $(OBJ)/db_schema.o $(OBJ_TO)/product_decoder.o $(LCJSON)
	$(P_COMP) "############ Compiling the database server ############"
	$(CC) -o $@ $^ -lsqlite3 -lm $(SF_LDFLAGS)

$(BIN)/$(SREADER): $(OBJ_S)/$(SREADER).o $(OBJ_TO)/log.o $(OBJ_TO)/config.o $(OBJ_TO)/bar.o $(OBJ_TO)/queue.o $(OBJ_TO)/downloader.o $(OBJ_TO)/product_decoder.o $(LCJSON)
	$(P_COMP) "############ Compiling the reader server ############"
	$(CC) -o $@ $^ $(CURL_LDFLAGS) $(SF_LDFLAGS)

$(BIN_T)/%: $(OBJ_TE)/%.o $(OBJ_TO)/log.o $(OBJ_TO)/config.o $(OBJ_TO)/bar.o
	$(P_COMP) "############ Compiling $(patsubst $(BIN_T)/%,%,$@) ############"
	$(CC) -o $@ $^  $(CURL_LDFLAGS) $(SF_LDFLAGS)

########### Obj
$(OBJ_S)/%.o: $(SRC)/%.c
	$(P_OBJ) "############ Generating $(patsubst $(OBJ_S)/sf_%.o,%,$@) server object ############"
	$(CC) -c $(SF_CFLAGS) $(EXT_VAR) -o $@ $^

$(OBJ_TO)/%.o: $(TOOLS)/%.c
	$(P_OBJ) "############ Generating $(patsubst $(OBJ_TO)/%.o,%,$@) object ############"
	$(CC) -c $(SF_CFLAGS) -o $@ $^

$(OBJ_TE)/%.o: $(TESTS)/%.c
	$(P_OBJ) "############ Generating $(patsubst $(OBJ_TE)/%.o,%,$@) object ############"
	$(CC) -c $(SF_CFLAGS) -o $@ $^

$(OBJ)/db_schema.o: $(SCHEMA_LOCATION)
	echo 'const char *database_schema = "'"`cat $< | tr -d \"\n\"`"'";' | gcc -xc -c -o $@ -

########### Libraries
$(ZBAR):
	@cd $(EXT)/zbar && ./configure --enable-static CFLAGS="" # --without-python --without-qt --without-gtk --without-jpeg --disable-video  --without-x
	@cd $(EXT)/zbar && make $(P_DEPS)
	@cp $(EXT)/zbar/zbar/.libs/libzbar.a $@

$(LDAP):
	@cd $(EXT)/openldap && ./configure # Protocols still need to be reduced
	@cd $(EXT)/openldap && make depend
	@cd $(EXT)/openldap && make $(P_DEPS)
	@cp $(EXT)/openldap/libraries/libldap/.libs/libldap.a $@
	@cp $(EXT)/openldap/libraries/liblber/.libs/liblber.a $(LLBER)

$(LZ):
	@cd $(EXT)/zlib && ./configure # Protocols still need to be reduced
	@cd $(EXT)/zlib && make $(P_DEPS)
	@cp $(EXT)/zlib/libz.a $@

$(LOPENSSL): $(LDAP) $(LZ)
	@cd $(EXT)/openssl && ./config # Protocols still need to be reduced
	@cd $(EXT)/openssl && make $(P_DEPS)
	@cp $(EXT)/openssl/libcrypto.a $(LCRYPTO)
	@cp $(EXT)/openssl/libssl.a $@

$(LCURL): $(LOPENSSL)
	@cd $(EXT)/curl && CPPFLAGS="-I$(OPENSSL_PATH)/include" LDFLAGS="-L$(OPENSSL_PATH)" LIBS="-ldl -lpthread" ./configure --enable-static $(LDAP_FLAG) #--with-ssl=/home/user/proyectos/TFG/nodos/newinteledison/smart-fridge/libs --with-zlib=/home/user/proyectos/TFG/nodos/newinteledison/smart-fridge/libs # Protocols still need to be reduced
	@cd $(EXT)/curl && make $(P_DEPS)
	@cp $(EXT)/curl/lib/.libs/libcurl.a $@

$(LYAML):
	@cd $(EXT)/yaml && ./configure # Protocols still need to be reduced
	@cd $(EXT)/yaml && make $(P_DEPS)
	@cp $(EXT)/yaml/src/.libs/libyaml.a $@

$(LSQLITE):
	@cd $(EXT)/sqlite && ./configure
	@cd $(EXT)/sqlite && make $(P_DEPS)
	@cp $(EXT)/sqlite/.libs/libsqlite3.a $@

$(LCJSON):
	$(CC) -c -o $@ $(EXT)/cjson/cJSON.c

########### Tools
DB:
	$(EXT)/sqlite/sqlite3 $(DB_LOCATION)

install:
	@mkdir -p $(BIN_T) $(OBJ_S) $(OBJ_TO) $(OBJ_TE) $(LIBS)

full_clean: clean_external
	$(P_TOOL) "Cleaning all..."
	@rm -rf $(BIN) $(OBJ) $(LIBS)

clean: clean_obj clean_binaries

clean_external:
	$(P_TOOL) "Cleaning external files..."
	@rm -rf $(EXT_D) $(EXT)/zbar

clean_libs:
	$(P_TOOL) "Cleaning libraries..."
	@rm -f $(LIBS)/*

clean_obj:
	$(P_TOOL) "Cleaning obj files..."
	@find . -type f -name '*.o' -delete

clean_binaries:
	$(P_TOOL) "Cleaning binaries..."
	@rm -f $(BIN)/$(SAPP) $(BIN)/$(SREADER) $(BIN)/$(SDB) $(BIN_T)/*

zip:
	$(P_TOOL) "Compressing..."
	@mkdir $(PROJECT_NAME)
	@cp -R $(SRC) $(PROJECT_NAME)
	@cp -R $(INC) $(PROJECT_NAME)
	@cp -R $(EXT) $(PROJECT_NAME)
	@cp -R $(CONFIGF) $(PROJECT_NAME)
	@cp -R $(DBF) $(PROJECT_NAME)
	@cp makefile $(PROJECT_NAME)
	@cp install.sh $(PROJECT_NAME)
	@zip -r $(PROJECT_NAME).zip $(PROJECT_NAME)/*
	@rm -rf $(PROJECT_NAME)
