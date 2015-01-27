CFLAG= -lm
DFLAG= -g
CC= gcc

report_switching: main.c transition.o read_fsm.o matrix_util.o global.h struct.h
	$(CC) -o report_switching main.c transition.o read_fsm.o matrix_util.o $(CFLAG) $(DFLAG)

transition.o: matrix_util.o global.h struct.h
	$(CC) -c transition.c $(DFLAG)

matrix_util.o: matrix_util.c global.h
	$(CC) -c matrix_util.c $(DFLAG)

read_fsm.o: read_fsm.c global.h struct.h
	$(CC) -c read_fsm.c $(DFLAG)

clean:
	\rm -f *.o report_switching