#include <stdio.h>
#include <stdlib.h>
#include "calc.h"
#include "intcalc.h"
#include "term_gcalc.h"
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

char help_msg[] = "termcalc\n"
    "A mostly functional, command-line calculator\n"
    "ARGUMENTS\n"
    "Only use one argument.\n"
    "-h --help: Displays this message.\n"
    "-i --interactive: Runs the interactive calculator\n"
    "-g --graph -d --display: Runs the graphing calculator\n"
    "-v --version: Displays the version";

char version_str[] = "termcalc 0.1.0";

bool my_stricmp(char *str1, char *str2) {
    register size_t len = strlen(str1);
    if (len != strlen(str2)) return false;
    for (register size_t i = 0; i < len; i++) {
        if (tolower(*(str1+i))!=tolower(*(str2+i))) return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    enum {INTERACTIVE, GRAPHING, HELP, VERSION, UNRECOGNIZEDERR} arg_state;
    if (argc == 1) {
        printf("ERROR: No arguments. Exiting...\n");
        exit(1);
    } else if (argc>2) {
        printf("ERROR: Too many arguments. Exiting...\n");
        exit(1);
    } else {
        char *arg = argv[1];
        if (my_stricmp(arg, "-h") || my_stricmp(arg, "--help")) {
            arg_state = HELP;
        } else if (my_stricmp(arg, "-i") || my_stricmp(arg, "--interactive")) {
            arg_state = INTERACTIVE;
        } else if (my_stricmp(arg, "-g") || my_stricmp(arg, "--graph") || my_stricmp(arg, "-d") || my_stricmp(arg, "--display")) {
            arg_state = GRAPHING;
        } else if (my_stricmp(arg, "-v") || my_stricmp(arg, "--version")) {
            arg_state = VERSION;
        } else {
            arg_state = UNRECOGNIZEDERR;
        }
    }
    switch (arg_state) {
        case HELP :
            puts(help_msg);break;
        case INTERACTIVE :
            return interactive_calc_main();
        case GRAPHING :
            return term_gcalc_main();
        case VERSION :
            puts(version_str);break;
        case UNRECOGNIZEDERR :
            puts("ERROR: Unrecognized argument. Exiting..."); exit(1);
    }
    return 0;
}