#!/usr/bin/env bash
echo "Welcome to GCalc, a graphing calculator I made for a Capstone project."
echo "Made by supremestdoggo"
if [ $# -eq 0 ]
then
  while true
  do
    echo "To use ROOT mode, type r. To use terminal mode, type t. To clear the screen, type c. To display README, type d. To quit, type q."
    read inp
    case "$inp" in
      "q") exit;;
      "r") echo "Please input '.q' into ROOT once you are finished viewing the graph.";root -l gcalc.C;;
      "t") ./term_gcalc;;
      "c") clear;;
      "d") clear;columns=$(stty -a | ggrep -Po '\d+(?= columns)');fold -sw $columns <README;;
    esac
    read
  done
else
  if [ $1 = "-t" ]
  then
    while true
    do
      columns=$(stty -a | ggrep -Po '\d+(?= columns)')
      echo "To use graph a function, type t. To use the interactive calculator, type i. To clear the screen, type c. To display README, type d."
      read inp
      case "$inp" in
        "t") ./term_gcalc;;
        "i") ./interactive_calc;;
        "c") clear;echo "Welcome to GCalc, a graphing calculator I made for a Capstone project.";;
        "d") clear;columns=$(stty -a | ggrep -Po '\d+(?= columns)');fold -sw $columns <README;;
      esac
    done
  else
    while true
    do
      columns=$(stty -a | ggrep -Po '\d+(?= columns)')
      echo "To use ROOT mode, type r. To use terminal mode, type t. To clear the screen, type c. To display README, type d."
      read inp
      case "$inp" in
        "r") root -q -l gcalc.C;;
        "t") ./term_gcalc;;
        "c") clear;;
        "d") clear;columns=$(stty -a | ggrep -Po '\d+(?= columns)');fold -sw $columns <README;;
      esac
      read
    done
  fi
fi