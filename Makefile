FILES = src/
INCLUDE = include/
OUT	= sniffer
CC	 = g++
FLAGS = -O3 -g -std=c++17 -o

all: $(OBJS) $(FILES)main.cpp $(INCLUDE)utils.hpp 
	$(CC) $(FLAGS) $(OUT) $(FILES)main.cpp
