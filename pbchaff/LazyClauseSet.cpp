/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#include "LazyClauseSet.h"
using namespace std;

LazyClauseSet::LazyClauseSet() : m_assignment(0)
{ 
  m_poolBegin = new PoolLiteral[STARTUP_LIT_POOL_SIZE];
  m_poolEnd = m_poolBegin;
  m_poolEndStorage = m_poolBegin + STARTUP_LIT_POOL_SIZE;
  poolPushNull(); // mark pool beginning
}


void LazyClauseSet::initialize(const PartialAssignment* ptr)
{
  m_assignment = ptr;

  // we don't allow clauseIDs of 0 so just push a dummy clause
  m_clauses.resize(1);
  m_clauses[0].initialize(0,0,0);
  m_currentCount.push_back(0);
  
  size_t numberVariables = m_assignment->getNumberVariables();
  for (size_t i=0; i <= numberVariables; ++i)
    {
      m_watchedLitIndex.push_back(Watchers());
      m_idLitIndex.push_back(LitIndex());
      m_scrapPaper.push_back(0);
      m_literalCounts[0].push_back(0);
      m_literalCounts[1].push_back(0);
    }
}




void LazyClauseSet::garbageCollect()
{
  // clear the watchers, we will rebuild them
  for (size_t i=1; i < m_watchedLitIndex.size(); ++i)
    {
      m_watchedLitIndex[i].getWatchers(true).clear();
      m_watchedLitIndex[i].getWatchers(false).clear();
    }


  // walk through the pool and rebuild it without the null literals
  // we rebuild the watchers as we go.
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
	  else if (pool[i].isWatched())
	    {
	      int atom = pool[i].getAtom();
	      bool sign = pool[i].getSign();
	      m_watchedLitIndex[atom].getWatchers(sign).push_back(&pool[newIndex]);
	    }
	  newIndex++;
	}
      else while (!pool[i].isID()) ++i; // scan to end of null clause
      
    }
  
  m_poolEnd = m_poolBegin + newIndex;
}




bool LazyClauseSet::enlargePool()
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
  vector<Clause>::iterator it = m_clauses.begin();
  vector<Clause>::iterator end = m_clauses.end();
  for ( ; it != end; ++it) it->incFirst(offset);

  vector<Watchers>::iterator wit = m_watchedLitIndex.begin();
  vector<Watchers>::iterator wend = m_watchedLitIndex.end();
  for ( ; wit != wend; ++wit)
    {
      vector<PoolLiteral*>::iterator plit = wit->getWatchers(true).begin();
      vector<PoolLiteral*>::iterator plend = wit->getWatchers(true).end();
      for ( ; plit != plend; ++plit) *plit += offset;
      
      plit = wit->getWatchers(false).begin();
      plend = wit->getWatchers(false).end();
      for ( ; plit != plend; ++plit) *plit += offset;
    }

  // delete the old pool
  delete [] oldBegin;
  return true;
  
}


/* this version of removing a clause removes it from all the
   literal indexes.  The function deleteClause doesn't do this.
*/
void LazyClauseSet::removeClause(int id)
{
  Clause& c = m_clauses[id];
  c.markUnused();
  int size = c.size();
  for (int i = 0; i < size; i++)
    {
      PoolLiteral &plit = c.getWriteLiteral(i);
      int atom = plit.getAtom();
      bool sign = plit.getSign();
      m_literalCounts[sign][atom]--;
      if (plit.isWatched())
	{
	  vector<PoolLiteral*> &watchers = m_watchedLitIndex[atom].getWatchers(sign);
	  for (size_t i=0,size=watchers.size(); i < size; ++i)
	    if (watchers[i] == &plit)
	      {
		watchers[i] = watchers[size - 1];
		watchers.pop_back();
		break;
	      }
	}
      plit.clear();
      vector<int> &index = m_idLitIndex[atom].getIndex(sign);
      for (size_t i=0,size=index.size(); i < size; ++i)
	if (id == index[i])
	  {
	    index[i] = index[size - 1];
	    index.pop_back();
	    break;
	  }
    }
  m_statistics.deletedClauseCount++;
  m_statistics.literalCount -= size;
  m_unusedIDs.push(id);
}



