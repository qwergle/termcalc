// ROOT macro used for graphing
// Do not use.
#include "calc.h"
#include <iostream>
#include <cstring>
#include <string>
#include <stdexcept>

void gcalc() {
  gSystem->Load("./libgcalc.so");
  char **vars = new char *[3];
  vars[0] = strdup("x");
  vars[1] = strdup("pi");
  vars[2] = strdup("e");
  double fvars[] = {1, 3.14, 2.71};
  std::cout << "Please input a mathematical expression... ";
  std::string ccexpr;
  std::cin >> ccexpr;
  char * expr = new char [ccexpr.length()+1];
  std::strcpy(expr, ccexpr.c_str());
  VM_Code code = compiler(cJSON_Parse(parser(expr, vars, VARS_LEN)), vars, VARS_LEN);
  double detail = 1.0;
  double maxX;
  double minX;
  int depth = 0;
  std::cout << "Minimum value of X: ";
  std::cin >> minX;
  std::cout << "Maximum value of X: ";
  std::cin >> maxX;
  TCanvas *c1 = new TCanvas("Graph","Graph",200,10,600,400);
  c1->Clear();
  c1->SetGrid();
  auto *gr = new TGraph();
  gr->SetTitle("Graph");
  gr->GetXaxis()->SetTitle("X");
  gr->GetYaxis()->SetTitle("Y");
  while (depth < 4) {
    double x = minX;
    while (x < maxX) {
      fvars[0] = x;
      gr->AddPoint(x, VM_Exec(code, fvars));
      x += detail;
    }
    detail = detail/8;
    depth++;
    gr->Draw("AC");
    c1->Update();
    c1->Modified();
  }
  return;
}