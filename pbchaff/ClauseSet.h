/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

/************************************************************/
/*

  OVERVIEW: (Understanding ClauseSets)

  We'd like to present a uniform view of a ClauseSet to the user.
  Access to the actual clauses is very restricted for reasons
  discussed below.  Users can keep pointers into the clause
  set by keeping ClauseIDs.  To look at the actual clause, the
  user can check out a copy of the clause with the ClauseID.
  All other read and write access to the clause set is done
  through very specified queries.  This is very different than
  my previous design and I'm still getting used to it.  It sometimes
  seems limiting at the Solver level to not be able
  to quickly walk a clause without having to copy it first,
  but I've found that a function I might previously have been
  able to write efficiently at the Solver level can still
  be done at the Solver level.  Only now it may require
  writing a few queries at the ClauseSet level to pull needed
  information.


  IMPLEMENTATION: What's really going on

  Ultimately, we'd like to support a variety of constraint
  types.  Even for a single constraint type, there may be
  different ways of representing the constraint each with
  different advantages and disadvantages.  Given
  a particular constraint type, there may also be different
  ways of indexing and managing a set of constraints of that
  type.  Now add on top of this that we'd like to do our
  own memory management for optimal performance and hide
  all this stuff from the user.

  This code currently supports two representations of a clause.
  It has two separate sub clause sets to accomodate these
  representations. The first is cardinality clause.  A cardinality clause
  is a pseudo-boolean clause where all the coefficients are
  required to be 1. The corresponding clause set is the
  LazyClauseSet. The other representation of a clause is a
  full pseudo-Boolean constraint where coefficients are
  allowed to take on integer values. The corresponding
  clause set is PBClauseSet.  Cardinality constraints can
  technically be placed in either type of clause set since
  PBClauses are a proper generalization of CNF and cardinality
  clauses.

  The LazyClauseSet uses a Watched literal strategy for maintaining
  consistency with repsect to the partial assignment.  It
  also has count based adjacency lists which are used during
  strenghtening.  The maintenence of the count based indexes can
  be turned off when not in use to improve performance.  The
  PBClauseSet uses a count based adjacency list approach.  The
  additional overhead of supporting coefficients and also the
  use of a count based adjacency list approach gives the PBClauseSet
  slower performace.

  The class ClauseSet is a wrapper class that provides the interface
  to the Solver class. It contains both a LazyClauseSet and a
  PBClauseSet.

  RELATED FILES: LazyClauseSet[.h,.cpp] PBClauseSet[.h,.cpp]
  Clause.h Literal.h

**************************************************************/

#ifndef _CLAUSE_SET_H
#define _CLAUSE_SET_H

#include "LazyClauseSet.h"
#include "Mod2ClauseSet.h"
#include "PBClauseSet.h"
#include "StackOfLists.h"

class ClauseSet {
 protected:
  LazyClauseSet m_lazyClauses;
  PBClauseSet m_PBClauses;
  Mod2ClauseSet m_mod2Clauses;
  int m_whereToGetConflict;
  const PartialAssignment* m_assignment;  // pointer to current assignment
  ClauseSetStatistics m_statistics;

  // for use in strengthening
  StackOfLists m_overSatClauses;
  AtomValueMap m_satLits;

 public:
  ClauseSet() : m_whereToGetConflict(true) {}
  // declared but not implemented to prevent use
  ClauseSet(const ClauseSet&);

  //**********************  READ *******************************

  inline const Conflict& getConflict() const;
  // check out a copy of a clause
  inline void initializeClause(FastClause&, ClauseID) const;
  inline int getLiteralCount(Literal l) const;
  inline bool initialClauseCheck(ImplicationList&) const;
  inline const StackOfLists& getOverSatClauses() const;

  inline void print(std::ostream& os = std::cout) const;
  inline void printDetail(std::ostream& os = std::cout) const;

  //*********************** WRITE ********************************

  inline void initialize(const PartialAssignment*);
  inline void setInitialClauseCount();

  inline bool getImplications(Literal, ImplicationList&);
  inline void unwind(Literal l);

