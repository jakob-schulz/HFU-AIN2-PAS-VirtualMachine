all: helloworld2
helloworld2: helloworld2.o increase.o twice.o
	${CC} helloworld2.o increase.o twice.o -o hello2
helloworld2.o: helloworld2.c
	${CC} -c helloworld2.c
increase.o: increase.c
	${CC} -c increase.c
twice.o: twice.c
	${CC} -c twice.c
