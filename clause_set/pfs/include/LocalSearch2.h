#ifndef _LOCALSEARCH_2_H
#define _LOCALSEARCH_2_H
#include "Counter.h"
#include "Transport.h"
#include "ProductSubgroup.h"
using namespace std;

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

class LocalSearchStatistics
{
  public:
  static long unsigned touches;
  size_t unit_clauses_found;
  LocalSearchStatistics() : unit_clauses_found(0) { }
  void clear() { unit_clauses_found = 0; }
};

class LocalSearchAction : public PermCounterWithFixing
{
  size_t                   index;
  vector<Column>           columns;
  vector<Column>           original_columns;
  vector<Column>           current_columns;
  vector<size_t>           column_value_map;
  vector<vector<size_t> >  lit_position_map;

  bool can_assign_value(size_t index, Column value);

  
public:
  LocalSearchAction(size_t nd, size_t i, const vector<Column>& c,const vector<vector<Column> >& lcm);
  LocalSearchAction() : PermCounterWithFixing(0,0), index(0) { }
  Column operator[](size_t i) const;
  bool operator++();
  void fix_literal(size_t i);
  void unfix_literal(size_t i);
  const vector<Column>& from() const;
  const vector<Column>& to();
  void apply_mapping(size_t index, const vector<Column>& mapping);
  bool can_be_mapped(size_t index, const vector<Column>& mapping);
  bool can_be_mapped2(size_t index, const vector<Column>& mapping);
  void reset();
  //  bool apply_mapping(const vector<Column>& before, const vector<Column>& after);
};

inline void LocalSearchAction::reset()
{
  PermCounterWithFixing::reset(); 
  current_columns = original_columns;
}




class LocalSearchPosition
{
public:
  vector<LocalSearchAction>  actions;
  Clause                     clause;
  Clause                     image;
  vector<vector<size_t> >    lit_action_map;
  Score                      score;
  vector<bool>               fixed_lits;
  
  void fix_literal(size_t i);
  void unfix_literal(size_t i);
  bool is_fixed(size_t i) { return fixed_lits[i]; }
  void reset();
  //  void map(Literal l1, Literal l2) { return; }
  
};


// fill in the basic functionality step by step
// start with initialization
class LocalSearch2
{
  static long unsigned      touches;
  PfsTransportVector*       transports;
  Ptr<ProductSubgroup>      group;
  LocalSearchPosition       s;
  LocalSearchStatistics     stats;
  //  vector<vector<Literal> >  lit_orbit_map;

  Literal map(size_t index);
  void map_clause();
  Literal score_clause(const Assignment& P);
  void apply_mapping(size_t index, Literal l);
  bool try_to_map(size_t index, Literal l);
  bool try_to_map_negated_lits(size_t index, vector<vector<Literal> > lit_orbit_map, Assignment& P);
public:
  void initialize(PfsTransportVector* t, const Ptr<ProductSubgroup>& pps); // done
  void update_transport_pointer(PfsTransportVector* t) { transports = t; }
  bool get_symmetric_unit_lits(Assignment& P, ClauseID id); // in progress
  bool k_transporter(Assignment& P, Literal l, ClauseID id) { return true; }

  long unsigned get_touches() const { return stats.touches; }
};



/*
  Neighborhood needs the unit lit index, the literal
  for each associated action: column_values for that index and the total columns for the action
 */
class Neighborhood
{
  PfsTransportVector*    transports;
  LocalSearchPosition*   search_position;
  int index;
  Literal literal;
  size_t  clause_index;
  vector<Column> original_vals;
  vector<Column> available_vals;
  size_t         current_val_ptr;
  size_t         num_actions;
  
public:
  Neighborhood(LocalSearchPosition* sp, PfsTransportVector* tv, Literal l, size_t ci);
    bool operator++();
};

  ////////////////////////////////////////   INLINES   ////////////////////////////////////////
/*

*/



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

