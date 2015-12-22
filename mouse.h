#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stdio.h>

#define STACK_SIZE 256U

struct mouse_interpreter {
  int16_t *variables;
  int16_t *stack;
  uint16_t stack_size;
  long int *loop_offsets;
  uint8_t loop_counter;
  struct mouse_macro **macros;
  struct macro_call *current_macro_call;
  uint8_t macro_call_counter;
  FILE *scratch_file;
};

struct mouse_macro {
  long int macro_offset;
  uint8_t num_params;
  char *local_var_names;
  uint8_t num_local_vars;
};

struct macro_call {
  struct macro_call *parent_call;
  struct mouse_macro *macro;
  char **params;
  int16_t *local_var_vals;
};

struct macro_param {
  char *instructions;
  
};

enum interpret_result {
  SUCCESS = 0,
  PROGRAM_END,
  BAD_PARAMS,
  FILE_EOF,
  FILE_ERROR,
  FILE_NOT_FOUND,
  STACK_OVERFLOW,
  STACK_UNDERFLOW,
  NUM_TOO_LARGE,
  TOO_MANY_IFS,
  TOO_MANY_LOOPS,
  MALFORMED_LOOP,
  INVALID_VAR,
  STRING_TOO_LONG,
  INVALID_CHAR,
  MALFORMED_CALL,
  MALFORMED_MACRO,
  MACRO_TOO_LONG,
  NOT_IN_MACRO
};

static char * read_file_as_string(FILE *file);
struct mouse_interpreter * new_interpreter(void);
void free_interpreter(struct mouse_interpreter *interpreter);
static enum interpret_result populate_macros(struct mouse_interpreter *interpreter, FILE *file);
enum interpret_result interpret(struct mouse_interpreter *interpreter, FILE *file, const long int offset);
static enum interpret_result push_stack(struct mouse_interpreter *interpreter, int16_t val);
static enum interpret_result pop_stack(struct mouse_interpreter *interpreter, int16_t *val);
static enum interpret_result parse_number(FILE *file, const long int offset, int16_t *val);

#endif /* MOUSE_H */
