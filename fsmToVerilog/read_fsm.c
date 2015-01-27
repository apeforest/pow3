#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "global.h"

// global variables 
fsm_t *_kiss_fsm;

/*************** begin forward function proto declaration *************/
boolean read_fsm_from_blif(char *file_name, fsm_t *fsm);
fsm_t *get_fsm();
void init_fsm();
void free_fsm();
void free_state(state_t *state);
state_t *add_state(fsm_t *fsm, char *state_name, int i);
boolean get_state(fsm_t *fsm, char *state_name, state_t **state);
trans_t *add_state_transition(fsm_t *fsm, char *input_str, state_t *current_state, state_t *next_state, char *output_str, int i);
void free_transition(trans_t *transition);
fsm_t *get_fsm();
char *get_name_without_suffix(char *name, char *suffix);
void set_fsm_name(fsm_t *fsm, char *name);
void set_fsm_init_state(fsm_t *fsm, char *state_name);
state_t *get_fsm_init_state(fsm_t *fsm, int *success_flag);
/*************** end forward function proto declaration **************/

void init_fsm()
{
  _kiss_fsm = (fsm_t *)malloc(sizeof(fsm_t));
  _kiss_fsm->name = NULL;
  _kiss_fsm->init_state = NULL;
  _kiss_fsm->num_input = 0;
  _kiss_fsm->num_output = 0;
  _kiss_fsm->num_transition = 0;
  _kiss_fsm->num_state = 0;
  _kiss_fsm->code_length = 0;
  _kiss_fsm->state = NULL;
  _kiss_fsm->transition = NULL;
}

void free_fsm()
{
  int i;

  if(_kiss_fsm->name) 
    free(_kiss_fsm->name);

  if(_kiss_fsm->init_state)
    free(_kiss_fsm->init_state);

  if(_kiss_fsm->state) {
    for(i = 0; i < _kiss_fsm->num_state; i++) {
      free_state(&_kiss_fsm->state[i]);
    }
    _kiss_fsm->state = NULL;
  }

  if(_kiss_fsm->transition) {
    for(i = 0; i < _kiss_fsm->num_transition; i++) {
      free_transition(&_kiss_fsm->transition[i]);
    }
    _kiss_fsm->transition = NULL;
  }

  _kiss_fsm->num_input = 0;
  _kiss_fsm->num_output = 0;
  _kiss_fsm->num_transition = 0;
  _kiss_fsm->num_state = 0;
  _kiss_fsm->code_length = 0;
    
  free(_kiss_fsm);
}

void free_state(state_t *state)
{
  state->index = 0;
  if(state->name)
    free(state->name);
  if(state->code) {
    free(state->code);
    state->code = NULL;
  }
}

state_t *add_state(fsm_t *fsm, char *state_name, int i)
{
  if(i >= fsm->num_state) {
    printf("ERROR: adding state %s exceeds the number of states %d.\n", state_name, fsm->num_state);
    return NULL;
  }
  
  free_state(&fsm->state[i]);
  fsm->state[i].index = i;
  fsm->state[i].name = (char *)calloc(strlen(state_name) + 1, sizeof(char));
  strcpy(fsm->state[i].name, state_name);

  return &(fsm->state[i]);
}

void set_state_code(state_t *state, char *code)
{
  if(state->code)
    free(state->code);
  state->code = (char *)calloc(strlen(code) + 1, sizeof(char));
  strcpy(state->code, code);
}

boolean get_state(fsm_t *fsm, char *state_name, state_t **state)
{
  int i = 0;
  int found_flag = FALSE;

  if(state_name == NULL)
    return FALSE;

  for(i = 0; i < fsm->num_state; i++) {
    if(fsm->state[i].name == NULL)
      continue;
    if(!strcmp(fsm->state[i].name, state_name)) {
      *state = &fsm->state[i];
      found_flag = TRUE;
      break;
    }
  }

  return found_flag;
} 

trans_t *add_state_transition(fsm_t *fsm, char *input_string, state_t *current_state, state_t *next_state, char *output_string, int i)
{
  if(i >= fsm->num_transition) {
    printf("ERROR: state transition counter exceeds the number of transitions.\n");
    return NULL;
  }

  fsm->transition[i].input = (char *)calloc(strlen(input_string) + 1, sizeof(char));
  strcpy(fsm->transition[i].input, input_string);
  fsm->transition[i].output = (char *)calloc(strlen(output_string) + 1, sizeof(char));
  strcpy(fsm->transition[i].output, output_string);
  fsm->transition[i].current_state = current_state;
  fsm->transition[i].next_state = next_state;

  return &(fsm->transition[i]);
}

