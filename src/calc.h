#ifndef CALC_H
#define CALC_H
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#include <vector>
const std::vector<std::string> calc_funcs{"ln", "log", "log\\d+", "sin", "cos", "tan", "csc", "sec", "cot", "arcsin", "arccos", "arctan", "asin", "acos", "atan", "sqrt"}; 
const std::vector<std::string> calc_vars{"x", "pi", "e"};
#else
#define VARS {"x", "pi", "e"}
#define FUNCS {"ln", "log", "log\\d+", "sin", "cos", "tan", "csc", "sec", "cot", "arcsin", "arccos", "arctan", "asin", "acos", "atan", "sqrt"}

char * getstr();
#endif
#define VARS_LEN 3
#define FUNCS_LEN 16
enum funcs_enum {LOG, SIN, COS, TAN, CSC, SEC, COT, ARCSIN, ARCCOS, ARCTAN, SQRT};
char * parser(char *input, char **variables, int varsc);

struct VM_Object {
  enum {NUMBER, VARIABLE} objectType;
  union {
    double number;
    int variableIndex;
  } value;
};

typedef struct VM_Object VM_Object;

struct VM_Instruction {
  enum {OPERATION, PUSH, FUNC, FACTORIAL} instructionType;
  unsigned int opType;
  enum funcs_enum funcType;
  VM_Object pushed;
  double base;
};

typedef struct VM_Instruction VM_Instruction;

struct VM_Code {
  VM_Instruction *instructions;
  int length;
};

typedef struct VM_Code VM_Code;

int str_index(char *item, char *arr[], int len);
void slice_str(char * str, char * buffer, size_t start, size_t end);
VM_Code compiler(cJSON *ast, char **variables, int varsc);
double VM_Exec(VM_Code code, double *fvars);
void VM_Print(VM_Code code);
char **append_string(char *array[], int arrayc, char *string);
char **append_strings(char *array[], int arrayc, char *strings[], int stringsc);
char *remove_whitespace(char *string);
#ifdef __cplusplus
}
#endif
#endif