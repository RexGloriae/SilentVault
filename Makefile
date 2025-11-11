CXX = g++
FLAGS = -Wall -g -I./src/PythonWrapper/python3 -I./src/PythonWrapper/python3/include -std=c++17
PYFLAGS = -lpython3.12
LDFLAGS = -L/usr/local/openssl/lib -lssl -lcrypto

CLIENT_DIR = src/Client/src
SERVER_DIR = src/Server/src
PYWRAPPER_DIR = src/PythonWrapper

CLIENT_BIN = src/Client/bin/client
SERVER_BIN = src/Server/bin/server

CLIENT_SRCS = $(wildcard $(CLIENT_DIR)/*.cpp)
SERVER_SRCS = $(wildcard $(SERVER_DIR)/*.cpp)
PYWRAPPER_SRCS = $(wildcard $(PYWRAPPER_DIR)/*.cpp)

CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
PYWRAPPER_OBJS = $(PYWRAPPER_SRCS:.cpp=.o)

.PHONY: all clean

all: client server

client: $(CLIENT_OBJS) $(PYWRAPPER_OBJS)
	$(CXX) -o $(CLIENT_BIN) $^ $(PYFLAGS) $(LDFLAGS)

server: $(SERVER_OBJS) $(PYWRAPPER_OBJS)
	$(CXX) -o $(SERVER_BIN) $^ $(PYFLAGS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(FLAGS) -c $< -o $@

clean_objs:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(PYWRAPPER_OBJS)

clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(PYWRAPPER_OBJS) $(CLIENT_BIN) $(SERVER_BIN)