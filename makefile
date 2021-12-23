C = gcc
G = g++
CFLAGS = -Wall -O -g
TARGET = ./main
 
%.o:%.c
	$(C) $(CFLAGS) -c $< -o $@
 
%.o:%.cpp
	$(G) $(CFLAGS) -c $< -o $@
 
SOURCES = $(wildcard *.c *.cpp)
 
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
 
$(TARGET):$(OBJS)
	$(G) $(OBJS) -o $(TARGET)
	chmod a+x $(TARGET)
 
clean:
	rm -rf *.o main
