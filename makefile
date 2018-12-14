all: sem_control.o sem_game.o
	gcc sem_control.o -o control
	gcc sem_game.o

sem_control.o:
	gcc -c sem_control.c

sem_game.o:
	gcc -c sem_game.c

setup:
	./control $(args)

run:
	./a.out

clean:
	rm -rf *.o ./a.out control *.txt
	