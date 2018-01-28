  # the compiler: gcc for C program, define as g++ for C++
  CC = gcc

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -lpthread -Wall

  # the build target executable:

  all: servidor cliente

  servidor: servidor.c
	$(CC) $(CFLAGS) -o servidor servidor.c
	
  cliente: cliente.c
	$(CC) $(CFLAGS) -o cliente cliente.c

  clean:
	rm servidor cliente
