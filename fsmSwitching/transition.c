#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "global.h"
#include "fsm.h"
#include "matrix_util.h"

/*****************************************
calculate the conditional probability
array in FSM
******************************************/
double **get_cond_trans_prob(fsm_t *fsm)
{
  int i, j;
  int dontcare = 0;
  double num_trans = 0;
  int cstate = 0, nstate = 0;
  double row_sum = 0, col_sum = 0;
  char *input_string = NULL;
  double **prob_array = (double **)calloc(fsm->num_state, sizeof(double *));

  for(i = 0; i < fsm->num_state; i++) {
    prob_array[i] = (double *)calloc(fsm->num_state, sizeof(double));
    for(j = 0; j < fsm->num_state; j++)
      prob_array[i][j] = 0;
  }

  for(i = 0; i < fsm->num_transition; i++) {
    dontcare = 0;
    input_string = fsm->transition[i].input;
    cstate = fsm->transition[i].current_state->index;
    nstate = fsm->transition[i].next_state->index;

    for(j = 0; j < fsm->num_input; j++) {
      if(input_string[j] == '-')
	dontcare++;
    }    
    num_trans = (double)pow(2, dontcare);
    prob_array[cstate][nstate] += num_trans;
  }

  for(i = 0; i < fsm->num_state; i++) {
    col_sum = 0;
    for(j = 0; j < fsm->num_state; j++)
      col_sum = col_sum + prob_array[j][i];

    if(col_sum == 0) {
      printf("Warning: state %s is an unreachable state!\n", fsm->state[i].name);
      goto failure;
    }
  }

  for(i = 0; i < fsm->num_state; i++) {
    row_sum = 0;
    for(j = 0; j < fsm->num_state; j++)
      row_sum = row_sum + prob_array[i][j];

    if(row_sum==0) {
      printf("Warning: state %s has no next state!\n",fsm->state[i].name);
      goto failure;
    }

    for(j = 0; j < fsm->num_state; j++) {
      prob_array[i][j] = prob_array[i][j]/row_sum;
    }
  }

  return prob_array;

 failure: 
  for(i = 0; i < fsm->num_state; i++)
    free(prob_array[i]);
  free(prob_array);

  return NULL;
}

/**********************************************
calculate the steady state probability, 
based on Markov chain model
***********************************************/
double *get_steady_state_prob(double **conditional, int n)
{
  int i,j;
  double *tmp_vec = (double *)calloc(n, sizeof(double));
  double *tmp_vec_plus =  (double *)calloc(n+1, sizeof(double));
  double **tmp_arr = (double **)calloc(n, sizeof(double *));
  double **b = (double **)calloc(n, sizeof(double *));
  double **bt = (double **)calloc(n+1, sizeof(double *));
  double *steady_prob = (double *)calloc(n, sizeof(double));
 
  for(i = 0; i < n; i++) {
    tmp_arr[i]=(double *)calloc(n, sizeof(double));
    b[i] = (double *)calloc(n+1, sizeof(double));
    for(j = 0; j < n+1; j++)
      b[i][j] = 0;
  }

  for(i = 0; i < n+1; i++) {
    bt[i]=(double *)calloc(n, sizeof(double));
    for(j = 0; j < n; j++)
      bt[i][j] = 0;
  }

  for(i = 0; i < n; i++) {
    tmp_vec_plus[i]=0;
    tmp_vec[i]=0;
    steady_prob[i] = 0;
  }

  tmp_vec_plus[n] = 1;
  
  b = Padding(conditional, n);
  bt = Transpose(b, n, n+1);
  tmp_arr = Multiply(b, bt, n, n, n+1);
  inverse(tmp_arr, n);
  
  for(i = 0; i < n; i++)
    for(j=0; j <= n; j++)
      tmp_vec[i] += tmp_vec_plus[j] * bt[j][i];
  
  for(i = 0; i < n; i++)
    for(j = 0; j < n; j++)
      steady_prob[i] += tmp_vec[j] * tmp_arr[j][i];

  free(tmp_vec);
  free(tmp_vec_plus);
  for(i = 0; i < n; i++) {
    free(b[i]);
    free(bt[i]);
    free(tmp_arr[i]);
  }
  free(bt[n]);
  free(b);
  free(bt);
  free(tmp_arr);

  return steady_prob;
}

/**********************************************
calcualte the total transition probability 
based on steady state probability and conditional
transition probability
***********************************************/
double **get_trans_prob(fsm_t *fsm)
{
  int i, j, n;
  double **cond_prob = NULL;
  double **trans_prob = NULL;
  double *steady_prob = NULL;
  
  n = fsm->num_state;
  cond_prob = get_cond_trans_prob(fsm);
  
  if(cond_prob == NULL)
    return NULL;

  trans_prob = (double **)calloc(n, sizeof(double *));
  steady_prob = (double *)calloc(n, sizeof(double));
  
  for(i = 0; i < n; i++) {
    trans_prob[i] = (double *)calloc(n, sizeof(double));
    steady_prob[i] = 0;
    for(j = 0; j < n; j++)
      trans_prob[i][j] = 0;
  }
  
  steady_prob = get_steady_state_prob(cond_prob, n);
  
  for(i = 0; i < n; i++) {
    if(steady_prob[i] < 0) {
      printf("ERROR: steady state probability of state %s is less than 0.\n", fsm->state[i].name);
      printf("The FSM may not be reducible and not applicable to Markov chain model.\n");
    }
    for(j = 0; j < n; j++)
      trans_prob[i][j] = cond_prob[i][j] * steady_prob[i];
  }
  free(cond_prob);
  free(steady_prob);
  
  return trans_prob;
}

/*******************************************
calculate the HAMMING distance between
two vectors
********************************************/
int hamming_distance(char *s1, char *s2, int n)
{
  int i;
  int result = 0;

  if(s1 == NULL || s2 == NULL) {
    printf("ERROR: Cannot compute HAMMING distance between NULL vectors.\n");
    return -1;
  } 

  for(i = 0; i < n; i++) 
    if(s1[i] != s2[i])
      result++;

  return result;
}

/********************************************
calculate the switching activity in a FSM
based on the total transition probability
*********************************************/
boolean get_switching_activity(fsm_t *fsm, double *total_sw, boolean print_prob)
{
  boolean ret_flag = TRUE;
  int ham_dist = 0;
  int i, j;
  double **transition = NULL;
  char file_name[128];
  FILE *ofp = NULL;

  transition = get_trans_prob(fsm);
  
  if(transition == NULL)
    return FALSE;
  
  if(print_prob == TRUE) {
    sprintf(file_name, "%s.prob", fsm->name);
    if((ofp = fopen(file_name, "w")) == NULL)
      printf("ERROR: cannot open output probability file %s\n", file_name);
  }
  
  for(i = 0; i < fsm->num_state; i++) {
    for(j = 0; j < fsm->num_state; j++) {
      ham_dist = hamming_distance(fsm->state[i].code, fsm->state[j].code, fsm->num_state);
      if(ham_dist == -1) {
	ret_flag = FALSE;
	goto failure;
      }

      *total_sw += ham_dist * transition[i][j];
      if(ofp)
	fprintf(ofp, "%.4f ", transition[i][j]);
    }
    if(ofp)
      fprintf(ofp, "\n");
  }

 failure: 
  for(i = 0; i < fsm->num_state; i++)
    free(transition[i]);
  free(transition);
  
  return ret_flag;
}
