CFLAG= -lm
DFLAG= -g
CC= gcc

bf_encode: main.c encode.o transition.o read_fsm.o matrix_util.o global.h struct.h
	$(CC) -o bf_encode main.c encode.o transition.o read_fsm.o matrix_util.o $(CFLAG) $(DFLAG)

encode.o: encode.c transition.o global.h struct.h pow3_struct.h
	$(CC) -c encode.c $(DFLAG)

transition.o: matrix_util.o global.h struct.h
	$(CC) -c transition.c $(DFLAG)

matrix_util.o: matrix_util.c global.h
	$(CC) -c matrix_util.c $(DFLAG)

read_fsm.o: read_fsm.c global.h struct.h
	$(CC) -c read_fsm.c $(DFLAG)

clean:
	\rm -f *.o pow3