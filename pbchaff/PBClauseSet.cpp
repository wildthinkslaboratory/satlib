/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#include "PBClauseSet.h"
#include <iostream>
#include <algorithm>
using namespace std;



PBClauseSet::PBClauseSet() : m_assignment(0)
{ 
  m_poolBegin = new PoolPBLiteral[STARTUP_LIT_POOL_SIZE];
  m_poolEnd = m_poolBegin;
  m_poolEndStorage = m_poolBegin + STARTUP_LIT_POOL_SIZE;
  poolPushNull(); // mark pool beginning
}


void PBClauseSet::initialize(const PartialAssignment* ptr)
{
  m_assignment = ptr;
  m_settings.memoryLimit = 1024*1024*32;
  // we don't allow clauseIDs of 0 so just push a dummy clause
  m_clauses.resize(1);
  m_clauses[0].initialize(0,0,0);
  m_possible.push_back(0);
  m_current.push_back(0);
  
  size_t numberVariables = m_assignment->getNumberVariables();
  for (size_t i=0; i <= numberVariables; i++)
    {
      m_literalIndex.push_back(PBLitIndex());
      m_literalCounts[0].push_back(0);
      m_literalCounts[1].push_back(0);
    }
}




void PBClauseSet::garbageCollect()
{
  // walk through the pool and rebuild it without the null literals
  int newIndex = 1;
  PoolPBLiteral *pool = m_poolBegin;
  size_t poolSize = getPoolSize();
  for (size_t i=1; i < poolSize; i++) //pool[0] is a marker
    {
      if (!pool[i].isNull())
	{
	  pool[newIndex] = pool[i];
	  if (pool[i].isID())
	    {
	      int id = pool[i].getID();
	      PoolPBLiteral* p = &pool[newIndex] - m_clauses[id].size();
	      m_clauses[id].setFirst(p);
	    }
	  newIndex++;
	}
      else while (!pool[i].isID()) i++; // scan to end of null clause
      
    }
  
  m_poolEnd = m_poolBegin + newIndex;
}




bool PBClauseSet::enlargePool()
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
  PoolPBLiteral * oldBegin = m_poolBegin;
  PoolPBLiteral * oldEnd = m_poolEnd;
  size_t oldSize = m_poolEndStorage - m_poolBegin;
  size_t newSize = (size_t)(oldSize * growthFactor);
  m_poolBegin = new PoolPBLiteral[newSize];
  m_poolEndStorage = m_poolBegin + newSize;
  memcpy(m_poolBegin,oldBegin,(oldEnd - oldBegin) * sizeof(PoolPBLiteral));
  m_poolEnd = m_poolBegin + (oldEnd - oldBegin);
  int offset = m_poolBegin - oldBegin;

  // update pointers into pool
  vector<PBClause>::iterator it = m_clauses.begin();
  vector<PBClause>::iterator end = m_clauses.end();
  for ( ; it != end; it++) it->incFirst(offset);

  // delete the old pool
  delete [] oldBegin;
  return true;
  
}


/* This function is different than deleteClause because it
   removes the clause from the literal index as well.  deleteClause
   does not.
*/
void PBClauseSet::removeClause(int id)
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
      vector<WeightedClauseID> &watchers = m_literalIndex[atom].getPBLitIndex(sign);
      for (size_t i=0,size=watchers.size(); i < size; ++i)
	if (id == watchers[i].id)
	  {
	    watchers[i] = watchers[size - 1];
	    watchers.pop_back();
	    break;
	  }
    }
  m_statistics.deletedClauseCount++;
  m_statistics.literalCount -= size;
  m_unusedIDs.push(id);
}

ClauseID PBClauseSet::addClause(FastClause &atoms)
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
	  {
	    m_statistics.outOfMemoryCount = 6;
	    deleteIrrelevantClauses();
	    garbageCollect();
	    ASSERT(getPoolFreeSpace() > atoms.size() + 1);
	  }
      }

  // get a constraint id
  if (m_unusedIDs.empty())
    {
      newID = m_clauses.size();
      m_clauses.resize(newID + 1);
      m_possible.push_back(0);
      m_current.push_back(0);
    }
  else
    {
      newID = m_unusedIDs.front();
      m_unusedIDs.pop();
    }
  
  // add the constriant to the pool
  m_clauses[newID].initialize(m_poolEnd, atoms.getRequired(), atoms.size());
  for (size_t i=0; i < atoms.size(); i++)
    {
      int atom = atoms.getAtom(i);
      int value = atoms.getValue(atom);
      bool sign = value > 0 ? true : false;
      poolPushLiteral(atom,sign,abs(value));
      m_literalCounts[sign][atom]++;
    }
  poolPushID(newID);

  PBClause &c = m_clauses[newID];

  // update some counts
  m_statistics.literalCount += c.size();
  m_statistics.addedClauseCount++;
  computeCounts(newID);

  // add to literal index
  for (size_t i=0, size=c.size(); i < size; i++)
    {
      bool sign = c.getSign(i);
      int atom = c.getAtom(i);
      int weight = c.getWeight(i);
      m_literalIndex[atom].getPBLitIndex(sign).push_back(WeightedClauseID(newID,weight));
    }

  return newID;
}