  inline ClauseID addClause(FastClause& atoms);
  inline void deleteIrrelevantClauses();
  inline void removeClause(ClauseID);

  inline void reduceCounts();
  inline const ClauseSetStatistics& getStatistics();
  inline void getWhyOverSat(FastClause&, AtomValueMap&, std::vector<int>&);
  inline void garbageCollect();
  inline void setStrengthen(bool b);

  // data-structure verification
  inline void verifyReason(Literal, ClauseID);
  inline void verifyConstraintSet();

  // declared but not implemented to prevent use
  ClauseSet& operator=(const ClauseSet&);
};

//********************* INLINE FUNCTIONS *********************************

inline const Conflict& ClauseSet::getConflict() const {
  switch (m_whereToGetConflict) {
    case LAZY:
      return m_lazyClauses.getConflict();
    case PB:
      return m_PBClauses.getConflict();
    case MOD2:
      return m_mod2Clauses.getConflict();
  }
}

inline void ClauseSet::initializeClause(FastClause& c, ClauseID id) const {
  int whichSet = id % 3;
  switch (whichSet) {
    case LAZY:
      m_lazyClauses.initializeClause(c, id / 3);
      break;
    case PB:
      m_PBClauses.initializeClause(c, id / 3);
      break;
    case MOD2:
      m_mod2Clauses.initializeClause(c, id / 3);
      break;
  }
}

inline const ClauseSetStatistics& ClauseSet::getStatistics() {
  const ClauseSetStatistics& lazyStats = m_lazyClauses.getStatistics();
  const ClauseSetStatistics& PBStats = m_PBClauses.getStatistics();
  const ClauseSetStatistics& mod2Stats = m_mod2Clauses.getStatistics();

  m_statistics.literalCount =
      lazyStats.literalCount + PBStats.literalCount + mod2Stats.literalCount;
  m_statistics.outOfMemoryCount = lazyStats.outOfMemoryCount +
                                  PBStats.outOfMemoryCount +
                                  mod2Stats.outOfMemoryCount;
  m_statistics.initialClauseCount = lazyStats.initialClauseCount +
                                    PBStats.initialClauseCount +
                                    mod2Stats.initialClauseCount;
  m_statistics.addedClauseCount = lazyStats.addedClauseCount +
                                  PBStats.addedClauseCount +
                                  mod2Stats.addedClauseCount;
  m_statistics.deletedClauseCount = lazyStats.deletedClauseCount +
                                    PBStats.deletedClauseCount +
                                    mod2Stats.deletedClauseCount;
  return m_statistics;
}

inline int ClauseSet::getLiteralCount(Literal l) const {
  return m_lazyClauses.getLiteralCount(l) + m_PBClauses.getLiteralCount(l);
}

inline bool ClauseSet::initialClauseCheck(ImplicationList& units) const {
  if (m_PBClauses.initialClauseCheck(units))
    if (m_lazyClauses.initialClauseCheck(units))
      return m_mod2Clauses.initialClauseCheck(units);
  return false;
}

inline void ClauseSet::verifyReason(Literal l, ClauseID id) {
  int whichSet = id % 3;
  switch (whichSet) {
    case LAZY:
      m_lazyClauses.verifyReason(l, id / 3);
      break;
    case PB:
      m_PBClauses.verifyReason(l, id / 3);
      break;
    case MOD2:
      m_mod2Clauses.verifyReason(l, id / 3);
      break;
  }
}

inline void ClauseSet::verifyConstraintSet() {
  for (std::size_t i = 0; i < m_assignment->size(); ++i) {
    Literal l = m_assignment->getLiteral(i);
    ClauseID reason = m_assignment->getReason(l.getAtom());
    verifyReason(l, reason);
  }

  m_PBClauses.verifyConstraintSet();
  m_lazyClauses.verifyConstraintSet();
  m_mod2Clauses.verifyConstraintSet();
}

inline void ClauseSet::print(std::ostream& os) const {
  os << "Constraint Database: " << std::endl;
  m_PBClauses.print();
  m_lazyClauses.print();
  m_mod2Clauses.print();
}

