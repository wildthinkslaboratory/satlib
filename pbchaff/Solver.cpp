/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#include <fstream>
#include "FastClause.h"
#include "Solver.h"
#include <algorithm>
#include <string>

using namespace std;





void Solver::solve()
{
  m_clauseSet.setInitialClauseCount();
  m_timer.initialize();
  m_statistics.outcome = realSolve();
  m_statistics.runTime = m_timer.getElapsedTime();
  if (m_statistics.outcome == SATISFIABLE)
    m_assignment.printNameForm();
}


int Solver::realSolve()
{
  if (!initialClauseCheck()) return UNSATISFIABLE;
  if (m_settings.failedLiteral)
    if (!failedLiteralTest()) return UNSATISFIABLE;
  
  while (!m_assignment.isFull())
    {
      runPeriodicFunctions();
      selectBranchVariable();
      while (!unitPropagate())
		if (!backtrack()) return UNSATISFIABLE; 
      if (m_timer.getElapsedTime() > m_settings.timeLimit) return TIME_OUT;
	  if (zap::global_vars.time_out > 0 && zap::global_vars.time_out < (zap::get_cpu_time()-zap::global_vars.start_time))
		return TIME_OUT;
    }
  return SATISFIABLE;

}



// the actual backjumping is called from learn
bool Solver::backtrack()
{
  zap::global_vars.number_backtracks++;
  m_statistics.backtrackCount++;
  return learn();
}



bool Solver::unitPropagate()
{
  while (!m_unitList.empty())
    {
      pair<Literal,int> p = m_unitList.pop();
      m_assignment.push(p.first,p.second);
      if (!m_clauseSet.getImplications(p.first,m_unitList))
	{
	  const Conflict& conflict = m_clauseSet.getConflict();
	  m_conflictAtom = conflict.atom;
	  m_clauseSet.initializeClause(m_parent1,conflict.reason1);
	  m_clauseSet.initializeClause(m_parent2,conflict.reason2);
	  m_unitList.clear();
	  return false;
	}
    }
  return true;
}





/* backjump to a given descision level.  Each assignment
   in the stack of assignments has a decision level which
   corresponds to the number of branch decisions that come
   before the assignment
*/
void Solver::backjumpToLevel(int level)
{
  ASSERT(level >= 0);
  while (true)
    {
      int lv = m_assignment.getDecisionLevel();
      if (lv <= level ) break;
      Literal l = m_assignment.getTop();
      m_assignment.pop();
      m_clauseSet.unwind(l);
      
    }
}

void Solver::backjumpToAtom(int atom)
{
  ASSERT(m_assignment.contains(atom)); 
  while (1)
    {
      Literal l = m_assignment.getTop();
      if ((int)(l.getAtom()) == atom) break;
      m_assignment.pop();
      m_clauseSet.unwind(l);
      
    }
}



/*************************** CONFLICT ANALYSIS ***************************/


bool Solver::learn()
{
  int id =-1;
  
  while (true)
    {
      if (m_settings.verbosity)
	{
	  m_assignment.print();
	  cout << endl << "Conflict with variable " << m_conflictAtom << endl;
	  cout << "Reasons for conflict: " << endl << m_parent1 << m_parent2 << endl;
	}

      if (m_parent1.isMod2() && m_parent2.isMod2())
	{
	  if (!learnMod2Constraint()) return false;
	}
      else
	{
	  if (m_parent1.isMod2()) mod2ToDisjunction(m_parent1);
	  else
	    {
	      if (m_parent2.isMod2()) mod2ToDisjunction(m_parent2);
	    }
	  
	  switch(m_settings.learnMethod)
	    {
	    case 0 : if (!learnCardinality()) return false; break;
	    case 1 : if (!learnPBConstraint()) return false; break;
	    default : if (!learnCardinality()) return false; break;
	    }
	}

      
      if (m_settings.verbosity) cout << "Learned nogood : " << m_parent1 << endl;

      if (m_level1 > m_level2)
	{
	  // if the clause is UIP then add to constaint set and we're done
	  id = m_clauseSet.addClause(m_parent1);
	  break;
	}
      else
	{
	  // prepare to do another round of learn and backup
	  int reasonID = m_assignment.getReason(m_atom1);
	  m_clauseSet.initializeClause(m_parent2,reasonID);
	  m_conflictAtom = m_atom1;
	  backjumpToAtom(m_atom1);
	}
      
    }

  // backup to the level where the nogood would become unit
  backjumpToLevel(m_level2);

  if (m_parent1.isMod2()) collectUnitPropsMod2(id);
  else collectUnitProps(id);
  


  return true;
}


