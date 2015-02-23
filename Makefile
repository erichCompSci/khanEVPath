OBJDIR = build
SRCDIR = src
BINDIR = bin

SERVER_SRCS  = $(SRCDIR)/khan.cpp \
							 $(SRCDIR)/data_analytics.cpp \
							 $(SRCDIR)/localizations.cpp \
							 $(SRCDIR)/fuseapi.cpp \
							 $(SRCDIR)/fuse_helper.cpp \
							 $(SRCDIR)/redis.cpp \
               $(SRCDIR)/utils.cpp \
               $(SRCDIR)/database.cpp \
               $(SRCDIR)/fileprocessor.cpp \
               $(SRCDIR)/threadpool.c \
               $(SRCDIR)/sink.cpp \
							 $(SRCDIR)/cloudupload_v1.c \
							 $(SRCDIR)/cloudupload_supplement.c \
							 $(SRCDIR)/stopwatch.cpp \
							 $(SRCDIR)/measurements.cpp \
							 $(SRCDIR)/dfg_functions.cpp

SERVER_OBJS  = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SERVER_SRCS))

CLIENT_SRCS  = $(SRCDIR)/source.cpp \
							 $(SRCDIR)/dfg_functions.cpp
CLIENT_OBJS  = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(CLIENT_SRCS))

DFG_MASTER_SRCS = $(SRCDIR)/dfg_master.cpp \
									$(SRCDIR)/dfg_functions.cpp

DFG_MASTER_OBJS  = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(DFG_MASTER_SRCS))

CCX = g++
CCXFLAGS = -Wall -D_FILE_OFFSET_BITS=64 -Wno-write-strings

EVPATH_LIB_DIRS = -L/net/hp41/chaos/rhe6-64-icc/atl/lib \
                  -L/net/hp41/chaos/rhe6-64-icc/evpath/lib \
                  -L/net/hp41/chaos/rhe6-64-icc/ffs/lib \
                  -L/net/hp41/chaos/rhe6-64-icc/dill/lib \
                  -L/net/hp41/chaos/rhe6-64-icc/cercs_env/lib \
                  -Wl,-rpath=/net/hp41/chaos/rhe6-64-icc/atl/lib \
                  -Wl,-rpath=/net/hp41/chaos/rhe6-64-icc/ffs/lib \
                  -Wl,-rpath=/net/hp41/chaos/rhe6-64-icc/dill/lib \
                  -Wl,-rpath=/net/hp41/chaos/rhe6-64-icc/cercs_env/lib

EVPATH_INCLUDE_DIRS = -I/net/hp41/chaos/rhe6-64-icc/atl/include \
                      -I/net/hp41/chaos/rhe6-64-icc/ffs/include \
                      -I/net/hp41/chaos/rhe6-64-icc/evpath/include

REDIS_LIB_DIRS  = -L/net/hu21/agangil3/hiredis -Wl,-rpath=/net/hu21/agangil3/hiredis 
REDIS_INCLUDE_DIRS=-I/net/hu21/agangil3/hiredis

PYTHON_INCLUDE_DIRS = -I/usr/include/python2.6 
PYTHON_LIB_DIRS = -L/usr/lib64/python -Wl,-rpath=/usr/lib64

EVPATH_LIBS  = -latl -levpath -lffs -ldill -lcercs_env -lrt
PYTHON_LIBS  = -lpython2.6
REDIS_LIBS   = -lhiredis
PTHREAD_LIBS = -lpthread
CURL_LIBS = -lcurl

FUSE_LIBS = `pkg-config fuse --cflags --libs`

SERVER = net_recv
CLIENT = net_send
DFG_MASTER = dfg_master

all: builddir bindir $(SERVER) $(CLIENT) $(DFG_MASTER)
  
builddir:
	mkdir -p $(OBJDIR)

bindir:
	mkdir -p $(BINDIR)

$(SERVER): $(SERVER_OBJS)
	$(CCX) $(SERVER_OBJS) $(EVPATH_LIB_DIRS) $(REDIS_LIB_DIRS) $(PYTHON_LIB_DIRS) \
  -o $(BINDIR)/$@ $(EVPATH_LIBS) $(PYTHON_LIBS) $(REDIS_LIBS) $(PTHREAD_LIBS) $(FUSE_LIBS) $(CURL_LIBS)

$(CLIENT): $(CLIENT_OBJS)
	$(CCX) $(CCXFLAGS) $(CLIENT_OBJS) $(EVPATH_LIB_DIRS) -o $(BINDIR)/$@ $(EVPATH_LIBS)

$(DFG_MASTER): $(DFG_MASTER_OBJS)
	$(CCX) $(CCXFLAGS) $(DFG_MASTER_OBJS) $(EVPATH_LIB_DIRS) -o $(BINDIR)/$@ $(EVPATH_LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CCX) $(CCXFLAGS) $(EVPATH_INCLUDE_DIRS) $(PYTHON_INCLUDE_DIRS) $(REDIS_INCLUDE_DIRS) $(OPTS) -c $< -o $@

clean:
	rm $(SERVER) $(CLIENT) $(DFG_MASTER) $(OBJDIR) $(BINDIR) -Rf