inline LocalSearchAction::LocalSearchAction(size_t nd, size_t i, const vector<Column>& c,
										   const vector<vector<Column> >& lcm)
  : PermCounterWithFixing(nd,c.size()), index(i), columns(c), lit_position_map(lcm.size(),vector<size_t>())
{
  size_t max = 0;
  if (columns.size()) max = *max_element(columns.begin(),columns.end());
  column_value_map = vector<size_t>(max+1,0);
  for (size_t i=0; i < columns.size(); i++)
	column_value_map[columns[i]] = i;
  
  for (size_t i=0; i < size(); i++) {
	original_columns.push_back(columns[i]);
	current_columns.push_back(columns[i]);
  }

  for (size_t i=0; i < lcm.size(); i++) {
	for (size_t j=0; j < lcm[i].size(); j++) {
	  lit_position_map[i].push_back(column_value_map[lcm[i][j]]);
	}
  }
}


inline Column LocalSearchAction::operator[](size_t i) const { return columns[PermCounterWithFixing::operator[](i)]; }

inline bool LocalSearchAction::operator++()
{
  bool b = PermCounterWithFixing::operator++();
  if (b)
	for (size_t i=0; i < size(); i++) current_columns[i] = operator[](i);
  return b;
}

inline void LocalSearchAction::fix_literal(size_t i)
{
  for (size_t j=0; j < lit_position_map[i].size(); j++) PermCounterWithFixing::fix(lit_position_map[i][j]);
}

inline void LocalSearchAction::unfix_literal(size_t i)
{
  for (size_t j=0; j < lit_position_map[i].size(); j++) PermCounterWithFixing::unfix(lit_position_map[i][j]);
}

inline const vector<Column>& LocalSearchAction::from() const {

  return original_columns; }


inline const vector<Column>& LocalSearchAction::to() {

  return current_columns; }


inline void LocalSearchPosition::fix_literal(size_t i)
{
  vector<size_t>& a = lit_action_map[i];
  for (size_t j=0; j < a.size(); j++) actions[a[j]].fix_literal(i);
  fixed_lits[i] = true;
}

  
inline void LocalSearchPosition::unfix_literal(size_t i)
{
  vector<size_t>& a = lit_action_map[i];
  for (size_t j=0; j < a.size(); j++) actions[a[j]].unfix_literal(i);
  fixed_lits[i] = false;
}


inline void LocalSearchPosition::reset()
{
  image = clause;
  for (size_t i=0; i < actions.size(); i++) actions[i].reset();
  for (size_t i=0; i < fixed_lits.size(); i++) fixed_lits[i] = false;
}


inline Neighborhood::Neighborhood(LocalSearchPosition* sp, PfsTransportVector* tv, Literal l, size_t ci) :
  transports(tv), search_position(sp), index(0), literal(l), clause_index(ci), current_val_ptr(0)
{
  num_actions = search_position->lit_action_map[clause_index].size();
  if (num_actions == 0) return;
  size_t i = search_position->lit_action_map[clause_index][index];
  original_vals = (*transports)[i].action().get_columns(literal);
  if (original_vals.size() > 1) quit("failure in Neighborhood constructor");
  available_vals = transports->operator[](i).columns();

}

// this is a crazy crazy function.  I promise to rewrite it soon.
inline bool Neighborhood::operator++()
{ 
  
  if (index >= num_actions) return false;
  size_t i = search_position->lit_action_map[clause_index][index];


  do {
	while (current_val_ptr < available_vals.size()) {
	  vector<Column> c(1,available_vals[current_val_ptr]);
	  if ((available_vals[current_val_ptr] != original_vals[0]) &&
		  ((*search_position).actions[i].can_be_mapped2(clause_index,c))) {
		(*search_position).actions[i].apply_mapping(clause_index,c);
		++current_val_ptr;
		return true;
	  }
	  ++current_val_ptr;
	}
	
	// first reset the current action to it's original values
	(*search_position).actions[i].apply_mapping(clause_index,original_vals);
	// now move on to the next action
	++index;
	if (index >= num_actions) return false;
	// reset all our stuff to the next action/transport
	i = search_position->lit_action_map[clause_index][index];
	original_vals = (*transports)[i].action().get_columns(literal);
	if (original_vals.size() > 1) quit("failure in Neighborhood constructor");
	available_vals = transports->operator[](i).columns();
	current_val_ptr = 0;
  } while (true);
  

  return false;
}



} // end namespace zap 


#endif
