#ifndef __COMMON__
#define __COMMON__

#include <iostream>
#include <vector>
#include <sstream>
#include "Set.h"
#include "AtomNameMap.h"

using namespace std;

namespace zap
{

typedef int       ClauseID;
typedef size_t    Variable;
typedef size_t Column;
typedef size_t Row;
enum Outcome { UNSAT, SAT, SAMPLE_FINISHED, TIME_OUT, MEMORY_OUT };
enum Result { FAILURE, SUCCESS, CONTRADICTION };
enum ClauseSetType { CNF, PFS, SYMRES, GROUP_BASED, NOT_SPECIFIED };

const double SCORE_INC_FACTOR           = 1 / 0.95;
const double SCORE_LIMIT                = 1e100;
const double SCORE_DIVIDER              = 1e-100;
const double CLAUSE_SCORE_INC_FACTOR    = 1 / 0.999;
const double CLAUSE_SCORE_LIMIT         = 1e20;
const double CLAUSE_SCORE_DIVIDER       = 1e-20;
const double CLAUSESET_SIZE_MULTIPLIER  = 1.5; 




////////////////////////////////////   LITERAL   ////////////////////////////////////////////

class Literal
{
  int                m_lit;
public:
  Literal(Variable v, bool s) { m_lit = (s ? v : -v); }
  Literal() :  m_lit(0) { }
  
  bool    sign()     const { return m_lit > 0; }
  size_t  variable() const { return abs(m_lit); }
  Literal negate()   const { return Literal(abs(m_lit),m_lit < 0); }
  int     int_lit()  const { return m_lit; }

  bool operator!=(const Literal& l) const { return m_lit != l.m_lit; }
  bool operator==(const Literal& l) const { return m_lit == l.m_lit; }
  bool operator< (const Literal& l) const
  {
	if (variable() == l.variable()) return sign() < l.sign();
	return variable() < l.variable();
  }
  friend ostream& operator<<(ostream& os, const Literal l);
};


///////////////////////////////////   CLAUSE   //////////////////////////////////////////

class Clause : public vector<Literal> {
public:
  size_t id;  // clauses should know their location in the main vector
  double score; // need a way of scoring and comparing clauses
  size_t  merge_size;
  string  group_identifier;
 Clause(const vector<Literal>& vl) : vector<Literal>(vl), id(0), score(0), merge_size(0), group_identifier("empty") { }
 Clause() : id(0), score(0), merge_size(0), group_identifier("empty") { }
  bool operator<(const Clause& c) const { return score > c.score; } // we want to sort from big to small
  void print(ostream& os = cout) const { for (size_t i=0; i < size(); i++) os << operator[](i) << ' '; }
  friend ostream& operator<<(ostream& os, const Clause& c) { c.print(os); return os; }
};

typedef ClauseID  Watcher;

class VariableWatch {
public:
  vector<Watcher>  watch_list[2];
};


class GlobalVars
{
public:
  long long unsigned clauses_touched;  // statistics we want all solvers to keep track of
  long long unsigned queue_pops;
  long long unsigned literals_touched;
  long unsigned number_branch_decisions;
  long unsigned number_backtracks;
  long long unsigned prop_from_nogoods;
  double solution_time;
  double time_out;
  double start_time;
  Outcome result;
  
  AtomNameMap atom_name_map;  // variable names to size_t variable ids (produced by parser)
  bool dpll; // run plain dpll and unit propagation testing rather than call the solver's solve method
  bool output_variable_names;
  bool read_branch_from_user;
  
  bool branching_heuristic_on;     // global flags for turning solver techniques on and off
  bool up_queue_pop_heuristic_on;
  bool restarts_on;
  bool forget_clauses_on;
  bool test_local_search_up;
  
  bool use_structure;
  size_t fix_attempts;
  size_t map_attempts;
  size_t structure_clause_limit;
  size_t length_bound;
  size_t up_structure_bound;
  size_t relevance_bound;
  size_t symres_bound;

