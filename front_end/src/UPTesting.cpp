#include "UPTesting.h"
#include "front_end.h"
#include <fstream>
#include <string>

namespace zap
{
ifstream branch_in;         // used to read branch decisions from a file
ofstream branch_out;        // used to write branch decisions to an output file
string branch_file_in;
string branch_file_out;

// for up trial limit
long unsigned trial_count = 0;        // the number of trials that have occurred so far
long unsigned sample_start_count = 0; // we start timing when we hit this many trials
long unsigned sample_size = 0;         // this is the total number of trials we want to sample
double sample_start_time = 0;
double sample_finish_time = 0;
bool sample_started = false;



//////////////////////////////////////////  HELPER FUNCTIONS FOR UPT INTERFACE /////////////////////////////



void start_sample()
{
  sample_start_time = get_cpu_time();
  global_vars.clauses_touched = 0;
  global_vars.literals_touched = 0;
}



void end_sample()
{
  sample_finish_time = get_cpu_time();
}



AnnotatedLiteral branch_from_file()
{
  bool sign = true;
  string line;
  string branch;
  if (!getline(branch_in,line)) {
    cerr << "Unexpected end of branch file " << branch_file_in << endl;
    exit(0);
  }
  
  get_token(branch,line);
  
  if (branch[0] == '-') {
    sign = false;
    branch.erase(0,1);
  }
  
  size_t atom = global_vars.atom_name_map.lookup(branch);
  return AnnotatedLiteral(Literal(atom,sign));
}



void write_branch_to_file(AnnotatedLiteral l)
{
  branch_out << (l.sign() ? " " : "-") << global_vars.atom_name_map.lookup(l.variable()) << endl;
}



///////////////////////////////////////// UNIT PROPAGATION TESTING INTERFACE ///////////////////////



// make sure unit clauses are dealt with
Outcome UPTestingInterface::upt_dpll_inner_loop()
{
  if (upt_preprocess() == CONTRADICTION) return UNSAT;

  AnnotatedLiteral l(Literal(),Reason(1));
  while (!upt_assignment_is_full()) {
    //////////////////////////////////////////////////////////
    if (!sample_started && (trial_count >= sample_start_count)) {  // start sampling
      sample_started = true;
      start_sample();
    }
    if (sample_size && (trial_count >= (sample_start_count + sample_size))) { // end sampling
      return SAMPLE_FINISHED;
    }
    trial_count++;
    /////////////////////////////////////////////////////////

    l = upt_select_branch();
    while (upt_unit_propagate(l) == CONTRADICTION) {
      if (upt_undo_decision(l) == FAILURE) return UNSAT;
    }
  }
  return SAT;
}



AnnotatedLiteral UPTestingInterface::upt_select_branch()
{
  AnnotatedLiteral b;
  if (branch_in.is_open()) b = branch_from_file();
  else b = upt_local_select_branch();

  if (branch_out.is_open()) write_branch_to_file(b);
  
  return b;
}



void UPTestingInterface::upt_dpll()
{
  Outcome result = upt_dpll_inner_loop();
  output_result(result);
  end_sample();
  output_up_stats();
}

} // end namespace zap
