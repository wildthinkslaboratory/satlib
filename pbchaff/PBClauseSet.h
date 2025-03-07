/**********************************/
/** written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

/************************************************************/
/**
   There are currently three versions of ClauseSet.  They implement
   the same interface but with different data structure maintenence.
   This version uses count based adjacency list data structures
   and can manage full Pseudo-Boolean constraints.
   
**************************************************************/
									       

#ifndef _PB_CLAUSE_SET_H
#define _PB_CLAUSE_SET_H

#include <vector>
#include <queue>
#include "Clause.h"
#include <iostream>
#include "StackOfLists.h"

class WeightedClauseID
{
public:
  int id;
  int weight;
  WeightedClauseID(int i, int w) : id(i), weight(w) {}
};

/* for each literal we keep a list of clauses that contain that literal */
class PBLitIndex
{
public:
  std::vector<WeightedClauseID> watchers[2];
  std::vector<WeightedClauseID>& getPBLitIndex(bool b)
  {
    return (b ? watchers[0] : watchers[1]);
  }
};






/********************************************************************/
/**
   CLASS: PBClauseSet
   
   PURPOSE:  A PBClauseSet maintains a database of constraints.

   INTERFACE: see ClauseSet.h
   
   IMPLEMENTATION:

   MEMORY MANAGEMENT PBClauseSet provides its own
   automatic memory management.  The class contains a large
   array of PoolPBLiterals.  PoolPBLiterals can contain a literal, a
   clauseID, or be null.  Clauses are represented as a series of
   literals in the pool followed by a clauseID which demarks the
   end of the clause.  Clauses are deleted lazily by voiding out
   each literal in the clause.  Periodically, garbage collecting
   is performed to compact the database and reclaim memory from
   deleted clauses.

   LITERAL INDEX: PBClauseSet uses a literal index to access
   subsets of clauses that contain a specific literal.  This is
   needed for determining unit implications quickly.

   COUNTS: For each
   clause we maintain two counts, possible, and current.  These
   counts represent how satisfied the clause is relative to the
   current partial assignment.  Current represents the current
   count, and possible tells us the count that would occur if
   all the remaining unvalued literals were valued favorably for
   the clause. 
   
***********************************************************************/


class PBClauseSet
{

protected:
  
  PoolPBLiteral* m_poolBegin;
  PoolPBLiteral* m_poolEnd;
  PoolPBLiteral* m_poolEndStorage;

  std::vector<PBLitIndex> m_literalIndex;
  std::vector<PBClause> m_clauses;
  std::vector<int> m_possible;
  std::vector<int> m_current;
  std::queue<ClauseID> m_unusedIDs;
  const PartialAssignment* m_assignment; // pointer to current assignment

  Conflict m_conflict;
  ClauseSetStatistics m_statistics;
  ClauseSetSettings m_settings;
  std::vector<int> m_literalCounts[2];

  // memory management functions
  std::size_t getPoolSize() const { return m_poolEnd - m_poolBegin; }
  std::size_t getPoolFreeSpace() const { return m_poolEndStorage - m_poolEnd; }
  inline std::size_t estimateMemoryUsage() const;
  bool enlargePool();
 
  // add to the literal pool
  void poolPushLiteral(int a, bool p, int w) { (m_poolEnd++)->setLiteral(a,p,w); }
  void poolPushID(int id) { (m_poolEnd++)->setID(id); }
  void poolPushNull() { (m_poolEnd++)->clear(); }

  // evaluate a literal under the partial assignment
  inline bool isUNSAT(const PoolPBLiteral&) const;
  inline bool isSAT(const PoolPBLiteral&) const;

  // maintain counts
  void computeCounts(ClauseID);
  inline void incPossible(Literal);
  inline void decPossible(Literal);
  inline void incCurrent(Literal);
  inline void incCurrent(Literal,StackOfLists&);
  inline void decCurrent(Literal);

  std::size_t getNumberConstraints() { return m_clauses.size() - m_unusedIDs.size(); }
  inline bool betterClause(ClauseID,ClauseID,int) const;
  inline bool isUnit(ClauseID id) const;
  inline void deleteClause(int) ;
public:

  PBClauseSet();
  // declared but not implemented to prevent use
  PBClauseSet(const PBClauseSet&);
  ~PBClauseSet() { delete [] m_poolBegin; }

  //**********************  READ *******************************

  const Conflict& getConflict() const { return m_conflict; }
  inline void initializeClause(FastClause&, ClauseID) const;
  const ClauseSetStatistics &getStatistics() const { return m_statistics; }
  inline int getLiteralCount(Literal l) const;
  bool initialClauseCheck(ImplicationList&) const;
  inline void getUnfavorables(int,AtomValueMap&) const;

  void print(std::ostream & os = std::cout) const;
  void printDetail(std::ostream& os = std::cout) const;

  //*********************** WRITE ********************************
  
  ClauseID addClause(FastClause &atoms);
  void removeClause(int);
  void deleteIrrelevantClauses();

  bool getImplications(Literal, ImplicationList&);
  void inline wind(Literal l, StackOfLists&);
  void inline unwind(Literal l);

  void garbageCollect();
  void initialize(const PartialAssignment*);
  inline void setInitialClauseCount() ;
  inline void reduceCounts();
  void setStrengthen(bool b) { m_settings.strengthenOn = b; }

  // data-structure verification
  void verifyReason(Literal,ClauseID);
  void verifyConstraintSet();

  // declared but not implemented to prevent use
  PBClauseSet& operator= (const PBClauseSet&);
};






//********************* INLINE FUNCTIONS *********************************


inline void PBClauseSet::setInitialClauseCount() 
{
  int count = m_statistics.initialClauseCount +
    m_statistics.addedClauseCount -
    m_statistics.deletedClauseCount;
  m_statistics.initialClauseCount = count;
  m_statistics.addedClauseCount = 0;
  m_statistics.deletedClauseCount = 0;
}


inline void PBClauseSet::deleteClause(ClauseID id)
{
  PBClause& c = m_clauses[id];
  c.markUnused();
  int size = c.size();
  for (int i = 0; i < size; i++)
    {
      PoolPBLiteral &plit = c.getWriteLiteral(i);
      int atom = plit.getAtom();
      bool sign = plit.getSign();
      m_literalCounts[sign][atom]--;
      plit.clear();
    }
  m_statistics.deletedClauseCount++;
  m_statistics.literalCount -= size;
  m_unusedIDs.push(id);
}

// shouldn't these all be size insteacd of capacity?
inline std::size_t PBClauseSet::estimateMemoryUsage() const
{
  return sizeof(PoolPBLiteral) * (getPoolSize() + getPoolFreeSpace()) +
    sizeof(PBLitIndex) * m_literalIndex.capacity() +
    sizeof(PBClause) * m_clauses.capacity() +
    sizeof(int) * m_unusedIDs.size();
}

inline bool PBClauseSet::isUNSAT(const PoolPBLiteral& p) const
{
    int atom = p.getAtom();
    if (!m_assignment->isValued(atom)) return false;
    return (m_assignment->getValue(atom) != p.getSign());
}

inline bool PBClauseSet::isSAT(const PoolPBLiteral& p) const
{
    int atom = p.getAtom();
    if (!m_assignment->isValued(atom)) return false;
    return (m_assignment->getValue(atom) == p.getSign());
}

inline int PBClauseSet::getLiteralCount(Literal l) const
{
  return m_literalCounts[l.getSign()][l.getAtom()];
}

inline void PBClauseSet::reduceCounts()
{
  std::size_t size = m_literalCounts[0].size();
  for (std::size_t i=0; i < size; i++)
    {
      m_literalCounts[0][i] /= 2;
      m_literalCounts[1][i] /= 2;
    }
}

inline bool PBClauseSet::betterClause(ClauseID id1, ClauseID outerID,int conflictAtom) const
{
  if ((outerID % 3) == PB)
    {
      // this assumes a valid reason
      const PBClause& c1 = m_clauses[id1];
      int length1 = 0;
      for (size_t i=0,sz=c1.size(); i < sz; ++i)
	{
	  PoolPBLiteral lit = c1.getReadLiteral(i);
	  if (isUNSAT(lit) || lit.getAtom() == conflictAtom)
	    ++length1;
	}
      const PBClause& c2 = m_clauses[outerID/3];
      int length2 = 0;
      for (size_t i=0,sz=c2.size(); i < sz; ++i)
	{
	  PoolPBLiteral lit = c2.getReadLiteral(i);
	  if (isUNSAT(lit) || lit.getAtom() == conflictAtom)
	    ++length2;
	}
      if (length1 == length2) return c1.size() > c2.size();
      return length1 < length2;
    }
  else return false;
}