bool PBClauseSet::getImplications(Literal l, ImplicationList& units)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  vector<WeightedClauseID>& watchers = m_literalIndex[atom].getPBLitIndex(!sign);
  vector<WeightedClauseID>::iterator it = watchers.begin();
  vector<WeightedClauseID>::iterator end = watchers.end();
  
  for ( ; it != end; it++)
    {
      int id = (*it).id;
      if (isUnit(id))
	{
	  const PBClause& c = m_clauses[id];
	  for (size_t i=0, size=c.size();
	       i < size && c.getWeight(i) > m_possible[id]; i++)
	    {
	      int a = c.getAtom(i);
	      if (!m_assignment->isValued(a))
		{
		  Literal l(a,c.getSign(i));
#ifdef VERIFY
		  verifyReason(l,id);
#endif
		  if (units.contains(l.getNegation()))
		    {
		      m_conflict.atom = a;
		      m_conflict.reason1 = (id * 3) + 1;
		      m_conflict.reason2 = units.getReason(a);
		      return false;
		    }
		   if (!units.contains(l) || betterClause(id,units.getReason(a),a))
		     units.push(l,(id * 3) + 1);
		}
	    }
	}
    }
  return true;
}


void PBClauseSet::computeCounts(ClauseID id)
{
  const PBClause& c = m_clauses[id];
  m_possible[id] = - c.getRequired();
  m_current[id] = - c.getRequired();
  
  for (size_t i=0, size=c.size(); i < size; i++)
    {
      int atom = c.getAtom(i);
      int weight = c.getWeight(i);
      if (!m_assignment->isValued(atom)) m_possible[id] += weight;
      else
	{
	  if (isSAT(c.getReadLiteral(i)))
	    {
	      m_possible[id] += weight;
	      m_current[id] += weight;
	    }
	}
    }
}


bool PBClauseSet::initialClauseCheck(ImplicationList &unitList) const
{
  size_t nc = m_clauses.size();
  for (size_t i=1; i < nc; i++)
    {
      const PBClause &c = m_clauses[i];
      if (c.inUse())
	{
	  if (isUnit(i))
	    {
	      for (size_t j=0,sz=c.size(); j < sz; j++)
		{
		  if (c.getWeight(j) <= m_possible[i]) break;
		  Literal l(c.getAtom(j),c.getSign(j));
		  if (unitList.contains(l.getNegation()))
		    return false;
		  unitList.push(l,(i * 3) + 1);
		}
	    }
	}
    }
  return true;
}


void PBClauseSet::deleteIrrelevantClauses()
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
  for (size_t i=ic; i < nc; i++)
    {
      size_t size = m_clauses[i].size() - m_clauses[i].getRequired();
      if (!m_clauses[i].inUse() || size < m_settings.lengthBound) continue;
      int possible = m_possible[i];
      int weight0 = m_clauses[i].getWeight(0);
      bool notReason = (weight0 <= possible);
      if ((possible >= (int)(m_settings.relevanceBound) && notReason))
	deleteClause(i);
      else
	if ((size > m_settings.maxLength) && notReason)
	  deleteClause(i);
    }

  if (oldDeletedClauseCount == m_statistics.deletedClauseCount) return;

   // update pointers into the literalPool
   for (size_t i=1; i < m_literalIndex.size(); i++)
    {
      vector<WeightedClauseID> &watchers1 = m_literalIndex[i].getPBLitIndex(true);
      vector<WeightedClauseID>::iterator it1 = watchers1.begin();
      for ( ; it1 != watchers1.end(); it1++)
	{
	  PBClause& c = m_clauses[(*it1).id];
	  if (c.getFirst()->isNull())
	    {
	      *it1 = watchers1.back();
	      watchers1.pop_back();
	      it1--;
	    }
	}
      
      
      vector<WeightedClauseID> &watchers0 = m_literalIndex[i].getPBLitIndex(false);
      vector<WeightedClauseID>::iterator it0 = watchers0.begin();
      for ( ; it0 != watchers0.end(); it0++)
	{
	  PBClause& c = m_clauses[(*it0).id];
	  if (c.getFirst()->isNull())
	    {
	      *it0 = watchers0.back();
	      watchers0.pop_back();
	      it0--;
	    }
	}
    }
}


void PBClauseSet::verifyReason(Literal l,ClauseID id) 
{
  int atom = l.getAtom();
  bool foundAtom = false;
  const PBClause &c = m_clauses[id];
  computeCounts(id);
  for (size_t i=0; i < c.size(); i++)
    {
      int a = c.getAtom(i);
      if (atom == a)
	{
	  foundAtom = true;
	  if (m_possible[id] >= c.getWeight(i))
	    {
	      cerr << "Bad reason for " << l << endl;
	      cerr << m_clauses[id] << endl;
	      exit(1);
	    }
	}
    }

  if (!foundAtom)
    {
      cerr << "atom not found in PBClauseSet::verifyReason"<< endl;
      cerr << "looking for literal " << l << endl;
      c.printDetail(cerr);
      exit(1);
    }
}

void PBClauseSet::verifyConstraintSet()
{
  for (size_t i=1; i < m_clauses.size(); i++)
    {
      if (m_clauses[i].inUse())
	{
	  int oldPossible = m_possible[i];
	  int oldCurrent = m_current[i];
	  computeCounts(i);

	  if (oldPossible != m_possible[i])
	    fatalError("bad possible value in PBClauseSet::verifyConstraintSet");

	  if (oldCurrent != m_current[i])
	    fatalError("bad current value in PBClauseSet::verifyConstraintSet");

	  if (m_possible[i] < 0)
	    {
	      cerr << "ID: " << i << "   " << m_clauses[i];
	      cerr << "UNSAT clause in PBClauseSet::verifyConstraintSet" << endl;
	      exit(1);
	    }
	}
    }
}



void PBClauseSet::print(ostream &os) const
{
  for (size_t i = 1; i < m_clauses.size(); i++)
    if (m_clauses[i].inUse()) m_clauses[i].print();
}

void PBClauseSet::printDetail(ostream &os) const
{
  os << "PBClauseSet: " << endl;
  for (size_t i = 1; i < m_clauses.size(); i++) m_clauses[i].printDetail();
}

