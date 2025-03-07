

////////////////////////////   UP TESTING  ///////////////////////////////////////////////////////////////////
/*
  This is code to implement the UP testing framework.  This framework is implemented by all the solvers we
  test for unit propagation performance.  We want to look at the performance of different unit propagation
  procedures in isolation from the rest of the solver settings and implementation choices.  To do this,
  we have each solver implement the UPTestingInterface and we have each solver use the same problem parser.
  The main public function in the UPTestingInterface is a basic DPLL algorithm (without learning or backjumping).
  Branch decisions can be read in from a file so we can ensure that different solvers are constructing
  identical trees.  As a result each solver must solve an identical set of unit propagation queries and
  we get a fair comparison of unit propagation speed.

*/

/*
  HOW TO SET UP A NEW SAT SOLVER FOR UP TESTING
  
  1. Call my parser to read in the dimacs file
  
       	vector<set<UPTestingLit> > clauses = read_cnf(argc,argv);
	
  2. copy the parsed instance into the LocalSolver's data structures.
     Usually this can be done by building a new constructor
  
       LocalClauseSet(vector<set<UPTestingLit> >& clauses);

  3. make the LocalSolver implement the UPTestingInterface
  
  4. The LocalMain function needs to test for the dpll flag and call LocalSolver.upt_dpll()
     if the flag is set to true.  
       
  5. put the clauses_touched counter and other statistical counters into the unit propagation procedure.

  6. put timing calls and output calls around the upt_dpll call in LocalMain
  
 */

#ifndef __UP_TESTING__
#define __UP_TESTING__

#include "front_end.h"
#include "Assignment.h"
using namespace std;

namespace zap
{
/*
  The upt prefix stands for unit propagation testing.  Solvers may already have functions with these
  names so the prefix keeps us from overlapping with the local namespace.
 */
class UPTestingInterface {
protected:
  UPTestingInterface() { }
  virtual ~UPTestingInterface() { }
  
  virtual AnnotatedLiteral upt_local_select_branch() = 0;   // wrapper for calling the local solver's branching function
  virtual bool             upt_assignment_is_full() = 0;            
  virtual Result           upt_unit_propagate(AnnotatedLiteral l) = 0;  
  virtual Result           upt_undo_decision(AnnotatedLiteral& l) = 0;
  virtual Result           upt_preprocess() = 0;
  AnnotatedLiteral         upt_select_branch();            // may branch from a file or defer to local_select_branch
  Outcome                  upt_dpll_inner_loop();
public:
  void                     upt_dpll();
};



extern long unsigned sample_size;  // the number of unit propagation calls we want to make/sample



} // end namespace zap
#endif
