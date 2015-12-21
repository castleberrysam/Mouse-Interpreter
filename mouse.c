#include "mouse.h"
#include <stdlib.h>
#include <string.h>

struct mouse_interpreter * new_interpreter(void)
{
  struct mouse_interpreter *interpreter = malloc(sizeof(struct mouse_interpreter));
  memset(interpreter, 0, sizeof(struct mouse_interpreter));
  interpreter->variables = malloc(sizeof(int16_t) * 26);
  interpreter->stack = malloc(sizeof(int16_t) * STACK_SIZE);
  interpreter->loop_offsets = malloc(sizeof(long int) * sizeof(interpreter->loop_counter));
  return interpreter;
}

void free_interpreter(struct mouse_interpreter *interpreter)
{
  free(interpreter->stack);
  free(interpreter->variables);
  free(interpreter->loop_offsets);
  free(interpreter);
}

enum interpret_result interpret(struct mouse_interpreter *interpreter, FILE *file, const long int offset)
{
  if(interpreter == NULL || interpreter->variables == NULL || interpreter->stack == NULL || 
     interpreter->loop_offsets == NULL || file == NULL) {
    return BAD_PARAMS;
  }
  if(feof(file) != 0) {return FILE_EOF;}
  if(fseek(file, offset, SEEK_SET) != 0 || ferror(file) != 0) {return FILE_ERROR;}
  
