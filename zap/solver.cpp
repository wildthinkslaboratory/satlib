#include "ClauseSet.h"
#include "common.h"
#include "front_end.h"
#include "UPTesting.h"
#include <cmath>
using namespace zap;

bool debug = false;

////////////////////////////////////////////////////////////////////////////////////////////////
/*                           INVARIANT DISCUSSION   (optional)                                */

/* The correctness of this code depends on maintaining certain invariants.  Understanding what these
   invariants are is also the best way to understand this code and sat solvers in general.  They are
   all various properties of partial assignments that are defined in relation to a set of clauses.
   We begin by defining what an annotated partial assignment is.


   DEFINITION: An annotated partial assignment P with respect to a clause set C is an ordered
   sequence {(l_1,c_1),(l_2,c_2),...,(l_n,c_n)} of literal clause pairs, where c_i is the reason
   for literal l_i and either c_i = NULL (indicating that l_i was a branch point) or c_i is
   a clause in C such that l_i is a literal in c_i and c_i is a unit consequence of {l_1,l_2,...,l_{i-1}}.
   An annotated partial assignment is partitioned into 0,1,...,k decision levels where each level
   i consists of the i^{th} branch decision together with the following assignments up to but not
   including the i+1 branch decision. (note that the 0th branch decision is the empty branch decision)
   
   1. An annotated  partial assignment P is invalid with respect to a ClauseSet C if there is a
      clause in C that evaluates to FALSE under P.  Otherwise a partial assignment is valid.

   2. A partial assignment P is closed with respect to a ClauseSet C if no clause in C
      has a unit consequence under P.

   3. For every decision level i in P, let d_i be the subset of assignments in P with decision level
      less than or equal to i.

      A partial assignment P is decision minimal with respect to a ClauseSet C if for every unit consequence l
      in P with decision level i, i is the minimum decision level for which l is a unit consequence of d_i.
 */



//////////////////////////////////////////   RESTARTS CODE   ////////////////////////////////////////////

/*  Periodically solvers erase their entire working partial assignment (with the exception of variables
    forced at level 0) and start over.  Restarts can turn complete methods into ones that are incomplete
    (not guaranteed to find a solution or prove the instance UNSAT) depending on what strategy is used
    to reduce the size of the learned clause set.  However in practice restarts improve performance.
    The progress made in reaching the erased assignment is typically easily reconstructed using the
    retained learned clauses.  Branch decisions made early in the search process can then be evaluated
    again in the light of new information.  Bad decisions made early in the search can be extremely
    costly since the size of the search tree can vary greatly depending on the order and choice of
    branch decisions.
*/

#define LUBY_UNIT  512
size_t number_restarts = 0;
long long unsigned int restart_threshold = LUBY_UNIT;

size_t get_luby(size_t i)
{
  if (i==1) return LUBY_UNIT;
  double k = log2(i+1);

  if (k==round(k)) return (size_t)(pow(2,k-1))*LUBY_UNIT;
  else {
    k = (size_t)floor(k);
    return get_luby(i-(size_t)pow(2,k)+1);
  }
}

void update_restart_parameters()
{
  ++number_restarts;
  restart_threshold = global_vars.number_branch_decisions + get_luby(number_restarts+1);
}



//////////////////////////////////////  BASIC SOLVER FUNCTIONS  //////////////////////////////////////////



AnnotatedLiteral select_branch(Assignment& P)
{
  global_vars.number_branch_decisions++;


  Literal l;
  if (global_vars.read_branch_from_user) {
	P.print_positive();
	l = P.user_input_literal();
  }
  else if (global_vars.branching_heuristic_on) l = P.vsids_literal();  // consult global variable to see which heuristic
  else l = P.random_unvalued_literal();                           // to use.

  if (debug) cout <<  "branching on literal " << l << endl;
  
  return AnnotatedLiteral(l);
}



Result unit_propagate(ClauseSet& C, Assignment& P)
{
  if (P.empty()) C.load_unit_literals(P);            // when we start, P is valid and decision minimal with
                                                     // respect to C, but may not be closed.  Here we compute
  while (!P.closed()) {                              // the closure.  If we make P invalid we return CONTRADICTION.
    Literal l = P.pop_unit_list();
    if (C.get_implications(P,l) == CONTRADICTION) 
      return CONTRADICTION;
  }
  return SUCCESS;
}





Result learn_and_backjump(ClauseSet& C, Assignment& P)
{
  global_vars.number_backtracks++;
  size_t lits_this_level;
  Literal unit_lit;
  ClauseID c_id = NULL_ID;

  do {
	do {
	  Variable v = P.contradiction_variable();
	  Reason c1 = P.get_reason(Literal(v,true));
	  Reason c2 = P.get_reason(Literal(v,false));
	  
	  if (debug) cout << "contradiction " << v << endl << P << endl;

	  c_id = C.resolve(c1,c2,P,lits_this_level,unit_lit);	  
	  const Clause& c = C.clause(c_id);
	  
	  if (c.empty()) return FAILURE; 
	  
	  size_t merge_size = c.merge_size;               // merging/factoring that occured when generating the clause
	  AnnotatedLiteral new_lit(unit_lit,Reason(c_id,c));  // the most recently bound literal is unit
	  P.extend(new_lit);	  
	  if (lits_this_level <= 1) C.add_learned_clause(c_id,P);
	  
	} while (lits_this_level > 1);             // stop when we have a UIP clause
	
        if (debug) cout << "backjumping " << endl;

	P.backjump(P.assertion_level(C.clause(c_id)));
  } while (!C.get_symmetric_unit_lits(P,c_id,unit_lit));
  
  if (debug) cout << " done backing up " << endl << P << endl;

  return SUCCESS;
}


Outcome solve(ClauseSet& C)
{
  GlobalVars& gv = global_vars;
  Assignment P(C.number_variables(),&C.VSIDS_counts());  // The empty assignment is trivially valid and minimal
                                                         // but possibly not closed.  We start with UP to close it.

  
  while (!P.full()) {
    if (unit_propagate(C,P) == CONTRADICTION) {         // At this point P is closed and decision minimal but
                                                        // invalid.  Learning and backjumping make P valid again.
      if (learn_and_backjump(C,P) == FAILURE)
	return UNSAT;
    } 
    else {
      if (gv.time_out > 0.0 && gv.time_out < (get_cpu_time() - gv.start_time))  
	return TIME_OUT;
      if (sample_size && gv.number_branch_decisions >= sample_size) return SAMPLE_FINISHED;  

      if (gv.restarts_on) {
	if (restart_threshold && gv.number_branch_decisions >= restart_threshold) {  // restart
	  while (P.current_level() > 0) P.undo_current_decision();
	  update_restart_parameters();
	}
      }

      if (gv.forget_clauses_on)
	C.reduce_knowledge_base(P);                       // if we've learned more than we can manage, delete some stuff
      
      AnnotatedLiteral l = select_branch(P);            // before branching, P is valid, closed and decision_minimal
      P.extend(l);                                      // in relation to C.
    }
  }
  return SAT;  
}




