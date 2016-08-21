ADAPTER_DIR = ../adapter
CXXFLAGS = -ggdb3 -std=c++11 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -I$(ADAPTER_DIR)/include -Iinclude -fmax-errors=10 -Wall
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

SOURCES_BASE := \
wrap_protocol.cpp \
wrap_misc.cpp \
wrap_mmictrl.cpp \
wrap_network.cpp \
wrap_client.cpp \
wrap_server.cpp \
wrap_mmictrl_remote.cpp

SOURCES_CLIENT := \
$(SOURCES_BASE) \
example_client.cpp

SOURCES_SERVER := \
$(SOURCES_BASE) \
example_server.cpp

SOURCES_MMICTRL_REMOTE := \
$(SOURCES_BASE) \
example_mmictrl_remote.cpp

OBJECTS_BASE := $(patsubst %.cpp, objs/%.o, $(SOURCES_BASE))
OBJECTS_CLIENT := $(patsubst %.cpp, objs/%.o, $(SOURCES_CLIENT))
OBJECTS_SERVER := $(patsubst %.cpp, objs/%.o, $(SOURCES_SERVER))
OBJECTS_MMICTRL_REMOTE := $(patsubst %.cpp, objs/%.o, $(SOURCES_MMICTRL_REMOTE))

SOURCES := $(SOURCES_CLIENT) $(SOURCES_SERVER) $(SOURCES_MMICTRL_REMOTE)
DEPFILES := $(patsubst %.cpp, %.d, $(SOURCES))

NODEPS := clean
SUFFIXES += .d

all: example_client example_server example_mmictrl_remote

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    -include $(DEPFILES)
endif

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.cpp, objs/%.o, $<)' $< -MF $@

objs/%.o: %.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

#all: libs

#libs: lib/libwrapper.a

lib/libwrapper.a: $(BASE_OBJECTS_PATHS)
	$(AR) rcs $@ $^

example_server: $(OBJECTS_SERVER)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

example_client: $(OBJECTS_CLIENT)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

example_mmictrl_remote: $(OBJECTS_MMICTRL_REMOTE)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(OBJECTS_SERVER) $(OBJECTS_CLIENT)
