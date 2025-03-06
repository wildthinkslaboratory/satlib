#include "Assignment.h"

namespace zap
{

Literal Assignment::pop_unit_list()
{
  global_vars.queue_pops++;
  Variable v = m_unit_list.top();
  m_unit_list.pop();                // pop a unit of the unit_list
  Literal a(v,m_value[v]);
  m_stack.push_back(a);                   // add it to the stack of assignments
  if (get_reason(a).is_branch()) m_current_level++;  // if its a branch we're in a new decision level
  
  m_level[a.variable()] = m_current_level;
  m_position[a.variable()] = (int)(m_stack.size() - 1);
//   if (m_current_level == 0 || m_current_level == 1) {
// 	cout << "level   " << m_current_level << "   stack size    "  << m_stack.size() << endl;
//   }
  return a;
}



bool Assignment::extend(AnnotatedLiteral l)   // extend the current partial assignment with literal l
{
  if (m_value[l.variable()] == (size_t)(!l.sign())) {  // check the stack for contradictions
    m_contradiction = l.variable();
    m_reason[l.variable()][l.sign()] = l.reason();
    return false;
  }

  if (m_value[l.variable()] != UNKNOWN) return true;  // it's already set
  
  m_reason[l.variable()][l.sign()] = l.reason();
  m_value[l.variable()] = l.sign();
  m_unit_list.insert(l.variable());
  return true;
}


bool Assignment::has_contradiction() const
{
  if (m_reason[m_contradiction][true].id() == NULL_ID) return false;
  if (m_reason[m_contradiction][false].id() == NULL_ID) return false;
  return true;
}


Literal Assignment::undo_current_decision()
{
  while (!m_unit_list.empty()) { //  clear the unit_list
    Variable v = m_unit_list.top();
	m_reason[v][m_value[v]] = Reason();
	m_value[v] = UNKNOWN;
	m_unit_list.pop();
  }
                                                 // clear all assignments in the current decision level
  Literal l = m_stack.back();                    // use l to hold the first assignment in the level (the branch)
  while (m_stack.size() && m_level[m_stack.back().variable()] == (int)m_current_level) {
    l = m_stack.back();
    m_level[l.variable()] = NULL_ID;
    m_position[l.variable()] = NULL_ID;
    m_value[l.variable()] = UNKNOWN;
    m_reason[l.variable()][true]  = Reason();
    m_reason[l.variable()][false] = Reason();
    m_stack.pop_back();
  }
  m_current_level--;

  return l;                                     // we return the associated branch decision so the calling
}                                               // function can try the other variable value



// Return all literals in clause c that are also in the current decision level
// We keep the literals in answer in the order they occur in the stack with deepest first
vector<Literal> Assignment::lits_at_current_level(const Clause& c) const
{
  int deepest = -1;
  vector<Literal> answer;
  for (size_t i=0; i < c.size(); i++) {
    if (decision_level(c[i].variable()) == (int)m_current_level) {
      if ((position(c[i].variable()) > deepest)) {
	deepest = position(c[i].variable());

	if (answer.empty()) answer.push_back(c[i]);
	else {
	  answer.push_back(answer[0]);
	  answer[0] = c[i];
	}
      }
      else answer.push_back(c[i]);
    }
  }
  return answer;
}

// The assertion level of a clause c is the first decision level in the partial assignment
// where the clause is unit.  The decision level where we would have added a unit literal
// with reason c if we had the clause at the time.  This function assumes that the clause is
// unsatisfied by this Assignment.
size_t Assignment::assertion_level(const Clause& c) const
{
  if (c.size() <= 1) return 0;
  
  // int deepest = NULL_ID;
  // int next_deepest = NULL_ID;
  // for (size_t i=0; i < c.size(); i++) {
  //   int level = decision_level(c[i].variable());
  //   if (level >= deepest) {
  //     next_deepest = deepest;
  //     deepest = level;
  //   }
  //   else {
  //     if (level > next_deepest) {
  // 	next_deepest = level;
  //     }
  //   }
  // }
  // // sanity test
  // if (next_deepest != decision_level(c[1].variable())) {
  //   cerr << "asserting lit not in the right place" << endl;
  //   cerr << next_deepest << " vs " << decision_level(c[1].variable()) << endl;
  //   exit(1);
  // }
  return decision_level(c[1].variable());
}



bool Assignment::operator<(const Assignment& P) const
{
  size_t i=0;
  for ( ; i < size() && i < P.size(); i++) {
    if (m_stack[i] != P.m_stack[i]) break;
    if (m_stack[i].sign() != P.m_stack[i].sign()) break;
  }

  if (i >= size()) return true;  //  P is an extension of current assignment
  if (i >= P.size()) {     // the current assignment is an extension of P
    cerr << *this << endl << P << endl;
    return false;
  }
  
  if (get_reason(m_stack[i]).is_branch() && !P.get_reason(P.m_stack[i]).is_branch())
    return true; // current assignment weaker because it has less propagation
  cerr << *this << endl << P << endl;
  return false;
  
}



ostream& operator<<( ostream& os, const Assignment& A) {
  os << "stack [ ";
  for (size_t i=0; i < A.size(); i++) {
	if (A.get_reason(A[i]).is_branch()) 
	  os << endl << "(" << A[i] << ',' << "branch";
	else os << "(" << A[i] << ',' << A.get_reason(A[i]);
    os << ") ";
  }
  os << "]  unit list [" << A.m_unit_list << "] " << endl << flush;
   return os;
}

void Assignment::print_positive(ostream& os) const
{
  os << "stack [ ";
  for (size_t i=0; i < size(); i++) {
	if (get_reason(operator[](i)).is_branch()) 
	  os << endl << "(" << operator[](i) << ',' << "branch";
	else {
	  if (operator[](i).sign() == true) {
		os << "(" << operator[](i) << ',' << get_reason(operator[](i));
		os << ") ";
	  }	
	}
  }
  os << "]  unit list [" << m_unit_list << "] " << endl << flush; 
}

} // ending namespace zap
