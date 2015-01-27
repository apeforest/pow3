/*************** begin forward function proto declaration *************/
extern boolean read_fsm_from_blif(char *file_name, fsm_t *fsm);
extern boolean write_fsm_to_blif(char *file_name, fsm_t *fsm);
extern fsm_t *get_fsm();
extern void init_fsm();
extern void free_fsm();
extern void free_state(state_t *state);
extern state_t *add_state(fsm_t *fsm, char *state_name, int i);
extern boolean get_state(fsm_t *fsm, char *state_name, state_t **state);
extern trans_t *add_state_transition(fsm_t *fsm, char *input_str, state_t *current_state, state_t *next_state, char *output_str, int i);
extern void free_transition(trans_t *transition);
extern fsm_t *get_fsm();
extern char *get_name_without_suffix(char *name, char *suffix);
extern void set_fsm_name(fsm_t *fsm, char *name);
extern void set_fsm_init_state(fsm_t *fsm, char *state_name);
extern state_t *get_fsm_init_state(fsm_t *fsm, int *success_flag);
/*************** end forward function proto declaration **************/
