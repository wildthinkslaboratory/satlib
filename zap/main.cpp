#include <iomanip>
#include "UPTesting.h"
#include "Cnf.h"
#include "UPTestingLocal.h"
#include "solver.h"
#include "Converter.h"

using namespace zap;


int main(int argc, char **argv){
   
   cout << "// heidi_sat " << endl;
   if (argc < 2) exit(0);

   cout << "// parsing problem " << argv[1] << endl;
   InputTheory input = parse_input(argc,argv);
   ClauseSetBuilder builder(input);
   ClauseSet* clauses = builder.convert_to(global_vars.desired_type);
   if (clauses == NULL) quit("Couldn't build PFS clause set");

   cout << "// solving problem " << argv[1] << endl;

   if (global_vars.dpll) {  ///  UP Testing
	  DPLLSolver solver(clauses);
	  solver.upt_dpll();
   }
   else {  /// regular call to solver
	  global_vars.start_time = get_cpu_time();
	  global_vars.result = solve(*clauses);
	  global_vars.solution_time = get_cpu_time() - global_vars.start_time;
	  output_solver_stats();
	  output_result(global_vars.result);
   }
}

