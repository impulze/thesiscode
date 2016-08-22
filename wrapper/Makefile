ADAPTER_DIR = ../adapter
ADAPTER_LDFLAGS = -L$(ADAPTER_DIR)/lib
ADAPTER_INCLUDE = -I$(ADAPTER_DIR)/include
ADAPTER_LIBS = -ladapter
CXXFLAGS = -ggdb3 -std=c++11 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -Iinclude -fmax-errors=10 -Wall
CXXFLAGS += $(ADAPTER_INCLUDE)
LDFLAGS = -pthread
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

SOURCES_BASE := \
wrap_protocol.cpp \
wrap_misc.cpp \
wrap_mmictrl.cpp \
wrap_network.cpp \
wrap_client.cpp \
wrap_server.cpp \
wrap_mmictrl_remote.cpp

SOURCES_LIB := \
$(SOURCES_BASE) \
wrapper.cpp

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
OBJECTS_LIB := $(patsubst %.cpp, objs/%.o, $(SOURCES_LIB))
OBJECTS_CLIENT := $(patsubst %.cpp, objs/%.o, $(SOURCES_CLIENT))
OBJECTS_SERVER := $(patsubst %.cpp, objs/%.o, $(SOURCES_SERVER))
OBJECTS_MMICTRL_REMOTE := $(patsubst %.cpp, objs/%.o, $(SOURCES_MMICTRL_REMOTE))

SOURCES := $(SOURCES_CLIENT) $(SOURCES_SERVER) $(SOURCES_MMICTRL_REMOTE)
DEPFILES := $(patsubst %.cpp, %.d, $(SOURCES))

NODEPS := clean
SUFFIXES += .d

all: example_client example_server example_mmictrl_remote libs

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    -include $(DEPFILES)
endif

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.cpp, objs/%.o, $<)' $< -MF $@

objs/%.o: %.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

libs: lib/libwrapper.a

lib/libwrapper.a: $(OBJECTS_LIB)
	$(AR) rcs $@ $^

example_server: $(OBJECTS_SERVER)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

example_client: $(OBJECTS_CLIENT)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

example_mmictrl_remote: $(OBJECTS_MMICTRL_REMOTE)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(OBJECTS_SERVER) $(OBJECTS_CLIENT)
