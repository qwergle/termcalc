#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "calc.h"
#include <math.h>
#include "intcalc.h"

int interactive_calc_main() {
    size_t line = 0;
    char *vars[] = {"pi","e"};
    double fvars[] = {M_PI, M_E};
    printf("Simply type in a mathematical expression and the result will be printed. Input \"exit\" to exit.\n");
    while (1) {
        printf("%lu: ", line+1);
        char *expr = getstr();
        if (strcmp(expr, "exit") == 0) break;
        char *parsed_code = parser(expr, vars, 2);
        if (parsed_code == NULL) {
            printf("Syntax Error\n");
        } else {
            VM_Code code = compiler(cJSON_Parse(parsed_code), vars, 2);
            double value = VM_Exec(code, fvars);
            if (isnan(value)) puts("NaN");
            else printf("%g\n", VM_Exec(code, fvars));
        }
        line++;
    }
    return 0;
}