#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "calc.h"
#include "term_gcalc.h"
#include <math.h>

char *string_of_char(char ch, size_t len) {
  char *out = malloc(len + 1);
  *(out + len) = '\0';
  memset(out, ch, len);
  return out;
}

char **alloc_grid(size_t width) {
  char **grid = malloc(width * sizeof(char*));
  int i = 0;
  while (i < width/2) {
    char *row = malloc(width + 1);
    *(row + width) = '\0';
    memset(row, ' ', width);
    *(grid + i) = row;
    i++;
  }
  return grid;
}

void print_grid(char **grid, size_t width) {
  int i = 0;
  printf("+%s+\n", string_of_char('-', width));
  while (i < width/2) {
    printf("|%s|\n", *(grid + i));
    i++;
  }
  printf("0%s+\n", string_of_char('-', width));
  return;
}

int term_gcalc_main() {
  char *vars[] = VARS;
  double fvars[] = {1, M_PI, M_E};
  printf("y = ");
  char *expr = getstr();
  char *parsed_code = parser(expr, vars, VARS_LEN);
  if (parsed_code == NULL) {
    printf("Syntax Error\n");
    exit(1);
  }
  VM_Code code = compiler(cJSON_Parse(parser(expr, vars, VARS_LEN)), vars, VARS_LEN);
  printf("Please input the grid's width... ");
  int width;
  scanf("%i", &width);
  int xOffset, yOffset;
  printf("Please input the center of the X axis... ");
  scanf("%i", &xOffset);
  xOffset = xOffset - width/2;
  printf("Please input the center of the Y axis... ");
  scanf("%i", &yOffset);
  yOffset = yOffset - width/2;
  printf("Please input the zoom/precision level (dots per unit)... ");
  int precision;
  scanf("%i", &precision);
  int maxX = width;
  int x = 0;
  char **grid = alloc_grid(width);
  while (x < maxX) {
    double fX = (double) x + xOffset;
    fvars[0] = fX/precision;
    double output = VM_Exec(code, fvars)*precision - yOffset;
    if (!isnan(output)) {
      double fY = output/2;
      int y = floor(fY);
      if ((y < width/2) && (y >= 0)) {
        char gchar = (fmod(output, 2) >= 1) ? '^' : '.';
        *(*(grid + width/2 - y - 1) + x) = gchar;
      }
    }
    x++;
  }
  print_grid(grid, width);
  return 0;
}