  ClauseSetType desired_type;

public:
  GlobalVars() : clauses_touched(0), queue_pops(0), literals_touched(0), number_branch_decisions(0),
				 number_backtracks(0), prop_from_nogoods(0),
				 solution_time(0), time_out(0), start_time(0), dpll(false), output_variable_names(false),
				 read_branch_from_user(false),
				 branching_heuristic_on(true), up_queue_pop_heuristic_on(true), restarts_on(true),
       	         forget_clauses_on(true), test_local_search_up(false), use_structure(true), fix_attempts(10),
				 map_attempts(10), structure_clause_limit(4), length_bound(2), up_structure_bound(2000),
                 relevance_bound(5), symres_bound(2), desired_type(NOT_SPECIFIED) { }

};

extern GlobalVars global_vars;



////////////////////////////////////   INLINES   ////////////////////////////////////////////


inline ostream& operator<<(ostream& os, const Literal l)
{
   if (global_vars.output_variable_names) {
      if (l.variable() == 0) os << "lit() ";
      else
         os << (l.sign() ? ' ':'-') << global_vars.atom_name_map.lookup(l.variable()) << ' ';
   }
  else os << l.m_lit << flush;
  return os;
}


inline double get_cpu_time(void) 
{
  struct rusage ru;
  getrusage(RUSAGE_SELF, &ru);
  return ( ru.ru_utime.tv_sec +
	   ru.ru_utime.tv_usec/1000000.0 +
	   ru.ru_stime.tv_sec +
	   ru.ru_stime.tv_usec/1000000.0 );
}


inline void quit(string s)
{
  cout << "Failure: " << s << endl;
  cerr << "Failure: " << s << endl;
  exit(0);
}

inline int logint(int n, int b)
{
  int ans = 0;
  while (n >= b) { 
    n /= b; 
    ++ans; 
  }
  return ans;
}

/* Determine if a clause is a tautology. */
inline bool is_tautology(vector<Literal>& c) 
{
  sort(c.begin(),c.end());
  for (size_t i=0; i < c.size()-1; i++)
    if (c[i].variable() == c[i+1].variable()) return true;
  return false;
}




inline ostream& operator<<(ostream& os, const vector<bool> v) {
  os << "[";
  for (size_t i=0; i < v.size()-1; i++)
	os << v[i];
  os << v[v.size()-1] << "]";
  return os;
}


inline double factorial(unsigned n)
{
  double ans = 1;
  for (unsigned i = 2 ; i <= n ; ++i) ans *= i;
  return ans;
}


/* Convert anything to a string */

template<class T>
string to_string(const T& t)
{
  ostringstream ss;
  ss << t;
  return ss.str();
}

inline Column column_image(Column c,const vector<Column>& before,const vector<Column>& after)
{
  // specialized code
  if (before.size() == 1) {
    if (before[0] == c) return after[0];
    if (after[0] == c) return before[0];
    return c;
  }
  
  for (size_t i=0; i < before.size(); i++) 
    if (before[i] == c) return after[i];
  
  if (after.size() == 1) {
    if (after[0] == c) return before[0];
    return c;
  }
  
  for (size_t i=0; i < after.size(); i++) {
    if (after[i] == c) {
      Column pre = before[i];
      while (true) {
		// find pre-image of pre
		size_t j=0;
		for ( ; j < after.size(); j++) {
		  if (after[j] == pre) break;
		}
		if (j >= after.size()) break;
		pre = before[j];
      }
      return pre;
    }
  }
  
  return c;
}

inline Clause boolean_resolve(const Clause& c1, const Clause& c2)
{
  Clause answer;
  size_t num_opposing_lits = 0;
  Set<Literal> r;
  for (size_t i=0; i < c1.size(); i++) {
	r.insert(c1[i]);
	answer.push_back(c1[i]);
  }
  for (size_t i=0; i < c2.size(); i++) {
	if (!r.contains(c2[i])) {
	  if (r.contains(c2[i].negate())) {	
		vector<Literal>::iterator it = find(answer.begin(),answer.end(),c2[i].negate());
		*it = answer.back();
		answer.pop_back();
		++num_opposing_lits;
	  }
	  else {
		r.insert(c2[i]);
		answer.push_back(c2[i]);
	  }
	}
  }
 
  return answer;
}


} // end namespace zap


#endif
