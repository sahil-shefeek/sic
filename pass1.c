#include <arm/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>

#define SYM_SIZE 25
#define OP_SIZE 25

#define MAX_OPTAB_LEN 25
#define MAX_SYMTAB_LEN 25

#define LABEL_BUFFER_SIZE 25
#define OPCODE_BUFFER_SIZE 25
#define OPERAND_BUFFER_SIZE 25
#define MNEMONIC_BUFFER_SIZE 25

enum AssemblerStatus { SUCCESS, DUPLICATE_SYMBOL, INVALID_OPERATION };

int status = SUCCESS;

typedef struct symtab_entry {
  char name[SYM_SIZE];
  int address;
} symtab_entry_t;

typedef struct symtab {
  symtab_entry_t *entries;
  int length;
} symtab_t;

typedef struct optab_entry {
  char opcode[OP_SIZE];
  int machine_code;
} optab_entry_t;

typedef struct optab {
  optab_entry_t *entries;
  int length;
} optab_t;

optab_t *create_optab() {
  optab_t *optab = (optab_t *)malloc(sizeof(optab_t));
  optab->length = 0;
  optab->entries =
      (optab_entry_t *)malloc(MAX_OPTAB_LEN * sizeof(optab_entry_t));
  return optab;
}

void build_optab(const char *file_name, optab_t *dest) {
  FILE *optab_file = fopen(file_name, "r");
  char mnemonic_buffer[MNEMONIC_BUFFER_SIZE];
  int machine_code, i = 0;
  fscanf(optab_file, "%s\t%d\n", mnemonic_buffer, &machine_code);
  while (feof(optab_file) == 0) {
    optab_entry_t entry;
    strcpy(entry.opcode, mnemonic_buffer);
    entry.machine_code = machine_code;
    dest->entries[i++] = entry;
    fscanf(optab_file, "%s\t%d\n", mnemonic_buffer, &machine_code);
  }
  dest->length = i;
  fclose(optab_file);
}

int search_optab(optab_t *optab, char *op) {
  for (int i = 0; i < optab->length; i++) {
    if (strcmp(optab->entries[i].opcode, op) == 0) {
      return 1;
    }
  }
  return 0;
}

void delete_optab(optab_t *optab) {
  free(optab->entries);
  free(optab);
}

symtab_t *create_symtab() {
  symtab_t *symtab = (symtab_t *)malloc(sizeof(symtab_t));
  symtab->length = 0;
  symtab->entries =
      (symtab_entry_t *)malloc(MAX_SYMTAB_LEN * sizeof(symtab_entry_t));
  return symtab;
}

void export_symtab(symtab_t *symtab) {
  FILE *symtab_file = fopen("symtab.txt", "w");
  for (int i = 0; i < symtab->length; i++) {
    fprintf(symtab_file, "%s\t%d\n", symtab->entries[i].name,
            symtab->entries[i].address);
  }
  fclose(symtab_file);
}

int search_symtab(symtab_t *symtab, char *label) {
  for (int i = 0; i < symtab->length; i++) {
    if (strcmp(symtab->entries[i].name, label) == 0) {
      return 1;
    }
  }
  return 0;
}

void delete_symtab(symtab_t *symtab) {
  free(symtab->entries);
  free(symtab);
}

int main(int argc, char *argv[]) {
  FILE *input = fopen("input.txt", "r");
  FILE *ifile = fopen("ifile.txt", "w");

  optab_t *optab = create_optab();
  build_optab("optab.txt", optab);

  symtab_t *symtab = create_symtab();

  char label_buf[LABEL_BUFFER_SIZE], opcode_buf[OPCODE_BUFFER_SIZE],
      operand_buf[OPERAND_BUFFER_SIZE];
  int start = 0, locctr = 0;

  fscanf(input, "%s\t%s\t%s\n", label_buf, opcode_buf, operand_buf);
  if (strcmp(opcode_buf, "START") == 0) {
    start = atoi(operand_buf);
    locctr = start;
    fprintf(ifile, "**\t%s\t%s\n", label_buf, operand_buf);
    fscanf(input, "%s\t%s\t%s\n", label_buf, opcode_buf, operand_buf);
  }
  printf("Program start is: %d\n", start);

  while (strcmp(opcode_buf, "END") != 0) {
    if (strcmp(label_buf, "**") != 0) {
      if (search_symtab(symtab, label_buf) == 1) {
        status = DUPLICATE_SYMBOL;
        goto end;
      } else {
        symtab_entry_t entry;
        strcpy(entry.name, label_buf);
        entry.address = locctr;
        symtab->entries[symtab->length++] = entry;
      }
    }
    if (search_optab(optab, opcode_buf) == 1) {
      locctr += 3;
    } else if (strcmp(opcode_buf, "WORD") == 0) {
      locctr += 3;
    } else if (strcmp(opcode_buf, "RESW") == 0) {
      locctr += (3 * atoi(operand_buf));
    } else if (strcmp(opcode_buf, "RESB") == 0) {
      locctr += atoi(operand_buf);
    } else if (strcmp(opcode_buf, "BYTE") == 0) {
      if (operand_buf[0] == 'C') {
        int len = strlen(operand_buf) - 3;
        locctr += len;
      }

    } else {
      status = INVALID_OPERATION;
      goto end;
    }
    fprintf(ifile, "%d\t%s\t%s\t%s\n", locctr, label_buf, opcode_buf,
            operand_buf);
    fscanf(input, "%s\t%s\t%s\n", label_buf, opcode_buf, operand_buf);
  }
  fprintf(ifile, "%d\t**\tEND\t**\n", locctr);
end:
  switch (status) {
  case SUCCESS:
    printf("Assemble successful.\nProgram length is: %d\n", locctr - start);
    break;
  case DUPLICATE_SYMBOL:
    printf("Error: Symbol %s was defined twice.\n", label_buf);
    printf("Assemble terminated!\n");
    break;
  case INVALID_OPERATION:
    printf("Error: Invalid operation '%s'.\n", opcode_buf);
    printf("Assemble terminated!\n");
    break;
  }

  export_symtab(symtab);
  delete_symtab(symtab);
  symtab = NULL;
  delete_optab(optab);
  optab = NULL;
  fclose(input);
  fclose(ifile);
  return 0;
}
