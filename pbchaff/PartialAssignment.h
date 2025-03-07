/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#ifndef _PARTIAL_ASSIGNMENT_H
#define _PARTIAL_ASSIGNMENT_H

#include <vector>
#include "Literal.h"
#include "AtomValueMap.h"
#include "AtomNameMapLocal.h"


/******************************************************************/
/*

  CLASS: PartialAssignment
  
  PURPOSE: The class PartialAssignment keeps a stack of
  variable assignments. Each assignment has a corresponding reason.
  A reason is the clause which under the partial assignment
  forces the assignment. Reasons are represented by their ClauseIDs.
  PartialAssignment has stack like functionality
  with assignments being pushed onto the top of the stack or popped
  off the top.  Assignments cannot be inserted in the middle, however,
  an established assignment may later have its reason updated to
  a different clause.  PartialAssignment also keeps track of the
  descision level of the assignment.  Some assignments may be the result
  of a branch choice.  These assignments will not have an associated
  reason. This is indicated by pushing an assignment with a
  ClauseID of 0. The descisionLevel is the number branch choices
  in the PartialAssignment.

  IMPLEMENTATION: Uses an AtomValueMap to represent a set of Literals,
  a vector of ClauseIDs to represent reasons, and a vector of int to
  give quick access to the decisionLevel of a given assignment.

*******************************************************************/


class PartialAssignment : public AtomValueMap {

  int m_decisionLevel;
  std::vector<ClauseID> m_reasons;
  std::vector<int> m_level;
  AtomNameMapLocal m_names;
  
public:

  PartialAssignment() : m_decisionLevel(0) {}

  /****************** READ ***************/

  std::size_t getNumberVariables() const { return m_reasons.size() - 1; }
  bool isFull() const { return (size() == (m_reasons.size() - 1)); }
  inline Literal getTop() const;
  inline bool isUnsat(Literal l) const;
  int getDecisionLevel() const { return m_decisionLevel; }
  std::size_t getNumberFreeVariables() const;
  
  // access by atom
  bool isValued(std::size_t a) const { return contains(a); }
  int getLevel(std::size_t a) const { return m_level[a]; }
  inline bool getValue(std::size_t a) const;
  ClauseID getReason(std::size_t a) const { return m_reasons[a]; }
  std::string lookup(int i) { return m_names.lookup(i); }
  
  void print(std::ostream& os = std::cout) const;
  void printListForm(std::ostream& os = std::cout) const;
  void printNameForm(std::ostream& os = std::cout);

  /****************** WRITE ***************/
 
  inline void initialize(std::size_t size);
  inline void clear();
  inline void push(Literal l,ClauseID id);  
  inline void pop();
  inline void updateReason(std::size_t a,ClauseID id) { m_reasons[a] = id; }
  int lookup(std::string key) { return m_names.lookup(key); }
};





/************************** INLINES ********************************/

inline Literal PartialAssignment::getTop() const
{
  return getLiteral(size() - 1);
}

inline bool PartialAssignment::getValue(std::size_t a) const 
{
  return (AtomValueMap::getValue(a) > 0);
}


inline void PartialAssignment::initialize(std::size_t size)
{
  AtomValueMap::initialize(size);
  for (std::size_t i=0; i < size; ++i)
    {
      m_reasons.push_back(0);
      m_level.push_back(0);
    }
}


inline void PartialAssignment::clear()
{
  AtomValueMap::clear();
  for (std::size_t i=0; i < m_reasons.size(); ++i)
    {
      m_reasons[i] = 0;
      m_level[i] = 0;
    }
}



inline  void PartialAssignment::push(Literal l,ClauseID id)
{
  if (id == 0) m_decisionLevel++;
  int atom = l.getAtom();
  bool sign = l.getSign();
  ASSERT(!AtomValueMap::contains(atom));
  addAtom(atom,(sign ? 1 : -1));
  m_reasons[atom] = id;
  m_level[atom] = m_decisionLevel;
}

inline void PartialAssignment::pop()
{
  ASSERT(!empty());

  int atom = getAtom(size() - 1);
  remove(atom);
  if (m_reasons[atom] == 0) m_decisionLevel--;
  m_reasons[atom] = 0;
  m_level[atom] = 0;
  
}

inline bool PartialAssignment::isUnsat(Literal l) const 
{
  if (!contains(l.getAtom())) return false;
  return getValue(l.getAtom()) != l.getSign();
}

inline std::size_t PartialAssignment::getNumberFreeVariables() const
{
  return (m_reasons.size() - 1 - size());
}

inline std::ostream& operator<<(std::ostream& os, const PartialAssignment& pa) {
  pa.print(os);
  return os;
}




#endif

