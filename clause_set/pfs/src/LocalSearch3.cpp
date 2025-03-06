#include "LocalSearch3.h"
#include "Assignment.h"

namespace zap 
{

void LocalSearch3::initialize(const Clause& c, PredicateSym* psp)
{
  symmetry = psp;
  for (size_t i=0; i < c.size(); i++) {
	clause_rep.push_back(LocalSearchLiteral(symmetry->lookup(c[i].variable()),c[i],UNKNOWN));
  }

  actions = vector<LocalSearchAction3>(symmetry->get_group().size());
  for (size_t i=0; i < actions.size(); i++) actions[i].domain_size = symmetry->get_group()[i].domain_size;

  size_t num_args = 0;
  for (size_t i=0; i < clause_rep.size(); i++) {
	const LocalSearchLiteral& l = clause_rep[i];
	vector<size_t> a;
	vector<size_t> v;
	Set<size_t> actions_in_lit;
	for (size_t j=0; j < l.index_array().size(); j++) {
	  num_args++;
	  size_t action_id = symmetry->get_group().get_action(ArgPtr(l.predicate(),j));
	  if (actions_in_lit.contains(action_id)) actions[action_id].turned_off = true;
	  else actions_in_lit.insert(action_id);
	  a.push_back(action_id);
	  size_t fo_value = l.index_array()[j];
	  
	  size_t k = 0;
	  for ( ; k < actions[action_id].fo_variable.size(); k++)
		if (actions[action_id].fo_variable[k].fo_value == fo_value) break;
	  if (k >= actions[action_id].fo_variable.size()) actions[action_id].fo_variable.push_back(LocalSearchVariable());
	  
	  v.push_back(k);
	  
	  actions[action_id].fo_variable[k].fo_value = fo_value;
	  actions[action_id].fo_variable[k].lit_ptr.push_back(LitPtr(i,j));
	  
	}
	lit_action_map.push_back(a);
	lit_variable_map.push_back(v);
	
	
	// 	  if (actions[action_id].fo_variable[i].domain.empty())
	// 		for (size_t k=0; k < actions[action_id].domain_size; k++)
	// 		  actions[action_id].fo_variable[i].domain.push_back(k);
  }
  clause_rep.count = 0;
  clause_rep.total_vars = num_args;
  
  
  // now initialize the assignment class for each action
  for (size_t i=0; i < actions.size(); i++) {
	actions[i].assigner = FOAssigner(actions[i].fo_variable.size(), actions[i].domain_size);
  }
  
}


// use the representative clause to reset the first order variable data structures
void LocalSearch3::reset_fo_variables()
{
  for (size_t i=0; i < actions.size(); i++) {
	for (size_t j=0; j < actions[i].fo_variable.size(); j++) {
	  LocalSearchVariable& v = actions[i].fo_variable[j];
	  for (size_t k=0; k < v.lit_ptr.size() && k < 1; k++) {
		size_t value = clause_rep[v.lit_ptr[k].lit].index_array()[v.lit_ptr[k].index];
		v.fo_value = value;
	  }
	}
  }
}



bool LocalSearch3::get_symmetric_unit_lits(Assignment& P, ClauseID id) {
  bool success = get_cluster(P,id);
  //  return success;
  
  vector<LocalSearchLiteral> candidates = get_candidates(P);
//   cout << "candidates " << candidates.size() << endl;
//   for (size_t i=0; i < candidates.size(); i++) {
// 	cout << candidates[i].lit << "\t";
//   }
//   cout << endl;


// 	for (size_t i=0; i < 4; i++) {
// 	  const MultiArray<size_t>& A = symmetry->get_predicate_array(i);
// 	  size_t unvalued = 0;
// 	  size_t sat = 0;
// 	  size_t unsat = 0;
// 	  for (size_t j=0; j < A.size(); j++) {
// 		if (P.value(A[j]) == UNKNOWN) ++unvalued;
// 		else if (P.value(A[j])) ++sat;
// 		else ++unsat;
// 	  }
// 	  cout << i << '\t' << unvalued << '\t' << sat << '\t' << unsat << endl;
// 	}
 

  
  
  while (!candidates.empty()) {
	size_t random_lit = rand() % candidates.size();
	LocalSearchLiteral l = candidates[random_lit];
	candidates[random_lit] = candidates.back();
	candidates.pop_back();
//  	cout << "trying to map to candidate " << l.lit << endl;
	if (P.is_on_unit_list(l.lit)) continue;
	
	LocalSearchClause c = clause_rep;
// 	cout << "total vars " << c.total_vars << "  count " << c.count << endl;
   	set_up_data_structures(c);
	lit_prop_list.clear();
	domain_prop_list.clear();
	for (size_t i=0; i < l.index_array().size(); i++) {
// 	  cout << "setting index " << i << " to " << l.index_array()[i] << "  in  " << l.lit << endl;
	  set_value(VarPtr(lit_action_map[unit_lit_ptr][i],lit_variable_map[unit_lit_ptr][i]),l.index_array()[i],c,P);
	}
	if (try_to_value_variables(P,c))  {
	  size_t poss = 0;
	  for (size_t j=0; j < c.size(); j++) {  // map the clause
		Variable v = symmetry->lookup(c[j]);
		bool s = c[j].lit.sign();
		c[j].lit = Literal(v,s);
		if (P.value(v) == UNKNOWN || P.value(v) == s) ++poss;
	  }
	  if (poss > 1) {
// 		for (size_t j=0; j < c.size(); j++) cout << c[j].lit << ' ';
// 		cout << endl;
		quit("bad unit clause in get_symmetric_lits");
	  }
	  success =  get_cluster(P,id,c);
	  if (!success) return success;
	}
  }
  
  
  
  return success;
}


bool LocalSearch3::try_to_value_variables(Assignment& P, LocalSearchClause& c)
{
  while (!c.assignment_full()) {
	if (!domain_prop_list.empty()) {
// 	  cout << "domain pop " << endl;
	  VarPtr vp = domain_prop_list.back();
	  domain_prop_list.pop_back();
	  size_t value = actions[vp.action].fo_variable[vp.var].domain[0];
 	  if (!set_value(vp,value,c,P)) return false;
 	}
	else if (!lit_prop_list.empty()) {
	  size_t lp = lit_prop_list.back();
	  lit_prop_list.pop_back();
	  if (!reduce_domains(lp,P,c)) return false;
	}
 	else {
	  branch(c);
	}
  }
  return true;
}

void LocalSearch3::branch(LocalSearchClause& c)
{
  for (size_t i=0; i < c.size(); i++) {
	if (c[i].number_free()) {
	  size_t j = 0;
	  for ( ; j < c[i].is_free.size(); j++) if (c[i].is_free[j]) break;
	  VarPtr vp(lit_action_map[i][j],lit_variable_map[i][j]);
	  size_t value = actions[vp.action].fo_variable[vp.var].domain[0];
// 	  cout << "branching on " << j << "th " << " var in lit " << c[i].lit << "  setting value " << value+1 << endl;
	  actions[vp.action].fo_variable[vp.var].domain = FastSet(actions[vp.action].domain_size);
	  actions[vp.action].fo_variable[vp.var].domain.insert(value);
	  domain_prop_list.push_back(vp);
	  break;
	}
  }

}

bool LocalSearch3::reduce_domains(size_t lp, const Assignment& P, LocalSearchClause& c)
{
//   cout << "reducing domain for lit " << c[lp].lit << endl;
  LocalSearchLiteral l = c[lp];
  size_t i = 0;
  for ( ; i < l.is_free.size(); i++) if (l.is_free[i]) break;
  if (i >= l.is_free.size()) {
// 	cout << "no free variable in " << l.lit << endl;
	// make sure it's set OK
	Variable s = symmetry->lookup(l);
	if (P.value(s) == UNKNOWN || P.value(s) == l.lit.sign()) return false;
	else return true;
  }
  VarPtr vp(lit_action_map[lp][i],lit_variable_map[lp][i]);
  LocalSearchVariable& v = actions[vp.action].fo_variable[vp.var];

  for (size_t j=0; j < actions[vp.action].domain_size; j++) {   // we should iterate through available domain values instead
	l.index_array()[i] = j;
	Variable var = symmetry->lookup(l);
	if (P.value(var) == UNKNOWN || (P.value(var) == l.lit.sign())) {
// 	  cout << "value " << j << " is no good because of " << Literal(var,l.lit.sign()) << endl;
	  if (v.domain.contains(j)) v.domain.remove(j);
	}
	if (v.domain.size() == 0) break;
  }
  if (v.domain.size() == 0) {//  cout << "fail : empty domain" << endl;
	return false; }
  if (v.domain.size() == 1) {
// 	cout << "down to one domain value, adding to domain prop list " << endl;
	domain_prop_list.push_back(vp);
  }
  
  return true;
}

bool LocalSearch3::set_value(const VarPtr& vp, size_t value, LocalSearchClause& c, const Assignment& P)
{
//   cout << "attempt to set " << vp.action << ":" << vp.var << " to " << value << endl;
  LocalSearchVariable& v = actions[vp.action].fo_variable[vp.var];
//   set<size_t>::iterator it = v.domain.find(value);
  if (!v.domain.contains(value)) return false;
  v.fo_value = value;
  v.domain = FastSet(actions[vp.action].domain_size);
  v.domain.insert(value);
  for (size_t i=0; i < actions[vp.action].fo_variable.size(); i++) {
	if (i == vp.var) continue;
	

	if (actions[vp.action].fo_variable[i].domain.contains(value)) {
	  actions[vp.action].fo_variable[i].domain.remove(value);
	  if (actions[vp.action].fo_variable[i].domain.size() == 1) domain_prop_list.push_back(VarPtr(vp.action,i));
	  if (actions[vp.action].fo_variable[i].domain.size() == 0) return false;
	}
  }
  for (size_t i=0; i < v.lit_ptr.size(); i++) {
	LocalSearchLiteral& l = c[v.lit_ptr[i].lit];
	if (l.free_count == 0) quit("assigning first order value to predicate with no free variables");
	
	l.index_array()[v.lit_ptr[i].index] = value;
	l.is_free[v.lit_ptr[i].index] = false;
	l.free_count--;
	c.count++;
	if (l.free_count == 1 && v.lit_ptr[i].lit != unit_lit_ptr) {
// 	  cout << "lit " << c[v.lit_ptr[i].lit].lit << " added to lit prop list " << endl;
	  lit_prop_list.push_back(v.lit_ptr[i].lit);
	}
	if (l.free_count == 0 && v.lit_ptr[i].lit != unit_lit_ptr) {
	  // check to make sure it's unsat otherwise return false
	  Variable s = symmetry->lookup(l);
	  if (P.value(s) == UNKNOWN || P.value(s) == l.lit.sign()) return false;
	}
  }
  return true;
}




void LocalSearch3::set_up_data_structures(LocalSearchClause& c)
{
  c.count = 0;
  for (size_t i=0; i < c.size(); i++) {
	c[i].free_count = c[i].index_array().size();
	c[i].is_free = vector<bool>(c[i].index_array().size(),true);
  }

  for (size_t i=0; i < actions.size(); i++) {
	for (size_t j=0; j < actions[i].fo_variable.size(); j++) {
	  // actions[i].fo_variable[j].domain.clear();
	  for (size_t k=0; k < actions[i].domain_size; k++) 
		actions[i].fo_variable[j].domain.insert(k);
	}
  }
}



/**

   an unset copy of the representative clause

   
   have a Literal l and a pointer to a LocalSearchLiteral where I have the ArrayLiteral that hooks into the PredicateSym
   first get the ArrayLiteral associated with my literal l.  I can probably produce these with the candidate list so lets assume the candidate list is
   a list of LocalSearchLiterals.

   How do I get from the ArrayLiteral to the first order variables in the actions?  I can use the cached unit_lit_actions and unit_lit_variables


   Then I can write set_value(action, variable, value)
       Get the variable and action.  It's domain goes to one value (value), set the fo_value
	   walk through the lit ptrs and update lits
	      decrease the count
		  set the value
		  if the count is 1 add to the prop list

  I can write bool reduce_domains(LocalSearchLiteral, P )
      Walk through the orbit allowing the remaining free variable to move and eliminate any domain values that have a bad assignment in P.  We'll need
	  to know the action and variable for this free variable.
	  If the domain size is empty return failure
	  If the domain size is 1 add to the values to set list

  Need to know if fo assignment is full.  the Clause can keep a count?

  Need a way to branch
      Walk down the clause and find a free literal and set a value in it from it's domain


  Things to write:
 x	  Write initialization of clause lit to action/variable maps.  Use it instead of old one for unit_lit
 x	  Write the initialization of unit_lit as a seperate function and call it at appropriate time
 x	  Write high-level code for try_to_value_variables
 x	  Store repclause and unit clause seperately in class and make sure they get used appropriately
 x	  Get candidates (exclude units found in get_cluster)
 x	  Write set_up_datastructures
 x		  or add a vector bool to say if a fo_value is set
 x		  initialize the counts of total vars and count
 x		  write assignment full and number_free_vars functions
 x		  zero counts, fasle vector<bools>, set domains to full
		  
	  Write lower level functions from try_to_value_variables
	  Write a seperate function to map a clause once the first order values are set
	  
   
 **/


// void LocalSearch3::fix_literal() {

// }



bool LocalSearch3::find_unit_lit(Assignment& P, ClauseID id)
{
    //  cout << "\t\t\t\t   get symmetric unit lits " << endl;
  // first find the unit lit and set up values
  int unit_lit_index = -1;
  size_t deepest = 0;
  for (size_t i=0; i < clause_rep.size(); i++) {
	clause_rep[i].value = P.value(clause_rep[i].lit.variable());
	if (clause_rep[i].value == UNKNOWN) unit_lit_index = i;
	if (P.position(clause_rep[i].lit.variable()) > P.position(clause_rep[deepest].lit.variable())) deepest = i;
  }
  if (unit_lit_index == -1) {
	unit_lit_index = deepest;
  }
  unit_lit_ptr = unit_lit_index;
  Clause image;  // make this chunk a subroutine
  for (size_t j=0; j < clause_rep.size(); j++) image.push_back(clause_rep[j].lit);
  bool extend_succeeded = P.extend(AnnotatedLiteral(clause_rep[unit_lit_ptr].lit,Reason(id,image)));
  if (!extend_succeeded) return false;
 
  
  clause_rep.unit_lit_ptr = unit_lit_ptr;
  return true;

}



vector<LocalSearchLiteral> LocalSearch3::get_candidates(const Assignment& P)
{
  vector<LocalSearchLiteral> candidates;
  size_t predicate_id = clause_rep[unit_lit_ptr].predicate();
  bool sign = clause_rep[unit_lit_ptr].lit.sign();
  const MultiArray<size_t>& A = symmetry->get_predicate_array(predicate_id);
  for (size_t i=0; i < A.size(); i++) {
	Variable v = A[i];
	Literal l(v,sign);
	if (P.value(v) == UNKNOWN && !P.is_on_unit_list(l)) {
	  ArrayLiteral al = symmetry->lookup(v);
	  candidates.push_back(LocalSearchLiteral(al,l,UNKNOWN));
	}
  }
  return candidates;
}



bool LocalSearch3::get_cluster(Assignment& P, ClauseID id, const LocalSearchClause& clause)
{
  if (!find_unit_lit(P,id)) return false;

  Clause c;
  Clause image;
  for (size_t j=0; j < clause.size(); j++) {  // map the clause
	Variable v = symmetry->lookup(clause[j]);
	bool s = clause[j].lit.sign();
	c.push_back(Literal(v,s));
  }
  bool extend_succeeded = P.extend(AnnotatedLiteral(c[unit_lit_ptr],Reason(id,c)));

//   cout << "original  ";
//   cout << c[unit_lit_ptr] << " CLAUSE : ";
//   for (size_t j=0; j < c.size(); j++) cout << c[j] << ' ';
//   cout << endl;
  
  
  // set up for unit lit, we need all the actions it uses
  reset_fo_variables();

  size_t num_pops = 0;
  Set<LocalSearchClause> open,closed;
  open.insert(clause);
  while (!open.empty()) {
 	LocalSearchClause c = open.pop();
	num_pops++;
	// 	cout << "pop " << endl;
// 	for (size_t i=0; i < c.size(); i++) cout << c[i].lit << ' ';
// 	cout << endl;
// 	for (size_t i=0; i < c.size(); i++) cout << P.value(c[i].lit.variable()) << ' ';
// 	cout << endl;
	
	closed.insert(c);
 	vector<LocalSearchClause> neighbors = get_neighbors(P,c,id);
	if (P.has_contradiction()) return false;
	for (size_t i=0; i < neighbors.size(); i++) {
 	  if (!closed.contains(neighbors[i])) {
		open.insert(neighbors[i]);
	  }
 	}
  }

//   if (num_pops > 1)
// 	cout << num_pops << endl;
  return true;
}



bool LocalSearchAction3::assign(size_t var, size_t val)
{
  assigner.clear();
  for (size_t i=0; i < fo_variable.size(); i++)   {  // add all possible assignments
// 	set<size_t>::iterator it = fo_variable[i].domain.begin();
// 	for ( ; it != fo_variable[i].domain.end(); it++)
	for (size_t j=0; j < fo_variable[i].domain.size(); j++)
	  assigner.add_assignment(i,fo_variable[i].domain[j]);
  }

  //  cout << assigner << endl;
  
  fo_variable[var].fo_value = val;
  assigner.set_assignment(var,val);
  //  cout << "set " << var << " to " << val << endl;
  size_t i=0;
  for ( ; i < fo_variable.size() - 1; i++) {
	//	cout << assigner << endl;
	if (assigner.empty()) break;
	list<NodePtr>::iterator min = assigner.get_min_node();
	//	cout << "set " << min->row << " to " << min->col << endl;
	fo_variable[min->row].fo_value = min->col;
	assigner.set_assignment(min);
  }


  if (i < fo_variable.size() - 1) return false;
  return true;
}


vector<LocalSearchClause> LocalSearch3::get_neighbors(Assignment& P, LocalSearchClause original_c, ClauseID id)
{
  vector<LocalSearchClause> neighbors;
  vector<size_t>& unit_lit_actions = lit_action_map[unit_lit_ptr];
  vector<size_t>& unit_lit_variables = lit_variable_map[unit_lit_ptr];
  for (size_t i=0; i < unit_lit_actions.size(); i++) {
	LocalSearchClause c = original_c;
	LocalSearchAction3& action = actions[unit_lit_actions[i]];
	if (action.turned_off) continue;
	size_t original_value = original_c[unit_lit_ptr].index_array()[i];
// 	cout << "orignial value " << original_value + 1 << endl;
	// if we don't move the unit lit then continue
	for (size_t j=0; j < action.fo_variable.size(); j++)  
	  reduce_domain(P,action.fo_variable[j],c,action.domain_size);

	FastSet& dom = action.fo_variable[unit_lit_variables[i]].domain;
  	for (size_t s=0; s < dom.size(); s++) 
	{  
 	  if (dom[s] != original_value && action.assign(unit_lit_variables[i],dom[s])) {
		// push the new assignment to the clause c
		for (size_t j=0; j < action.fo_variable.size(); j++) {
		  vector<LitPtr>& lps = action.fo_variable[j].lit_ptr;
		  for (size_t k=0; k < lps.size(); k++) {
			c[lps[k].lit].index_array()[lps[k].index] = action.fo_variable[j].fo_value;
		  }
		}
		Clause image;
		for (size_t j=0; j < c.size(); j++) {  // map the clause
		  Variable v = symmetry->lookup(c[j]);
		  bool s = c[j].lit.sign();
		  c[j].lit = Literal(v,s);
		  image.push_back(c[j].lit);
		}
		
		bool extend_succeeded = P.extend(AnnotatedLiteral(image[unit_lit_ptr],Reason(id,image)));
	
// 		cout << "found neighbor ";
// 		cout << image[unit_lit_ptr] << " CLAUSE : ";
// 		size_t num_unval = 0;
// 		for (size_t j=0; j < c.size(); j++) {
// 		  cout << c[j].lit << ' ';
// 		  if (P.value(c[j].lit.variable()) == (size_t)c[j].lit.sign()) num_unval++;
// 		  if (P.value(c[j].lit.variable()) == UNKNOWN) num_unval++;
// 		}
// 		cout << endl;
// 		if (num_unval > 1) exit(1);

		
 		neighbors.push_back(c);
	  }
	}
  }
  return neighbors;
}

void LocalSearch3::reduce_domain(Assignment& P, LocalSearchVariable& l, LocalSearchClause& c, size_t domain_size)
{
  l.domain.clear();
  for (size_t i=0; i < domain_size; i++) l.domain.insert(i);
  for (size_t j=0; j < l.lit_ptr.size(); j++) {
	LocalSearchLiteral& lit = c[l.lit_ptr[j].lit];
	FastSet d;
	//	cout << lit.lit << endl;
	size_t index_ptr        = l.lit_ptr[j].index;
	ArrayIndex& index = lit.index_array();
	for (size_t k=0; k < domain_size; k++) {
	  //	  cout << "trying " << k << "  ";
	  index[index_ptr] = k;
	  Variable v = symmetry->lookup(lit);
	  //	  cout << " v " << v << " " << Literal(v,lit.lit.sign()) << endl;
	  if (l.lit_ptr[j].lit == unit_lit_ptr) {
// 		cout << "Literal " << Literal(v,lit.lit.sign())
// 			  << " on unit list " << P.is_on_unit_list(Literal(v,lit.lit.sign())) << endl;
		if (!P.is_on_unit_list(Literal(v,lit.lit.sign()))) {
		  if (lit.value == UNKNOWN || P.value(v) == (!lit.lit.sign())) d.insert(k);
		}
	  }
	  else 
		if (P.value(v) == lit.value) d.insert(k);
	}
	FastSet inter;
	l.domain.intersection(d,inter);


	l.domain = inter;
  }
  
}

// 		cout << "Literal " << Literal(v,lit.lit.sign())
// 			 << " on unit list " << P.is_on_unit_list(Literal(v,lit.lit.sign())) << endl;

} // end namespace zap 

