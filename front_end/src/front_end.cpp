#include <fstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <sys/resource.h>
#include "front_end.h"
#include "InputTheory.h"
#include "Cnf.h"
#include "Converter.h"
using namespace zap;

int yyparse();
extern FILE* yyin;
extern zap::InputTheory mainTheory;

namespace zap
{

// for up trial limit
extern long unsigned trial_count;        // the number of trials that have occurred so far
extern long unsigned sample_start_count; // we start timing when we hit this many trials
extern long unsigned sample_size;         // this is the total number of trials we want to sample
extern ifstream branch_in;         // used to read branch decisions from a file
extern ofstream branch_out;        // used to write branch decisions to an output file
extern string branch_file_in;
extern string branch_file_out;
extern double sample_start_time;
extern double sample_finish_time;


//////////////////////////////////////  PARSING INPUT FILE  ///////////////////////////////////////////

FILE * open_file(string name)
{
  FILE * ans = fopen(name.c_str(),"r");
  if (ans) return ans;
  string longname = name + ".zap";
  ans = fopen(longname.c_str(),"r");
  if (ans) return ans;
  longname = name + ".cnf";
  return fopen(longname.c_str(),"r");
}


string delimiters("\t");
string zeroStr("0");
bool get_token(string &token, string &line)
{
  string::size_type begIdx = line.find_first_not_of(delimiters);
  if (begIdx == string::npos) return false;
  string::size_type endIdx = line.find_first_of(delimiters,begIdx+1);
  if (endIdx == string::npos) endIdx = line.size();
  int offset = endIdx - begIdx;
  token = line.substr(begIdx,offset);
  line.erase(0,endIdx);
  return true;
}



void output_commandline_args()
{
  cout << "Command line arguments are: " << endl;
  cout << setw(20) << left << "     -c #"
       << "desired clauseset type 0:CNF, 1:PFS, 2:SYMRES, 3:GROUP_BASED" << endl
       << setw(20) << left << "     -b #"
       << "symres length bound" << endl
       << setw(20) << left << "     -a"
       << "turn off vsids" << endl
       << setw(20) << left << "     -e"
       << "random seed" << endl
       << setw(20) << left << "     -r"
       << "read branch decisions from user" << endl
       << setw(20) << left << "     -d"
       << "run DPLL and don't do any learning" << endl
       << setw(20) << left << "     -u"
       << "don't use the structure when in PFS mode" << endl
       << setw(20) << left << "     -z"
       << "sample size for UP testing" << endl
       << setw(20) << left << "     -s"
       << "number of trials that go by before starting the sample" << endl
       << setw(20) << left << "     -t #"
       << "time out" << endl
       << setw(20) << left << "     -i <file>"
       << "file to read branch decisions from" << endl
       << setw(20) << left << "     -l"
       << "test local search unit propagation" << endl
       << setw(20) << left << "     -f #"
       << "local search fix attempts " << endl
       << setw(20) << left << "     -m #"
       << "local search map attempts " << endl
       << setw(20) << left << "     -o <file>"
       << "file to write branch decisions to" << endl;
}



void read_testing_params(int argc, char **argv)
{
  if (argc < 2) return;
  
  for (int i=2; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'a' : global_vars.branching_heuristic_on = false; break;
      case 'd' : global_vars.dpll = true; break; // UP testing on  (call dpll instead of regular solve)
      case 'z' : ++i; sample_size = atoi(argv[i]); break;
      case 'e' : ++i; srand(atoi(argv[i])); break;
      case 'b' : ++i; global_vars.symres_bound = atoi(argv[i]); break;
      case 's' : ++i; sample_start_count = atoi(argv[i]); global_vars.dpll = true; break;
      case 't' : ++i; global_vars.time_out = atof(argv[i]); break;
      case 'c' : ++i; global_vars.desired_type = (ClauseSetType)(atoi(argv[i])); break;
      case 'i' : ++i; branch_file_in = argv[i]; break;
      case 'o' : ++i; branch_file_out = argv[i]; break;
      case 'l' : ++i; global_vars.test_local_search_up = true; break;
      case 'u' : ++i; global_vars.use_structure = false; break;
      case 'f' : ++i; global_vars.fix_attempts = atoi(argv[i]); break;
      case 'm' : ++i; global_vars.map_attempts = atoi(argv[i]); break;
      case 'r' : ++i; global_vars.read_branch_from_user = true; break;
      default: output_commandline_args(); exit(0);
      }
    }
    else {
      cerr << "Bad command line argument " << argv[i] << endl;
      exit(0);
    }
  }
  
  if (branch_file_in.length()) branch_in.open(branch_file_in.c_str());
  if (branch_file_out.length()) branch_out.open(branch_file_out.c_str());
}



