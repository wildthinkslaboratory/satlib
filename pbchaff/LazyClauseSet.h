/**********************************/
/** written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/
									       
#ifndef _LAZY_CLAUSE_SET_H
#define _LAZY_CLAUSE_SET_H

#include "Clause.h"
#include "StackOfLists.h"
#include <vector>
#include <queue>

// simple little struct to hold watched literals
class Watchers
{
public:
  std::vector<PoolLiteral*> watchers[2];
  std::vector<PoolLiteral*>& getWatchers(bool b)
  {
    return (b ? watchers[0] : watchers[1]);
  }
};

// simple struct to hold ClauseIDs
class LitIndex
{
 public:
  std::vector<ClauseID> index[2];
  std::vector<ClauseID>& getIndex(bool b)
    {
      return (b ? index[0] : index[1]);
    }
};


/********************************************************************/
/**

   CLASS: LazyClauseSet

   PURPOSE:  A LazyClauseSet maintains a database of cardinality
   constraints.

   IMPLEMENTATION:

   MEMORY MANAGEMENT LazyClauseSet provides its own
   automatic memory management.  The class contains a large
   array of PoolLiterals.  PoolLiterals can contain a literal, a
   clauseID, or be null.  Clauses are represented as a series of
   literals in the pool followed by a clauseID which demarks the
   end of the clause.  Clauses are deleted lazily by voiding out
   each literal in the clause.  Periodically, garbage collecting
   is performed to compact the database and reclaim memory from
   deleted clauses.

   LITERAL INDEXS: LazyClauseSet uses a literal index to access
   subsets of clauses that contain a specific literal.  This is
   needed for determining unit implications quickly.  The indexing
   scheme used here is the watched literal implementation of zchaff.
   I've also added a count based index.  This is for use in
   strengthening.  The maintenance of this structure can be turned
   off when not in use to improve performance.  
   
***********************************************************************/


class LazyClauseSet
{

 protected:
  
  PoolLiteral* m_poolBegin;
  PoolLiteral* m_poolEnd;
  PoolLiteral* m_poolEndStorage;

  std::vector<Watchers> m_watchedLitIndex;
  std::vector<LitIndex> m_idLitIndex;
  std::vector<Clause> m_clauses;
  std::vector<int> m_currentCount;
  std::queue<ClauseID> m_unusedIDs;
  const PartialAssignment* m_assignment; // pointer to current assignment

  // temp object that I just reuse for speed
  std::vector<PoolLiteral*> m_scrapPaper;

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
  void poolPushLiteral(int a, bool p) { (m_poolEnd++)->setLiteral(a,p); }
  void poolPushID(int id) { (m_poolEnd++)->setID(id); }
  void poolPushNull() { (m_poolEnd++)->clear(); }

  // evaluate a literal under the partial assignment
  inline bool isUNSAT(const PoolLiteral*) const ;
  inline bool isSAT(const PoolLiteral*) const ;

  // maintain counts
  void computeCounts(ClauseID);
  inline void incCurrent(Literal,StackOfLists&);
  inline void decCurrent(Literal);
  
  std::size_t getNumberConstraints() { return m_clauses.size() - m_unusedIDs.size(); }
  inline bool betterClause(ClauseID,ClauseID) const;
  inline void deleteClause(int) ;

public:

  
  LazyClauseSet();
  // declared but not implemented to prevent use
  LazyClauseSet(const LazyClauseSet&);
  ~LazyClauseSet() { delete [] m_poolBegin; }

  //**********************  READ *******************************

  const Conflict& getConflict() const { return m_conflict; }
  inline void initializeClause(FastClause&, ClauseID) const;
  const ClauseSetStatistics &getStatistics() const { return m_statistics; }
  inline int getLiteralCount(Literal l) const;
  bool initialClauseCheck(ImplicationList&) const;
  inline void getUnfavorables(int, AtomValueMap&) const;
  
  // data-structure verification
  void verifyReason(Literal,ClauseID) const;
  void verifyConstraintSet() const;

  void print(std::ostream & os = std::cout) const;
  void printDetail(std::ostream& os = std::cout) const;
  
  //*********************** WRITE ********************************
  
  ClauseID addClause(FastClause &atoms);
  void removeClause(int);
  void deleteIrrelevantClauses();
   
  bool getImplications(Literal, ImplicationList&);
  inline void wind(Literal l, StackOfLists&);
  inline void unwind(Literal l);
  
  void initialize(const PartialAssignment*);
  inline void setInitialClauseCount() ;
  inline void reduceCounts() ;
  void setStrengthen(bool b) { m_settings.strengthenOn = b; }
  void garbageCollect();
  
