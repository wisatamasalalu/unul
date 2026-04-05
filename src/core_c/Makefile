CC = gcc
# Flag -g untuk mode debug, -Wall untuk memunculkan semua peringatan
CFLAGS = -Wall -Wextra -g

# Target eksekusi akan langsung diletakkan di folder bin!
TARGET = ../../bin/unul -lm -lreadline -pthread -lcurl

# Semua file dapur mesin C kita
SRCS = main.c enki_os.c enki_lexer.c enki_parser.c enki_interpreter.c enki_scheduler.c enki_network.c enki_file_system.c \
       ../core/enki_object.c \
       ../core/array/ku.c \
       ../core/array/ko.c \
       ../core/array/panjang.c

build:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)