inline void PBClauseSet::incPossible(Literal l)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  std::vector<WeightedClauseID>::iterator it = m_literalIndex[atom].getPBLitIndex(sign).begin();
  std::vector<WeightedClauseID>::iterator end = m_literalIndex[atom].getPBLitIndex(sign).end();
  for (;it != end; it++) m_possible[(*it).id] += (*it).weight;
}



inline void PBClauseSet::decCurrent(Literal l)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  std::vector<WeightedClauseID>::iterator it = m_literalIndex[atom].getPBLitIndex(sign).begin();
  std::vector<WeightedClauseID>::iterator end = m_literalIndex[atom].getPBLitIndex(sign).end();
  for (;it != end; it++) m_current[(*it).id] -= (*it).weight;
}

inline void PBClauseSet::decPossible(Literal l)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  std::vector<WeightedClauseID>::iterator it = m_literalIndex[atom].getPBLitIndex(sign).begin();
  std::vector<WeightedClauseID>::iterator end = m_literalIndex[atom].getPBLitIndex(sign).end();
  for (;it != end; it++) m_possible[(*it).id] -= (*it).weight;
}

inline void PBClauseSet::incCurrent(Literal l)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  std::vector<WeightedClauseID>::iterator it = m_literalIndex[atom].getPBLitIndex(sign).begin();
  std::vector<WeightedClauseID>::iterator end = m_literalIndex[atom].getPBLitIndex(sign).end();
  for (;it != end; it++) m_current[(*it).id] += (*it).weight;
}

inline void PBClauseSet::incCurrent(Literal l,StackOfLists& overSat)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  std::vector<WeightedClauseID>::iterator it = m_literalIndex[atom].getPBLitIndex(sign).begin();
  std::vector<WeightedClauseID>::iterator end = m_literalIndex[atom].getPBLitIndex(sign).end();
  for (;it != end; it++)
    {
      int oldCurrent = m_current[(*it).id];
      m_current[(*it).id] += (*it).weight;
      int newCurrent = m_current[(*it).id];
      if (oldCurrent <= 0 && newCurrent > 0)
	overSat.addElement(((*it).id * 3) + 1);
    }
}

inline void PBClauseSet::wind(Literal l, StackOfLists& overSat)
{
  if (m_settings.strengthenOn) incCurrent(l,overSat);
  else incCurrent(l);
  decPossible(l.getNegation());
}

inline void PBClauseSet::unwind(Literal l)
{
  incPossible(l.getNegation());
  decCurrent(l);
}


inline void PBClauseSet::initializeClause(FastClause& c, ClauseID id) const
{
  c.initialize(m_clauses[id]);
  c.setCurrent(m_current[id]);
}

inline bool PBClauseSet::isUnit(ClauseID id) const
{
  // this is probably where the aloul pointer thing should be done
  if (m_current[id] >= 0) return false;
  const PBClause &c = m_clauses[id];
  if (c.getWeight(0) == 1) return m_possible[id] == 0;
  if (m_possible[id] == m_current[id]) return false;
  if (c.getWeight(c.size() - 1) > m_possible[id]) return true;
  for (size_t i=0, size = c.size(); i < size; i++)
    if (c.getWeight(i) <= m_possible[id]) return false;
    else if (!m_assignment->isValued(c.getAtom(i))) return true;
  return false;
}


inline void PBClauseSet::getUnfavorables(int id, AtomValueMap& unfaves) const
{
  const PBClause &c = m_clauses[id];
  for (size_t i=0, size=c.size(); i < size; i++)
    {
      int atom = c.getAtom(i);
      bool sign = c.getSign(i);
      if (m_assignment->isUnsat(Literal(atom,sign)))
	{
	  unfaves.addIfAbsent(atom,sign ? 1 : -1);
	}
    }
}


#endif
