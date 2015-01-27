#include <stdio.h>

typedef enum {
  FALSE = 0, 
  TRUE
} boolean;

typedef struct state_struct {
  void *data; // for future usage
  int visited;
  int index;
  char *name;
  char *code;
} state_t;

typedef struct trans_struct {
  int index;
  char *input;
  char *output;
  state_t *current_state;
  state_t *next_state;
} trans_t;
 
typedef struct fsm_struct {
  int num_input;
  int num_output;
  int num_transition;
  int num_state;
  int code_length;
  int num_w_edge;
  char *name;
  char *init_state;
  state_t *state;
  trans_t *transition;
} fsm_t;

typedef struct w_edge_struct {
  int begin;
  int end;
  float weight;
} w_edge_t;

typedef struct node_struct {
  int class;
  /* 
     double r1;
     int r2;
     int r3;
  */
} node_t;

