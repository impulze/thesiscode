CXXFLAGS = -ggdb3 -std=c++11 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -Iinclude
BASE_OBJECTS = wrap_communication.o wrap_protocol.o wrap_mmictrl.o misc.o adapter.o
BASE_OBJECTS_PATHS := $(addprefix objs/, $(BASE_OBJECTS))
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

all: libs

objs:
	mkdir objs

objs/%.o: %.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

client: $(BASE_OBJECTS_PATHS) objs/client.o
libs: lib/libadapter.so

lib/libadapter.so: $(BASE_OBJECTS_PATHS)
	$(LINK.o) -shared -o $@ $^

objs/adapter.o: include/adapter/adapter.h wrap_mmictrl.h
objs/wrap_mmictrl.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h
objs/wrap_communication.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h misc.h
objs/wrap_protocol.o: wrap_protocol.h
objs/misc.o: misc.h
objs/client.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h
objs/server.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h
