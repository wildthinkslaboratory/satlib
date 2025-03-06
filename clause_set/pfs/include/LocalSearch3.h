#ifndef _LOCALSEARCH3_H
#define _LOCALSEARCH3_H
#include "SmartPointer.h"
#include "Assignment.h" 
#include "PredicateSym.h"
#include "FOAssigner.h"
#include "FastSet.h"
#include <set>

namespace zap 
{

class LocalSearchLiteral : public ArrayLiteral
{
public:
  Literal        lit;
  size_t         value;
  size_t         free_count;
  vector<bool>   is_free;
  LocalSearchLiteral(const ArrayLiteral al, Literal l, size_t v) :
	ArrayLiteral(al), lit(l), value(v),	free_count(al.index.size()), is_free(al.index.size(),true) { }
  ArrayIndex& index_array() { return ArrayLiteral::index; }
  const ArrayIndex& index_array() const { return ArrayLiteral::index; }
  size_t predicate() const { return ArrayLiteral::predicate; }
  size_t number_free() const { return free_count; }
};

class LocalSearchClause : public vector<LocalSearchLiteral>
{
public:
  size_t unit_lit_ptr;
  size_t count;
  size_t total_vars;
  
  bool operator==(const LocalSearchClause& c) const
  {
	if (operator[](unit_lit_ptr).lit == c[c.unit_lit_ptr].lit) return true;
	return false;
  }
  bool operator<(const LocalSearchClause& c) const
  {
	return operator[](unit_lit_ptr).lit < c[c.unit_lit_ptr].lit;
  }
  bool assignment_full() const { return count >= total_vars; }
};

class LitPtr
{
public:
  size_t  lit;
  size_t  index;
  
  LitPtr(size_t l, size_t i) : lit(l), index(i) { }
};

class VarPtr
{
public:
  size_t action;
  size_t var;
  VarPtr(size_t a, size_t v) : action(a), var(v) { }
};



class LocalSearchVariable
{
public:
  size_t             fo_value;
  vector<LitPtr>     lit_ptr;
  FastSet            domain;

  LocalSearchVariable() : fo_value(0) { }
};


class LocalSearchAction3
{
public:
  vector<LocalSearchVariable>      fo_variable;
  size_t                           domain_size;
  string                           domain_name;
  FOAssigner                       assigner;
  bool                             turned_off;
  LocalSearchAction3() : domain_size(0), assigner(0,0), turned_off(false) { }
  LocalSearchAction3(const LocalSearchAction3& lsa);
  LocalSearchAction3& operator=(const LocalSearchAction3& lsa);
  bool assign(size_t var, size_t val);
};



class LocalSearch3
{
  PredicateSym*              symmetry;
  LocalSearchClause           clause_rep;
  vector<LocalSearchAction3>   actions;
  size_t                       unit_lit_ptr;  // clause index of unit lit
  vector<vector<size_t> >      lit_action_map;
  vector<vector<size_t> >      lit_variable_map;
  vector<size_t>               lit_prop_list;
  vector<VarPtr>               domain_prop_list;

//   vector<LocalSearchClause>   open;
//   vector<LocalSearchClause>   closed;

  vector<LocalSearchClause> get_neighbors(Assignment& P, LocalSearchClause c,ClauseID id);
  void reduce_domain(Assignment& P, LocalSearchVariable& v, LocalSearchClause& c, size_t domain_size);
  void reset_fo_variables();
  bool find_unit_lit(Assignment& P, ClauseID id);
  bool try_to_value_variables(Assignment& P,LocalSearchClause& c);
  bool get_cluster(Assignment& P, ClauseID id, const LocalSearchClause& c);
  vector<LocalSearchLiteral> get_candidates(const Assignment& P);
  void set_up_data_structures(LocalSearchClause& c);
  bool set_value(const VarPtr& vp, size_t value, LocalSearchClause& c, const Assignment& P);
  bool reduce_domains(size_t lp, const Assignment& P, LocalSearchClause& c);
  void branch(LocalSearchClause& c);

public:
  LocalSearch3() { }
  LocalSearch3(const LocalSearch3& ls);
  void initialize(const Clause& c, PredicateSym* psp);
  bool get_symmetric_unit_lits(Assignment& P, ClauseID id);
  bool get_cluster(Assignment& P, ClauseID id) { return get_cluster(P,id,clause_rep); }
};

inline LocalSearchAction3::LocalSearchAction3(const LocalSearchAction3& lsa) :
  fo_variable(lsa.fo_variable), domain_size(lsa.domain_size),
  assigner(FOAssigner(lsa.assigner.num_rows(),lsa.assigner.num_cols())), turned_off(lsa.turned_off)
{

}

inline LocalSearchAction3& LocalSearchAction3::operator=(const LocalSearchAction3& lsa)
{
  fo_variable = lsa.fo_variable;
  domain_size = lsa.domain_size;
  assigner = FOAssigner(lsa.assigner.num_rows(),lsa.assigner.num_cols());
  return *this;
}


inline LocalSearch3::LocalSearch3(const LocalSearch3& ls) :
  symmetry(ls.symmetry), clause_rep(ls.clause_rep), actions(ls.actions), unit_lit_ptr(ls.unit_lit_ptr),
  lit_action_map(ls.lit_action_map), lit_variable_map(ls.lit_variable_map), lit_prop_list(ls.lit_prop_list),
  domain_prop_list(ls.domain_prop_list)
{

}


} // end namespace zap 


#endif
