#ifndef __FRONT_END__
#define __FRONT_END__

#include <iostream>
#include <set>
#include "InputTheory.h"
#include "common.h"
#include "Cnf.h"
using namespace std;

namespace zap
{
Cnf read_cnf(int argc, char** argv);  
InputTheory parse_input(int argc, char** argv);
void output_up_stats();
void output_solver_stats();
void output_result(Outcome result);
bool get_token(string& token, string& line);

}
#endif
