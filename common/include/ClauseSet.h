#ifndef __CLAUSE_SET__
#define __CLAUSE_SET__

#include "Assignment.h"
#include "InputTheory.h"

namespace zap
{

/*  The classe ClauseSet describes the interface used by the basic solver functions
    described in heidi_sat.cpp.  This allows multiple representations of a clause or clause set.
    A clause may be a simple disjunction of literals, or an augmented clause.  In these cases,
    the underlying implementations of these interface functions will be very different.
 */


///////////////////////////////////   CLAUSE SET   /////////////////////////////////////


class ClauseSet {
public:
  ClauseSet() { }
  ClauseSet(InputTheory& input) { }
  virtual ~ClauseSet() { }
  
  virtual size_t    number_variables() const = 0;
  virtual Result    get_implications(Assignment& P, Literal l) = 0;
  virtual bool      get_symmetric_unit_lits(Assignment& P, ClauseID c_id, Literal unit_lit) = 0;
  virtual ClauseID  resolve(const Reason& r1, const Reason& r2, const Assignment& P,
							size_t& lits_this_level, Literal& unit_lit) = 0;
  virtual void      add_learned_clause(ClauseID c_id, const Assignment& P) = 0;
  virtual Result    load_unit_literals(Assignment& P) = 0;
  virtual void      reduce_knowledge_base(Assignment& P) = 0;
  
  virtual const Clause&          clause(ClauseID c) const = 0;  // reference to a clause via ID
  virtual const vector<double>&  VSIDS_counts() const = 0;

  virtual bool      valid(const Assignment& P) const = 0;  // for checking the invariants
  virtual bool      closed(const Assignment& P) const = 0;
  virtual bool      decision_minimal(const Assignment& P) const = 0;
  friend ostream& operator<<(ostream& os, const ClauseSet& c); // there needs to be a way to call this on a ClauseSet
};




} // end namespace zap
#endif
