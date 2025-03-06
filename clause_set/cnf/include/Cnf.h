#ifndef __CNF__
#define __CNF__

#include <vector>
#include "ClauseSet.h"
#include "FastSet.h"
using namespace std;

namespace zap
{



/////////////////////////////   CNF CLAUSE   ///////////////////////////////////


class CnfClause : public Clause {
public:
  CnfClause() { }
  CnfClause(const Clause& c) : Clause(c) { }
  friend ostream& operator<<(ostream& os, const Clause& c);
};



/////////////////////////   CNF CLAUSE SET   //////////////////////////////////

/*  Class Cnf describes a clause set in CNF (conjunctive normal form).  Cnf implements
    the ClauseSet interface so we can hide as much as possible the details of working with
    this type of constraint set from the high-level solver code.  
    
    We store all the clauses in one big vector. Since a clause is implemented as a vector<size_t>,
    the whole thing is a vector<vector<size_t> >.  The original set of input clauses are at
    the beginning of the vector and learned clauses come after that.  This is different from
    what I've seen in most other solvers that either do their own memory management for clauses
    or allocate clauses on the heap.  The major benefit of doing it this way is that it makes for
    simple easily readable code.  Watchers are just clauseIDs (ints) instead of pointers.
    It has an additional advantage: because we sort learned clauses by how often they are used
    and delete those that are less frequently used, highly used clauses are grouped together in
    the vector and this appears to improve cache performance.  The down sides to this approach
    are 1) performance and 2) Watchers become invalid when we shuffle the order of the clauses
    in the vector.  I was happy to find that experimentally the overall performance degradation
    is insignificant when compared to rsat.  The code to update the watchers after shuffling the
    list of clauses is the price paid, but I grouped it seperately so you don't have to look at
    it if you don't want to.  

    We keep an extra clause at the end of the vector after all the learned clauses.  This is
    temporary storage for the learning process.  We generate numerous clauses during learning
    and we don't always add them to the clause set.  The extra clause allows us a temporary
    storage space for these clauses that can be accesses in the high-level solver code just like
    any other clause.  If we decide we want to keep a clause we call add_clause.  This commits
    it to the clause database.

    
    Clauses are indexed by the watched literal method.   A Watcher for a literal l in a clause c is
    a clauseID (int) that gives the index position of the clause in the main vector.  The first two
    lits every clause are always the two watched lits, so l will be in one of these positions.  Every
    clause gets two Watchers and all Watchers are stored in m_watchers.  This allows us in the
    get_implications function to walk all clauses that have a watcher on a particular literal l.

    There are a bunch of data structures for maintaining scores.  We keep scores for every variable
    (vsids scores) and for every clause.  The variable scores are sort of statisticalish counts of
    how often a variable appears in the learned clause set or the learning process.  We use these
    counts to select branch decisions and to which literals to pop of the unit_queue first during
    unit propagation (see solver/Assignment.h).  The clause scores are statisticalish counts of
    how often clauses are used during learning.  We use them to sort the learned clause list and
    decide which clauses to delete when the number of learned clauses gets to big.  The score
    for a particular clause c is stored in the actual clause.  

    When the number of clauses gets to be too big we delete a whole bunch of learned clauses.
    This code is all modelled on the rsat code.  I've copied their heuristic choices as closely as
    possible.
 */



class Cnf : public vector<CnfClause>, public virtual ClauseSet
{
 protected:
  size_t                 m_num_vars;
  size_t                 m_num_clauses;
  size_t                 m_end_original_clauses; // marks end of original input and beginning of learned clauses
  vector<VariableWatch>  m_watchers;

  // this stuff is all for maintaining vsids scores and clause scores
  vector<double >        m_vsids_counts;
  double                 m_score_inc;
  double                 m_clause_score_inc;
  FastSet                m_score_set;  // used in incrementing vsids scores
  double                 m_max_clauseset_size;
  vector<bool>           seen;
  bool                   first_round;
  CnfClause              resolve_clause;
  
  void      increment_variable_score(size_t v);
  void      rescale_variable_scores();
  void      rescale_clause_scores();
  void      increment_clause_score(ClauseID id);

  // these functions are all for bounding/reducing the size of the learned clause set
  void      remove_permanently_sat_clauses(const Assignment& P);
  void      remove_irrelevant_clauses(const Assignment& P);
  void      update_pointers(Assignment& P, size_t max_id);
  bool      satisfied_at_level_0(const Assignment& P, ClauseID id);
  bool      is_a_reason(const Assignment& P, ClauseID id) const;

  size_t    add_to_analysis(const CnfClause& c, const Assignment& P);
  
public:
  Cnf() : m_num_vars(0), m_num_clauses(0) { }
  Cnf(const InputTheory& intput);
  Cnf(const vector<Clause>& clauses);
  void initialize_clause_set(const vector<Clause>& clauses);
  size_t    number_clauses()                           const { return m_num_clauses; }

  // the ClauseSet interface functions
  size_t    number_variables()                         const { return m_num_vars; }
  Result    get_implications(Assignment& P, Literal l);
  bool      get_symmetric_unit_lits(Assignment& P, ClauseID c_id, Literal unit_lit);
  ClauseID  resolve(const Reason& r1, const Reason& r2, const Assignment& P,
					size_t& lits_this_level, Literal& unit_lit);
  void      add_learned_clause(ClauseID c_id, const Assignment& P);
  Result    load_unit_literals(Assignment& P);
  void      reduce_knowledge_base(Assignment& P);
  
  const Clause&          clause(ClauseID c) const { return operator[](c); }
  const vector<double>&  VSIDS_counts()        const { return m_vsids_counts; }
  
  // invariant properties we may want to test for
  bool      valid(const Assignment& P) const;
  bool      closed(const Assignment& P) const;
  bool      decision_minimal(const Assignment& P) const;
  
  friend ostream& operator<<(ostream& os, const Cnf& cnf);
};



//////////////////////////   INLINES   ///////////////////////////////////////



inline ostream& operator<<(ostream& os, const CnfClause& c) {
  for (size_t i=0; i < c.size(); i++)
    os << c[i] << ' ' << flush;
  return os;
}

inline ostream& operator<<(ostream& os, const Cnf& cnf) {
  for (size_t i=0; i < cnf.number_clauses(); i++)
    os << cnf[i] << endl;
  return os;
}



} // end namespace zap
#endif

