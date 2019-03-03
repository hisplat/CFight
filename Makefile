
CXX	:= clang++
CC	:= clang
LD	:= clang++

# CXX	:= g++
# LD	:= g++

AR	:= ar
CXXFLAGS := -Wall -Werror -O2 -g -Wunused -Wunreachable-code -Wno-unused-parameter -Wno-unused-result -Wno-deprecated -fPIC -std=c++11 
CFLAGS := -Wall -Werror -O2 -g -Wunused -Wunreachable-code -Wno-unused-parameter -Wno-unused-result -Wno-deprecated -fPIC -std=c99
INCLUDES = -Icommon -I.

LINKS	= -L. -ldl -rdynamic -lrt
LIBS	= -lpthread -lrt

common_src_files := common/logging.cpp
common_src_files += common/socket.cpp
common_src_files += common/buffer.cpp
COMMON_OBJS := $(common_src_files:.cpp=.o)
COMMON_DEPS := $(common_src_files:.cpp=.h)

server_src_files := server/main.c
server_src_files += server/game.c
server_src_files += server/client.c
server_src_files += server/player.c
SERVER_OBJS := $(server_src_files:.c=.o)
SERVER_DEPS := $(server_src_files:.c=.d)


all: cfserver

libcommon.a: $(COMMON_OBJS)
	@echo Linking $@ ...
	ar rv libcommon.a $(COMMON_OBJS)
	@echo -------------------------------------------
	echo done.

cfserver: $(SERVER_OBJS) libcommon.a
	@echo Linking $@ ...
	$(LD) $(SERVER_OBJS) $(LINKS) $(LIBS) libcommon.a -o$@
	@echo -------------------------------------------
	@echo done.

.cpp.o:
	@echo Compling $@ ...
	$(CXX) -c $< $(INCLUDES) $(CXXFLAGS)  -o $@
	@echo -------------------------------------------

.c.o:
	@echo Compling $@ ...
	$(CC) -c $< $(INCLUDES) $(CFLAGS)  -o $@
	@echo -------------------------------------------

%.d:%.cpp
	$(CXX) -MM $< $(INCLUDES) $(CXXFLAGS) -o $@

%.d:%.c
	$(CC) -MM $< $(INCLUDES) $(CXXFLAGS) -o $@

prebuild:
	#echo -------

# include $(COMMON_DEPS)
# include $(SERVER_DEPS)

clean:
	rm -fr $(SERVER_OBJS) $(SERVER_DEPS) cfserver libcommon.a

