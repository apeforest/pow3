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

// global variable
pow3_stg_t *_pow3_stg;

/******************************** 
initialize the matrix for pow3 
**********************************/
boolean initialize_stg(fsm_t *fsm)
{
  int i, j;
  int num_edge;
  double **trans_prob = NULL;
  double **weight_matrix = NULL;

  if(fsm == NULL)
    return FALSE;

  trans_prob = get_trans_prob(fsm);  
  if(trans_prob == NULL)
    return FALSE;

  weight_matrix = (double **)calloc(fsm->num_state, sizeof(double *));
  for(i = 0; i < fsm->num_state; i++) {
    weight_matrix[i] = (double *)calloc(fsm->num_state, sizeof(double));
    for(j = 0; j < fsm->num_state; j++) {
      if(trans_prob[i][j] > 0 || trans_prob[j][i] > 0)
	weight_matrix[i][j] = trans_prob[i][j] + trans_prob[j][i];
      else
	weight_matrix[i][j] = 0;
    }
  }

  _pow3_stg = (pow3_stg_t *)malloc(sizeof(pow3_stg_t));
  _pow3_stg->num_node = fsm->num_state; 
  _pow3_stg->code_length = fsm->code_length;
  _pow3_stg->weight_matrix = weight_matrix;
  _pow3_stg->node = (pow3_node_t *)calloc(_pow3_stg->num_node, sizeof(pow3_node_t));
  _pow3_stg->set = (pow3_set_t *)calloc(_pow3_stg->num_node, sizeof(pow3_set_t));

  _pow3_stg->num_set = 0;
  for(i = 0; i < _pow3_stg->num_node; i++) {
    _pow3_stg->set[i].set_size = 0;
    _pow3_stg->set[i].node_list = NULL;
  }
  

  for(i = 0; i < _pow3_stg->num_node; i++) {
    _pow3_stg->node[i].index = i;
    _pow3_stg->node[i].state = &fsm->state[i];
    _pow3_stg->node[i].set_id = UNDEFINE;
    _pow3_stg->node[i].code = (char *)calloc(_pow3_stg->code_length + 1, sizeof(char));
    for(j = 0; j < _pow3_stg->code_length; j++)
      _pow3_stg->node[i].code[j] = 'x';
  }

  num_edge = 0;
  // don't consider self loop as an edge
  for(i = 0; i < fsm->num_state; i++) {
    for(j = i + 1; j < fsm->num_state; j++) {
      if(weight_matrix[i][j] > 0) {
	num_edge++;
	if(num_edge == 1)
	  _pow3_stg->edge_list = (pow3_edge_t **)calloc(1, sizeof(pow3_edge_t *));
	else
	  _pow3_stg->edge_list = (pow3_edge_t **)realloc(_pow3_stg->edge_list, num_edge*sizeof(pow3_edge_t *));
	_pow3_stg->edge_list[num_edge-1] = (pow3_edge_t *)malloc(sizeof(pow3_edge_t));
	_pow3_stg->edge_list[num_edge-1]->n1 = &_pow3_stg->node[i];
	_pow3_stg->edge_list[num_edge-1]->n2 = &_pow3_stg->node[j];
	_pow3_stg->edge_list[num_edge-1]->weight = weight_matrix[i][j];
      }
    }  
  }
  
  _pow3_stg->num_edge = num_edge;
  free(trans_prob);
}

/*****************************************
 sort array of edges by weights 
 in decreasing order
******************************************/
void sort_edge(pow3_edge_t **edge_list, int num_edge)
{
  int i, j;
  pow3_edge_t *temp_edge;

  for(i = 0; i < num_edge; i++) {
    for(j = i + 1; j < num_edge; j++) {
      if(edge_list[i]->weight < edge_list[j]->weight) {
	temp_edge = edge_list[i];
	edge_list[i] = edge_list[j];
	edge_list[j] = temp_edge;
      }
    }
  }
}
  
