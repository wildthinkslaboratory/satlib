#ifndef __ASSIGNMENT__
#define __ASSIGNMENT__

#include <vector>
#include <set>
#include "common.h"
#include "heap.h"
#include <fstream>

using namespace std;

namespace zap
{

const int UNKNOWN       =  2 ;  
const int NULL_ID       = -1 ;
const int PURE          = -2 ;

////////////////////////////////////   REASON   ////////////////////////////////////////////


/* A reason is an annotation for a variable assignment.  It's a clauseID
   (an index into a ClauseSet). We'll need to add a data field to store either a ground clause
   or a permutation. This is used when working with Augmented Clauses to generate the ground clause that
   caused a variable assignment when it is not the representative clause of the AugmentedClause m_id.
   I'm not sure how I'm going to implement it yet.
   If the ClauseID is the NULL_ID (-1) that indicates a branch assignment.  */

class Reason
{
  ClauseID	    m_id;
  Clause        m_clause_instance;
public:
  Reason(ClauseID id = NULL_ID) : m_id(id)  { }
  Reason(ClauseID id, const Clause& c) : m_id(id), m_clause_instance(c) { }
  
  bool is_branch()   const { return m_id == NULL_ID; }
  bool is_pure()     const { return m_id == PURE; }
  bool good_reason() const { return m_id >= 0; }
  ClauseID      id() const    { return m_id; }
  ClauseID &    id()        { return m_id; }
  const Clause& clause_instance() const { return m_clause_instance; }
  
  friend ostream& operator << (ostream& os, const Reason& r);
};



//////////////////////////////   ANNOTATED LITERAL   /////////////////////////////////////////////


class AnnotatedLiteral : public Literal
{
  Reason               m_reason;
public:
  AnnotatedLiteral(const Literal& l = Literal(), const Reason& r = Reason()) : Literal(l), m_reason(r) { }
  Reason reason() const { return m_reason; }
  friend ostream& operator<<(ostream& os, const AnnotatedLiteral& a);
};



/////////////////////////////////////   ASSIGNMENT   ////////////////////////////////////////////

/* A possibly partial assignment of values to Boolean variables.  When we assign values to variables,
   we have to seperate assignments whose unit consequences have  been computed (stored in m_stack)
   from those whose unit consequences still need to be checked (stored in m_unit_list).
   Data members m_level, m_value and m_reason are all indexed by variable so provide
   constant time access.  Assignment won't allow you to add both positive and negated versions of
   a literal.  If you do, the contradiction variable is recorded in the m_contradiction data member
   and we keep the reasons for both assignments in m_reason.  
   
 */

class Assignment
{  
  vector<Literal>           m_stack; 
  Heap                      m_unit_list;
  size_t                    m_current_level;  // current decision level
  Variable                  m_contradiction;
  
  // these are all indexed by variable. 
  vector<int>             m_level;    
  vector<size_t>          m_value;
  vector<int>             m_position; // in the stack
  vector<vector<Reason> > m_reason;
  const vector<double>*   m_vsids_counts;
  
  void resize(size_t sz);
  
public:
  Assignment();
  Assignment(size_t nv, const vector<double>* vc);
  void initialize(size_t nv, const vector<double>* vc); 

  // properties of assignments
  bool empty() const;
  bool full() const;
  bool closed() const;

  // access by variable or literal
  size_t         value(Variable v) const;
  int            decision_level(Variable v) const;
  const Reason   get_reason(Literal l) const;
  bool           watchable(Literal l) const;            // Assignment satisfies l or leaves it unvalued
  int            position(Variable l) const;
  bool           is_on_unit_list(Literal l) const;
  bool           has_contradiction() const;

  // ways of selecting unvalued variables for branching 
  Literal    random_unvalued_literal() const;
  Literal    first_unvalued_literal() const;
  Literal    vsids_literal() const;
  Literal    user_input_literal() const;
  
  size_t     size() const;
  size_t     unit_list_size() const { return m_unit_list.size(); }
  Variable   contradiction_variable() const;
  int        current_level() const;
  bool       extend(AnnotatedLiteral l);
  Literal    undo_current_decision();            // and return the associated branch
  Literal    pop_unit_list();
  
  void       update_reasons(const vector<int>& indexes);  // we may shuffle around clauses in our ClauseSet
                                                          // and change the ClauseIDs.  This maps old ids to new
                                                          // ones.  Ugly.  Barely respects the interface.
                                                          // Needs to change.


