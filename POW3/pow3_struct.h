/*
 * The Data Structures use in POW3 encoding
 *
 */


/***********************************
data structure for a node in STG. 
***********************************/
typedef struct pow3_node_struct {
  int index;
  int set_id;
  char *code;     // state code
  state_t *state; // link to the state in original FSM
} pow3_node_t;

/**********************************
data structure for a weighted 
undirectional edge in STG
**********************************/
typedef struct pow3_edge_struct {
  pow3_node_t *n1; 
  pow3_node_t *n2;
  double weight;
} pow3_edge_t;

/**********************************
data structure for a set of nodes
***********************************/
typedef struct pow3_set_struct {
  int set_size;
  pow3_node_t **node_list;
} pow3_set_t;
 
/*********************************
data structure for STG in POW3
algorithm.
**********************************/
typedef struct pow3_stg_struct {
  int num_node;
  int num_edge;
  int code_length;
  int num_set;
  double **weight_matrix;
  pow3_node_t *node;
  pow3_edge_t **edge_list;
  pow3_set_t *set;
} pow3_stg_t;