inline void ClauseSet::printDetail(std::ostream& os) const {
  m_PBClauses.printDetail();
  m_lazyClauses.printDetail();
  m_mod2Clauses.printDetail();
}

inline ClauseID ClauseSet::addClause(FastClause& atoms) {
  if (atoms.isMod2()) return (3 * m_mod2Clauses.addClause(atoms)) + 2;
  if (atoms.getWeight(0) > 1) return (3 * m_PBClauses.addClause(atoms)) + 1;
  return 3 * m_lazyClauses.addClause(atoms);
}

inline void ClauseSet::initialize(const PartialAssignment* pa) {
  m_assignment = pa;
  m_PBClauses.initialize(pa);
  m_lazyClauses.initialize(pa);
  m_mod2Clauses.initialize(pa);
  m_satLits.initialize(pa->getNumberVariables() + 1);
}

inline void ClauseSet::setInitialClauseCount() {
  m_lazyClauses.setInitialClauseCount();
  m_PBClauses.setInitialClauseCount();
  m_mod2Clauses.setInitialClauseCount();
}

inline bool ClauseSet::getImplications(Literal l, ImplicationList& units) {
  m_overSatClauses.addNewList();

  m_lazyClauses.wind(l, m_overSatClauses);
  m_PBClauses.wind(l, m_overSatClauses);
  m_mod2Clauses.wind(l);

  m_whereToGetConflict = LAZY;
  if (m_lazyClauses.getImplications(l, units)) {
    m_whereToGetConflict = PB;
    if (m_PBClauses.getImplications(l, units)) {
      m_whereToGetConflict = MOD2;
      return m_mod2Clauses.getImplications(l, units);
    }
  }
  return false;
}

inline void ClauseSet::deleteIrrelevantClauses() {
  m_lazyClauses.deleteIrrelevantClauses();
  m_PBClauses.deleteIrrelevantClauses();
  m_mod2Clauses.deleteIrrelevantClauses();
}

inline void ClauseSet::unwind(Literal l) {
  m_overSatClauses.popTopList();
  m_lazyClauses.unwind(l);
  m_PBClauses.unwind(l);
}

inline void ClauseSet::reduceCounts() {
  m_lazyClauses.reduceCounts();
  m_PBClauses.reduceCounts();
}

inline void ClauseSet::getWhyOverSat(FastClause& c, AtomValueMap& whyLits,
                                     std::vector<int>& reasons) {
  ASSERT(!c.isMod2());
  m_satLits.clear();
  c.getFavorables(m_satLits, *m_assignment);
  for (std::size_t i = 0; i < m_satLits.size(); i++) {
    int atom = m_satLits.getAtom(i);
    int id = m_assignment->getReason(atom);
    if (id == 0)
      whyLits.addAtom(atom, m_satLits.getValue(atom));
    else {
      int whichSet = id % 3;
      if (whichSet != MOD2) reasons.push_back(atom);
      switch (whichSet) {
        case LAZY:
          m_lazyClauses.getUnfavorables(id / 3, whyLits);
          break;
        case PB:
          m_PBClauses.getUnfavorables(id / 3, whyLits);
          break;
        case MOD2:
          m_mod2Clauses.getUnfavorables(id / 3, whyLits, atom);
          break;
      }
    }
  }
}

inline const StackOfLists& ClauseSet::getOverSatClauses() const {
  return m_overSatClauses;
}

inline void ClauseSet::removeClause(ClauseID id) {
  int whichSet = id % 3;
  switch (whichSet) {
    case LAZY:
      m_lazyClauses.removeClause(id / 3);
      break;
    case PB:
      m_PBClauses.removeClause(id / 3);
      break;
    default:
      fatalError("removeClause not supported in ClauseSet::removeClause");
  }
}

inline void ClauseSet::setStrengthen(bool b) {
  m_lazyClauses.setStrengthen(b);
  m_PBClauses.setStrengthen(b);
}

inline void ClauseSet::garbageCollect() {
  m_lazyClauses.garbageCollect();
  m_PBClauses.garbageCollect();
}

#endif