void Solver::collectUnitProps(ClauseID id)
{
  // there may be more than one unit propagation so get them all
  int possible = m_parent1.getPossible();
  size_t size = m_parent1.size();
  int maxNeeded = - m_parent1.getRequired();
  for (size_t i=0; i < size && m_parent1.getWeight(i) > possible; ++i)
    {
      Literal lit = m_parent1.getLiteral(i);
      if (!m_assignment.isValued(lit.getAtom()))
	{
#ifdef VERIFY
	  m_clauseSet.verifyReason(lit,id);
#endif
	  m_unitList.push(lit,id);
	  
	  // don't walk any more than necessary
	  maxNeeded += m_parent1.getWeight(i);
	  if (maxNeeded >= 0) break;
	}
    }
}


void Solver::collectUnitPropsMod2(ClauseID id)
{
    // there may be more than one unit propagation so get them all
  size_t size = m_parent1.size();
  for (size_t i=0; i < size; ++i)
    {
      int atom = m_parent1.getAtom(i);
      if (!m_assignment.isValued(atom))
	{
	  Literal l(atom,m_parent1.getPossible() > 0 ? true : false);
#ifdef VERIFY
	  m_clauseSet.verifyReason(l,id);
#endif
	  m_unitList.push(l,id);
	  break;
	}
    }
}



bool Solver::learnCardinality()
{
  // determine if we need to weaken a constraint before combining them.
  int coefficient1 = m_parent1.getValue(m_conflictAtom);
  int coefficient2 = m_parent2.getValue(m_conflictAtom);
  
  if ((abs(coefficient1) != 1) && (abs(coefficient2) != 1))
    m_parent1.weakenToCardinality(m_assignment,m_conflictAtom);
  
  // generate a new constraint
  m_parent1.resolve(m_parent2,m_conflictAtom);

  if (m_settings.andrewOpt)
    {
      // remove literals that are permanently forced
      for (size_t i=0; i < m_parent1.size(); ++i)
	{
	  Literal l = m_parent1.getLiteral(i);
	  if (m_assignment.isValued(l.getAtom()) && m_assignment.getLevel(l.getAtom()) == 0)
	    {
	      int weight = m_parent1.getWeight(i);
	      m_parent1.remove(l.getAtom());
	      --i;
	      if (!m_assignment.isUnsat(l)) m_parent1.incRequired(-weight);
	    }
	}
    }
  
    // make sure its not the empty clause
  int sum = m_parent1.sumCoefficients();
  if (sum < m_parent1.getRequired()) return false;

  // reduce it to a cardinality constraint
  m_parent1.simplify();
  if (m_parent1.getWeight(0) != 1)
    m_parent1.weakenToCardinality(m_assignment,-1);
  
  // calculate possible
  int possible = - m_parent1.getRequired();
  for (size_t i=0, size = m_parent1.size(); i < size; ++i)
    {
      if (!m_assignment.isUnsat(m_parent1.getLiteral(i)))
	++possible;
    }
  
  // calculate m_atom1 (backup to m_atom1 and constraint nolonger UNSAT)
  // and m_atom2 (backup to m_atom2 and constraint will unit propagate)
  m_atom1 = m_atom2 = -1;
  for (int i=m_assignment.size() - 1; i >= 0; --i)
    {
      int atom = m_assignment.getAtom(i);
      bool sign = m_assignment.getValue(atom);
      if (m_parent1.contains(atom))
	{
	  bool s = m_parent1.getValue(atom) > 0;
	  if (s != sign)
	    {
	      if (possible == -1) m_atom1 = atom;
	      if (possible == 0) {
		m_atom2 = atom;
		break;
	      }
	      ++possible;
	    }
	}
    }

  m_parent1.setPossible(0);
  
  // get the decision level of these literals
  ASSERT(m_atom1 != -1);
  m_level1 = m_assignment.getLevel(m_atom1);
  m_level2 = 0;
  if (m_atom2 != -1) m_level2 = m_assignment.getLevel(m_atom2);
  return true;
  
}


