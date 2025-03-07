/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#ifndef _IMPLICATION_LIST_H
#define _IMPLICATION_LIST_H

#include <vector>
#include "Literal.h"
#include "AtomValueMap.h"


/******************************************************************/
/*

  CLASS: ImplicationList
  
  PURPOSE: The class ImplicationList keeps a list of literals and
  associates  a reason with each literal.  It is designed to hold
  unit propagations implied by a given partial assignment over
  a set of constraints.  A reason for a literal is the ClauseID
  of the clause that caused the literal to unit propagate.

  IMPLEMENTATION: Uses an AtomValueMap to hold a set of literals
  and a vector of ClauseIDs. ClauseIDs are ints.

*******************************************************************/


class ImplicationList : public AtomValueMap {
 
  std::vector<ClauseID> m_reasons;
  
  inline std::pair<Literal,ClauseID> pop(int);
  
public:

  /****************** READ ***************/
  
  inline bool contains(Literal l) const;
  // get reason associated with an implication. 
  ClauseID getReason(std::size_t atom) const { return m_reasons[atom]; }
  
  /****************** WRITE ***************/
 
  inline void initialize(std::size_t size);
  inline void clear();
  inline void push(Literal l,ClauseID id);
  inline std::pair<Literal,ClauseID> pop() {  return pop(0); }
  inline std::pair<Literal,ClauseID> randomPop() { return pop(rand() % size()); }
};





/************************** INLINES ********************************/


inline bool ImplicationList::contains(Literal l) const
{
  int atom = l.getAtom();
  if (AtomValueMap::contains(atom) && (getValue(atom) > 0) == l.getSign())
    return true;
  
  return false;
}


inline void ImplicationList::initialize(std::size_t size)
{
  AtomValueMap::initialize(size);
  for (std::size_t i=0; i < size; i++) m_reasons.push_back(0);      
}


inline void ImplicationList::clear() {
  AtomValueMap::clear();
  for (std::size_t i=0; i < m_reasons.size(); i++) m_reasons[i] = 0;
}



inline  void ImplicationList::push(Literal l, ClauseID id)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  addIfAbsent(atom,(sign ? 1 : -1));
  m_reasons[atom] = id;
}

inline std::pair<Literal,ClauseID> ImplicationList::pop(int i)
{
  ASSERT(!empty());

  Literal l = getLiteral(i);
  int atom = l.getAtom();
  ClauseID id = m_reasons[atom];
  remove(atom);
  m_reasons[atom] = 0;
  return std::make_pair(l,id);
}



#endif
