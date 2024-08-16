#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int locctr = 0, start = 0;
  FILE *optab = fopen("optab.txt", "r");
  FILE *input = fopen("input.txt", "r");
  FILE *symtab = fopen("symtab.txt", "w");
  FILE *ifile = fopen("ifile.txt", "w");
  FILE *len = fopen("len.txt", "w");
  char mnemonic[15], opcode[25], operand[25], label[25], code[25];
  fscanf(input, "%s\t%s\t%s", label, opcode, operand);
  if (strcmp(opcode, "START") == 0) {
    start = atoi(operand);
    locctr = start;
    fprintf(ifile, "%s\t%s\t%s\n", label, opcode, operand);
  }
  printf("Start is: %d\n", start);
  fscanf(input, "%s\t%s\t%s", label, opcode, operand);
  while (strcmp(opcode, "END") != 0) {
    fprintf(ifile, "%d\t", locctr);
    if (strcmp("**", label) != 0) {
      fprintf(symtab, "%s\t%d\n", label, locctr);
    }
    fscanf(optab, "%s\t%s", code, mnemonic);
    while (!feof(optab)) {
      if (strcmp(code, opcode) == 0) {
        locctr += 3;
        rewind(optab);
      }
      fscanf(optab, "%s\t%s", code, mnemonic);
    }
    if (strcmp(opcode, "WORD") == 0) {
      locctr += 3;
    } else if (strcmp(opcode, "RESW") == 0) {
      locctr += 3 * atoi(operand);
    } else if (strcmp(opcode, "RESB")) {
      locctr += atoi(operand);
    } else if (strcmp(opcode, "BYTE")) {
      if (operand[0] == 'C') {
        locctr += strlen(operand) - 3;
      } else if (operand[0] == 'X') {
        printf("\n##1##");
        locctr++;
      }
    }
    fprintf(ifile, "%s\t%s\t%s\n", label, opcode, operand);
    fscanf(input, "%s\t%s\t%s", label, opcode, operand);
  }
  fprintf(ifile, "%s\t%s\t%s\n", label, opcode, operand);
  printf("Length of the program is: %d", locctr - start);
  fclose(len);
  fclose(optab);
  fclose(symtab);
  return 0;
}
