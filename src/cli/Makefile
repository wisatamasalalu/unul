CC = gcc
CFLAGS = -Wall -Wextra -g

# Target eksekusi di folder bin
TARGET = ../../bin/unul

# LIBS harus diletakkan terpisah di akhir agar kompilasi di Linux modern tidak rewel
LIBS = -lm -lreadline -pthread -lcurl

# Semua file dari berbagai dimensi
SRCS = main.c \
       tui_renderer.c \
       ../core_c/enki_os.c \
       ../core_c/enki_lexer.c \
       ../core_c/enki_parser.c \
       ../core_c/enki_interpreter.c \
       ../core_c/enki_scheduler.c \
       ../core_c/enki_network.c \
       ../core_c/enki_file_system.c \
       ../core/enki_object.c \
       ../core/array/ku.c \
       ../core/array/ko.c \
       ../core/array/panjang.c \
       ../otim/otim_lexer.c \
       ../otim/otim_parser.c \
       ../snul/snul_lexer.c \
       ../snul/snul_parser.c

build:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)