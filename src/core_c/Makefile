CC = gcc
# Flag -g untuk mode debug, -Wall untuk memunculkan semua peringatan
CFLAGS = -Wall -Wextra -g

# Target eksekusi akan langsung diletakkan di folder bin!
TARGET = ../../bin/unul -lm -lreadline -pthread

# Semua file dapur mesin C kita
SRCS = main.c enki_lexer.c enki_parser.c enki_interpreter.c

build:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)