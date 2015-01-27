CFLAG= -lm
DFLAG= -g
CC= gcc

optimize: fsm2verilog.c read_fsm.o global.h struct.h fsm.h
	$(CC) -o fsm2v fsm2verilog.c read_fsm.o $(CFLAG) $(DFLAG)

read_fsm.o: read_fsm.c global.h struct.h
	$(CC) -c read_fsm.c $(DFLAG)

clean:
	rm -rf *.o fsm2v