CC = g++
FLAGS = -std=c++17 -Wall -Wextra #-Wno-missing-field-initializers
LIB = raw_sockets.hpp kermit.hpp pacman.hpp logging.hpp client.hpp server.hpp
SRC = raw_sockets.cpp kermit.cpp pacman.cpp logging.cpp client.cpp server.cpp
MAIN = main.cpp
OBJ = raw_sockets.o kermit.o pacman.o logging.o server.o client.o main.o
TARGET = blind_pacman
TEST = pacmanTest client
TESTFILES = *_received.txt *_received.jpg *_received.mp4 *.log

all: compile
	$(CC) $(OBJ) -o $(TARGET)

pacman:
	$(CC)  $(OBJ) pacmanTest.cpp -o pacmanTest 

compile: $(SRC) $(LIB)
	$(CC) $(FLAGS) -c $(SRC) $(MAIN)

clean purge: 
	rm -f $(OBJ) $(TARGET) $(TEST) $(TESTFILES)
