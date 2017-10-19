ifeq ($(config),debug)
override config := Debug
endif
ifeq ($(config),release)
override config := Release
endif
ifeq ($(config),Debug)
override CPPFLAGS += -DDEBUG
override CFLAGS += -g -O0
override CXXFLAGS += -g -O0
override LDFLAGS += -g
else ifeq ($(config),Release)
override CPPFLAGS += -DNDEBUG
override CFLAGS += -O2
override CXXFLAGS += -O2
else ifneq (,$(config))
$(warning Unknown configuration "$(config)")
endif

# Use "make RANLIB=''" for platforms without ranlib.
RANLIB ?= ranlib

CC := cc
CXX := cc

# The directory for the build files, may be overridden on make command line.
builddir = .

ifneq ($(builddir),.)
_builddir := $(builddir)/
_builddir_error := $(shell mkdir -p $(_builddir) 2>&1)
$(if $(_builddir_error),$(error Failed to create build directory: $(_builddir_error)))
endif
all: $(_builddir)Server $(_builddir)Client

$(_builddir)Server: $(_builddir)Server_server.o $(_builddir)Server_common.o $(_builddir)Server_list.o $(_builddir)Server_map.o
	$(CXX) -o $@ $(LDFLAGS) $(_builddir)Server_server.o $(_builddir)Server_common.o $(_builddir)Server_list.o $(_builddir)Server_map.o -pthread

$(_builddir)Server_server.o: server/server.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) -MD -MP -pthread server/server.c

$(_builddir)Server_common.o: common/common.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) -MD -MP -pthread common/common.c

$(_builddir)Server_list.o: common/list.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) -MD -MP -pthread common/list.c

$(_builddir)Server_map.o: common/map.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) -MD -MP -pthread common/map.c

$(_builddir)Client: $(_builddir)Client_client.o $(_builddir)Client_common.o $(_builddir)Client_list.o $(_builddir)Client_map.o
	$(CXX) -o $@ $(LDFLAGS) $(_builddir)Client_client.o $(_builddir)Client_common.o $(_builddir)Client_list.o $(_builddir)Client_map.o -pthread

$(_builddir)Client_client.o: client/client.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) -MD -MP -pthread client/client.c

$(_builddir)Client_common.o: common/common.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) -MD -MP -pthread common/common.c

$(_builddir)Client_list.o: common/list.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) -MD -MP -pthread common/list.c

$(_builddir)Client_map.o: common/map.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) -MD -MP -pthread common/map.c

clean:
	rm -f $(_builddir)*.o
	rm -f $(_builddir)*.d
	rm -f $(_builddir)Server
	rm -f $(_builddir)Client

.PHONY: all clean

# Dependencies tracking:
-include $(_builddir)*.d
