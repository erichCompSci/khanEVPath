OBJDIR = build
SRCDIR = src
BINDIR = bin

SERVER_SRCS  = $(SRCDIR)/khan.cpp \
							 $(SRCDIR)/fuse_helper.cpp \
							 $(SRCDIR)/data_analytics.cpp \
							 $(SRCDIR)/localizations.cpp \
							 $(SRCDIR)/redis.cpp \
               $(SRCDIR)/utils.cpp \
               $(SRCDIR)/database.cpp \
               $(SRCDIR)/fileprocessor.cpp \
               $(SRCDIR)/threadpool.c \
							 $(SRCDIR)/stopwatch.cpp \
							 $(SRCDIR)/measurements.cpp \
               $(SRCDIR)/storage_stone.cpp \
							 $(SRCDIR)/dfg_functions.cpp
               

DFG_STORE_SRCS = $(SERVER_SRCS) \
                    $(SRCDIR)/dfg_general_client.cpp \
                            $(SRCDIR)/readConfig.cpp \
                            $(SRCDIR)/cfgparser.cpp \
                            $(SRCDIR)/configwrapper.cpp 

DFG_STORE_OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(DFG_STORE_SRCS))

DFG_MASTER_SRCS = $(SERVER_SRCS) \
                  $(SRCDIR)/dfg_master.cpp \
                  $(SRCDIR)/cfgparser.cpp \
                  $(SRCDIR)/configwrapper.cpp \
                  $(SRCDIR)/readConfig.cpp 

DFG_MASTER_OBJS  = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(DFG_MASTER_SRCS))

FS_CLIENT_SRCS = $(SRCDIR)/fs_client.cpp \
								 $(SRCDIR)/fuseapi.cpp \
								 $(SRCDIR)/fuse_helper.cpp \
								 $(SRCDIR)/database.cpp \
								 $(SRCDIR)/utils.cpp \
								 $(SRCDIR)/localizations.cpp \
								 $(SRCDIR)/redis.cpp \
								 $(SRCDIR)/khan.cpp \
								 $(SRCDIR)/stopwatch.cpp \
								 $(SRCDIR)/measurements.cpp \
								 $(SRCDIR)/fileprocessor.cpp

FS_CLIENT_OBJS  = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(FS_CLIENT_SRCS))

CCX = g++
CCXFLAGS = -Wall -D_FILE_OFFSET_BITS=64 -Wno-write-strings -g

EVPATH_LIB_DIRS = -L/net/hp41/chaos/rhe6-64/lib \
                  -Wl,-rpath=/net/hp41/chaos/rhe6-64/lib

EVPATH_INCLUDE_DIRS = -I/net/hp41/chaos/rhe6-64/include

REDIS_LIB_DIRS  = -L$(PWD)/hiredis -Wl,-rpath=$(PWD)/hiredis 
REDIS_INCLUDE_DIRS=-I$(PWD)/hiredis

PYTHON_INCLUDE_DIRS = -I/usr/include/python2.6
PYTHON_LIB_DIRS = -L/usr/lib64 -Wl,-rpath=/usr/lib64
# -I/net/hp100/elohrman/Python-2.7.11/Include 
# -L -Wl,-rpath=/net/hp100/elohrman/Python-2.7.11/Lib
EVPATH_LIBS  = -latl -levpath -lffs -ldill -lcercs_env -lrt
PYTHON_LIBS  = -lpython2.6
REDIS_LIBS   = -lhiredis
PTHREAD_LIBS = -lpthread
CURL_LIBS = 
#-lcurl

FUSE_LIBS = `pkg-config fuse --cflags --libs`

DFG_MASTER = dfg_master
DFG_STORE = dfg_general_client
FS_CLIENT = fs_client

all: builddir bindir $(DFG_STORE) $(DFG_MASTER) $(FS_CLIENT)
  
builddir:
	mkdir -p $(OBJDIR)

bindir:
	mkdir -p $(BINDIR)

$(SERVER): $(SERVER_OBJS)
	$(CCX) $(SERVER_OBJS) $(EVPATH_LIB_DIRS) $(REDIS_LIB_DIRS) $(PYTHON_LIB_DIRS) \
  -o $(BINDIR)/$@ $(EVPATH_LIBS) $(PYTHON_LIBS) $(REDIS_LIBS) $(PTHREAD_LIBS) $(CURL_LIBS)

$(DFG_STORE): $(DFG_STORE_OBJS)
	$(CCX) $(CCXFLAGS) $(DFG_STORE_OBJS) $(EVPATH_LIB_DIRS) $(REDIS_LIB_DIRS) $(PYTHON_LIB_DIRS) -o $(BINDIR)/$@ $(EVPATH_LIBS) \
    $(REDIS_LIBS) $(PYTHON_LIBS) $(PTHREAD_LIBS) $(CURL_LIBS)

$(DFG_MASTER): $(DFG_MASTER_OBJS)
	$(CCX) $(CCXFLAGS) $(DFG_MASTER_OBJS) $(EVPATH_LIB_DIRS) $(REDIS_LIB_DIRS) $(PYTHON_LIB_DIRS) -o $(BINDIR)/$@ $(EVPATH_LIBS) \
    $(REDIS_LIBS) $(PYTHON_LIBS) $(PTHREAD_LIBS) $(CURL_LIBS)

$(FS_CLIENT): $(FS_CLIENT_OBJS)
	$(CCX) $(CCXFLAGS) $(FS_CLIENT_OBJS) $(EVPATH_LIB_DIRS) $(PYTHON_LIB_DIRS) $(REDIS_LIB_DIRS) \
  -o $(BINDIR)/$@ $(FUSE_LIBS) $(REDIS_LIBS) $(PYTHON_LIBS) 

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CCX) $(CCXFLAGS) $(EVPATH_INCLUDE_DIRS) $(PYTHON_INCLUDE_DIRS) $(REDIS_INCLUDE_DIRS) $(OPTS) -c $< -o $@

clean:
	rm $(SERVER) $(CLIENT) $(DFG_MASTER) $(OBJDIR) $(FS_CLIENT) $(BINDIR) -Rf

