#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 100
#define LABEL_BUFFER_SIZE 10
#define NAME_BUFFER_SIZE 10
#define OPERAND_BUFFER_SIZE 10
#define OPCODE_BUFFER_SIZE 30
#define MAX_ARGS_LEN 10
#define IO_FORMAT "%s\t%s\t%s"
#define DEFTAB_FORMAT "%s\t%s\n"

typedef struct {
  char macro_name[NAME_BUFFER_SIZE];
  long start_pos, end_pos;
} namtab_entry;

bool EXPANDING = false;

FILE *input_file, *deftab_file, *expanded_file, *namtab_file, *argtab_file;

char input_buf[INPUT_BUFFER_SIZE];
char buf_copy[INPUT_BUFFER_SIZE];
char label_buf[LABEL_BUFFER_SIZE], opcode_buf[OPCODE_BUFFER_SIZE],
    operand_buf[OPERAND_BUFFER_SIZE];
char arg_buf[OPERAND_BUFFER_SIZE];

void expand(namtab_entry entry);
void define();
namtab_entry check_namtab(char *macro_name);
void get_line();
void process_line();
int get_pos_arg(char args_list[MAX_ARGS_LEN][OPERAND_BUFFER_SIZE], char *arg);
void load_args(int pos);

int main() {
  input_file = fopen("input.txt", "r");
  deftab_file = fopen("deftab.txt", "w+");
  expanded_file = fopen("output.txt", "w");
  namtab_file = fopen("namtab.txt", "w+");
  argtab_file = fopen("argtab.txt", "w+");
  fscanf(input_file, "%s\t%s\t%s\n", label_buf, opcode_buf, operand_buf);
  while (strcmp(opcode_buf, "END") != 0) {
    get_line();
    process_line();
  }
  fclose(input_file);
  fclose(deftab_file);
  fclose(namtab_file);
  fclose(expanded_file);
  fclose(argtab_file);
  return 0;
}

void get_line() {
  if (EXPANDING) {
    char temp_buf[OPERAND_BUFFER_SIZE];
    sprintf(label_buf, "**\t");
    fscanf(deftab_file, "%s\t%s\n", opcode_buf, temp_buf);
    if (temp_buf[0] == 'X' || temp_buf[0] == 'C') {
      temp_buf[strlen(temp_buf) - 1] = '\0';
      load_args(atoi(temp_buf + 3));
      if (temp_buf[0] == 'X') {
        sprintf(operand_buf, "X'%s'", arg_buf);
      } else {
        sprintf(operand_buf, "C'%s'", arg_buf);
      }
    } else {
      char *operand = strtok(temp_buf, ",");
      load_args(atoi(operand + 1));
      sprintf(operand_buf, "%s", arg_buf);
      operand = strtok(NULL, ",");
      while (operand != NULL) {
        load_args(atoi(operand + 1));
        sprintf(operand_buf, ",%s", arg_buf);
        operand = strtok(NULL, ",");
      }
    }
  } else {
    fscanf(input_file, IO_FORMAT, label_buf, opcode_buf, operand_buf);
  }
}

void process_line() {
  namtab_entry entry = check_namtab(opcode_buf);
  if (strlen(entry.macro_name) > 0)
    expand(entry);
  else if (strcmp(opcode_buf, "MACRO") == 0) {
    define();
  } else {
    fprintf(expanded_file, IO_FORMAT "\n", label_buf, opcode_buf, operand_buf);
  }
}

void expand(namtab_entry entry) {
  EXPANDING = true;
  fseek(deftab_file, entry.start_pos, SEEK_SET);
  fprintf(expanded_file, ". " IO_FORMAT "\n", label_buf, opcode_buf,
          operand_buf);
  rewind(argtab_file);
  char *operand = strtok(operand_buf, ",");
  while (operand != NULL) {
    strncpy(arg_buf, operand, strlen(operand));
    fprintf(argtab_file, "%s\n", arg_buf);
    operand = strtok(NULL, ",");
  }
  fscanf(deftab_file, "%s\t%s\n", opcode_buf, operand_buf);
  while (true) {
    get_line();
    if (strcmp(opcode_buf, "MEND") == 0)
      break;
    process_line();
  }
  EXPANDING = false;
};

void define() {
  fseek(namtab_file, 0L, SEEK_END);
  fseek(deftab_file, 0L, SEEK_END);
  long start_pos = ftell(deftab_file);
  fprintf(namtab_file, "%s\t%ld\t", label_buf, start_pos);
  fprintf(deftab_file, DEFTAB_FORMAT, label_buf, operand_buf);
  char args[MAX_ARGS_LEN][OPERAND_BUFFER_SIZE];
  int args_pos = 1;
  char *operand = strtok(operand_buf, ",");
  while (operand != NULL) {
    strcpy(args[args_pos++], operand + 1);
    operand = strtok(NULL, ",");
  }

  while (true) {
    get_line();
    if (strcmp(opcode_buf, "MEND") == 0) {
      fprintf(deftab_file, DEFTAB_FORMAT, opcode_buf, operand_buf);
      break;
    }
    char temp_buf[OPERAND_BUFFER_SIZE];
    if (operand_buf[0] == 'X' || operand_buf[0] == 'C') {
      operand_buf[strlen(operand_buf) - 1] = '\0';
      if (operand_buf[0] == 'X') {
        sprintf(temp_buf, "X'?%d'", get_pos_arg(args, operand_buf + 3));
      } else {
        sprintf(temp_buf, "C'?%d'", get_pos_arg(args, operand_buf + 3));
      }
    } else {
      char *operand = strtok(operand_buf, ",");
      sprintf(temp_buf, "?%d", get_pos_arg(args, operand + 1));
      operand = strtok(NULL, ",");
      while (operand != NULL) {
        sprintf(temp_buf, ",?%d", get_pos_arg(args, operand + 1));
        operand = strtok(NULL, ",");
      }
    }
    strcpy(operand_buf, temp_buf);
    fprintf(deftab_file, DEFTAB_FORMAT, opcode_buf, operand_buf);
  }
  long end_pos = ftell(deftab_file);
  fprintf(namtab_file, "%ld\n", end_pos);
}

namtab_entry check_namtab(char *macro_name) {
  namtab_entry entry;
  rewind(namtab_file);
  while (fscanf(namtab_file, "%s\t%ld\t%ld\n", entry.macro_name,
                &entry.start_pos, &entry.end_pos) != EOF) {
    if (strcmp(entry.macro_name, macro_name) == 0) {
      return entry;
    }
  }
  strcpy(entry.macro_name, "");
  return entry;
}

int get_pos_arg(char args_list[MAX_ARGS_LEN][OPERAND_BUFFER_SIZE], char *arg) {
  for (int i = 1; i < MAX_ARGS_LEN; i++) {

    if (strcmp(args_list[i], arg) == 0)
      return i;
  }
  return -1;
}

void load_args(int pos) {
  rewind(argtab_file);
  for (int i = 0; i < pos; i++) {
    fscanf(argtab_file, "%s\n", arg_buf);
  }
}