/***************************************
 assign the l-th bit to all states 
****************************************/  
void assign(pow3_stg_t *stg, int l)
{
  int i,j,k;
  int x;
  int num_zeros = 0;
  int num_ones = 0;
  pow3_node_t *node1 = NULL;
  pow3_node_t *node2 = NULL;

  sort_edge(stg->edge_list, stg->num_edge);
  
  // assign codes to two states in decreasing order of the weights on edges
  for(i = 0; i < stg->num_edge; i++) {
    node1 = stg->edge_list[i]->n1;
    node2 = stg->edge_list[i]->n2;

    if(node1->code[l] == 'x' && node2->code[l] == 'x') {
      // if both states have not been assigned at bit l
      x = select_bit(stg, node1, node2, l);
      if(class_violation(node1, node2, l) == 0) { // no class violation
	node1->code[l] = '0' + x;
	node2->code[l] = '0' + x;
      }
      else if(class_violation(node1, node2, l) == 2 && x != 0) { // If assign bit 1, no class violation
	node1->code[l] = '0' + x;
	node2->code[l] = '0' + x;
      }
      else if(class_violation(node1, node2, l) == 3 && x != 1) { // If assign bit 0, no class violation
	node1->code[l] = '0' + x;
	node2->code[l] = '0' + x;
      }
      else { // class violation
	node1->code[l] = '0' + x;
	node2->code[l] = '0' + 1 - x;
      }
    }
    else {
      if(node1->code[l] == 'x') { // if fisrt state has NOT been assigned
	if(!class_violation(node1, node2, l)) {
	  x = select_bit(stg, node1, node2, l);
	  node1->code[l] = '0' + x;
	}
	else {
	  if(node2->code[l] == '0')
	    node1->code[l] = '1';
	  else
	    node1->code[l] = '0';
	}
      }
      if(node2->code[l] == 'x')  { // if second state has NOT been assigned
	if(!class_violation(node1, node2, l)) {
	  x = select_bit(stg, node1, node2, l);
	  node2->code[l] = '0' + x;
	}
	else {
	  if(node1->code[l] = '0')
	    node2->code[l] = '1';
	  else
	    node2->code[l] = '0';
	}
      }
    }
    
    pow3_set_t *node_set;
    // assign code to the rest states that have only one code to not violate class constraints
    for(j = 0; j < stg->num_node; j++) {
      node_set = &(stg->set[stg->node[j].set_id]);
      if(node_set->set_size >= pow(2, stg->code_length -l -1)) {
	num_ones = 0;
	num_zeros = 0;
	// check how many ones or zeros for each class
	for(k = 0; k < stg->num_node; k++) {
	  if(stg->node[k].set_id == stg->node[j].set_id) {
	    if(stg->node[k].code[l] == '1')
	      num_ones++;
	    if(stg->node[k].code[l] == '0')
	      num_zeros++;
	  }
	}
	// assign the unassigned states in the class where there are ENOUGH ones or zeros
	if(num_ones >= pow(2, stg->code_length-l-1))
	  for(k = 0; k < stg->num_node; k++)
	    if(stg->node[k].set_id == stg->node[j].set_id && stg->node[k].code[l] == 'x')
	      stg->node[k].code[l] = '0';
	if(num_zeros >= pow(2, stg->code_length-l-1))
	  for(k = 0; k < stg->num_node; k++)
	    if(stg->node[k].set_id == stg->node[j].set_id && stg->node[k].code[l] == 'x')
	      stg->node[k].code[l] = '1';
      }
    }
  }
}

/*************************
get the current STG
**************************/
pow3_stg_t *get_stg(void)
{
  return _pow3_stg;
}

/**************************************************** 
decide whether there will be class violation if two 
states are assigned the same code at the k-th bit 
*****************************************************/
int class_violation(pow3_node_t *n1, pow3_node_t *n2, int k)
{
  int i;
  pow3_stg_t *stg = get_stg();
  int undistinct_zero = 0;
  int undistinct_one = 0;
 
  if(n1->set_id != n2->set_id)
    return 0;                                       //No class violation

  for(i = 0; i < stg->num_node; i++) {
    if(&stg->node[i] != n1 && &stg->node[i] != n2 && stg->node[i].set_id == n1->set_id) {
      if(stg->node[i].code[k] == '0')
	undistinct_zero++;
      if(stg->node[i].code[k] == '1')
	undistinct_one++;
    }
  }

  if(n1->code[k] == 'x') {
    /* if both state m and n have not been assigned */
    if((undistinct_zero + 2) > pow(2, stg->code_length-k-1) && (undistinct_one + 2) > pow(2, stg->code_length-k-1))
      return 1;                                            /* class violated */
    else if(undistinct_zero + 2 > pow(2, stg->code_length-k-1))
      return 2;                                            /* no violation if assgin bit 1 */
    else if(undistinct_one + 2 > pow(2, stg->code_length-k-1))
      return 3;                                            /* no violation if assign bit 0 */
    else
      return 0;                                            /* no violation */
  }
  else {
    /* if one state has been assigned */
    if(n1->code[k] == '1') {
      if(undistinct_one + 1 > pow(2, stg->code_length-k-1))
	return 1;                                        /* class violated */
      else
	return 0;                                        /* no class violation */
    }
    if(n1->code[k] == '0') {
      if(undistinct_zero+1>pow(2, stg->code_length-k-1))
	return 1;                                        /* class violated */
      else
	return 0;                                        /* no class violation */
    }
  }
}
  
/****************************************************
select the code for k-th bit of two states 
*****************************************************/
int select_bit(pow3_stg_t *stg, pow3_node_t *n1, pow3_node_t *n2, int k)
{
  int i;
  double oneEdgeVio = 0;
  double zeroEdgeVio = 0;
  int assignBit = 0;
  double **weight = stg->weight_matrix;

  zeroEdgeVio = 0; // total edge violation if assign bit 0
  oneEdgeVio = 0;  // total edge violation if assign bit 1

  for(i = 0; i < stg->num_node; i++) {
    if(&stg->node[i] != n1 && &stg->node[i] != n2) {
      if(stg->node[i].code[k] == '0')
	oneEdgeVio = oneEdgeVio + weight[n1->index][i] + weight[n2->index][i];
      if(stg->node[i].code[k] == '1')
	zeroEdgeVio = zeroEdgeVio + weight[n1->index][i] + weight[n2->index][i];
    }
  }

  if(zeroEdgeVio <= oneEdgeVio)
    assignBit = 0;
  else
    assignBit = 1;
  
  return assignBit;
}