  // declared but not implemented to prevent use
  LazyClauseSet& operator= (const LazyClauseSet&);
};






//********************* INLINE FUNCTIONS *********************************



inline void LazyClauseSet::setInitialClauseCount()
{
  int count = m_statistics.initialClauseCount +
    m_statistics.addedClauseCount -
    m_statistics.deletedClauseCount;
  m_statistics.initialClauseCount = count;
  m_statistics.addedClauseCount = 0;
  m_statistics.deletedClauseCount = 0;
}


inline void LazyClauseSet::deleteClause(ClauseID id)
{
  Clause &c = m_clauses[id];
  c.markUnused();
  int size = c.size();
  for (int i = 0; i < size; ++i)
    {
      PoolLiteral &plit = c.getWriteLiteral(i);
      int atom = plit.getAtom();
      bool sign = plit.getSign();
      m_literalCounts[sign][atom]--;
      plit.clear();
    }
  ++m_statistics.deletedClauseCount;
  m_statistics.literalCount -= size;
  m_unusedIDs.push(id);
}

// shouldn't these all be size insteacd of capacity?
inline std::size_t LazyClauseSet::estimateMemoryUsage() const
{
  return sizeof(PoolLiteral) * (getPoolSize() + getPoolFreeSpace()) +
    sizeof(Watchers) * m_watchedLitIndex.capacity() +
    sizeof(Clause) * m_clauses.capacity() +
    sizeof(int) * m_unusedIDs.size();
}

inline bool LazyClauseSet::isUNSAT(const PoolLiteral* p) const
{
    int atom = p->getAtom();
    if (!m_assignment->isValued(atom)) return false;
    return (m_assignment->getValue(atom) != p->getSign());
}

inline bool LazyClauseSet::isSAT(const PoolLiteral* p) const
{
    int atom = p->getAtom();
    if (!m_assignment->isValued(atom)) return false;
    return (m_assignment->getValue(atom) == p->getSign());
}



inline int LazyClauseSet::getLiteralCount(Literal l) const
{
  return m_literalCounts[l.getSign()][l.getAtom()];
}

// these counts are used by the VSIDS branching heuristic
inline void LazyClauseSet::reduceCounts()
{
  std::size_t size = m_literalCounts[0].size();
  for (std::size_t i=0; i < size; ++i)
    {
      m_literalCounts[0][i] /= 2;
      m_literalCounts[1][i] /= 2;
    }
}

// checkout a copy of a clause and put it in c
inline void LazyClauseSet::initializeClause(FastClause& c, ClauseID id) const
{
  c.initialize(m_clauses[id]);
  c.setCurrent(m_currentCount[id]);
}


inline bool LazyClauseSet::betterClause(ClauseID id1, ClauseID id2) const
{
  if ((id2 % 3) == 0)
    {
      int length1 = m_clauses[id1].size() - m_clauses[id1].getRequired();
      int length2 = m_clauses[id2/3].size() - m_clauses[id2/3].getRequired();
      if (length1 == length2) return m_clauses[id1].size() > m_clauses[id2/3].size();
      return length1 < length2;
    }
  else return false;
  //  return coinFlip();
}


inline void LazyClauseSet::incCurrent(Literal l,StackOfLists& overSat)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  std::vector<int>::iterator it = m_idLitIndex[atom].getIndex(sign).begin();
  std::vector<int>::iterator end = m_idLitIndex[atom].getIndex(sign).end();
  for (;it != end; ++it)
    {
      int id = *it;
      ++m_currentCount[id];
      if (m_currentCount[id] == 1) overSat.addElement(id*3);
    }
}

inline void LazyClauseSet::decCurrent(Literal l)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  std::vector<int>::iterator it = m_idLitIndex[atom].getIndex(sign).begin();
  std::vector<int>::iterator end = m_idLitIndex[atom].getIndex(sign).end();
  for (;it != end; ++it) --m_currentCount[(*it)];
}

inline void LazyClauseSet::wind(Literal l, StackOfLists& overSat)
{
  if (m_settings.strengthenOn) incCurrent(l,overSat);
}

inline void LazyClauseSet::unwind(Literal l)
{
  if (m_settings.strengthenOn) decCurrent(l);
}


inline void LazyClauseSet::getUnfavorables(int id, AtomValueMap& unfaves) const
{
  const Clause &c = m_clauses[id];
  for (size_t i=0, size=c.size(); i < size; i++)
    {
      int atom = c.getAtom(i);
      bool sign = c.getSign(i);
      if (m_assignment->isUnsat(Literal(atom,sign))) {
	unfaves.addIfAbsent(atom,sign ? 1 : -1);
      }
    }
}

#endif