bool Solver::learnPBConstraint()
{
  // determine if we need to weaken a constraint before combining them.
  int coefficient1 = m_parent1.getValue(m_conflictAtom);
  int coefficient2 = m_parent2.getValue(m_conflictAtom);
  
  if ((abs(coefficient1) != 1) && (abs(coefficient2) != 1))
    m_parent1.weakenToCardinality(m_assignment,m_conflictAtom);

  if (m_parent1.getWeight(0) >= 8192)
    m_parent1.weakenToCardinality(m_assignment,m_conflictAtom);
      
  if (m_parent2.getWeight(0) >= 8192)
    m_parent2.weakenToCardinality(m_assignment,m_conflictAtom);
  
  // generate a new constraint
  m_parent1.resolve(m_parent2,m_conflictAtom);

  // remove literals that are permanently forced
  for (size_t i=0; i < m_parent1.size(); ++i)
    {
      Literal l = m_parent1.getLiteral(i);
      if (m_assignment.isValued(l.getAtom()) && m_assignment.getLevel(l.getAtom()) == 0)
	{
	  int weight = m_parent1.getWeight(i);
	  m_parent1.remove(l.getAtom());
	  --i;
	  if (!m_assignment.isUnsat(l)) m_parent1.incRequired(-weight);
	}
    }

    // make sure its not the empty clause
  int sum = m_parent1.sumCoefficients();
  if (sum < m_parent1.getRequired()) return false;
  
  m_parent1.simplify();
  
  // calculate possible
  int possible = - m_parent1.getRequired();
  for (size_t i=0, size = m_parent1.size(); i < size; ++i)
    {
      if (!m_assignment.isUnsat(m_parent1.getLiteral(i)))
	possible += m_parent1.getWeight(i);
    }
  
  // calculate m_atom1 (backup to m_atom1 and constraint nolonger UNSAT)
  // and m_atom2 (backup to m_atom2 and constraint will unit propagate)
  m_atom1 = m_atom2 = -1;
  bool foundAtom1 = false;
  int atom1Weight = 0;
  for (int i=m_assignment.size() - 1; i >= 0; --i)
    {
      int atom = m_assignment.getAtom(i);
      bool sign = m_assignment.getValue(atom);
      if (m_parent1.contains(atom))
	{
	  int value = m_parent1.getValue(atom);
	  int weight = abs(value);
	  bool s = value > 0;
	  if (s != sign)
	    {
	      possible += weight;
	      
	      if (foundAtom1 && atom1Weight <= possible)
		{
		  m_atom2 = atom;
		  possible -= weight;
		  m_parent1.setPossible(possible);
		  break;
		}
	      
	      if (!foundAtom1 && possible >= 0)
		{
		  m_atom1 = atom;
		  atom1Weight = weight;
		  foundAtom1 = true;
		}
	    }
	}
    }
  if (m_atom2 == -1) m_parent1.setPossible(possible);
  
  // get the decision level of these literals
  ASSERT(m_atom1 != -1);
  m_level1 = m_assignment.getLevel(m_atom1);
  m_level2 = 0;
  if (m_atom2 != -1) m_level2 = m_assignment.getLevel(m_atom2);
  return true;
  
}





bool Solver::learnMod2Constraint()
{

  ASSERT(m_parent1.contains(m_conflictAtom));
  ASSERT(m_parent2.contains(m_conflictAtom));
  ASSERT(m_parent1.isMod2());
  ASSERT(m_parent2.isMod2());

  m_parent1.add(m_parent2);
  for (size_t i=0; i < m_parent1.size(); ++i)
    {
      if (m_parent1.getWeight(i) == 2)
	{
	  m_parent1.removeByIndex(i);
	  --i;
	}
    }

  if (m_parent1.getRequired() == 2)
    m_parent1.setRequired(0);

  // remove literals that are permanently forced

  // make sure its not the empty clause
  if (m_parent1.size() == 0) return false;
 
  // calculate m_atom1 (backup to m_atom1 and constraint nolonger UNSAT)
  // and m_atom2 (backup to m_atom2 and constraint will unit propagate)
  int unvaluedCount = 0;
  m_atom1 = m_atom2 = -1;
  for (int i=m_assignment.size() - 1; i >= 0; --i)
    {
      int atom = m_assignment.getAtom(i);
      if (m_parent1.contains(atom))
	{
	  if (unvaluedCount == 0) m_atom1 = atom;
	  if (unvaluedCount == 1) {
	    m_atom2 = atom;
	    break;
	  }
	  ++unvaluedCount;
	}
    }
  int sign = (!m_assignment.getValue(m_atom1)) ? 1 : 0;
  m_parent1.setPossible(sign);
  
  // get the decision level of these literals
  ASSERT(m_atom1 != -1);
  m_level1 = m_assignment.getLevel(m_atom1);
  m_level2 = 0;
  if (m_atom2 != -1) m_level2 = m_assignment.getLevel(m_atom2);
  return true;
  
}


