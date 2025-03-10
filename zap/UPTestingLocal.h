////////////////////////////   UP TESTING  ///////////////////////////////////////////////////////////////////
/*
  This is code to implement the UP testing framework.  This framework is implemented by all the solvers we
  test for unit propagation performance.  Take a look in common/front_end/UPTesting.h for more on what this is.

*/
#include "UPTesting.h"
using namespace zap;

class DPLLSolver : public virtual UPTestingInterface
{
  Assignment    upt_assignment;
  ClauseSet*    upt_clause_set;
public:
  DPLLSolver(ClauseSet* c) : upt_clause_set(c)
  { upt_assignment.initialize(upt_clause_set->number_variables(),&upt_clause_set->VSIDS_counts()); }
  
  AnnotatedLiteral upt_local_select_branch();
  bool             upt_assignment_is_full();
  Result           upt_unit_propagate(AnnotatedLiteral l);
  Result           upt_undo_decision(AnnotatedLiteral& l);
  Result           upt_preprocess();
};