ClauseID LazyClauseSet::addClause(FastClause &atoms)
{
  
  int newID;
  size_t numberLits = atoms.size();

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
      m_currentCount.push_back(0);
    }
  else
    {
      newID = m_unusedIDs.front();
      m_unusedIDs.pop();
    }

  // add the constriant
  int noCNFlits = 1;
  if (m_statistics.initialClauseCount == 0)
    noCNFlits = choose(atoms.size() -1, atoms.getRequired() -1);
  m_clauses[newID].initialize(m_poolEnd, atoms.getRequired(), atoms.size());
  for (size_t i=0; i < atoms.size(); ++i)
    {
      int atom = atoms.getAtom(i);
      bool sign = (atoms.getValue(atom) > 0 ? true : false);
      poolPushLiteral(atom,sign);
//        if (m_statistics.initialClauseCount == 0 ||
//  	  (m_assignment->isValued(atom) && sign != m_assignment->getValue(atom)))
	m_literalCounts[sign][atom] += noCNFlits;
    }
  poolPushID(newID);
  Clause &c = m_clauses[newID];
  ++m_statistics.addedClauseCount;
  m_statistics.literalCount += c.size();

  // if the clause is unit return
  if (c.size() == c.getRequired()) return newID;
  
  //set the watched literals
  // calculate number of watchers needed and
  // divide them between UP and DOWN
  int needed = c.getRequired() + 1;
  int numUP = needed / 2;
  int foundUP = 0;
  int numDOWN = needed - numUP;
  int foundDOWN = 0;

  // first the UP watchers
  // look for SAT or unvalued literals
  for (size_t i =0 ; i < numberLits; ++i)
    {
      PoolLiteral* lit = & c.getWriteLiteral(i);
      if (!isUNSAT(lit))
	{
	  int atom = lit->getAtom();
	  bool sign = lit->getSign();
	  m_watchedLitIndex[atom].getWatchers(sign).push_back(lit);
	  lit->setWatch(UP);
	  ++foundUP;
	  if (foundUP == numUP) break;
	}
    }

  // new we do the DOWN watchers
  if (foundUP == numUP)
    {
      for (size_t i = numberLits - 1; i >= 0; i--)
	{
	  PoolLiteral* lit = & c.getWriteLiteral(i);
	  if (lit->isWatched()) break; // we've hit the ups
	  if (!isUNSAT(lit))
	    {
	      int atom = lit->getAtom();
	      bool sign = lit->getSign();
	      m_watchedLitIndex[atom].getWatchers(sign).push_back(lit);
	      lit->setWatch(DOWN);
	      ++foundDOWN;
	      if (foundDOWN == numDOWN) break;
	    }     
	}
    }

  // remaining unwatched lits are UNSAT but we still need more watchers.
  // just walk up the assignment stack and watch clause lits
  // valued deepest
  if (foundDOWN != numDOWN)
    {
      int stillNeeded = needed - foundUP - foundDOWN;
      int halfWayMark = c.size() / 2;
      
      for (size_t i = m_assignment->size() - 1 ; i >= 0; --i)
	{
	  Literal l = m_assignment->getLiteral(i);
	  int a = l.getAtom();
	  bool s = l.getSign();

	  if (atoms.contains(a))
	    {
	      int v = atoms.getValue(a);
	      // it's valued unfavorably
	      if ((v > 0) != s)
		{
		  int index = atoms.getPosition(a);
		  PoolLiteral* lit = &c.getWriteLiteral(index);
		  m_watchedLitIndex[a].getWatchers(!s).push_back(lit);
		  lit->setWatch((index < halfWayMark) ? UP : DOWN);
		  --stillNeeded;
		}
	    }
	  if (stillNeeded == 0) break;
	}
    }

  // add to fullIndex
  if (m_settings.strengthenOn)
    {
      computeCounts(newID);
      
      for (size_t i=0; i < c.size(); ++i)
	{
	  const PoolLiteral &pl = c.getReadLiteral(i);
	  m_idLitIndex[pl.getAtom()].getIndex(pl.getSign()).push_back(newID);
	}
    }
  return newID;
}

