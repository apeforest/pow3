#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "global.h"
#include "fsm.h"

extern boolean get_switching_activity(fsm_t *fsm, double *total_sw, boolean print_prob);

int main(int argc, char **argv)
{
  fsm_t *fsm;
  char *infile_name;
  double switching = 0;
  
  infile_name = argv[1];
  
  init_fsm();  
  fsm = get_fsm();

  if(read_fsm_from_blif(infile_name, fsm) == FALSE) {
    printf("ERROR: Unable to read FSM from the input blif file.\n");
    free_fsm();
    exit(1);
  }

  print_fsm(fsm);

  if(get_switching_activity(fsm, &switching, TRUE) == FALSE)
    exit(1);

  printf("\n");
  printf("-----------------------------------------------\n");
  printf("Total switching activity: %.2f\n", switching);
  printf("-----------------------------------------------\n");

  return 0;
}
