#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include "cJSON.h"
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include "calc.h"


char *gcalc_funcs[] = FUNCS;

// Get full-length, allocated string from stdin
char * getstr() {
  size_t buf_size = 32;
  char *buf = malloc(buf_size);
  char input_ch = getchar();
  size_t i = 0;
  while (input_ch != '\n') {
    *(buf+i) = input_ch;
    i++;
    if (i == buf_size) {
      buf_size = buf_size + 32;
      buf = realloc(buf, buf_size);
    }
    input_ch = getchar();
  }
  *(buf + i) = '\0';
  char *output = malloc(i + 1);
  memcpy(output, buf, i + 1);
  free(buf);
  return output;
}

// Simple wrapper around pcre2_compile
pcre2_code *re_compile(char * pattern) {
  pcre2_code *re;
  PCRE2_SPTR re_pattern;
  re_pattern = (PCRE2_SPTR)pattern;
  int errornumber;
  PCRE2_SIZE erroroffset;
  re = pcre2_compile(
  re_pattern,               /* the pattern */
PCRE2_ZERO_TERMINATED | PCRE2_EXTENDED,
  0,                     /* default options */
  &errornumber,          /* for error number */
  &erroroffset,          /* for error offset */
  NULL);                 /* use default compile context */
  if (re == NULL)
  {
  PCRE2_UCHAR buffer[256];
  pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
  printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,
    buffer);
  exit(1);
  }
  return re;
}

// Regex replacement
int re_replace(pcre2_code *pattern, char *replacement, char *subject, PCRE2_UCHAR *output, PCRE2_SIZE output_len) {
  return pcre2_substitute(
  pattern,
  (PCRE2_SPTR)subject,
  strlen(subject),
  0,
  PCRE2_SUBSTITUTE_GLOBAL | PCRE2_SUBSTITUTE_EXTENDED,
  NULL,
  NULL,
  (PCRE2_SPTR)replacement,
  strlen(replacement),
  output,
  &output_len);
}

// Create a PCRE2 pattern from a list of words
char *listpat(char **words, size_t wordsc) {
  size_t i = 0;
  size_t end = 0;
  size_t size = 256;
  char *pat = malloc(size);
  *pat = '\0';
  while (i < wordsc) {
    if (strlen(pat) + strlen(words[i]) > size) {
      size += strlen(pat) + strlen(words[i]) + 16;
      pat = realloc(pat, size);
    }
    strcat(pat, words[i]);
    strcat(pat, "|");
    end += strlen(words[i]) + 1;
    i++;
  }
  *(pat + end - 1) = '\0';
  return pat;
}

// Count the occurences of a char in a string
size_t countchr(char *str, char ch) {
  size_t out = 0;
  for (size_t i = 0; i < strlen(str); i++) {if (str[i] == ch) out++;}
  return out;
}

char *remove_whitespace(char *string) {
  char *output = malloc(strlen(string) + 1);
  size_t i = 0;
  size_t j = 0;
  while (i < strlen(string)) {
    register char input_ch = *(string + i);
    if (input_ch != ' ' && input_ch != '\t') {
        *(output+j) = input_ch;
        j++;
      }
    i++;
  }
  *(output + j + 1) = '\0';
  return output;
}

