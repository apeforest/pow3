#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "global.h"
#include "fsm.h"

extern boolean write_fsm_to_blif_by_index(char *file_name, fsm_t *fsm);
extern boolean encode_brute_force(fsm_t *fsm);

int main(int argc, char **argv)
{
  fsm_t *fsm;
  char *infile_name;
  char *outfile_name;
  char *temp_name;
  double switching = 0;
  
  infile_name = argv[1];


  init_fsm();  
  fsm = get_fsm();

  if(read_fsm_from_blif(infile_name, fsm) == FALSE) {
    printf("ERROR: Unable to build FSM for %s.\n", fsm->name);
    free_fsm();
    exit(1);
  }

  //print_fsm(fsm);

  printf("Begin encoding for %s\n", fsm->name);  
  if(encode_brute_force(fsm) == FALSE)
    exit(1);

  temp_name = get_name_without_suffix(infile_name, ".kiss2");
  outfile_name = (char *)calloc(strlen(temp_name) + 6, sizeof(char));
  sprintf(outfile_name, "%s.blif", temp_name);

  write_fsm_to_blif_by_index(outfile_name, fsm);

  free(temp_name);
  free(outfile_name);
  free(fsm);

  return 0;
}