void Solver::mod2ToDisjunction(FastClause& c)
{
  int sum = 0;
  for (size_t i=0,sz=c.size(); i < sz; ++i)
    {
      int atom = c.getAtom(i);
      if (atom != m_conflictAtom)
	{
	  bool value = m_assignment.getValue(atom);
	  if (value == true)
	    {
	      sum++;
	      c.increment(atom,-2);
	    }
	}
    }

  
  if (sum % 2 == c.getRequired()) c.increment(m_conflictAtom,-2);
  c.setRequired(1);
}



//************************ STRENGTHENING ******************************



void Solver::strengthenConstraintSet()
{
  vector<int> reasons;
  
  const StackOfLists &overSatClauses = m_clauseSet.getOverSatClauses();
  for (size_t i=0; i < overSatClauses.size(); ++i)
    {
      int id = overSatClauses.elementAt(i);
      m_clauseSet.initializeClause(m_strengthen1,id);
      m_assumptions.clear();
      reasons.clear();
      m_clauseSet.getWhyOverSat(m_strengthen1,m_assumptions,reasons);
      if (m_assumptions.size() == m_settings.strengthenBound)
	{
	  m_strengthen1.strengthen(m_assumptions);
	  int newID = m_clauseSet.addClause(m_strengthen1);
	  m_clauseSet.initializeClause(m_strengthen2,id);
	  if (m_strengthen1.subsumes(m_strengthen2))
	    m_clauseSet.removeClause(id);
	  removeSubsumedClauses(reasons,newID);
	}
    }
}

/* we've just strengthened a clause so we'd like to remove
   any newly subsumed clauses.  We pass in a vector reasonsToCheck
   that has likely candidates for subsumption.  If we find
   a subsumed clause we first need to check and see if there are
   other pointers out to this clause before we remove it.  We
   then replace all pointers to subsumed clauses with a
   pointer to the new clause
*/
void Solver::removeSubsumedClauses(vector<int>& reasonsToCheck, int newID)
{
  for (size_t i=0,size=reasonsToCheck.size(); i < size; ++i)
    {
      int id = m_assignment.getReason(reasonsToCheck[i]);
      if (id == newID) continue;
      m_clauseSet.initializeClause(m_strengthen2,id);
      if (m_strengthen1.subsumes(m_strengthen2))
	{
	  // compute possible
	  int possible = - m_strengthen2.getRequired();
	  for (size_t j=0,s=m_strengthen2.size(); j < s; ++j)
	    {
	      Literal l = m_strengthen2.getLiteral(j);
	      if (!m_assignment.isUnsat(l)) possible += m_strengthen2.getWeight(j); 
	    }
	  
	  // check for other pointers to the subsumed clause
	  vector<int> toUpdate;
	  for (size_t j=0,s=m_strengthen2.size(); j < s; ++j)
	    {
	      if (m_strengthen2.getWeight(j) <= possible) break;
	      Literal l = m_strengthen2.getLiteral(j);
	      if (!m_assignment.isUnsat(l) && m_assignment.isValued(l.getAtom()))
		{
		  toUpdate.push_back(l.getAtom());
		}
	    }
	  
	  // update all pointers to the new clause
	  for (size_t j=0,s=toUpdate.size(); j < s; ++j)
	    {
	      m_assignment.updateReason(toUpdate[j],newID);
	    }
	  m_clauseSet.removeClause(id);
	}
    }
}