// Parse mathematical expressions
char * parser(char *input, char **variables, int varsc) {
  if (countchr(input, '(') > countchr(input, ')')) {return NULL;}
  char *input2 = remove_whitespace(input);
  // Add 0s to the start of decimals beginning with . (like .1, .2, .159)
  pcre2_code *pat = re_compile("(?<!\\d)\\.([\\d]+)");
  static PCRE2_UCHAR output[1024] = "";
  PCRE2_UCHAR noutput[1024];
  re_replace(pat, "0.$1", input2, output, 1024);
  // Parse functions
  char *func_pat_str = malloc(strlen("(?<name>") + strlen(listpat(gcalc_funcs, FUNCS_LEN)) + strlen("))(?<para>\\((?<pcontent>[^()]*|\\g<para>)\\))") + 1);
  *func_pat_str = '\0';
  strcat(func_pat_str, "(?<name>");
  strcat(func_pat_str, listpat(gcalc_funcs, FUNCS_LEN));
  strcat(func_pat_str, ")(?<para>\\((?<pcontent>[^()]*|\\g<para>)\\))");
  pcre2_code *func_pat = re_compile(func_pat_str);
  pcre2_match_data *f_match_data = pcre2_match_data_create_from_pattern(func_pat, NULL);
  while (1) {
    re_replace(func_pat, "{\"t\":4,\"f\":\"$name\",\"v\":$pcontent}", (char*)output, noutput, 1024);
    re_replace(func_pat, "{\"t\":4,\"f\":\"$name\",\"v\":$pcontent}", (char*)noutput, output, 1024);
    int rc = pcre2_match(func_pat,(PCRE2_SPTR)output,1024,0,0,f_match_data,NULL);
    if (rc == PCRE2_ERROR_NOMATCH) break;
  }
  // Compile values
  pcre2_code *num_pat = re_compile("(?<!log)((?>\\d+\\.\\d+)|(?>(?<!\\.)\\d+))");
  re_replace(num_pat, "{\"t\":0,\"v\":$0}", (char*)output, noutput, 1024);
  pcre2_code *word_pat = re_compile(listpat(variables, varsc));
  re_replace(word_pat, "{\"t\":1,\"v\":\"$0\"}", (char*)noutput, output, 1024);
  // Parse parentheses
  pcre2_code *para_pat = re_compile("(?<para>\\((?<pcontent>[^()]*|\\g<para>)\\))");
  while (strchr((char*)output, '(') != NULL) {
      re_replace(para_pat, "{\"t\":3,\"v\":$pcontent}", (char*)output, noutput, 1024);
      re_replace(para_pat, "{\"t\":3,\"v\":$pcontent}", (char*)noutput, output, 1024);
  }
  // Implicit multiplication
  pcre2_code *impmul_pat = re_compile("}{");
  while (strstr((char*)output, "}{") != NULL) {
      re_replace(impmul_pat, "}*{", (char*)output, noutput, 1024);
      re_replace(impmul_pat, "}*{", (char*)noutput, output, 1024);
  }
  // Parse factorials
  pcre2_code *factorial_pat = re_compile("(\\{(?>[^{}]|\\1)*\\})!");
  while (strstr((char*)output, "}!") != NULL) {
    re_replace(factorial_pat, "{\"t\":5,\"v\":$1}", (char*)output, noutput, 1024);
    re_replace(factorial_pat, "{\"t\":5,\"v\":$1}", (char*)noutput, output, 1024);
  }
  // Parse operations
  pcre2_code *exp_pat = re_compile("(?<lbrack>\\{(?>[^{}]|\\g<lbrack>)*\\})\\^(?<rbrack>\\{(?>[^{}]|\\g<rbrack>)*\\})");
  while (strstr((char*)output, "}^{") != NULL) {
      re_replace(exp_pat, "{\"t\":2,\"o\":0,\"l\":$lbrack,\"r\":$rbrack}", (char*)output, noutput, 1024);
      re_replace(exp_pat, "{\"t\":2,\"o\":0,\"l\":$lbrack,\"r\":$rbrack}", (char*)noutput, output, 1024);
  }
  // Check for negative values
  pcre2_code *neg_pat = re_compile("(?<!})-(?<brack>\\{(?>[^{}]|\\g<brack>)*\\})");
  pcre2_match_data *neg_match_data = pcre2_match_data_create_from_pattern(neg_pat, NULL);
  while (1) {
    re_replace(neg_pat, "{\"t\":3,\"v\":{\"t\":0,\"v\":0}-$brack}", (char*)output, noutput, 1024);
    re_replace(neg_pat, "{\"t\":3,\"v\":{\"t\":0,\"v\":0}-$brack}", (char*)noutput, output, 1024);
    int rc = pcre2_match(neg_pat,(PCRE2_SPTR)output,1024,0,0,neg_match_data,NULL);
    if (rc == PCRE2_ERROR_NOMATCH) break;
  }
  pcre2_code *mul_pat = re_compile("(?<lbrack>\\{(?>[^{}]|\\g<lbrack>)*\\})\\*(?<rbrack>\\{(?>[^{}]|\\g<rbrack>)*\\})");
  while (strstr((char*)output, "}*{") != NULL) {
      re_replace(mul_pat, "{\"t\":2,\"o\":1,\"l\":$lbrack,\"r\":$rbrack}", (char*)output, noutput, 1024);
      re_replace(mul_pat, "{\"t\":2,\"o\":1,\"l\":$lbrack,\"r\":$rbrack}", (char*)noutput, output, 1024);
  }
  pcre2_code *div_pat = re_compile("(?<lbrack>\\{(?>[^{}]|\\g<lbrack>)*\\})\\/(?<rbrack>\\{(?>[^{}]|\\g<rbrack>)*\\})");
  while (strstr((char*)output, "}/{") != NULL) {
      re_replace(div_pat, "{\"t\":2,\"o\":2,\"l\":$lbrack,\"r\":$rbrack}", (char*)output, noutput, 1024);
      re_replace(div_pat, "{\"t\":2,\"o\":2,\"l\":$lbrack,\"r\":$rbrack}", (char*)noutput, output, 1024);
  }
  pcre2_code *mod_pat = re_compile("(?<lbrack>\\{(?>[^{}]|\\g<lbrack>)*\\})%(?<rbrack>\\{(?>[^{}]|\\g<rbrack>)*\\})");
  while (strstr((char*)output, "}%{") != NULL) {
      re_replace(mod_pat, "{\"t\":2,\"o\":5,\"l\":$lbrack,\"r\":$rbrack}", (char*)output, noutput, 1024);
      re_replace(mod_pat, "{\"t\":2,\"o\":5,\"l\":$lbrack,\"r\":$rbrack}", (char*)noutput, output, 1024);
  }
  pcre2_code *add_pat = re_compile("(?<lbrack>\\{(?>[^{}]|\\g<lbrack>)*\\})\\+(?<rbrack>\\{(?>[^{}]|\\g<rbrack>)*\\})");
  while (strstr((char*)output, "}+{") != NULL) {
      re_replace(add_pat, "{\"t\":2,\"o\":3,\"l\":$lbrack,\"r\":$rbrack}", (char*)output, noutput, 1024);
      re_replace(add_pat, "{\"t\":2,\"o\":3,\"l\":$lbrack,\"r\":$rbrack}", (char*)noutput, output, 1024);
  }
  pcre2_code *sub_pat = re_compile("(?<lbrack>\\{(?>[^{}]|\\g<lbrack>)*\\})-(?<rbrack>\\{(?>[^{}]|\\g<rbrack>)*\\})");
  while (strstr((char*)output, "}-{") != NULL) {
      re_replace(sub_pat, "{\"t\":2,\"o\":4,\"l\":$lbrack,\"r\":$rbrack}", (char*)output, noutput, 1024);
      re_replace(sub_pat, "{\"t\":2,\"o\":4,\"l\":$lbrack,\"r\":$rbrack}", (char*)noutput, output, 1024);
  }
  cJSON *validation_json = cJSON_Parse((char*)output);
  if (validation_json == NULL) {
    printf("%s\n", cJSON_Print(validation_json));
    cJSON_free(validation_json);
    return NULL;
  } else {
    cJSON_free(validation_json);
    return (char*) output;
  }
}

