CXXFLAGS = -ggdb3 -std=c++11
BASE_OBJECTS = wrap_communication.o wrap_protocol.o wrap_mmictrl.o

LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

all: client

server: $(BASE_OBJECTS) server.o
client: $(BASE_OBJECTS) client.o