/*
  We walk through all clauses that are watching the literal l.

  For each clause we begin at the watched literal
  and we walk in the specified direction. When we hit an end
  marker ( a non-literal PoolLiteral) we change direction
  and walk from the original watched literal in the other
  direction again until we hit the end of the clause.
  We record a pointer to all other watched lits we encounter.
  We can stop if we encounter an unvalued or satisfied
  literal that is not already watched. We make this the the new
  watched literal.
  If we walk all the literals (they are all unsat except
  the other watched literals), then we check the watched literals
  we found for unit propagations.  Before adding a new unit
  propagation to the list, we make sure its negation is not
  already there.  If it is return contradiction.
*/

bool LazyClauseSet::getImplications(Literal l, ImplicationList& units)
{
  int atom = l.getAtom();
  bool sign = l.getSign();
  vector<PoolLiteral*>& watched = m_watchedLitIndex[atom].getWatchers(!sign);
  vector<PoolLiteral*>::iterator it = watched.begin();

  for ( ; it != watched.end(); ++it)
    {
      PoolLiteral* watchedLit = *it;
      PoolLiteral* ptr = watchedLit;

      // don't bother if the clause isn't in use
      if (ptr->isNull()) continue;
      
      int dir = (watchedLit->getDirection() ? 1 : -1);
      ClauseID clauseID = -1;
      size_t watchCount = 0;
      
      while(true)
	{
	  ptr += dir;
	  if (!ptr->isLiteral())
	    {
	      if (dir == 1) clauseID = ptr->getID(); 
	      if ((dir > 0) == watchedLit->getDirection())
		{
		  ptr = watchedLit;
		  dir = -dir;
		  continue;
		}

	      for (size_t j = 0; j < watchCount; ++j)
		{
		  int atom = m_scrapPaper[j]->getAtom();
		  if (!m_assignment->isValued(atom)) {
		    Literal l(atom,m_scrapPaper[j]->getSign());
#ifdef VERIFY
		    verifyReason(l,clauseID);
#endif
		    if (units.contains(l.getNegation()))
		      {
			m_conflict.atom = atom;
			m_conflict.reason1 = clauseID * 3;
			m_conflict.reason2 = units.getReason(atom);
			return false;
		      }
  		    if (!units.contains(l) || betterClause(clauseID,units.getReason(atom)))
		      units.push(l,clauseID * 3);
		  }
		}
	      break;
	    }

	  if (ptr->isWatched())
	    {
	      m_scrapPaper[watchCount] = ptr;
	      ++watchCount;
	      continue;
	    }

	  if (isUNSAT(ptr)) continue;

	  int a = ptr->getAtom();
	  int s = ptr->getSign();
	  m_watchedLitIndex[a].getWatchers(s).push_back(ptr);
	  watchedLit->unsetWatch();
	  ptr->setWatch((dir > 0) ? true : false);

	  *(it--) = watched.back();
	  watched.pop_back();
	  break;
	}
    }
  return true;
}


void LazyClauseSet::computeCounts(ClauseID id)
{
  Clause& c = m_clauses[id];
  m_currentCount[id] = - c.getRequired();
  
  for (size_t i=0; i < c.size(); ++i)
    {
      const PoolLiteral& pl = c.getReadLiteral(i);
      if (isSAT(&pl)) ++m_currentCount[id];
    }
}


bool LazyClauseSet::initialClauseCheck(ImplicationList &unitList) const
{
  size_t nc = m_clauses.size();
  for (size_t i=1; i < nc; ++i)
    {
      if (m_clauses[i].inUse())
	{
	  const Clause &c = m_clauses[i];
	  if ( c.size() == c.getRequired())
	    {
	      for (size_t j=0; j < c.size(); ++j)
		{
		  Literal l(c.getAtom(j),c.getSign(j));
		  if (unitList.contains(l.getNegation()))
		    return false;
		  unitList.push(l,i * 3);
		}
	    }
	}
    }
  return true;
}


