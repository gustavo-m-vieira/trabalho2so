# Copiado do arquivo pingExample com pequenas alterações

# the compiler
CC = gcc

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:
TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -pthread -lpthread -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)
