#ifndef TERM_GCALC_H
#define TERM_GCALC_H
char *string_of_char(char s, size_t len);
char **alloc_grid(size_t width);
void print_grid(char **grid, size_t width);
int term_gcalc_main();
#endif