//************************ PREPROCESSING *******************************




void Solver::preprocess()
{
  m_timer.initialize();
  preprocessStrengthen();
  m_statistics.preprocessTime = m_timer.getElapsedTime();
}


void Solver::preprocessStrengthen()
{
  m_clauseSet.setStrengthen(true);
  
  size_t numVars = m_assignment.getNumberVariables();
  for (size_t i=1; i < numVars; ++i)
    {
      m_unitList.push(Literal(i,true),0);
      unitPropagate();
      strengthenConstraintSet();
      backjumpToLevel(0);
      
      m_unitList.push(Literal(i,false),0);
      unitPropagate();
      strengthenConstraintSet();
      backjumpToLevel(0);
    }
  m_clauseSet.garbageCollect();
  m_clauseSet.setStrengthen(false);
}


bool Solver::initialClauseCheck()
{
  if (!m_clauseSet.initialClauseCheck(m_unitList))
    return false;

  return unitPropagate();
}

bool Solver::failedLiteralTest()
{ 
  size_t numVars = m_assignment.getNumberVariables();
  for (size_t i=1; i < numVars; ++i)
    {
      int level = m_assignment.getDecisionLevel();
      if (!m_assignment.isValued(i))
	{
	  m_unitList.push(Literal(i,true),0);
	  while (!unitPropagate())
	    if (!backtrack()) return false;
	  
	  if (level < m_assignment.getDecisionLevel()) backjumpToLevel(level);
	  
	  if (!m_assignment.isValued(i))
	    { 
	      m_unitList.push(Literal(i,false),0);
	      while (!unitPropagate())
		if (!backtrack()) return false;
	      backjumpToLevel(level);
	    }
	}
    }
  return true;
}



//********************* BRANCH SELECTION ******************************






void Solver::selectBranchVariable()
{
  Literal l;
  switch (m_settings.branchHeuristic)
    {
    case 0: l = getVsidsLiteral(); break;
    case 1: l = getFirstUnvalued(); break;
    case 2: l = getRandomUnvalued(); break;
    default: l = getVsidsLiteral(); break;
    }

  if (m_settings.verbosity) cout << "Branching on literal " << l << endl;
  
  m_unitList.push(l,0);
  m_statistics.nodeCount++;
  zap::global_vars.number_branch_decisions++;
}


Literal Solver::getFirstUnvalued()
{
  
  size_t nv = m_assignment.getNumberVariables();
  size_t i = 1;
  for (; i < nv; ++i) 
    if (!m_assignment.isValued(i)) break;

  return Literal(i,true);
}

Literal Solver::getRandomUnvalued()
{
  size_t nv = m_assignment.getNumberVariables();
  vector<int> candidates;
  candidates.reserve(nv);
  for (size_t i=0; i < nv; ++i) 
    if (!m_assignment.isValued(i)) candidates.push_back(i);
  
  return Literal(candidates[(rand() % candidates.size())],coinFlip());
}

inline bool compare_var_stat(const pair<int,double> & v1, 
			    const pair<int,double> & v2) 
{	
    if (v1.second > v2.second) return true;
    return false;
}


void Solver::updateVsidsCounts()
{
  size_t nv = m_assignment.getNumberVariables(); 
  for (size_t i=1; i <= nv; ++i) {
    m_vsidsScores[0][i] = m_clauseSet.getLiteralCount(Literal(i,false));
    m_vsidsScores[1][i] = m_clauseSet.getLiteralCount(Literal(i,true));
    bool bigger = (m_vsidsScores[1][i] > m_vsidsScores[0][i]);
    m_sortedScores[i] = make_pair(int(i),(bigger ? m_vsidsScores[1][i] : m_vsidsScores[0][i]));
  }
  stable_sort(m_sortedScores.begin(), m_sortedScores.end(), compare_var_stat);
  m_clauseSet.reduceCounts();
}