void free_transition(trans_t *transition)
{
  if(transition->input)
    free(transition->input);

  if(transition->output)
    free(transition->output);

  transition->current_state = NULL;
  transition->next_state = NULL;
}

fsm_t *get_fsm()
{
  return _kiss_fsm;
}

void set_fsm_name(fsm_t *fsm, char *name)
{
  if(fsm) {
    if(fsm->name)
      free(fsm->name);
    
    fsm->name = (char *)calloc(strlen(name) + 1, sizeof(char));
    strcpy(fsm->name, name);
  }
}

void set_fsm_init_state(fsm_t *fsm, char *state_name)
{
  if(fsm && state_name) {
    if(fsm->init_state)
      free(fsm->init_state);

    fsm->init_state = (char *)calloc(strlen(state_name) + 1, sizeof(char));
    strcpy(fsm->init_state, state_name);
  }
}

state_t *get_fsm_init_state(fsm_t *fsm, int *success_flag) 
{
  state_t *init_state;
  *success_flag = FALSE;

  if(fsm->state == NULL) {
    return NULL;
  }
  
  if(get_state(fsm, fsm->init_state, &init_state)) {
    *success_flag = TRUE;
    return init_state;
  }
  else
    return NULL;
}

/*******************************************************
  strip the suffix from original name and return 
  the new name. caller's responsibility to free 
  the new name.
*******************************************************/
char *get_name_without_suffix(char *name, char *suffix)
{
  char *new_name = NULL;
  int name_len = 0;
  int suffix_len = 0;

  if(name == NULL || suffix == NULL)
    return NULL;

  new_name = (char *)calloc(strlen(name) + 1, sizeof(char));
  strcpy(new_name, name);
  name_len = strlen(name); 
  suffix_len = strlen(suffix);
  
  if(strstr(name, suffix) != NULL) {
    new_name[name_len - suffix_len] = '\0';
  }

  return new_name;
}

/* read in fsm */
boolean read_fsm_from_blif(char *file_name, fsm_t *fsm)
{
  FILE *fp_input = NULL;
  static int state_count = 0;
  static int trans_count = 0;
  char value[64];
  char tag[16];
  char input_str[64];
  char output_str[64];
  char cstate_str[64];
  char nstate_str[64];
  char code_str[64];
  char line[1024] = "\0";
  char *temp_name = NULL;
  state_t *current_state = NULL;
  state_t *next_state = NULL;
  trans_t *transition = NULL;

  if((fp_input = fopen(file_name, "r")) == NULL) {
    printf("ERROR: Cannot open input file %s\n", file_name);
    return FALSE;
  }

  if(strstr(file_name, ".kiss2"))
    temp_name = get_name_without_suffix(file_name, ".kiss2");
  else
    temp_name = get_name_without_suffix(file_name, ".blif");

  set_fsm_name(fsm, temp_name);
  free(temp_name);

  printf("Reading FSM %s...\n", fsm->name);

  while(fgets(line, 1024, fp_input) != NULL) {
    if(line[0] == ' ' || line[0] == '\n')
      continue;
    if(line[0] == '.') {
      sscanf(line, "%s %s", tag, value);
      if(!strcmp(tag, ".model") || !strcmp(tag, ".start_kiss") || !strcmp(tag, ".end_kiss"))
	continue;
      else if(!strcmp(tag, ".i"))
	fsm->num_input = atoi(value);
      else if(!strcmp(tag, ".o"))
	fsm->num_output = atoi(value);
      else if(!strcmp(tag, ".s")) {
	fsm->num_state = atoi(value);
	fsm->state = (state_t *)calloc(atoi(value), sizeof(state_t));
      }
      else if(!strcmp(tag, ".p")) {
	fsm->num_transition = atoi(value);
	fsm->transition = (trans_t *)calloc(atoi(value), sizeof(trans_t));
      }
      else if(!strcmp(tag, ".r")) {
	set_fsm_init_state(fsm, value);
      }
      else if(!strcmp(tag, ".end")) {
	break;
      }
      else if(!strcmp(tag, ".code")) {
	sscanf(line, "%s %s %s", tag, cstate_str, code_str);
	if(get_state(fsm, cstate_str, &current_state) == FALSE) {
	  printf("ERROR: cannot find state %s in the FSM.\n");
	  return FALSE;
	}

	set_state_code(current_state, code_str);
      }
      else {
	printf("ERROR: Incorrect in line %s\n", line);
	return FALSE;
      }
    
    }
    else {
      if(sscanf(line, "%s %s %s %s", input_str, cstate_str, nstate_str, output_str) != 4) {
	printf("ERROR: Incorrect in line %s\n", line);
	return FALSE;
      }

      if(get_state(fsm, cstate_str, &current_state) == FALSE) {
	current_state = add_state(fsm, cstate_str, state_count++);
	if(current_state == NULL)
	  return FALSE;
      }

      if(get_state(fsm, nstate_str, &next_state) == FALSE) {
	next_state = add_state(fsm, nstate_str, state_count++);
	if(next_state == NULL)
	  return FALSE;
      }

      transition = add_state_transition(fsm, input_str, current_state, next_state, output_str, trans_count++);
    }  
  }

  double temp = log2((double)fsm->num_state);
  // make sure there is no rounding up due to precision
  fsm->code_length = ceil(temp);

  return TRUE;
}