// Linear search for a string
int str_index(char *item, char *arr[], int len) {
  int i = 0;
  while (i < len) {
    if (strcmp(item, *(arr+i)) == 0) return i;
    i++;
  }
  return -1;
}

// Slice a string
void slice_str(char * str, char * buffer, size_t start, size_t end)
{
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

// Check if a str starts with substr
bool startswith(char *str, char *substr) {
  return strncmp(str, substr, strlen(substr)) == 0;
}

// Recursive compilation
VM_Code compiler(cJSON *ast, char **variables, int varsc) {
  // Get top node type
  cJSON *type_cjson = cJSON_GetObjectItemCaseSensitive(ast, "t");
  char *node_type_ch = cJSON_Print(type_cjson);
  int node_type = atoi(node_type_ch);
  // Instruction array
  VM_Instruction instructions[1024];
  int pos = 0;
  // Handle top node
  if (node_type == 0) {
    instructions[0].instructionType = PUSH;
    VM_Object num;
    cJSON *value_cjson = cJSON_GetObjectItemCaseSensitive(ast, "v");
    char *value_ch = cJSON_Print(value_cjson);
    double val;
    sscanf(value_ch, "%lf", &val);
    num.objectType = NUMBER;
    num.value.number = val;
    instructions[0].pushed = num;
  } else if (node_type == 1) {
    VM_Object word;
    cJSON *value_cjson = cJSON_GetObjectItemCaseSensitive(ast, "v");
    char *value_old = cJSON_Print(value_cjson);
    char *value = malloc(strlen(value_old)-1);
    slice_str(value_old, value, 1, strlen(value_old)-2);
    word.objectType = VARIABLE;
    word.value.variableIndex = str_index(value, variables, varsc);
    instructions[0].pushed = word;
    instructions[0].instructionType = PUSH;
  } else if (node_type == 2) {
    VM_Instruction op_instruction;
    op_instruction.instructionType = OPERATION;
    cJSON *op_cjson = cJSON_GetObjectItemCaseSensitive(ast, "o");
    char *op_ch = cJSON_Print(op_cjson);
    int op_type = atoi(op_ch);
    op_instruction.opType = op_type;
    // Compile left and right
    cJSON *left_cjson = cJSON_GetObjectItemCaseSensitive(ast, "l");
    cJSON *right_cjson = cJSON_GetObjectItemCaseSensitive(ast, "r");
    VM_Code left_code = compiler(left_cjson, variables, varsc);
    VM_Code right_code = compiler(right_cjson, variables, varsc);
    // Append code to instructions
    int i = 0;
    while (i < left_code.length) {
      instructions[pos] = *(left_code.instructions+i);
      i++;
      pos++;
    }
    i = 0;
    while (i < right_code.length) {
      instructions[pos] = *(right_code.instructions+i);
      i++;
      pos++;
    }
    instructions[pos] = op_instruction; // Append operation instruction
  } else if (node_type == 3) {
    // Compile parenthesis content
    cJSON *para_cjson = cJSON_GetObjectItemCaseSensitive(ast, "v");
    VM_Code para_code = compiler(para_cjson, variables, varsc);
    int i = 0;
    while (i < para_code.length) {
      instructions[pos] = *(para_code.instructions+i);
      i++;
      pos++;
    }
  } else if (node_type == 4) {
    // Compile function content
    cJSON *para_cjson = cJSON_GetObjectItemCaseSensitive(ast, "v");
    VM_Code para_code = compiler(para_cjson, variables, varsc);
    int i = 0;
    while (i < para_code.length) {
      instructions[pos] = *(para_code.instructions+i);
      i++;
      pos++;
    }
    // Add function call
    char *name_old = cJSON_Print(cJSON_GetObjectItemCaseSensitive(ast, "f"));
    char *name = malloc(strlen(name_old)-1);
    slice_str(name_old, name, 1, strlen(name_old)-2);
    VM_Instruction func_instruction;
    func_instruction.instructionType = FUNC;
    // Logarithms
    if (startswith(name, "log") && strcmp(name, "log")) {
      double log_base = (double) atoi(name + 3);
      func_instruction.funcType = LOG;
      func_instruction.base = log_base;
    } else if (strcmp(name, "log") == 0) {
      double log_base = 10;
      func_instruction.funcType = LOG;
      func_instruction.base = log_base;
    } else if (strcmp(name, "ln") == 0) {
      double log_base = M_E;
      func_instruction.funcType = LOG;
      func_instruction.base = log_base;
    } else if (strcmp(name, "sin") == 0) {
      func_instruction.funcType = SIN;
    } else if (strcmp(name, "cos") == 0) {
      func_instruction.funcType = COS;
    } else if (strcmp(name, "tan") == 0) {
      func_instruction.funcType = TAN;
    } else if (strcmp(name, "csc") == 0) {
      func_instruction.funcType = CSC;
    } else if (strcmp(name, "sec") == 0) {
      func_instruction.funcType = SEC;
    } else if (strcmp(name, "cot") == 0) {
      func_instruction.funcType = COT;
    } else if (strcmp(name, "arcsin") == 0 || strcmp(name, "asin") == 0) {
      func_instruction.funcType = ARCSIN;
    } else if (strcmp(name, "arccos") == 0 || strcmp(name, "acos") == 0) {
      func_instruction.funcType = ARCCOS;
    } else if (strcmp(name, "arctan") == 0 || strcmp(name, "atan") == 0) {
      func_instruction.funcType = ARCTAN;
    } else if (strcmp(name, "sqrt") == 0) {
      func_instruction.funcType = SQRT;
    }
    instructions[pos] = func_instruction; // Append function instruction
  } else if (node_type == 5) {
    // Compile factorial content
    cJSON *para_cjson = cJSON_GetObjectItemCaseSensitive(ast, "v");
    VM_Code para_code = compiler(para_cjson, variables, varsc);
    int i = 0;
    while (i < para_code.length) {
      instructions[pos] = *(para_code.instructions+i);
      i++;
      pos++;
    }
    VM_Instruction factorial_instruction;
    factorial_instruction.instructionType = FACTORIAL;
    instructions[pos] = factorial_instruction;
  }
  
  // Return
  VM_Instruction *output_instructions = malloc(sizeof(VM_Instruction) * (pos+1));
  memcpy(output_instructions, instructions, sizeof(VM_Instruction) * (pos+1));
  VM_Code output;
  output.instructions = output_instructions;
  output.length = pos+1;
  return output;
}

// Execute stack machine instructions
double VM_Exec(VM_Code code, double *fvars) {
  size_t i = 0;
  double stack[512];
  int stack_pos = -1;
  while (i < code.length) {
    VM_Instruction instruction = *(code.instructions + i);
    if (instruction.instructionType == PUSH) {
      VM_Object object = instruction.pushed;
      if (object.objectType == NUMBER) {
        stack[++stack_pos] = object.value.number;
      } else if (object.objectType == VARIABLE) {
        stack[++stack_pos] = *(fvars+object.value.variableIndex);
      }
    } else if (instruction.instructionType == OPERATION) {
      int op_type = instruction.opType;
      double left = stack[stack_pos - 1];
      double right = stack[stack_pos];
      stack_pos--;
      if (op_type == 0) {
        stack[stack_pos] = pow(left, right);
      } else if (op_type == 1) {
        stack[stack_pos] = left*right;
      } else if (op_type == 2) {
        stack[stack_pos] = left/right;
      } else if (op_type == 3) {
        stack[stack_pos] = left+right;
      } else if (op_type == 4) {
        stack[stack_pos] = left-right;
      } else if (op_type == 5) {
        stack[stack_pos] = fmod(left, right);
      }
    } else if (instruction.instructionType == FUNC) {
      enum funcs_enum ftype = instruction.funcType;
      if (ftype == LOG) {
        stack[stack_pos] = stack[stack_pos] > 0 ? log(stack[stack_pos]) / log(instruction.base) : NAN;
      } else if (ftype == SIN) {
        stack[stack_pos] = sin(stack[stack_pos]);
      } else if (ftype == COS) {
        stack[stack_pos] = cos(stack[stack_pos]);
      } else if (ftype == TAN) {
        stack[stack_pos] = tan(stack[stack_pos]);
      } else if (ftype == CSC) {
        stack[stack_pos] = 1/sin(stack[stack_pos]);
      } else if (ftype == SEC) {
        stack[stack_pos] = 1/cos(stack[stack_pos]);
      } else if (ftype == COT) {
        stack[stack_pos] = 1/tan(stack[stack_pos]);
      } else if (ftype == ARCSIN) {
        stack[stack_pos] = asin(stack[stack_pos]);
      } else if (ftype == ARCCOS) {
        stack[stack_pos] = acos(stack[stack_pos]);
      } else if (ftype == ARCTAN) {
        stack[stack_pos] = atan(stack[stack_pos]);
      } else if (ftype == SQRT) {
        stack[stack_pos] = stack[stack_pos] >= 0 ? sqrt(stack[stack_pos]) : NAN;
      }
    } else if (instruction.instructionType == FACTORIAL) {
      stack[stack_pos] = tgamma(stack[stack_pos]);
    }
    i++;
  }
  return stack[0];
}

// Used for debugging
void VM_Print(VM_Code code) {
  size_t i = 0;
  while (i < code.length) {
    VM_Instruction instruction = *(code.instructions + i);
    if (instruction.instructionType == PUSH) {
      if (instruction.pushed.objectType == NUMBER) {
        printf("PUSH %g\n", instruction.pushed.value.number);
      } else {
        printf("PUSH VAR%i\n", instruction.pushed.value.variableIndex);
      }
    } else if (instruction.instructionType == OPERATION) {
      char *ops[] = {"EXP","MUL","DIV","ADD","SUB","MOD"};
      char *operation = ops[instruction.opType];
      printf("%s\n",operation);
    } else if (instruction.instructionType == FUNC) {
      enum funcs_enum ftype = instruction.funcType;
      if (ftype == LOG) {
        printf("LOGARITHM BASE%f \n", instruction.base);
      } else if (ftype == SIN) {
        printf("SINE\n");
      } else if (ftype == COS) {
        printf("COSINE\n");
      } else if (ftype == TAN) {
        printf("TANGENT\n");
      } else if (ftype == CSC) {
        printf("COSECANT\n");
      } else if (ftype == SEC) {
        printf("SECANT\n");
      } else if (ftype == COT) {
        printf("COTANGENT\n");
      } else if (ftype == ARCSIN) {
        printf("ARCSINE\n");
      } else if (ftype == ARCCOS) {
        printf("ARCCOS\n");
      } else if (ftype == ARCTAN) {
        printf("ARCTANGENT\n");
      } else if (ftype == SQRT) {
        printf("SQRT\n");
      }
    } else if (instruction.instructionType == FACTORIAL) printf("FACTORIAL\n");
    i++;
  }
}