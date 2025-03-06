#ifndef _LOCALSEARCH_H
#define _LOCALSEARCH_H
#include "Transport.h"
#include "LocalSearchCounter.h"

namespace zap 
{

enum Value { UNSATISFIED, SATISFIED, UNVALUED };
class Score 
{
  size_t  data[3];
  public:
  Score() { clear(); }
  size_t& operator[](size_t i) { return data[i]; }
  size_t operator[](size_t i) const { return data[i]; }
  bool is_unit() const;
  bool is_unsat() const;
  void clear()  { for (size_t i=0; i < 3; i++) data[i] = 0; }
  bool operator<(const Score& s) const;
  friend ostream& operator<<(ostream& os, const Score& s)
  {
	os << s.data[UNSATISFIED] << " "
	   << s.data[SATISFIED] << " "
	   << s.data[UNVALUED] << ' ';
	return os;
  }
};


class Choice : public vector<size_t>
{
public:
  vector<bool> fixed;
  Choice(const vector<size_t>& v) : vector<size_t>(v), fixed(vector<bool>(v.size(),false)) { }
  friend ostream& operator<<(ostream& os, const Choice& c)
  {
	os << "[";
	for (size_t i=0; i < c.size(); i++)
	  os << c.operator[](i) << (c.fixed[i] ? 'f' : ' ') << ' ';
	os << "]";
	return os;
  }
};


class Solution {
  public:
  Clause          clause;
  vector<Choice>  choices;
  Score           score;
  friend ostream& operator<<(ostream& os, const Solution& s)
  { os << s.clause << "\t";
	for (size_t i=0; i < s.choices.size(); i++) os << s.choices[i] << " ";
	os << "\t" << s.score;	return os;}
};

class FixedLitReps
{
public:
  vector<Solution>  reps[2];
  const vector<Solution>& get_reps(bool b) const { return (b ? reps[1] : reps[0]); }
  vector<Solution>& get_reps(bool b) { return (b ? reps[1] : reps[0]); }
};


class LocalSearchParameters
{
  public:
  size_t number_iterations;
  size_t number_candidates;
  LocalSearchParameters() : number_iterations(20), number_candidates(0) { }
};

class LocalSearchStatistics
{
  public:
  size_t clauses_touched;
  size_t unit_clauses_found;
  LocalSearchStatistics() : clauses_touched(0), unit_clauses_found(0) { }
  void clear() { clauses_touched = 0; unit_clauses_found = 0; }
};


// A local search should know the id of it's pfs clause
class LocalSearch {
  static long unsigned    touches;
  PfsTransportVector*     transports;
  vector<vector<size_t> >       lits_to_choices;
  vector<vector<size_t> >       choices_to_lits;
  vector<FixedLitReps>          fixed_lit_index;
  Solution                base_solution;
  Solution                best_solution;
  LocalSearchParameters   parameters;

  size_t expand_node(Assignment& P, ClauseID id, size_t depth, Solution candidate);
  size_t expand_node(Assignment& P, ClauseID id, size_t depth, Solution candidate, vector<size_t>& choices);
  bool k_transporter_complete(Assignment& P, Literal l, ClauseID id);
  bool k_transporter_incomplete(Assignment& P, Literal l, ClauseID id);
  bool symmetric_lits_inner_call(Assignment& P, ClauseID id);
  bool symmetric_lits_inner_call2(Assignment& P, ClauseID id);
  public:
  LocalSearchStatistics   stats;

  LocalSearch() : transports(NULL) { }
  Literal score(Solution& s,const Assignment& P);
  Solution map_solution(Literal from, Literal to, const Solution& base);
  Literal map_literal(Literal l, vector<Choice>& choices);
  void map_clause(Solution& s);
  bool k_transporter(Assignment& P, Literal l, ClauseID id);
  bool get_symmetric_unit_lits(Assignment& P,ClauseID id);
  void initialize(PfsTransportVector* t);
  long unsigned get_touches() const { return touches; }
 
};

/* // need a general column image function */





//////////////////////////////////////  INLINES //////////////////////////////////////////////////

inline bool Score::operator<(const Score& s) const
{
  if ((data[SATISFIED] + data[UNVALUED]) == (s[SATISFIED] + s[UNVALUED]))
	return data[SATISFIED] < s[SATISFIED];
  return (data[SATISFIED] + data[UNVALUED]) < (s[SATISFIED] + s[UNVALUED]);
}

inline bool Score::is_unit() const
{
  if (data[SATISFIED] == 0 && data[UNVALUED] == 1) return true;
  return false;
}

inline bool Score::is_unsat() const
{
  if (data[SATISFIED] == 0 && data[UNVALUED] == 0) return true;
  return false;
}

inline Literal LocalSearch::score(Solution& s,const Assignment& P)
{
  s.score.clear();
  Literal deepest_lit = s.clause[0];
  Literal unit_lit;
  for (size_t i=0; i < s.clause.size(); i++) {
	//	cout << s.clause[i] << ":" << P.value(s.clause[i].variable()) << " ";
	if (P.position(deepest_lit.variable()) < P.position(s.clause[i].variable())) deepest_lit = s.clause[i];
	if (P.value(s.clause[i].variable()) == UNKNOWN) {
	  s.score[UNVALUED]++;
	  unit_lit = s.clause[i];
	}
	else
	  if (P.value(s.clause[i].variable()) == s.clause[i].sign()) s.score[SATISFIED]++;
	  else s.score[UNSATISFIED]++;
  }
//   cout << endl;
//   cout << "unit lit " << unit_lit << endl;

  if (s.score.is_unsat()) return deepest_lit;
  return unit_lit;
}

} // end namespace zap 

#endif