Literal Solver::getVsidsLiteral()
{
  size_t size = m_sortedScores.size();
  int atom = 0;
  for (size_t i= 0; i < size; ++i)
    {
      atom = m_sortedScores[i].first;
      if (!m_assignment.isValued(atom))
	{ 
	  // reduce the randomness but not below the base value
	  m_settings.randomness--;
	  if (m_settings.randomness < m_settings.baseRandomness)
	    m_settings.randomness = m_settings.baseRandomness;

	  int randomness = m_settings.randomness;
	  int freeCount = m_assignment.getNumberFreeVariables();
	  if (randomness >= freeCount)
	    randomness = freeCount - 1;
	  int skip = rand() % (1 + randomness);
	  int index = i;
	  while (skip > 0)
	    {
	      index++;
	      atom = m_sortedScores[index].first;
	      if (!m_assignment.isValued(atom)) skip--;
	    }
	  break;
	}
    }
  return Literal(atom,(m_vsidsScores[1][atom] > m_vsidsScores[0][atom]));
}



//******************************** MISCELLANEOUS *****************************


void Solver::restart()
{
  if (m_assignment.getDecisionLevel() > 0)
    {
      for (std::size_t i=1, size=m_vsidsScores[0].size(); i<size; ++i)
	{
	  m_vsidsScores[0][i] = 0;
	  m_vsidsScores[1][i] = 0;
	}
    }
  updateVsidsCounts();
  m_unitList.clear();
  backjumpToLevel(0);
  
}


void Solver::initialize(size_t size)
{
  /// initialize the problem
  m_assignment.initialize(size);
  m_clauseSet.initialize(&m_assignment);
  m_unitList.initialize(size);
  m_parent1.initialize(size);
  m_parent2.initialize(size);
  m_strengthen1.initialize(size);
  m_strengthen2.initialize(size);
  m_assumptions.initialize(size);
  for (size_t i=0; i < size; i++)
    {
      m_sortedScores.push_back(pair<int,double>(0,0.0));
      m_vsidsScores[0].push_back(0.0);
      m_vsidsScores[1].push_back(0.0);
    }
}



void Solver::runPeriodicFunctions()
{
#ifdef VERIFY
  m_clauseSet.verifyConstraintSet();
#endif

  if (m_statistics.backtrackCount % m_settings.deletionInterval == 0)
    m_clauseSet.deleteIrrelevantClauses();

  if (m_settings.allowRestarts && m_statistics.backtrackCount > m_settings.nextRestartBacktrack)
    {
      m_settings.nextRestartBacktrack += m_settings.restartBacktrackInc;
      m_settings.restartBacktrackInc += m_settings.restartBacktrackIncInc;
      float current = m_timer.getElapsedTime();
      if (current > m_settings.nextRestartTime)
	{
	  cout << "restart...... " << endl;
	  m_settings.nextRestartTime = int(current) + m_settings.restartTimeInc;
	  m_settings.restartTimeInc += m_settings.restartTimeIncInc;
	  m_settings.randomness = m_settings.restartRandomness;
	  restart();
	}
    }
  if (m_statistics.nodeCount % m_settings.vsidsUpdateInterval == 0)
    updateVsidsCounts();
}

// works but needs cleaning
int Solver::ISAMPwithLearning()
{
  if (!initialClauseCheck()) return UNSATISFIABLE;
  runPeriodicFunctions();
  selectBranchVariable();
  
  while (true)
    {
      while (!m_assignment.isFull() && unitPropagate())
	selectBranchVariable();
      
      if (m_assignment.isFull()) return SATISFIABLE;
      else
	if (!backtrack()) return UNSATISFIABLE;
      if (m_assignment.getDecisionLevel() > 0)
	{
	  for (std::size_t i=1, size=m_vsidsScores[0].size(); i<size; ++i)
	    {
	      m_vsidsScores[0][i] = 0;
	      m_vsidsScores[1][i] = 0;
	    }
	  m_unitList.clear();
	}
      updateVsidsCounts();
      backjumpToLevel(0);
    }
}


//******************************** PARSER *****************************


void Solver::load_to_structures(const zap::Cnf& clauses)
{
  size_t nv = zap::global_vars.atom_name_map.size();
  initialize(nv + 1);
  FastClause a(nv + 1);

  m_clauseSet.setStrengthen(true);
  
  for (size_t i=0; i < clauses.size(); i++) {
	if (clauses[i].size() == 0) continue;
	a.clear();
	for (size_t j=0; j < clauses[i].size(); j++) {
	  a.addIfAbsent(clauses[i][j].variable(),(clauses[i][j].sign() ? 1 : -1));
	}
	a.setRequired(1);
	m_clauseSet.addClause(a);
  }

  m_clauseSet.setInitialClauseCount();
  m_clauseSet.setStrengthen(false);
}