  // these are used for learning analysis
  vector<Literal>  lits_at_current_level(const Clause& c) const;
  size_t           assertion_level(const Clause& c) const;
  void             backjump(size_t al);

  
  bool operator< (const Assignment& P) const;
  Literal operator[](size_t i) const;
  friend ostream& operator<<(ostream& os, const Assignment& A);
  void print_positive(ostream& os = cout) const;
};




////////////////////////////////// INLINES /////////////////////////////////////////////////////////////////


inline ostream& operator << (ostream& os, const Reason& r) {
  os << r.m_id << flush;
  return os;
}


inline ostream& operator<<(ostream& os, const AnnotatedLiteral& a) {
  os << "(" << Literal(a) << "," << a.m_reason << ")" << flush;
  return os;
}



inline Assignment::Assignment() : m_current_level(0), m_contradiction(0), m_vsids_counts(NULL) { }
inline Assignment::Assignment(size_t nv, const vector<double>* vc) :
  m_unit_list(nv,vc),
  m_current_level(0),
  m_contradiction(0),
  m_vsids_counts(vc) { resize(nv); }



inline bool Assignment::empty() const {  return m_stack.size() == 0; }
inline bool Assignment::full() const { return m_stack.size() == (m_value.size()-1); }
inline bool Assignment::closed() const { return m_unit_list.empty(); }

inline size_t Assignment::size() const { return m_stack.size(); }
inline int Assignment::current_level() const { return m_current_level; }
inline Literal Assignment::operator[](size_t i) const { return m_stack[i]; }
inline Variable Assignment::contradiction_variable() const { return m_contradiction; }

inline void Assignment::initialize(size_t nv, const vector<double>* vc)
{
  resize(nv);
  m_vsids_counts = vc;
  m_unit_list = Heap(nv,vc);
}


inline size_t Assignment::value(Variable v) const
{
  if (v >= m_value.size()) return UNKNOWN;
  if (m_position[v] == NULL_ID) return UNKNOWN;  // it's still on the unit_list
  return m_value[v];
}

inline bool Assignment::is_on_unit_list(Literal l) const
{
  if (m_value[l.variable()] == UNKNOWN) return false;
  return m_value[l.variable()] == l.sign();
}

inline int Assignment::decision_level(Variable v) const
{
  if (v >= m_level.size()) return NULL_ID;
  return m_level[v];
}

inline const Reason Assignment::get_reason(Literal l) const { return m_reason[l.variable()][l.sign()]; }

inline bool Assignment::watchable(Literal l) const
{
  return m_value[l.variable()] == UNKNOWN || // it's unvalued
	m_value[l.variable()] == l.sign() ||     // it's valued favorably (or will be soon) 
	m_level[l.variable()] == NULL_ID;  // its valued unfavorably but we haven't popped it off the unit list yet
}                                      // so we have to wait and do these things in order

inline int Assignment::position(Variable v) const
{
  if (v >= m_position.size()) return NULL_ID;
  return m_position[v];
}



inline Literal Assignment::random_unvalued_literal() const
{
  vector<size_t> candidates;
  for (size_t i=1; i < m_value.size(); i++) 
    if (m_value[i] == UNKNOWN) candidates.push_back(i);
  //  bool sign = rand() % 2;
  bool sign = true;
  size_t var = 0;
  if (!candidates.empty()) var = candidates[rand() % candidates.size()];
  return Literal(var, sign);
}


inline Literal Assignment::user_input_literal() const
{
  bool found_branch = false;
  bool sign = true;
  Variable v = 0;
  while (!found_branch) {
	cout << endl << "Enter branch decision : " << flush;
	string branch;
	getline(cin,branch);
	if (branch[0] == '-') {
	  sign = false;
	  branch.erase(0);
	}
    v = global_vars.atom_name_map.lookup(branch);
	if (v >= m_value.size()) {
	  cout << branch << " is not a valid variable name " << endl;
	  continue;
	}
	if (m_value[v] != UNKNOWN) cout << "That variable is already valued to " << m_value[v] << endl;
	else found_branch = true;

  }
  
  return Literal(v,sign);
}



inline Literal Assignment::first_unvalued_literal() const
{

//   cout << "Enter branch choice : " << flush;
//   string decision;
//   getline(cin,decision);
//   bool sign = true;
//   if (decision[0] == '-') {
// 	sign = false;
// 	decision.erase(0,1);
//   }
//   size_t atom = global_vars.atom_name_map.lookup(decision);
//   return Literal(atom,sign);
  
  for (size_t i=1; i < m_value.size(); i++)
    if (m_value[i] == UNKNOWN) return Literal(i,true);
  return Literal(0,true);
}

inline Literal Assignment::vsids_literal() const
{
  Variable max_var = first_unvalued_literal().variable();
  for (size_t i=1; i < m_value.size(); i++) {
    if (m_value[i] == UNKNOWN && (*m_vsids_counts)[i] > (*m_vsids_counts)[max_var])
      max_var = i;
  }
//   cout << "branching with score " << (*m_vsids_counts)[max_var] << endl;
  return Literal(max_var,true);
}



inline void Assignment::resize(size_t sz) // sz is the highest index we want to use
{
  size_t more = sz + 1 - m_value.size();
  m_value.insert(m_value.end(),more,UNKNOWN);
  m_reason.insert(m_reason.end(),more,vector<Reason>(2,Reason()));
  m_level.insert(m_level.end(),more,NULL_ID);
  m_position.insert(m_position.end(),more,NULL_ID);
}



inline void Assignment::backjump(size_t al)
{
  while (m_current_level > al)
    undo_current_decision();
}



// we may shuffle around clauses in our ClauseSet and change the ClauseIDs.  indexes maps old ids to new
// ones.  Ugly.  Barely respects the interface.  Needs to change.
inline void Assignment::update_reasons(const vector<int>& indexes)
{
  for (size_t i=0; i < m_stack.size(); i++) {
    Reason& r = m_reason[m_stack[i].variable()][m_stack[i].sign()];
    if (r.is_branch() || r.is_pure()) continue;
    if (indexes[r.id()] == -1) {
      cout << "we deleted this reason " << endl;
      exit(1);
    }
    r = Reason(indexes[r.id()],r.clause_instance());
  }
}

} // ending namespace zap

#endif
