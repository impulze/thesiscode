CXXFLAGS = -ggdb3 -std=c++11 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0
BASE_OBJECTS = wrap_communication.o wrap_protocol.o wrap_mmictrl.o misc.o adapter.o

LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

all: client libs

client: $(BASE_OBJECTS) client.o main.o
libs: libadapter.so

libadapter.so: $(BASE_OBJECTS)
	$(LINK.o) -shared -o $@ $^

adapter.o: adapter.h wrap_mmictrl.h
wrap_mmictrl.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h
wrap_communication.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h misc.h
wrap_protocol.o: wrap_protocol.h
misc.o: misc.h
client.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h
server.o: wrap_communication.h wrap_protocol.h wrap_mmictrl.h