InputTheory parse_input(int argc, char **argv)
{
  read_testing_params(argc,argv);
  string input_filename = argv[1];
  yyin = open_file(input_filename);

  yyparse();

  return mainTheory;
}



Cnf read_cnf(int argc, char **argv)
{
  InputTheory input = parse_input(argc,argv);
  ClauseSetBuilder builder(input);
  builder.convert_to(CNF);
  return builder.get_cnf();
}




//////////////////////////////////////////////  OUTPUT RESULTS /////////////////////////////////////////

double precision = 4.0;

double round_time(double t)
{
  double factor = pow(10.0,precision);
  double min_time = 1 / factor;
  if (t < min_time) return min_time;
  
  return round( t * factor) / factor;
}

void output_up_stats()
{
  cout << "***********************************  UP Testing Results  *************************************" << endl;
  if (global_vars.dpll) cout << "Solving with DPLL" << endl;

  double time = round_time(sample_finish_time - sample_start_time);
  
  cout << setw(20) << left << "sample start"
       << setw(20) << left << "sample end"
       << setw(20) << left << "clauses touched"
       << setw(20) << left << "sample time"
       << setw(20) << left << "touch/time" << endl;
  cout << "--------------------------------------------------"
       << "-----------------------------------------------###" << endl;
  cout << setw(20) << left << sample_start_count
       << setw(20) << left << trial_count
       << setw(20) << left << global_vars.clauses_touched
       << setw(20) << left << time
       << setw(20) << left << (time > 0 ? global_vars.clauses_touched/time : 0.0) << endl;
  
  cout << "**********************************************************************************************" << endl;
}



void output_solver_stats()
{
  cout << "***********************************  Solver Results  ****************************************" << endl;

  cout << setw(20) << left << "Result code"
       << setw(20) << left << "decisions"
       << setw(20) << left << "clauses touched"
       << setw(20) << left << "time(sec)"
       << setw(20) << left << "queue pops" 
       << setw(20) << left << "literals touched"
	   << setw(20) << left << "nogoods used"
	   << setw(20) << left << "backtracks" << endl;
  cout << "-------------------------------------------------"
       << "----------------------------------------------###" << endl;
  cout << setw(20) << left << global_vars.result
       << setw(20) << left << global_vars.number_branch_decisions
       << setw(20) << left << global_vars.clauses_touched
       << setw(20) << left << round_time(global_vars.solution_time)
       << setw(20) << left << global_vars.queue_pops 
       << setw(20) << left << global_vars.literals_touched
	   << setw(20) << left << global_vars.prop_from_nogoods
	   << setw(20) << left << global_vars.number_backtracks << endl;
  
  cout << "********************************************************************************************" << endl;
}


void output_result(Outcome result)
{
  switch (result) {
  case UNSAT : cout << "Result:  UNSAT" << endl; break;
  case SAT : cout << "Result:  SAT" << endl; break;
  case TIME_OUT : cout << "Result: TIME_OUT" << endl; break;
  case SAMPLE_FINISHED : cout << "Result:  SAMPLE_FINISHED" << endl; break;
  default : cout << "Unknown return value " << endl; break;
  };
}


} // end namespace zap