void Solver::parse(const char * filename) {

  ifstream infile(filename);
  
  char buffer[5000];
  char c;
  int nv, nc, ClauseCount = 0;
  int lineNumber = 1;

  m_clauseSet.setStrengthen(true);
  
  /// get the number of variables and clauses
  while (1) {
    infile >> c;
    if (c == 'c') {
      infile.getline(buffer, 5000);
      lineNumber++;
      continue;
    }
    else if (c == 'p') {
      infile >> buffer;
      infile >> nv;
      infile >> nc;
      lineNumber++;
      break;
    }
    else {
      cout << "\nError: File not in DIMACS format?\n";
      exit(1);
    }
  }

  initialize(nv + 1);
  FastClause a(nv + 1);
  FastClause b(nv + 1);
  bool equality = false;
  bool mod2 = false;
  
  /// start reading in clauses
  while (1)
    {
      if (infile.eof())
	{
	  cout << "\nError: Unexpected end of file.\n";
	  exit(1);
	}
      infile >> c;
      if (c == 'c')
	{
	  infile.getline(buffer, 5000);
	  lineNumber++;
	  continue;
	}
      else infile.putback(c);
      
      a.clear();
      equality = false;
      mod2 = false;
      int required = 1;
      do {
	int varID, weight = 1;
	/// read a variable and weight
	/// or read end of clause
	infile >> c;
	if (c == '>' || c == '0' || c == '=' || c == 'm')
	  {
	    switch (c)
	      {
	      case '0': break;
	      case '>':
		{
		  infile >> buffer;
		  infile >> required;
		  infile >> varID;
		  if (varID != 0)
		    {
		      cout << filename << " : line " << lineNumber << " : "
			   << "end of line character '0' expected" << endl;
		      exit(1);
		    }
		  break;
		}
	      case '=':
		{
		  infile >> required;
		  infile >> varID;
		  if (varID != 0)
		    {
		      cout << filename << " : line " << lineNumber << " : "
			   << "end of line character '0' expected" << endl;
		      exit(1);
		    }
		  equality = true;
		  break;
		}
	      case 'm':
		{
		  infile >> buffer;
		  infile >> required;
		  infile >> varID;
		  if (varID != 0)
		    {
		      cout << filename << " : line " << lineNumber << " : "
			   << "end of line character '0' expected" << endl;
		      exit(1);
		    }
		  mod2 = true;;
		  break;
		}
		
	      }
	    break;
	  }
	else {
	  /// variable
	  infile.putback(c);
	  infile >> varID;
	  infile >> c;
	  if (c == ':') infile >> weight;
	  else infile.putback(c);
	  
	  if (abs(varID) > nv) {
	    cout << filename << " : line " << lineNumber << " : " << "atom value "
		 << varID << " greater than number of atoms " << endl;
	    exit(1);
	  }
	  
	  if (weight == 0) {
	    cout << filename << ":line " << lineNumber << ":"
		 << "improper atom weight value 0" << endl;
	    exit(1);
	  }
	  
	  if (a.addIfAbsent(abs(varID),(varID > 0) ? weight : -weight)) {
	    a.print();
	    cout << filename << " : line " << lineNumber << " : "
		 << "atom identifier " << varID
		 << " appears twice in constraint " << endl;
	    exit(1);
	  }
	}
      } while (1);

      if (mod2)
	{
	  if (required == 1 || required == 0)
	    a.setRequired(required);
	  else
	    {
	      cout << filename << " : line " << lineNumber << " : "
		   << "bad mod2 constraint" << endl;
	      exit(1);
	    }
	  for (size_t i=0; i < a.size(); ++i)
	    if (a.getWeight(i) != 1 || a.getSign(i) != true)
	      {
		cout << filename << " : line " << lineNumber << " : "
		     << "bad mod2 constraint" << endl;
		exit(1);
	      }
	  a.setMod2();
	}
      else
	{  
	  a.setRequired(required); 
	  if (equality)
	    {
	      b.clear();
	      int sum = 0;
	      for (size_t i=0; i < a.size(); i++)
		{
		  int atom = a.getAtom(i);
		  int value = a.getValue(atom);
		  b.addAtom(atom,-value);
		  sum += abs(value);
		}
	      b.setRequired(sum - a.getRequired());
	      b.simplify();
	      m_clauseSet.addClause(b);
	    }
	  a.simplify();
	  int sum = a.sumCoefficients();
	  if (sum < required)
	    {
	      cout << filename << " : line " << lineNumber << " : "
		   << "UNSAT clause" << endl;
	      exit(1);
	    }
	}
      m_clauseSet.addClause(a);
      
      ClauseCount++;
      if (ClauseCount == nc) break;
    }
  m_clauseSet.setInitialClauseCount();
  m_clauseSet.setStrengthen(false);
}


