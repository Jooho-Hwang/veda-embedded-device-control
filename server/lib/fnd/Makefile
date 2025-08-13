CC = gcc

CFLAGS = -std=c99 -fPIC -pthread -Wall -Wextra -Werror -D_GNU_SOURCE \
         -Wpedantic -Wconversion -Wshadow -Wformat=2 -Wcast-align \
         -Wpointer-arith -Winit-self -Wuninitialized -Wunused \
         -Wstrict-overflow -Wstack-protector -fstack-protector-strong

LDFLAGS = -shared -lwiringPi -lpthread

INCLUDE_DIR = ../../include

SRC = fnd.c
TARGET = libfnd.so

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
