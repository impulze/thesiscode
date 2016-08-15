CXXFLAGS = -ggdb3 -std=c++11
BASE_OBJECTS = wrap_communication.o wrap_protocol.o

LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

all: server client

server: $(BASE_OBJECTS) server.o
client: $(BASE_OBJECTS) client.o