  int character = fgetc(file);
  if(character == EOF) {return FILE_EOF;}
  int return_val, second_char;
  int16_t scratch1, scratch2, scratch3;
  long int scratch_pos;
  switch((char) character) {
  case ' ':
  case '\n':
    return SUCCESS;
  case '$':
    return PROGRAM_END;
  case ':':
    return_val = pop_stack(interpreter, &scratch1);
    if(return_val != SUCCESS) {return return_val;}
    return_val = pop_stack(interpreter, &scratch2);
    if(return_val != SUCCESS) {return return_val;}
    
    if(scratch1 < 0 || scratch1 > 25) {return INVALID_VAR;}
    interpreter->variables[scratch1] = scratch2;
    return SUCCESS;
  case '.':
    return_val = pop_stack(interpreter, &scratch1);
    if(return_val != SUCCESS) {return return_val;}
    
    if(scratch1 < 0 || scratch1 > 25) {return INVALID_VAR;}
    return push_stack(interpreter, interpreter->variables[scratch1]);
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
    return push_stack(interpreter, character - 0x41);
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return_val = parse_number(file, offset, &scratch1);
    if(return_val != SUCCESS) {return return_val;}
    return push_stack(interpreter, scratch1);
  case '+':
  case '-':
  case '*':
  case '/':
  case '\\':
    return_val = pop_stack(interpreter, &scratch1);
    if(return_val != SUCCESS) {return return_val;}
    return_val = pop_stack(interpreter, &scratch2);
    if(return_val != SUCCESS) {return return_val;}
    
    switch(character) {
    case '+':
      scratch3 = scratch2 + scratch1;
      break;
    case '-':
      scratch3 = scratch2 - scratch1;
      break;
    case '*':
      scratch3 = scratch2 * scratch1;
      break;
    case '/':
      scratch3 = scratch2 / scratch1;
      break;
    case '\\':
      scratch3 = scratch2 % scratch1;
      break;
    }
    
    return push_stack(interpreter, scratch3);
  case '?':
    scratch_pos = ftell(file);
    if(scratch_pos == -1L) {return FILE_ERROR;}
    
    second_char = fgetc(file);
    if(second_char == EOF || ((char) second_char) != '\'') {
      return_val = fseek(file, scratch_pos, SEEK_SET);
      if(return_val != SUCCESS) {return FILE_ERROR;}
      
      long int input_pos = ftell(stdin);
      if(input_pos == -1L) {return FILE_ERROR;}
      return_val = parse_number(stdin, input_pos, &scratch1);
      if(return_val != SUCCESS) {return return_val;}
      return push_stack(interpreter, scratch1);
    }
    
    int input_char = fgetc(stdin);
    if(input_char == EOF) {return FILE_EOF;}
    return push_stack(interpreter, input_char);
  case '!':
    return_val = pop_stack(interpreter, &scratch1);
    if(return_val != SUCCESS) {return return_val;}
    
    scratch_pos = ftell(file);
    if(scratch_pos == -1L) {return FILE_ERROR;}
    
    second_char = fgetc(file);
    if(second_char == EOF || ((char) second_char) != '\'') {
      return_val = fseek(file, scratch_pos, SEEK_SET);
      if(return_val != SUCCESS) {return FILE_ERROR;}
      
      printf("%d", scratch1);
      return SUCCESS;
    }
    
    printf("%c", (char) scratch1);
    return SUCCESS;
  case '\'':
    second_char = fgetc(file);
    if(second_char == EOF) {return FILE_EOF;}
    return push_stack(interpreter, second_char);
  case '"': ;
    char string_literal[256];
    memset(string_literal, 0, 256);
    
    uint8_t string_len = 0;
    second_char = fgetc(file);
    while(second_char != '"') {
      if(second_char == EOF) {return FILE_EOF;}
      if(((char) second_char) == '!') {second_char = '\n';}
      string_literal[string_len] = second_char;
      ++string_len;
      if(string_len == 0) {return STRING_TOO_LONG;}
      second_char = fgetc(file);
    }
    
    printf("%s", string_literal);
    return SUCCESS;
  case '<':
  case '=':
  case '>':
    return_val = pop_stack(interpreter, &scratch1);
    if(return_val != SUCCESS) {return return_val;}
    return_val = pop_stack(interpreter, &scratch2);
    if(return_val != SUCCESS) {return return_val;}
    
    switch(character) {
    case '<':
      return push_stack(interpreter, scratch2 < scratch1 ? 1 : 0);
    case '=':
      return push_stack(interpreter, scratch2 = scratch1 ? 1 : 0);
    case '>':
      return push_stack(interpreter, scratch2 > scratch1 ? 1 : 0);
    }
  case '[':
    return_val = pop_stack(interpreter, &scratch1);
    if(return_val != SUCCESS) {return return_val;}
    
    if(scratch1 != 0) {return SUCCESS;}
    
    uint8_t if_counter = 1;
    do {
      second_char = fgetc(file);
      if(second_char == EOF) {return FILE_EOF;}
      if(((char) second_char) == '[') {
	++if_counter;
	if(if_counter == 0) {return TOO_MANY_IFS;}
      } else if(((char) second_char) == ']') {
	--if_counter;
      }
    } while(if_counter > 0);
    
    return SUCCESS;
  case ']':
    return SUCCESS;
  case '(':
    interpreter->loop_offsets[interpreter->loop_counter] = offset;
    
    ++interpreter->loop_counter;
    if(interpreter->loop_counter == 0) {return TOO_MANY_LOOPS;}
    return SUCCESS;
  case ')':
    --interpreter->loop_counter;
    if(interpreter->loop_counter == 255) {return MALFORMED_LOOP;}
    
    return_val = fseek(file, interpreter->loop_offsets[interpreter->loop_counter], SEEK_SET);
    if(return_val != SUCCESS) {return FILE_ERROR;}
    return SUCCESS;
  case '^':
    return_val = pop_stack(interpreter, &scratch1);
    if(return_val != SUCCESS) {return return_val;}
    
    if(scratch1 != 0) {return SUCCESS;}
    
    --interpreter->loop_counter;
    if(interpreter->loop_counter == 255) {return MALFORMED_LOOP;}
    
    uint8_t loop_counter = 1;
    do {
      second_char = fgetc(file);
      if(second_char == EOF) {return FILE_EOF;}
      if(((char) second_char) == '(') {
	++loop_counter;
	if(loop_counter == 0) {return TOO_MANY_LOOPS;}
      } else if(((char) second_char) == ')') {
	--loop_counter;
      }
    } while(loop_counter > 0);
    return SUCCESS;
  case '#':
    return SUCCESS;
  case '@':
    return SUCCESS;
  case '%':
    return SUCCESS;
  case ',':
    return SUCCESS;
  case ';':
    return SUCCESS;
  case '{':
    return SUCCESS;
  case '}':
    return SUCCESS;
  case '~':
    do {
      second_char = fgetc(file);
      if(second_char == EOF) {return FILE_EOF;}
    } while(((char) second_char) != '\n');
    return SUCCESS;
  default:
    return INVALID_CHAR;
  }
}

