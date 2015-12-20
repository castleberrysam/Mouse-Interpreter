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
  INVALID_CHAR
};

struct mouse_interpreter * new_interpreter(void);
void free_interpreter(struct mouse_interpreter *interpreter);
enum interpret_result interpret(struct mouse_interpreter *interpreter, FILE *file, const long int offset);
static enum interpret_result push_stack(struct mouse_interpreter *interpreter, int16_t val);
static enum interpret_result pop_stack(struct mouse_interpreter *interpreter, int16_t *val);
static enum interpret_result parse_number(FILE *file, const long int offset, int16_t *val);

#endif /* MOUSE_H */