/*********************************************
add a node to the k-th set in STG
*********************************************/
void add_node_to_set(pow3_stg_t *stg, pow3_node_t *node, int k)
{
  int num_element;
  node->set_id = k;
  stg->set[k].set_size++;
  num_element = stg->set[k].set_size;
  if(num_element == 1)
    stg->set[k].node_list = (pow3_node_t **)calloc(num_element, sizeof(pow3_node_t *));
  else
    stg->set[k].node_list = (pow3_node_t **)realloc(stg->set[k].node_list, num_element * sizeof(pow3_node_t *));
  
  stg->set[k].node_list[num_element - 1] = node;
}

/**************************************************
  calculate the classes and the size of each class 
**************************************************/
void adjust_class_constr(pow3_stg_t *stg)
{
  int i,j;
  int set_id = 0;

  for(i = 0; i < stg->num_node; i++) {
    stg->node[i].set_id = UNDEFINE;
  }

  // reset all the sets in STG
  for(i = 0; i < stg->num_set; i++) {
    stg->set[i].set_size = 0;
    free(stg->set[i].node_list);
    stg->set[i].node_list = NULL;
  }
  stg->num_set = 0;

  for(i = 0; i < stg->num_node; i++) {
    // if two nodes have same partial code, they belong to same set
    for(j = 0; j < i; j++) {
      if(stg->node[j].set_id != UNDEFINE && hamming_distance(stg->node[i].code, stg->node[j].code, stg->code_length) == 0) {
	set_id = stg->node[j].set_id;
	add_node_to_set(stg, &stg->node[i], set_id);
	break;
      }
    }
    // else add this node to a new set
    if(stg->node[i].set_id == UNDEFINE) {
      add_node_to_set(stg, &stg->node[i], stg->num_set);
      stg->num_set++;
    }
  }
}

/************************************************
 update edge weights after each bit assignment
*************************************************/
void adjust_edge_weight(pow3_stg_t *stg)
{
  int i,j;
  for(i = 0; i < stg->num_node; i++)
    for(j = 0; j < stg->num_node; j++)
      stg->weight_matrix[i][j] *= hamming_distance(stg->node[i].code, stg->node[j].code, stg->code_length) +1;
  
  for(i = 0; i < stg->num_edge; i++) {
    stg->edge_list[i]->weight *= hamming_distance(stg->edge_list[i]->n1->code, stg->edge_list[i]->n2->code, stg->code_length) + 1; 
  }
}

/************************************************
assign the state codes in STG to FSM
************************************************/
void update_fsm_code(pow3_stg_t *stg, fsm_t *fsm)
{
  int i;
  state_t *state;

  for(i = 0; i < stg->num_node; i++) {
    state = stg->node[i].state;
    if(state->code != NULL)
      free(state->code);
    state->code = (char *)calloc(stg->code_length + 1, sizeof(char *));
    strcpy(state->code, stg->node[i].code);
  }

  fsm->code_length = stg->code_length;
}

void free_node(pow3_node_t *node)
{
  if(node->code)
    free(node->code);
}

void free_set(pow3_set_t *set)
{
  int i;

  if(set->node_list) 
    free(set->node_list);
}

/**********************************
free STG data structure
**********************************/
void free_stg(pow3_stg_t *stg)
{
  int i;

  if(stg->node) {
    for(i = 0; i < stg->num_node; i++)
      free_node(&stg->node[i]);
    free(stg->node);
  }

  if(stg->edge_list) {
    for(i = 0; i < stg->num_edge; i++)
      free(stg->edge_list[i]);
    free(stg->edge_list);
  }
  
  if(stg->set) {
    for(i = 0; i < stg->num_set; i++)
      free_set(&stg->set[i]);
    free(stg->set);
  }

  if(stg->weight_matrix) {
    for(i = 0;i < stg->num_node; i++)
      free(stg->weight_matrix[i]);
    free(stg->weight_matrix);
  }

  free(stg);

  _pow3_stg = NULL;
}

/***********************************************
main engine for POW3 encoding
***********************************************/
boolean encode_pow3(fsm_t *fsm)
{
  int i;
  pow3_stg_t *stg = NULL;

  // build STG based on input FSM
  if(initialize_stg(fsm) == FALSE) {
    printf("ERROR: cannot build STG.\n");
    return FALSE;
  }
  
  stg = get_stg();

  // assign code to all the states bit by bit
  for(i = 0; i < fsm->code_length; i++) {
    adjust_class_constr(stg);
    assign(stg, i);
    adjust_edge_weight(stg);
  }

  // return the code to FSM
  update_fsm_code(stg, fsm);

  free_stg(stg);

  return TRUE;
}







