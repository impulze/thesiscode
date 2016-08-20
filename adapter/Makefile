CXXFLAGS = -ggdb3 -std=c++11 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -Iinclude
#BASE_OBJECTS = wrap_communication.o wrap_protocol.o wrap_mmictrl.o misc.o
BASE_OBJECTS = adapter.o
BASE_OBJECTS_PATHS := $(addprefix objs/, $(BASE_OBJECTS))
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

all: libs

libs: lib/libadapter.a

#lib/libadapter.so: $(BASE_OBJECTS_PATHS)
#	$(LINK.o) -shared -o $@ $^


lib/libadapter.a: $(BASE_OBJECTS_PATHS)
	$(AR) rcs $@ $^

objs/%.o: %.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

client: $(BASE_OBJECTS_PATHS) objs/client.o

objs/adapter.o: adapter.cpp
