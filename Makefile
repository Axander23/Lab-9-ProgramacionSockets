CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -pthread

all: server_exec client_exec

server_exec: server/server.cpp
	$(CXX) $(CXXFLAGS) -o server/server server/server.cpp

client_exec: client/client.cpp
	$(CXX) $(CXXFLAGS) -o client/client client/client.cpp

clean:
	rm -f server/server client/client
