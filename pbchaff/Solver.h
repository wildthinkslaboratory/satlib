/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#ifndef _SOLVER_H
#define _SOLVER_H

#include <vector>
#include "PartialAssignment.h"
#include "ImplicationList.h"
#include "ClauseSet.h"
#include "TimeTracker.h"
#include "Cnf.h"

enum SolutionStatus {
    UNDETERMINED,
    UNSATISFIABLE,
    SATISFIABLE,
    TIME_OUT,
    MEMORY_OUT,
    ABORTED
};

// simple struct to hold statistics
class SolverStatistics
{
public:
  long nodeCount;
  double runTime;
  double preprocessTime;
  long backtrackCount;
  int outcome;

  SolverStatistics() :
    nodeCount(0),
    runTime(0.0),
    preprocessTime(0.0),
    backtrackCount(0),
    outcome(UNDETERMINED) {}
};

// simple struct to hold settings.
class SolverSettings
{
public:
  bool verbosity;
  std::size_t branchHeuristic;
  std::size_t learnMethod;
  std::size_t strengthenBound;
  double timeLimit;
  int deletionInterval;
  int vsidsUpdateInterval;
  int randomness;
  int baseRandomness;
  bool allowRestarts;
  int nextRestartTime;
  int restartTimeInc;
  int restartTimeIncInc;
  int nextRestartBacktrack;
  int restartBacktrackInc;
  int restartBacktrackIncInc;
  int restartRandomness;
  bool failedLiteral;
  bool andrewOpt;

  SolverSettings() :
    verbosity(false),
    branchHeuristic(0),
    learnMethod(0),
    strengthenBound(1),
    timeLimit(24 * 3600),
    deletionInterval(5000),
    vsidsUpdateInterval(256),
    randomness(0),
    baseRandomness(0),
    allowRestarts(true),
    nextRestartTime(50),
    restartTimeInc(0),
    restartTimeIncInc(0),
    nextRestartBacktrack(0),
    restartBacktrackInc(40000),
    restartBacktrackIncInc(100),
    restartRandomness(0),
    failedLiteral(false),
    andrewOpt(true) {}
};


/*************************************************************************/
/*
  CLASS: Solver

  PURPOSE: Main Solver class.


**************************************************************************/
class Solver {

protected:

  ClauseSet m_clauseSet;
  ImplicationList m_unitList;
  PartialAssignment m_assignment;
  SolverStatistics m_statistics;
  SolverSettings m_settings;

  // for conflict analysis
  FastClause m_parent1;
  FastClause m_parent2;
  int m_conflictAtom;
  int m_atom1;
  int m_atom2;
  int m_level1;
  int m_level2;

  // for strengthening
  FastClause m_strengthen1;
  FastClause m_strengthen2;
  AtomValueMap m_assumptions;
  
  // for vsids branching heuristic
  std::vector<double> m_vsidsScores[2];
  std::vector<std::pair<int,double> > m_sortedScores;

  TimeTracker m_timer;

  int realSolve();
  bool unitPropagate();
  bool backtrack();
  void backjumpToLevel(int);
  void backjumpToAtom(int);
  
  // conflict analysis
  bool learn();
  void collectUnitProps(ClauseID);
  void collectUnitPropsMod2(ClauseID);
  bool learnCardinality();
  bool learnPBConstraint();
  bool learnMod2Constraint();
  void mod2ToDisjunction(FastClause&);

  // strengthening
  void strengthenConstraintSet();
  void removeSubsumedClauses(std::vector<int>&,int);

  // preprocessing
  void preprocessStrengthen();
  bool initialClauseCheck();
  bool failedLiteralTest();
  
  // branch selection
  void selectBranchVariable();
  Literal getVsidsLiteral();
  Literal getFirstUnvalued();
  Literal getRandomUnvalued();
  void updateVsidsCounts();

  // miscellaneous
  void initialize(std::size_t);
  void runPeriodicFunctions();
  void restart();
  int ISAMPwithLearning();

public:

  void parse(const char *filename);
  void parseOPB(const char *filename);
  void load_to_structures(const zap::Cnf& clauses);
  
  void preprocess();
  void solve();
  
  void setVerbosity(bool b) { m_settings.verbosity = b; }
  void setTimeLimit(int i) { m_settings.timeLimit = (double)(i); }
  void setBranchHeuristic(int i) { m_settings.branchHeuristic = i; }
  void setLearnMethod(int i) { m_settings.learnMethod = i; }
  void setFailedLiteral(bool b) { m_settings.failedLiteral = b; }
  void setAndrewOpt(bool b) { m_settings.andrewOpt = b; }

  void printAssignment() const { m_assignment.printListForm(); }
  const SolverStatistics& getSolverStatistics() const { return m_statistics; }
  const ClauseSetStatistics& getClauseSetStatistics() { return m_clauseSet.getStatistics();}
};



#endif
