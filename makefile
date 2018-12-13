compile:
	gcc -o control sem_control.c
	gcc -o play sem_game.c

control:
	./setup $(args)

run:
	./play

clean:
	-rm -rf control play

all:
	make clean
	make compile
	./setup -r
	./setup -c
	make run
