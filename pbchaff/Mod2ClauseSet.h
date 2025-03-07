/**********************************/
/** written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

								       

#ifndef _MOD2_CLAUSE_SET_H
#define _MOD2_CLAUSE_SET_H

#include <vector>
#include <queue>
#include "Clause.h"
#include <iostream>



class Mod2ClauseSet
{

private:
  
  PoolLiteral* m_poolBegin;
  PoolLiteral* m_poolEnd;
  PoolLiteral* m_poolEndStorage;

  std::vector<std::vector<int> > m_literalIndex;
  std::vector<Mod2Clause> m_clauses;
  std::vector<int> m_unvaluedCount;
  std::vector<bool> m_currentSumMod2;
  std::queue<ClauseID> m_unusedIDs;
  const PartialAssignment* m_assignment; // pointer to current assignment

  Conflict m_conflict;
  ClauseSetStatistics m_statistics;
  ClauseSetSettings m_settings;

  // memory management functions
  std::size_t getPoolSize() const { return m_poolEnd - m_poolBegin; }
  std::size_t getPoolFreeSpace() const { return m_poolEndStorage - m_poolEnd; }
  inline std::size_t estimateMemoryUsage() const;
  bool enlargePool();
  void garbageCollect();

  // add to the literal pool
  void poolPushLiteral(int a, bool p) { (m_poolEnd++)->setLiteral(a,p); }
  void poolPushID(int id) { (m_poolEnd++)->setID(id); }
  void poolPushNull() { (m_poolEnd++)->clear(); }

  // maintain counts
  void computeCounts(ClauseID);
  inline void valuePositive(int);
  inline void valueNegative(int);
  inline void unvaluePositive(int);  
  inline void unvalueNegative(int);

  std::size_t getNumberConstraints() { return m_clauses.size() - m_unusedIDs.size(); }
  inline void deleteClause(int) ;
  
public:

  Mod2ClauseSet();
  // declared but not implemented to prevent use
  Mod2ClauseSet(const Mod2ClauseSet&);
  ~Mod2ClauseSet() { delete [] m_poolBegin; }

  //**********************  READ *******************************

  const Conflict& getConflict() const { return m_conflict; }
  inline void initializeClause(FastClause&, ClauseID) const;
  const ClauseSetStatistics &getStatistics() const { return m_statistics; }
  bool initialClauseCheck(ImplicationList&) const;
  inline void getUnfavorables(int, AtomValueMap&, int) const;
  
  // data-structure verification
  void verifyReason(Literal,ClauseID);
  void verifyConstraintSet();

  void print(std::ostream & os = std::cout) const;
  void printDetail(std::ostream& os = std::cout) const;
  
  //*********************** WRITE ********************************
  
  ClauseID addClause(FastClause &atoms);
  void initialize(const PartialAssignment*);
  inline void setInitialClauseCount() ;
  bool getImplications(Literal, ImplicationList&);
  void deleteIrrelevantClauses();
  inline void wind(Literal l);
  inline void unwind(Literal l);
  
  // declared but not implemented to prevent use
  Mod2ClauseSet& operator= (const Mod2ClauseSet&);
};






//********************* INLINE FUNCTIONS *********************************


inline void Mod2ClauseSet::setInitialClauseCount() 
{  
  int count = m_statistics.initialClauseCount +
    m_statistics.addedClauseCount -
    m_statistics.deletedClauseCount;
  m_statistics.initialClauseCount = count;
  m_statistics.addedClauseCount = 0;
  m_statistics.deletedClauseCount = 0;
}


inline void Mod2ClauseSet::deleteClause(ClauseID id)
{
  Mod2Clause& c = m_clauses[id];
  c.markUnused();
  int size = c.size();
  for (int i = 0; i < size; i++)
    {
      PoolLiteral &plit = c.getWriteLiteral(i);
      plit.clear();
    }
  m_statistics.deletedClauseCount++;
  m_unusedIDs.push(id);
}

// shouldn't these all be size insteacd of capacity?
inline std::size_t Mod2ClauseSet::estimateMemoryUsage() const
{
  return sizeof(PoolLiteral) * (getPoolSize() + getPoolFreeSpace()) +
    sizeof(std::vector<int>) * m_literalIndex.capacity() +
    sizeof(Clause) * m_clauses.capacity() +
    sizeof(int) * m_unusedIDs.size();
}



inline void Mod2ClauseSet::valuePositive(int atom)
{
  std::vector<int>::iterator it = m_literalIndex[atom].begin();
  std::vector<int>::iterator end = m_literalIndex[atom].end();
  for (;it != end; ++it)
    {
      int id = *it;
      m_currentSumMod2[id] = !m_currentSumMod2[id];
      --m_unvaluedCount[id];
    }
}

inline void Mod2ClauseSet::valueNegative(int atom)
{
  std::vector<int>::iterator it = m_literalIndex[atom].begin();
  std::vector<int>::iterator end = m_literalIndex[atom].end();
  for (;it != end; ++it) --m_unvaluedCount[*it];
}

inline void Mod2ClauseSet::unvaluePositive(int atom)
{
  std::vector<int>::iterator it = m_literalIndex[atom].begin();
  std::vector<int>::iterator end = m_literalIndex[atom].end();
  for (;it != end; ++it)
    {
      int id = *it;
      m_currentSumMod2[id] = !m_currentSumMod2[id];
      ++m_unvaluedCount[id];
    }
}

inline void Mod2ClauseSet::unvalueNegative(int atom)
{
  std::vector<int>::iterator it = m_literalIndex[atom].begin();
  std::vector<int>::iterator end = m_literalIndex[atom].end();
  for (;it != end; ++it) ++m_unvaluedCount[*it];
}

inline void Mod2ClauseSet::wind(Literal l)
{
  int atom = l.getAtom();
  bool sign = l.getSign();

  if (sign) valuePositive(atom);
  else valueNegative(atom);
}

inline void Mod2ClauseSet::unwind(Literal l)
{
  if (l.getSign()) unvaluePositive(l.getAtom());
  else unvalueNegative(l.getAtom());
}



inline void Mod2ClauseSet::initializeClause(FastClause& c, ClauseID id) const
{
  c.initialize(m_clauses[id]);
}

inline void Mod2ClauseSet::getUnfavorables(int id, AtomValueMap& unfaves, int a ) const
{
  const Mod2Clause &c = m_clauses[id];
  for (size_t i=0, size=c.size(); i < size; i++)
    {
      int atom = c.getAtom(i);
      if (atom != a)
	unfaves.addIfAbsent(atom,m_assignment->getValue(atom));
    }
}

#endif
