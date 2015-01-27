/*
 *
 * Lin Yuan
 * University of Maryland, College Park
 *
 * Implementation of POW3 FSM state encoding
 * The transition probability is calculated 
 * by using Markov chain model.
 *
 * Psuedo code of the POW3 algorithm refers to 
 * Luca Benini et al.'s State Assignment for Low Power Dissipation
 * in IEEE Journal of Solid-State Circuits 1995
 *
 * This is a modified pow3 encode algorithm
 * It can be simply embedded to other program
 * 
 *
 * Update: removed a few bugs. It can process any large FSM benchmarks from kiss2
 * Update: use the fsm_t datastructure.
 *
 * Last modified May 4, 2009
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "struct.h"
#include "global.h"
#include "pow3_struct.h"

extern int hamming_distance(char *s1, char *s2, int n);
extern double **get_trans_prob(fsm_t *fsm);
void optimize_peak(int k, int n, int *output_vector, int *value_vector);
extern fsm_t *get_fsm();

// global variable
int **l2h_switch_tbl;
int **h2l_switch_tbl;
int globalPeakSwitch;
int *globalOptVector;

int vector_to_integer(int *vec, int n)
{
  int i;
  int value;
  value = 0;
  for(i=0;i<n;i++)
    value = value + (int)(vec[i]*pow(2,i));
  return value;
}

int *integer_to_vector(int value, int n)
{
  int i,m;
  int *vector;
  vector = (int *)malloc(n * sizeof(int));
  for(i=0; i<n; i++)
    {
      m = floor(value/pow(2,i));
      if( m % 2 ==0)
	vector[i]=0;
      else
	vector[i]=1;
    }
  return vector;
}

void build_peak_switch_table(int num_state, int code_len)
{
  int i, j, k;
  int max_num_state = pow(2, code_len);
  int quo1, quo2;
  int res1, res2;

  //Obtain switch look-up l2h__switch_tbl(0->1) and h2l_switch_tbl(1->0)
  l2h_switch_tbl = (int **)malloc(max_num_state * sizeof(int *));
  h2l_switch_tbl = (int **)malloc(max_num_state * sizeof(int *));
  for(i = 0; i < max_num_state; i++) {
    l2h_switch_tbl[i] = (int *)malloc(max_num_state * sizeof(int));
    h2l_switch_tbl[i] = (int *)malloc(max_num_state * sizeof(int));
  }

  for(i = 0; i < max_num_state; i++) {
    for(j = 0; j < max_num_state; j++) {
      l2h_switch_tbl[i][j] = 0;
      h2l_switch_tbl[i][j] = 0;
    }
  }

  for(i = 0; i < max_num_state; i++) {
    for(j = 0; j < max_num_state; j++) {
      quo1 = i;
      quo2 = j;
      for(k = 0; k < code_len; k++) {
	res1 = quo1%2;
	quo1 = quo1/2;
	res2 = quo2%2;
	quo2 = quo2/2;
	if(res1 < res2)
	  l2h_switch_tbl[i][j]++;
	else if(res1 > res2)
	  h2l_switch_tbl[i][j]++;
      }
    }
  }  
}

int get_peak_switch(int *code_list)
{
  fsm_t *fsm = get_fsm();
  int i;
  int cs_id, ns_id;
  int cs_code_value, ns_code_value;
  int peak_switch;

  peak_switch = 0;

  for(i = 0; i < fsm->num_transition; i++) {
    cs_id = (fsm->transition[i].current_state)->index;
    ns_id = (fsm->transition[i].next_state)->index;
    cs_code_value = code_list[cs_id];
    ns_code_value = code_list[ns_id];
    if(l2h_switch_tbl[cs_code_value][ns_code_value] > peak_switch)
      peak_switch = l2h_switch_tbl[cs_code_value][ns_code_value];
    if(h2l_switch_tbl[cs_code_value][ns_code_value] > peak_switch)
      peak_switch = h2l_switch_tbl[cs_code_value][ns_code_value];
  }
}

/**************************************************
recursively apply all possible encoding to FSM
and get the best encoding
***************************************************/
void optimize_peak(int k, int n, int *output_vector, int *value_vector) 
{
  fsm_t *fsm = get_fsm();
  int i = 0, j = 0;
  int *new_value_vec = NULL;
  int peak_switch = 0;

  for(i = n-1; i >= 0; i--) {
    // select a value from the value vector
    output_vector[k] = value_vector[i];
    if(k == 0) {
      // if it's the last value, print out the permutation
      peak_switch = get_peak_switch(output_vector);
      if(peak_switch < globalPeakSwitch) {
	globalPeakSwitch = peak_switch;
	for(j = 0; j < fsm->num_state; j++)
	  globalOptVector[j] = output_vector[j];
      }
    }
    else {
      // exlude the selected value from value vector
      new_value_vec = (int *)calloc(n - 1, sizeof(int));
      for(j = 0; j < i; j++)
	new_value_vec[j] = value_vector[j];
      for(j = i; j < n-1; j++)
	new_value_vec[j] = value_vector[j+1];

      optimize_peak(k-1, n-1, output_vector, new_value_vec);
      free(new_value_vec);
    }
  }
}

/***********************************************
main engine for brute force encoding
***********************************************/
boolean encode_brute_force(fsm_t *fsm)
{
  int i;
  int max_num_code = pow(2, fsm->code_length);
  int *code_vector = (int *)calloc(fsm->num_state, sizeof(int));
  int *value_vector = (int *)calloc(max_num_code, sizeof(int));

  globalPeakSwitch = 0;
  globalOptVector = (int *)calloc(fsm->num_state, sizeof(int));
  for(i = 0; i < fsm->num_state; i++)
    globalOptVector[i] = 0;

  for(i = 0; i < max_num_code; i++)
    value_vector[i] = i;
  
  // return the code to FSM
  optimize_peak(fsm->num_state, max_num_code, code_vector, value_vector); 

  printf("The peak switch is %d\n", globalPeakSwitch);
  printf("The optimal state code vector is:");
  for(i = 0; fsm->num_state; i++)
    printf(" %d", globalOptVector[i]);
  printf("\n");
  return TRUE;
}
