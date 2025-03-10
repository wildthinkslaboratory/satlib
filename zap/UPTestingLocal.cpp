
////////////////////////////   UP TESTING  ///////////////////////////////////////////////////////////////////
/*
  This is code to implement the UP testing framework.  This framework is implemented by all the solvers we
  test for unit propagation performance.  The UP testing framework and this interface are described in
  common/front_end/UPTesting.h and common/front_end/UPTesting.cpp.  

*/
#include "UPTestingLocal.h"
#include "ClauseSet.h"
#include "solver.h"
#include "Cnf.h"

AnnotatedLiteral DPLLSolver::upt_local_select_branch()   // UP testing has a setting that allows the solver to read
{                                                    // branch decisions from a file.  If this setting is off
  AnnotatedLiteral l = select_branch(upt_assignment);// then call the method local to this solver called
  return AnnotatedLiteral(Literal(l.variable(),l.sign()));   // select_branch.
}


bool DPLLSolver::upt_assignment_is_full()
{
  return upt_assignment.full();
}

Result DPLLSolver::upt_unit_propagate(AnnotatedLiteral l) 
{
  upt_assignment.extend(l);
  Result r = unit_propagate(*upt_clause_set,upt_assignment);
  return r;
}

Result DPLLSolver::upt_undo_decision(AnnotatedLiteral& l)
{
  if (upt_assignment.current_level() == 0) return FAILURE;
  Literal lit = upt_assignment.undo_current_decision();  // undo the current decision and return the branch decision
  l = AnnotatedLiteral(lit.negate(),Reason(true));    // pass the negation of the branch back so we can try
  return SUCCESS;                                        // the other value
}

Result DPLLSolver::upt_preprocess()
{
  return unit_propagate(*upt_clause_set,upt_assignment);
}




