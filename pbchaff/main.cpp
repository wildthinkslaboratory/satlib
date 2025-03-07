/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/
#include "UPTesting.h"

#include <iostream>
#include <iomanip>
#include <string>

#include "Solver.h"



using namespace std;

void showCommandLine(char c)
{
  cout << "Unknown flag -" << c << endl;
  cout << endl << "recognized flags: " << endl;
  cout << "   -h #     branch heuristic " << endl;
  cout << "                0 : VSIDs heuristic (default)" << endl;
  cout << "                1 : first unvalued literal " << endl;
  cout << "                2 : random unvalued literal " << endl;
  cout << "   -m       learn method " << endl;
  cout << "                0 : learn cardinality constraints (default) " << endl;
  cout << "                1 : learn pseudo-Boolean constraint" << endl;
  cout << "   -l #     time limit" << endl;
  cout << "   -p       preprocess problem" << endl;
  cout << "   -a       turn off Andrew's optimization (default to on) " << endl;
  cout << "   -f       turn on failed literal testing preprocessing " << endl;
  cout << "   -o       input file is in .opb format " << endl;
  cout << "   -d       display  solution " << endl;
  cout << "   -s #     random seed " << endl;
  cout << "   -v       use verbose mode" << endl << endl;
}



int main(int argc, char *argv[]) {

  bool displaySolution = true;
  bool opbFormat = false;
  bool verbosity = false;
  bool preprocess = true;
  bool failedLiteral = false;
  bool andrewOpt = true;
  int timeLimit = 0;
  int branchHeuristic = 0;
  int learnMethod = 0;
  int randomSeed = 0;
  string filename;
  
  // get the command line information
//   for (size_t i = 1 ; i < argc ; ++i) {
//     if (argv[i][0] == '-') {
// 	switch (argv[i][1])
// 	  {
// 	  case 'h':
// 	    {
// 	      branchHeuristic = atoi(argv[++i]);
// 	      cout << "c  Branching Heuristic : ";
// 	      switch (branchHeuristic)
// 		{
// 		case 0: cout << "VSIDS"; break;
// 		case 1: cout << "First unvalued"; break;
// 		case 2: cout << "Random unvalued"; break;
// 		default:  cout << "Unknown, using default"; break;
// 		}
// 	      cout << endl;
// 	      break;
// 	    }
// 	    case 'm':
// 	    {
// 	      learnMethod = atoi(argv[++i]);
// 	      cout << "c  Learn Method : ";
// 	      switch (learnMethod)
// 		{
// 		case 0: cout << "cardinality"; break;
// 		case 1: cout << "pseudo-Boolean"; break;
// 		default:  cout << "Unknown, using default"; break;
// 		}
// 	      cout << endl;
// 	      break;
// 	    }

// 	  case 's':
// 	    {
// 	      randomSeed = atoi(argv[++i]);
// 	      cout << "c  Random seed : " << randomSeed << endl;
// 	      break;
// 	    }
// 	  case 'l': timeLimit = atoi(argv[++i]); break;
// 	  case 'a': andrewOpt = false; break;
// 	  case 'f': failedLiteral = true; break;
// 	  case 'p': preprocess = true; break;
// 	  case 'v': verbosity = true; break;
// 	  case 'o': opbFormat = true; break;
// 	  case 'd': displaySolution = true; break;
// 	  default:
// 	    showCommandLine(argv[i][1]);
// 	    exit(1);
// 	  }
//     }
//     else
//       {
// 	if (filename.length() != 0)
// 	  {
// 	    fatalError("Too many files specified");
// 	  }
// 	filename = argv[i];
//       }
//   }


  cout << "c  Solving instance " << argv[1] << "......" << endl << endl;
  
#ifdef VERIFY
  cout << endl
       << "\t\t\t****************" << endl;
  cout << "\t\t\tValidation is ON" << endl;
  cout << "\t\t\t****************" << endl;
#endif

  SolverStatistics stats;
  ClauseSetStatistics clauseStats;
  cout.setf(ios::left, ios::adjustfield);
  
  Solver s;
  srand(randomSeed);
  s.setBranchHeuristic(branchHeuristic);
  s.setLearnMethod(learnMethod);
  s.setVerbosity(verbosity);
  s.setFailedLiteral(failedLiteral);
  s.setAndrewOpt(andrewOpt);
  if (timeLimit) s.setTimeLimit(timeLimit);
//   if (opbFormat) s.parseOPB(filename.data());
//   else s.parse(filename.data());

  zap::Cnf clauses = zap::read_cnf(argc,argv);
  s.load_to_structures(clauses);
  zap::global_vars.start_time = zap::get_cpu_time();
  
  if (preprocess)
    {
      s.preprocess();
      stats = s.getSolverStatistics();
      clauseStats = s.getClauseSetStatistics();
      cout << "Preprocessing Statistics" << endl;
      cout << "--------------------------------------------" << endl;
      cout << setw(30) << "Preprocess Time" << setw(15) << stats.preprocessTime << endl;
      cout << setw(30) << "Initial Clause Count" << setw(15) << clauseStats.initialClauseCount
	   << endl;
      int newClauseCount = clauseStats.initialClauseCount +
	clauseStats.addedClauseCount - clauseStats.deletedClauseCount;
      cout << setw(30) << "Final Clause Count" << setw(15) << newClauseCount
	   << endl << endl << endl;
    }
  
  s.solve();

  zap::global_vars.solution_time = zap::get_cpu_time() - zap::global_vars.start_time;

  stats = s.getSolverStatistics();
  clauseStats = s.getClauseSetStatistics();
  cout << "c  Solution Statistics" << endl;
  cout << "c  --------------------------------------------" << endl;
  cout << setw(30) << "c  Node count" << setw(15) << stats.nodeCount << endl;
  cout << setw(30) << "c  Backtrack count" << setw(15) << stats.backtrackCount << endl;
  cout << setw(30) << "c  Initial Clause Count" << setw(15) << clauseStats.initialClauseCount << endl;
  cout << setw(30) << "c  Added Clauses" << setw(15) << clauseStats.addedClauseCount << endl;
  cout << setw(30) << "c  Deleted Clauses" << setw(15) << clauseStats.deletedClauseCount << endl;

  switch (stats.outcome)
  {
  case SATISFIABLE:
	if (displaySolution)
	{
	  cout << "v ";
	  s.printAssignment();
	}
	zap::global_vars.result = zap::SAT;
	cout << "s SATISFIABLE" << endl;
	break;
  case UNSATISFIABLE:
	zap::global_vars.result = zap::UNSAT;
	cout << "s UNSATISFIABLE" << endl;
	break;
  case TIME_OUT:
	zap::global_vars.result = zap::TIME_OUT;
	cout << "c  Time limit was exceeded.  Excecution time : "
		 << stats.runTime << endl;
	cout << "s UNKNOWN" << endl;
	break;
  default:
	cout << "c  Termination for unknown reason" << endl;
	cout << "s UNKNOWN" << endl;
	exit(5);
  }
  

  cout << endl << filename << "c Run time : "
       << stats.runTime
       << endl << endl;
  
  zap::output_solver_stats();
  zap::output_result(zap::global_vars.result);
}
