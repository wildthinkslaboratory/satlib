/**********************************/
/** written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#include "Mod2ClauseSet.h"
#include <iostream>
using namespace std;

Mod2ClauseSet::Mod2ClauseSet() : m_assignment(0)
{ 
  m_poolBegin = new PoolLiteral[STARTUP_LIT_POOL_SIZE];
  m_poolEnd = m_poolBegin;
  m_poolEndStorage = m_poolBegin + STARTUP_LIT_POOL_SIZE;
  poolPushNull(); // mark pool beginning
}


void Mod2ClauseSet::initialize(const PartialAssignment* ptr)
{
  m_assignment = ptr;

  // we don't allow clauseIDs of 0 so just push a dummy clause
  m_clauses.resize(1);
  m_clauses[0].initialize(0,0,0);
  m_currentSumMod2.push_back(false);
  m_unvaluedCount.push_back(0);
  
  size_t numberVariables = m_assignment->getNumberVariables();
  for (size_t i=0; i <= numberVariables; ++i)
    {
      m_literalIndex.push_back(vector<int>());
    }
}




void Mod2ClauseSet::garbageCollect()
{
  // walk through the pool and rebuild it without the null literals
  int newIndex = 1;
  PoolLiteral *pool = m_poolBegin;
  size_t poolSize = getPoolSize();
  for (size_t i=1; i < poolSize; ++i) //pool[0] is a marker
    {
      if (!pool[i].isNull())
	{
	  pool[newIndex] = pool[i];
	  if (pool[i].isID())
	    {
	      int id = pool[i].getID();
	      PoolLiteral* p = &pool[newIndex] - m_clauses[id].size();
	      m_clauses[id].setFirst(p);
	    }
	  newIndex++;
	}
      else while (!pool[i].isID()) ++i; // scan to end of null clause
      
    }
  
  m_poolEnd = m_poolBegin + newIndex;
}




bool Mod2ClauseSet::enlargePool()
{
  // first try to get space by cleaning up
  if (getPoolSize() - getNumberConstraints() > m_statistics.literalCount * 2)
    {
      garbageCollect();
      return true;
    }


  // we try to double the size of the pool.  If we can't, we settle
  // for a factor of 1.2.  If that doesn't work we make a last
  // ditch effort to garbageCollect and quit.
  int currentUsage = estimateMemoryUsage();
  float growthFactor = 1;

  if (currentUsage < (int)(m_settings.memoryLimit / 2)) growthFactor = 2;
  else if (currentUsage < m_settings.memoryLimit * 0.8) growthFactor = 1.2;
  else
    {
      m_statistics.outOfMemory = true;
      if (getPoolSize() - getNumberConstraints() > m_statistics.literalCount * 1.1)
	{
	  garbageCollect();
	  return true;
	}
      else return false;
    }


  // reallocate and do a memcopy.  Adjust all the pointers into the pool.
  PoolLiteral * oldBegin = m_poolBegin;
  PoolLiteral * oldEnd = m_poolEnd;
  size_t oldSize = m_poolEndStorage - m_poolBegin;
  size_t newSize = (size_t)(oldSize * growthFactor);
  m_poolBegin = new PoolLiteral[newSize];
  m_poolEndStorage = m_poolBegin + newSize;
  memcpy(m_poolBegin,oldBegin,(oldEnd - oldBegin) * sizeof(PoolLiteral));
  m_poolEnd = m_poolBegin + (oldEnd - oldBegin);
  int offset = m_poolBegin - oldBegin;

  // update pointers into pool
  vector<Mod2Clause>::iterator it = m_clauses.begin();
  vector<Mod2Clause>::iterator end = m_clauses.end();
  for ( ; it != end; ++it) it->incFirst(offset);

  // delete the old pool
  delete [] oldBegin;
  return true;
  
}

ClauseID Mod2ClauseSet::addClause(FastClause &atoms)
{
  int newID;

  // make sure there is space in the pool
  while (getPoolFreeSpace() <= atoms.size() + 1) 
    if (enlargePool() == false)
      {
	m_statistics.outOfMemory = true;
	deleteIrrelevantClauses();
	garbageCollect();
	if (getPoolFreeSpace() <= atoms.size() + 1)
	  return -1; // failure marker
      }

  // get a constraint id
  if (m_unusedIDs.empty())
    {
      newID = m_clauses.size();
      m_clauses.resize(newID + 1);
      m_unvaluedCount.push_back(0);
      m_currentSumMod2.push_back(0);
    }
  else
    {
      newID = m_unusedIDs.front();
      m_unusedIDs.pop();
    }
  
  // add the constriant to the pool
  m_clauses[newID].initialize(m_poolEnd, atoms.getRequired(), atoms.size());
  for (size_t i=0; i < atoms.size(); ++i)
    {
      int atom = atoms.getAtom(i);
      poolPushLiteral(atom,true);
    }
  poolPushID(newID);

  // update some counts
  Mod2Clause &c = m_clauses[newID];
  m_statistics.literalCount += c.size();
  ++m_statistics.addedClauseCount;
  computeCounts(newID);

  // add to literal index
  for (size_t i=0; i < c.size(); ++i)
    {
      const PoolLiteral &pl = c.getReadLiteral(i);
      m_literalIndex[pl.getAtom()].push_back(newID);
    }

  return newID;
}


bool Mod2ClauseSet::getImplications(Literal l, ImplicationList& units)
{
  int atom = l.getAtom();
  vector<int>& watchers = m_literalIndex[atom];
  vector<int>::iterator it = watchers.begin();
  vector<int>::iterator end = watchers.end();

  for ( ; it != end; ++it)
    {
      int id = *it;
      if (m_unvaluedCount[id] == 1)
	{
	  Mod2Clause& c = m_clauses[id];
	  for (size_t i=0; i < c.size(); ++i)
	    {
	      const PoolLiteral& pl = c.getReadLiteral(i);
	      int forcedAtom = pl.getAtom();
	      if (!m_assignment->isValued(forcedAtom))
		{
		  Literal l(forcedAtom,m_currentSumMod2[id] != c.getSumMod2());
#ifdef VERIFY
		  verifyReason(l,id);
#endif
		  if (units.contains(l.getNegation()))
		    {
		      m_conflict.atom = forcedAtom;
		      m_conflict.reason1 = (id * 3) + 2;
		      m_conflict.reason2 = units.getReason(forcedAtom);
		      return false;
		    }
		  units.push(l,(id * 3) + 2);
		}
	    }
	}
    }
  return true;
}

void Mod2ClauseSet::computeCounts(ClauseID id)
{
  Mod2Clause& c = m_clauses[id];
  m_unvaluedCount[id] = c.size();
  m_currentSumMod2[id] = 0;
  
  for (size_t i=0; i < c.size(); ++i)
    {
      const PoolLiteral& pl = c.getReadLiteral(i);
      int atom = pl.getAtom();
      if (m_assignment->isValued(atom)) 
	{
	  m_unvaluedCount[id]--;
	  if (m_assignment->getValue(atom))
	    m_currentSumMod2[id] = !m_currentSumMod2[id];
	}
    }
}


bool Mod2ClauseSet::initialClauseCheck(ImplicationList &unitList) const
{
  size_t nc = m_clauses.size();
  for (size_t i=1; i < nc; ++i)
    {
      if (m_clauses[i].inUse())
	{
	  const Mod2Clause &c = m_clauses[i];
	  if ( c.size() == 1)
	    {
	      Literal l(c.getAtom(0),c.getSumMod2());
	      if (unitList.contains(l.getNegation()))
		return false;
	      unitList.push(l,(i * 3) + 2);
	    }
	}
    }
  return true;
}


void Mod2ClauseSet::deleteIrrelevantClauses()
{
  // if we've been running out of memory reduce the relevance bounds
  if (m_statistics.outOfMemory)
    {
      m_statistics.outOfMemory = false;
      m_statistics.outOfMemoryCount++;
      if (m_statistics.outOfMemoryCount > 5)
	{
	  m_settings.relevanceBound = (int) (m_settings.relevanceBound * 0.8);
	  if (m_settings.relevanceBound < 4)
	    m_settings.relevanceBound = 4;
	  m_settings.lengthBound = (int) (m_settings.lengthBound * 0.8);
	  if (m_settings.lengthBound < 10)
	    m_settings.lengthBound = 10;
	  m_settings.maxLength = (int) (m_settings.maxLength * 0.8);
	  if (m_settings.maxLength < 50 )
	    m_settings.maxLength = 50;
	  
	}
    }


  // delete the clauses
  size_t oldDeletedClauseCount = m_statistics.deletedClauseCount;
  size_t nc = m_clauses.size();
  size_t ic = m_statistics.initialClauseCount + 1;
  for (size_t i=ic; i < nc; ++i)
    {
      size_t size = m_clauses[i].size();
      if (!m_clauses[i].inUse() || size < m_settings.lengthBound) continue;
      int unvaluedCount = m_unvaluedCount[i];
      if (unvaluedCount >= (int)(m_settings.relevanceBound))
	deleteClause(i);
      
      if (size > m_settings.maxLength && (unvaluedCount > 0))
	deleteClause(i);
    }

  if (oldDeletedClauseCount == m_statistics.deletedClauseCount) return;

   // update pointers into the literalPool
   for (size_t i=1; i < m_literalIndex.size(); ++i)
    {
      vector<int> &watchers = m_literalIndex[i];
      vector<int>::iterator it = watchers.begin();
      for ( ; it != watchers.end(); ++it)
	{
	  Mod2Clause& c = m_clauses[*it];
	  if (c.getFirst()->isNull())
	    {
	      *it = watchers.back();
	      watchers.pop_back();
	      --it;
	    }
	}  
    }
}


void Mod2ClauseSet::verifyReason(Literal l,ClauseID id)
{
  const Mod2Clause& c = m_clauses[id];
  computeCounts(id);
  if (m_unvaluedCount[id] > 1)
    fatalError("too many unvalued lits in Mod2ClauseSet::verifyReason");

  bool foundAtom = false;
  for (size_t i=0; i < c.size(); ++i)
    {
      if (c.getAtom(i) == (int)(l.getAtom()))
	{
	  foundAtom = true;
	  if (m_unvaluedCount[id] == 1 && m_assignment->isValued(l.getAtom()))
	    fatalError("too many unvalued lits in Mod2ClauseSet::verifyReason");
	  if ((m_currentSumMod2[id] == c.getSumMod2()) == l.getSign())
	    fatalError("bad value in Mod2ClauseSet::verifyReason");;
	  break;
	}
    }

  if (!foundAtom) fatalError("atom not in clause in Mod2ClauseSet::verifyReason");
}

void Mod2ClauseSet::verifyConstraintSet()
{
  for (size_t i=1; i < m_clauses.size(); ++i)
    {
      if (m_clauses[i].inUse())
	{
	  computeCounts(i);
	  if ((m_unvaluedCount[i] == 0) &&
	      m_currentSumMod2[i] != m_clauses[i].getSumMod2())
	    {
	      cerr << "ID " << i << "   " << m_clauses[i];
	      fatalError("UNSAT constraint in Mod2ClauseSet::verifyConstraintSet");
	    }
	}
    }
}



void Mod2ClauseSet::print(ostream &os) const
{
  for (size_t i = 1; i < m_clauses.size(); ++i)
    if (m_clauses[i].inUse()) m_clauses[i].print();
}

void Mod2ClauseSet::printDetail(ostream &os) const
{
  os << "Mod2ClauseSet: " << endl;
  for (size_t i = 1; i < m_clauses.size(); ++i) m_clauses[i].printDetail();
}