void getToken(string &token, string &line, string delimiters)
{
  string::size_type begIdx = line.find_first_not_of(delimiters);
  string::size_type endIdx = line.find_first_of(delimiters,begIdx);
  token.assign(line,begIdx,endIdx);
  line.erase(0,endIdx);
}


void Solver::parseOPB(const char * filename) {

  ifstream infile(filename);
  
  char buffer[5000];
  char c;
  string variableName;
  int nv,nc,ClauseCount = 0;
  int lineNumber = 1;
  string token, line;
  string delimiters(" \t");

  
  m_clauseSet.setStrengthen(true);
  
  /// get the number of variables and clauses
  while (1)
    {
      infile >> c;
      if (c == '*') {
	getline(infile,line);
	getToken(token,line,delimiters);
	if (token[0] == '#')
	  {
	    getToken(token,line,delimiters);
	    nv = atoi(token.c_str());
	    getToken(token,line,delimiters);
	    getToken(token,line,delimiters);
	    nc = atoi(token.c_str());
	    lineNumber++;
	    break;
	  }
	lineNumber++;
	continue;
      }
      else {
	cout << "\nError: Need to list number of variables and clauses.\n";
	exit(1);
      }
    }

  initialize(nv + 1);
  FastClause a(nv + 1);
  FastClause b(nv + 1);

  /// start reading in clauses
  while (1) {
    if (infile.eof()) {
      cout << "\nError: Unexpected end of file.\n";
      exit(1);
    }
    infile >> c;
    if ((c == '*') || (c == 'm')) {  // skip comments and "min" line... It might work
      infile.getline(buffer,5000);
      lineNumber++;
      continue;
    }
    else infile.putback(c);

    a.clear();
    int required;
    do {
      int varID, weight = 1;
      /// read a variable and weight
      /// or read end of clause
      infile >> c;
      if (c == '>') {
	/// end of clause
	infile >> buffer;
	infile >> required;
	infile.getline(buffer,5000);
	lineNumber++;
	break;
      }
      else {
	if ( c == '+' ) {
	  /// variable
	  infile >> weight;
	  infile >> variableName;
	  string::iterator si = variableName.begin();
	  if (*si == '~') {
	    // should truncate variableName
	    variableName.erase(si);
	    weight = -weight;
	  }
	  varID = m_assignment.lookup(variableName);
	  if (varID > nv) {
	    cout << endl << endl;
	    cout << filename << " : line " << lineNumber << " : " << "atom value "
		 << variableName << " : " << varID << " greater than number of atoms "
		 << nv << endl;
	    exit(1);
	  }
	  
	  if (weight == 0) {
	    cout << filename << ":line " << lineNumber << ":"
		 << "improper atom weight value 0" << endl;
	    exit(1);
	  }
	  if (a.addIfAbsent(varID,weight)) {
	    cout << filename << " : line " << lineNumber << " : "
		 << "atom identifier " << variableName
		 << " appears twice in constraint " << endl;
	    exit(1);
	  }
	}
      }
    } while (1);

    a.setRequired(required);
    a.simplify();
    int sum = a.sumCoefficients();
    if (sum < required)
      {
	cout << filename << " : line " << lineNumber << " : "
	     << "UNSAT clause" << endl;
	exit(1);
      }
    a.print();
    m_clauseSet.addClause(a);
    
    ClauseCount++;
    if (ClauseCount == nc) break;

  }
  m_clauseSet.setInitialClauseCount();
  m_clauseSet.setStrengthen(false);
}