void LazyClauseSet::deleteIrrelevantClauses()
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
      Clause &c = m_clauses[i];
      size_t size = c.size() - c.getRequired();
      if (!c.inUse() || size < m_settings.lengthBound) continue;
      int possible = - c.getRequired();
      for (size_t j=0; j < c.size(); j++)
	{
	  PoolLiteral* lit = & c.getWriteLiteral(j);
	  if (!isUNSAT(lit)) possible++;

	  if (possible >= (int)(m_settings.relevanceBound))
	    {
	      deleteClause(i);
	      break;
	    }

	  if (size > m_settings.maxLength && (possible > 0))
	    {
	      deleteClause(i);
	      break;
	    }
	}
    }

  if (oldDeletedClauseCount == m_statistics.deletedClauseCount) return;

   // update pointers into the literalPool
   for (size_t i=1; i < m_watchedLitIndex.size(); ++i)
    {
      vector<PoolLiteral*> &watchers1 = m_watchedLitIndex[i].getWatchers(true);
      vector<PoolLiteral*>::iterator it1 = watchers1.begin();
      for ( ; it1 != watchers1.end(); ++it1)
	{
	  if ((*it1)->isNull())
	    {
	      *it1 = watchers1.back();
	      watchers1.pop_back();
	      --it1;
	    }
	}
      
      
      vector<PoolLiteral*> &watchers0 = m_watchedLitIndex[i].getWatchers(false);
      vector<PoolLiteral*>::iterator it0 = watchers0.begin();
      for ( ; it0 != watchers0.end(); ++it0)
	{
	  if ((*it0)->isNull())
	    {
	      *it0 = watchers0.back();
	      watchers0.pop_back();
	      --it0;
	    }
	}

      if (m_settings.strengthenOn)
	{
	  vector<int> &index1 = m_idLitIndex[i].getIndex(true);
	  vector<int>::iterator it3 = index1.begin();
	  for ( ; it3 != index1.end(); ++it3)
	    {
	      Clause& c = m_clauses[*it3];
	      if (c.getFirst()->isNull())
		{
		  *it3 = index1.back();
		  index1.pop_back();
		  --it3;
		}
	    }
	  
	  
	  vector<int> &index0 = m_idLitIndex[i].getIndex(false);
	  vector<int>::iterator it2 = index0.begin();
	  for ( ; it2 != index0.end(); ++it2)
	    {
	      Clause& c = m_clauses[*it2];
	      if (c.getFirst()->isNull())
		{
		  *it2 = index0.back();
		  index0.pop_back();
		  --it2;
		}
	    }
	}
    }
}

void LazyClauseSet::verifyReason(Literal l,ClauseID id) const
{
  int atom = l.getAtom();
  const Clause &c = m_clauses[id];
  int required = c.getRequired();
  int possible = 1;
  for (size_t i=0; i < c.size(); ++i) {
    int a = c.getAtom(i);
    bool s = c.getSign(i);
    if (m_assignment->isValued(a))
      {
	bool value = m_assignment->getValue(a);
	if ((value == s) && (atom != a))
	  {
	    ++possible;
	    if (possible > required)
	      {
		cerr << c << " reason for " << l << endl;
		cerr << "extra sat literal in LazyClauseSet::verifyReason" << endl;
		exit(1);
	      }
	  }
      }
    else
      {
	if (atom != a)
	  {
	    ++possible;
	    if (possible > required)
	      {
		cerr << c << " reason for " << l << endl;
		cerr << "bad reason in LazyClauseSet::verifyReason" << endl;
		exit(1);
	      }
	  }
      }
  }
}

void LazyClauseSet::verifyConstraintSet() const
{
  for (size_t i=1; i < m_clauses.size(); ++i)
    {
      if (m_clauses[i].inUse())
	{
	  const Clause &c = m_clauses[i];
	  int possible = 0;
	  for (size_t j=0; j < c.size(); ++j)
	    {
	      int atom = c.getAtom(j);
	      if (m_assignment->isValued(atom))
		{
		  bool sign = c.getSign(j);
		  if (m_assignment->getValue(atom) == sign) ++possible;
		}
	      else ++possible;
	    }
	  if (possible < (int)(c.getRequired()))
	    {
	      cerr << "UNSAT clause in LazyClauseSet::verifyConstraintSet" << endl;
	      exit(1);
	    }
	}
    }
}


void LazyClauseSet::print(ostream &os) const
{
  for (size_t i = 1; i < m_clauses.size(); ++i)
    if (m_clauses[i].inUse()) m_clauses[i].print();
}

void LazyClauseSet::printDetail(ostream &os) const
{
  os << "LazyClauseSet: " << endl;
  for (size_t i = 1; i < m_clauses.size(); ++i) m_clauses[i].printDetail();
}

