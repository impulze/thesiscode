ADAPTER_DIR = ../adapter
CXXFLAGS = -ggdb3 -std=c++11 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I$(ADAPTER_DIR)/include -Iinclude
BASE_OBJECTS = wrap_communication.o wrap_protocol.o wrap_mmictrl.o misc.o wrapper.o
BASE_OBJECTS_PATHS := $(addprefix objs/, $(BASE_OBJECTS))
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

all: libs

libs: lib/libwrapper.a

lib/libwrapper.a: $(BASE_OBJECTS_PATHS)
	$(AR) rcs $@ $^

client: $(BASE_OBJECTS_PATHS) objs/main.o objs/client.o $(ADAPTER_DIR)/lib/libadapter.a
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

objs/%.o: %.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

objs/wrap_mmictrl.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h
objs/wrap_communication.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h misc.h
objs/wrap_protocol.o: wrap_protocol.h
objs/misc.o: misc.h
objs/client.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h
objs/server.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h