void traverse_fsm(fsm_t *fsm)
{
  int i;

  if(fsm == NULL)
    return;

  for(i = 0; i < fsm->num_state; i++) {
    fsm->state[i].visited = FALSE;
  }
}

boolean write_fsm_to_blif_by_index(char *file_name, fsm_t *fsm)
{
  FILE *fp_output = NULL;
  int i, j;

  if((fp_output = fopen(file_name, "w")) == NULL) {
    printf("ERROR: Cannot open output file %s\n", file_name);
    return FALSE;
  }

  fprintf(fp_output,".model pow3_output\n");
  fprintf(fp_output,".start_kiss\n");
  fprintf(fp_output,".i %d\n", fsm->num_input);
  fprintf(fp_output,".o %d\n", fsm->num_output);
  fprintf(fp_output,".s %d\n", fsm->num_state);
  fprintf(fp_output,".p %d\n", fsm->num_transition);
  fprintf(fp_output,".r 0\n");
  
  for(i = 0; i < fsm->num_transition; i++) {
    fprintf(fp_output, "%s %d %d %s\n", fsm->transition[i].input, (fsm->transition[i].current_state)->index, (fsm->transition[i].next_state)->index, fsm->transition[i].output);
  }

  fprintf(fp_output,".end_kiss\n");
  for(i = 0; i < fsm->num_state; i++) {
    fprintf(fp_output,".code %d %s\n", fsm->state[i].index, fsm->state[i].code);
  }
  fprintf(fp_output,".end\n");

  return TRUE;
}

boolean write_fsm_to_blif(char *file_name, fsm_t *fsm)
{
  FILE *fp_output = NULL;
  int i, j;

  if((fp_output = fopen(file_name, "w")) == NULL) {
    printf("ERROR: Cannot open output file %s\n", file_name);
    return FALSE;
  }

  fprintf(fp_output,".model pow3_output\n");
  fprintf(fp_output,".start_kiss\n");
  fprintf(fp_output,".i %d\n", fsm->num_input);
  fprintf(fp_output,".o %d\n", fsm->num_output);
  fprintf(fp_output,".s %d\n", fsm->num_state);
  fprintf(fp_output,".p %d\n", fsm->num_transition);
  fprintf(fp_output,".r %s\n", fsm->state[0].name);
  
  for(i = 0; i < fsm->num_transition; i++) {
    fprintf(fp_output, "%s %s %s %s\n", fsm->transition[i].input, (fsm->transition[i].current_state)->name, (fsm->transition[i].next_state)->name, fsm->transition[i].output);
  }

  fprintf(fp_output,".end_kiss\n");
  for(i = 0; i < fsm->num_state; i++) {
    fprintf(fp_output,".code %s %s\n", fsm->state[i].name, fsm->state[i].code);
  }
  fprintf(fp_output,".end\n");

  return TRUE;
}


void print_fsm(fsm_t *fsm)
{
  int i = 0;

  printf("Number of state:         %d\n", fsm->num_state);
  printf("Number of input:         %d\n", fsm->num_input);
  printf("Number of output:        %d\n", fsm->num_output);
  printf("Number of transition:    %d\n", fsm->num_transition);
  printf("--------------------------------------------------------------\n");
  printf("Input    Current State       Next State     Output\n");
  printf("--------------------------------------------------------------\n");
  
  for(i = 0; i < fsm->num_transition; i++) {
    printf("%s           %s                 %s              %s\n", fsm->transition[i].input, (fsm->transition[i].current_state)->name, (fsm->transition[i].next_state)->name, fsm->transition[i].output);
  }

  printf("---------------------------------------------------------------\n");
  printf("State Table\n");
  printf("---------------------------------------------------------------\n");
  for(i = 0; i < fsm->num_state; i++) {
    printf("%s          %s\n", fsm->state[i].name, fsm->state[i].code);
  }
}