static enum interpret_result push_stack(struct mouse_interpreter *interpreter, int16_t val)
{
  if(interpreter == NULL || interpreter->stack == NULL) {
    return BAD_PARAMS;
  }
  if(interpreter->stack_size >= STACK_SIZE) {
    return STACK_OVERFLOW;
  }
  interpreter->stack[interpreter->stack_size] = val;
  ++interpreter->stack_size;
  return SUCCESS;
}

static enum interpret_result pop_stack(struct mouse_interpreter *interpreter, int16_t *val)
{
  if(interpreter == NULL || interpreter->stack == NULL) {
    return BAD_PARAMS;
  }
  if(interpreter->stack_size <= 0) {
    return STACK_UNDERFLOW;
  }
  --interpreter->stack_size;
  *val = interpreter->stack[interpreter->stack_size];
  return SUCCESS;
}

static enum interpret_result parse_number(FILE *file, const long int offset, int16_t *val)
{
  if(file == NULL || val == NULL) {
    return BAD_PARAMS;
  }
  if(feof(file) != 0) {return FILE_EOF;}
  if(fseek(file, offset, SEEK_SET) != 0 || ferror(file) != 0) {return FILE_ERROR;}
  
  char ascii_val[6];
  uint8_t num_digits = 0;
  long int file_pos;
  for(;num_digits<5;++num_digits) {
    file_pos = ftell(file);
    if(file_pos == -1L) {return FILE_ERROR;}
    
    int digit = fgetc(file);
    if(digit == EOF || digit < 0x30 || digit > 0x39) {break;}
    ascii_val[num_digits] = (char) digit;
  }
  ascii_val[num_digits] = 0;
  
  if(fseek(file, file_pos, SEEK_SET) != 0) {return FILE_ERROR;}
  
  int numerical_val = atoi(ascii_val);
  if(numerical_val > 65535) {return NUM_TOO_LARGE;}
  *val = (int16_t) numerical_val;
  return SUCCESS;
}

int main(int argc, char **argv)
{
  if(argc < 2) {
    printf("No program file specified.\n");
    return SUCCESS;
  }
  
  FILE *file = fopen(argv[1], "r");
  if(file == NULL) {return FILE_NOT_FOUND;}
  struct mouse_interpreter *interpreter = new_interpreter();
  
  enum interpret_result return_status;
  do {
    return_status = interpret(interpreter, file, ftell(file));
  } while(return_status == SUCCESS);
  
  free_interpreter(interpreter);
  fclose(file);
  
  switch(return_status) {
  case PROGRAM_END:
    printf("\nThe program exited successfully.\n");
    break;
  case BAD_PARAMS:
    printf("\nAn internal error occurred in the interpreter.\n");
    break;
  case FILE_EOF:
    printf("\nThe end of the file was encountered unexpectedly.\n");
    break;
  case FILE_ERROR:
    printf("\nAn error occurred during file operations.\n");
    break;
  case FILE_NOT_FOUND:
    printf("\nThe specified program file couldn't be found.\n");
    break;
  case STACK_OVERFLOW:
    printf("\nThe stack overflowed while executing the program.\n");
    break;
  case STACK_UNDERFLOW:
    printf("\nThe stack underflowed while executing the program.\n");
    break;
  case NUM_TOO_LARGE:
    printf("\nThe program contains a numerical constant that is too large.\n");
    break;
  case TOO_MANY_IFS:
    printf("\nThe program contains too many nested if statements (>255).\n");
    break;
  case TOO_MANY_LOOPS:
    printf("\nThe program contains too many nested loops (>255).\n");
    break;
  case MALFORMED_LOOP:
    printf("\nThe program contains a malformed loop (end but no beginning).\n");
    break;
  case INVALID_VAR:
    printf("\nThe program attempted to store from or write to a non-existent variable.\n");
    break;
  case STRING_TOO_LONG:
    printf("\nThe program contains a string that is too long (>255 chars).\n");
    break;
  case INVALID_CHAR:
    printf("\nAn invalid character was encountered in the program.\n");
    break;
  }
  
  return return_status == PROGRAM_END ? SUCCESS : return_status;
}
