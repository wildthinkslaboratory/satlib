/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#ifndef _FAST_CLAUSE_H
#define _FAST_CLAUSE_H

#include "AtomValueMap.h"
#include "Literal.h"
#include "PartialAssignment.h"


class Clause;
class PBClause;
class Mod2Clause;

/****************************************************************/
/*
  CLASS: FastClause							
  
  PURPOSE: To represent a variety of constraint forms.  A FastClause
  contains a set of literals or weighted literals and currently
  the fields m_required and m_current.  It can be used to represent
  a CNF clause, a Pseudo-Boolean constraint, and also potentially
  an equality or a XOR constraint.  This class is used to provide
  a uniform way of viewing constraints at the Solver level, since
  the actual constraint set may represent different types of
  constraints with different types and in funky ways that enable
  certain optimizations.  The field m_required can be used to
  represent the right hand side of an inequality or an equality.
  m_current can be used to cache how oversatisfied a constraint is.
  Other uses of these fields are possible for additional constraint
  types.  Provides constant time contains(Literal).

  IMPLEMENTATION: Uses an AtomValueMap to represent a set of
  literals or weighted literals, and two ints to represent
  m_current and m_required.

  DISADVANTAGES:  There are some real costs to using this data
  structure at the Solver level.  They are big! I generally
  keep a few as class members in classes that make heavy use
  of them since continually constructing them and destructing
  them is expensive.  This way I just clear them and reinitialize
  them when I want to reuse them.
  
******************************************************************/
  
class FastClause : public AtomValueMap {
 protected:
  int m_required;
  int m_possible;
  int m_current;
  int m_flags;
  
 public:                   
  
  FastClause(): m_required(0), m_possible(0), m_current(0), m_flags(0) {}
  FastClause(std::size_t size):
    AtomValueMap(size),
    m_required(0),
    m_possible(0),
    m_current(0),
    m_flags(0) {}
  
  /**************** READ **********************/

  int getRequired() const { return m_required; }
  int getCurrent() const { return m_current; }
  int getPossible() const { return m_possible; }
  bool isMod2() const { return m_flags & 1; }
  inline int sumCoefficients() const;
  bool subsumes(const FastClause&) const;

  /* get all literals valued favorably with respect to 
     the PartialAssignment */
  void getFavorables(AtomValueMap&,const PartialAssignment&) const;
  void getUnfavorables(AtomValueMap&,const PartialAssignment&) const;
  
  
  void print(std::ostream& os = std::cout) const;

   /**************** WRITE *********************/         

  void initialize(int i) { AtomValueMap::initialize(i); }
  void initialize(const Clause &c);
  void initialize(const PBClause &c);
  void initialize(const Mod2Clause &c);
  
  void setCurrent(int i) { m_current = i; }
  void setPossible(int i) { m_possible = i; }
  void setRequired(int i) { m_required = i; }
  void incRequired(int i) { m_required += i; }
  void setMod2() { m_flags |= 1; }

  inline void multiply(int i);
  void divideQuick(int);
  void divideSmart(int);
  void add(const FastClause &);
  
  void resolve(FastClause&, int);
  /* almost all of these FastClause functions assume
     that a constraint has been simplified */
  void simplify();
  void weakenToCardinality(const PartialAssignment&,int);
  void strengthen(AtomValueMap&);
  
  inline void clear();

};






/************************** INLINE ********************************/


inline void FastClause::clear()
{
  m_required = 0;
  m_flags = 0;
  AtomValueMap::clear();
}


inline void FastClause::multiply(int v)
{
  ASSERT(v >= 0);
  for (std::size_t i = 0; i < m_size; ++i) m_value[m_atoms[i]] *= v;
  m_required *= v; 
}



inline int FastClause::sumCoefficients() const
{
  int sum = 0;
  for (std::size_t i = 0; i < m_size; ++i) sum += abs(m_value[m_atoms[i]]);
  return sum;
}

inline std::ostream& operator<<(std::ostream& os, const FastClause& fc)
{
  fc.print(os);
  return os;
}

#endif




