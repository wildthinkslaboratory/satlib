#include "Pfs.h"

namespace zap
{

Pfs::Pfs(const GlobalProductGroup& gpg)
   :  m_num_vars(gpg.number_variables()),
	  m_num_clauses(0), m_end_original_clauses(0),	m_vsids_counts(gpg.number_variables()+1,0.0),
	  m_score_inc(1), m_clause_score_inc(1),
	  m_global_group(gpg)
{
  m_score_set = FastSet(m_num_vars+1);
  m_watchers.insert(m_watchers.end(),number_variables()+1,VariableWatch());
}

void Pfs::initialize_clause_set(const vector<PfsClause>& clauses)
{
  reserve(2000);
  size_t number_unit_clauses = 0;
  for (size_t i=0; i < clauses.size(); i++) {
	if (clauses[i].size() == 1 && size() > number_unit_clauses) {
	  push_back(operator[](number_unit_clauses));
	  operator[](number_unit_clauses) = clauses[i];
	}
	else push_back(clauses[i]);
	if (clauses[i].size() ==1) number_unit_clauses++;
	m_num_clauses++;
  }

  m_end_original_clauses = m_num_clauses;
  m_max_clauseset_size = m_num_clauses + (m_num_clauses/3);
 
  for (size_t i=0; i < size(); i++) operator[](i).id = i;  // set the internal clause ids

  push_back(PfsClause()); // temporary storage for learned clause
  back().id = size()-1;

  for (size_t i=0; i < size()-1; i++) {
	//	cout << operator[](i) << endl;
	operator[](i).build_transports();
	//	cout << operator[](i) << endl;
	if (operator[](i).size() > 1) operator[](i).build_watch_tree();
	//	operator[](i).initialize_local_search();
  }

}

//  This is the major computational loop of the solver.  80-90% of execution time will be spent here.
Result Pfs::get_implications_ground(Assignment& P, Literal a)
{
  vector<Watcher>& w = m_watchers[a.variable()].watch_list[!a.sign()]; 
  for (size_t i=0; i < w.size(); i++) {
	global_vars.clauses_touched++; 
    PfsClause& c = operator[](w[i]);
    if (c.begin()->variable() == a.variable()) { Literal tmp = c[0]; c[0] = c[1]; c[1] = tmp; }  // swap
    Literal& watcher = c[1];
    Literal& other_watcher = c[0];

    if (P.value(other_watcher.variable()) == other_watcher.sign())  // check if the other watcher is sat
      continue;
     
    bool found_new_watcher = false;
    for (size_t j = 2; j < c.size(); j++)  { // try to replace this watcher
      global_vars.literals_touched++;
      if (P.watchable(c[j])) {
	m_watchers[c[j].variable()].watch_list[c[j].sign()].push_back(w[i]);  // add the new watcher to watch list
	
	Literal tmp = watcher;  // swap with the old watcher
	watcher = c[j];
	c[j] = tmp;
	
	w[i] = w.back();         // remove old watcher from watch list
	w.pop_back();
	--i;
	found_new_watcher = true;
	break;
      }
    }

    if (found_new_watcher) continue;

	c.score++;
	++global_vars.prop_from_nogoods;
    if (!P.extend(AnnotatedLiteral(other_watcher, Reason(w[i],operator[](w[i])))))
      return CONTRADICTION;

	if (global_vars.up_structure_bound <= c.size()) {
	  size_t old_size = P.unit_list_size();
	  if (!c.get_cluster(P,other_watcher)) {
		return CONTRADICTION;
	  }
	  global_vars.prop_from_nogoods += (P.unit_list_size() - old_size);
	}
  }
  return SUCCESS;
}


void Pfs::increment_variable_score(size_t v)
{
  if (global_vars.up_queue_pop_heuristic_on) {    //  we turn the up queue pop heuristic off here using global variable 
	m_vsids_counts[v] += m_score_inc;
  }
  if (m_vsids_counts[v] > SCORE_LIMIT)
    rescale_variable_scores();
}


void Pfs::rescale_variable_scores()
{	
// 		cout << "found neighbor ";
// 		cout << image[unit_lit_ptr] << " CLAUSE : ";
// 		for (size_t j=0; j < c.size(); j++) cout << c[j].lit << ' ';
// 		cout << endl;


  for (size_t i=1; i < m_vsids_counts.size(); i++)
    m_vsids_counts[i] *= SCORE_DIVIDER;
  m_score_inc *= SCORE_DIVIDER;
}

void Pfs::increment_clause_score(ClauseID id)
{
  operator[](id).score += m_clause_score_inc;
  if (operator[](id).score > CLAUSE_SCORE_LIMIT)
    rescale_clause_scores();
}


void Pfs::rescale_clause_scores()
{
  for (size_t i=m_end_original_clauses; i < m_num_clauses; i++)
    operator[](i).score *= CLAUSE_SCORE_DIVIDER;
  m_clause_score_inc *= CLAUSE_SCORE_DIVIDER;
}


// void Pfs::adjust_counts(size_t index)
// {
//   PfsClause& cl = operator[](index);
//   set<Literal> c(cl.begin(), cl.end());
//   double total_size = cl.group().total_size();
//   vector<set<Literal> > orbits = cl.group().orbits(Clause(cl));
//   for (size_t i=0; i < orbits.size(); i++) {
// 	set<Literal> intersection;
// 	set_intersection(orbits[i].begin(),orbits[i].end(),c.begin(),c.end(),inserter(intersection,intersection.begin()));
// 	double d = intersection.size() * total_size / orbits[i].size();
// 	for (set<Literal>::iterator it = orbits[i].begin(); it != orbits[i].end(); it++) {
// 	  Literal l = *it;
// 	  m_vsids_counts[l.variable()] += d;
// 	}
//   }
// }



void Pfs::get_ground_clauses(vector<Clause>& ground_clauses)
{
  size_t number_ground_clauses = 0;
  for (size_t i=0; i < size()-1; i++)
	number_ground_clauses += operator[](i).number_ground_clauses();
  ground_clauses.reserve(number_ground_clauses);
  for (size_t i=0; i < size()-1; i++)
	operator[](i).get_ground_clauses(ground_clauses);
}


Result Pfs::get_implications(Assignment& P, Literal l)
{
  for (size_t i=0; i < m_end_original_clauses; i++) {  // propagate original clauses
	if (operator[](i).size() > 1) {
	  if (!operator[](i).k_transporter(P,l.negate())) return CONTRADICTION;
	}
  }
  
  return get_implications_ground(P,l);
//   if (i > m_end_original_clauses) 
// 	if (!operator[](i).k_transport_local_search(P,l.negate())) return CONTRADICTION;
}

bool Pfs::get_symmetric_unit_lits(Assignment& P, ClauseID c_id, Literal unit_lit)
{
  PfsClause& c = operator[](c_id);
//   if (global_vars.use_structure && global_vars.structure_clause_limit >= c.size()) {
  if (global_vars.use_structure) {
	//	global_group().print_assignment(P);
//   	cout << "Clause " << Clause(c) << endl;
	return c.get_symmetric_unit_lits(P,unit_lit);  // this could find a contradiction.  It shouldn't be void
  }
  return P.extend(AnnotatedLiteral(unit_lit,Reason(c_id,c)));
}


bool Pfs::get_cluster(Assignment& P, ClauseID c_id, Literal unit_lit)
{
  PfsClause& c = operator[](c_id);
//   if (global_vars.use_structure && global_vars.structure_clause_limit >= c.size()) {
  if (global_vars.use_structure) {
 	// global_group().print_assignment(P);
//   	cout << "Clause " << Clause(c) << endl;
	return c.get_cluster(P,unit_lit);  // this could find a contradiction.  It shouldn't be void
  }
  return P.extend(AnnotatedLiteral(unit_lit,Reason(c_id,c)));
}



bool PfsClause::get_symmetric_unit_lits(Assignment& P, Literal unit_lit)
{
  return m_local_search3.get_symmetric_unit_lits(P,id);
}

bool PfsClause::get_cluster(Assignment& P, Literal unit_lit)
{
  return m_local_search3.get_cluster(P,id);
}



// PfsClause::PfsClause(const Clause& c, const ProductSubgroup& ps, PredicateSym* psp) :
//   Clause(c), m_subgroup(ps), m_predicate_sym(psp), m_root(c)
// {

// }

PfsClause::PfsClause(const Clause& c, const Ptr<ProductSubgroup>& ps) :
  Clause(c), m_subgroup(ps), m_predicate_sym(NULL), m_root(c)
{

}

PfsClause::PfsClause(const Clause& c, const Ptr<ProductSubgroup>& pps, PredicateSym* psp)
  : Clause(c), m_subgroup(pps), m_predicate_sym(psp), m_root(c)
{

}

// if first round add both clauses
// else add only the new  clause ( not the resolvent from previous resolution )
vector<bool> pfs_seen;  // why don't I add these to the class?  
bool pfs_first_round;
Clause resolvent;

void Pfs::add_to_analysis(const Clause& c, const Assignment& P) {
  
  //    cout << P << endl;
  //    cout << "adding clause " << c.id << ": " << c << endl;
  for (size_t i=0; i < c.size(); i++) {
	if (!pfs_seen[c[i].variable()]) {
	  pfs_seen[c[i].variable()] = true;
	  increment_variable_score(c[i].variable());
	}
  }
}


ClauseID Pfs::resolve(const Reason& r1, const Reason& r2, const Assignment& P, size_t& lits_this_level,
					  Literal& unit_lit)
{

  if (pfs_seen.size() == 0) {
	pfs_seen.insert(pfs_seen.end(),number_variables() + 1, false);
	pfs_first_round = true;
  }
  
  // get the clauses
  const PfsClause& c1 = operator[](r1.id());
  const PfsClause& c2 = operator[](r2.id());

  if (pfs_first_round) pfs_first_round = false;  
	
//   // resolve the instances
  Clause c = boolean_resolve(r1.clause_instance(),r2.clause_instance());
  add_to_analysis(c,P);
  
  lits_this_level = 0;   // calculate lits_this_level and unit_lit
  unit_lit = Literal();
  if (c.size() > 0) {
	Literal deepest = c[0];
	for (size_t i=0; i < c.size(); i++) {
	  if (P.decision_level(c[i].variable()) == P.current_level()) ++lits_this_level;
	  if (P.position(c[i].variable()) > P.position(deepest.variable())) deepest = c[i];
	}
	unit_lit = deepest;
  }
  
  // take the intersection of the groups
//   cout << "resolving : " << endl << "1:  " << r1.clause_instance() << endl
//  	   << "2:  " << r2.clause_instance() << endl;
//   cout << "intersecting : " << endl << "1: " << c1.group() << endl << "2: " << c2.group() << endl;
  Ptr<ProductSubgroup> g;
  if (global_vars.use_structure) {
	if (c1.group_ptr() == c2.group_ptr()) g = c1.group_ptr();
	else {
	  g = c1.group().intersection(c2.group(),r1.clause_instance(),r2.clause_instance());
	}
  }
//   cout  << "resolvent " << c << " and group " << g << endl << endl;

  //  cout << P << endl;
  
  // add it to the back of the stack.  We'll decide later if we want to keep it
  // will need to switch this to just  adding the Product Subgroup
  back() = PfsClause(c,g,&m_predicate_sym);
  back().build_transports();
  back().id = size()-1;
  return back().id;
}


void Pfs::add_learned_clause(ClauseID c_id, const Assignment& P)
{

  pfs_first_round = true;
  pfs_seen.clear();
  
  if (c_id != (int)(size()-1)) quit(string("adding clause that isn't temp clause"));
  m_num_clauses++;
  push_back(PfsClause());
  back().id = size()-1;

  Clause& c = operator[](c_id);

  if (c.size() <= 1) {
	if (global_vars.use_structure)
	  operator[](c_id).initialize_local_search();
	return;
  }

  size_t deepest = 0;
  for (size_t i=0; i < c.size(); i++) // find the most recently valued literal to be first watcher
	if (P.position(c[i].variable()) > P.position(c[deepest].variable())) deepest = i;
  
  Literal temp = c[0]; c[0] = c[deepest]; c[deepest] = temp; // put it in the first array position
  
  deepest = 1;
  for (size_t i=1; i < c.size(); i++)  // 
	if (P.decision_level(c[i].variable()) > P.decision_level(c[deepest].variable())) deepest = i;
  
  temp = c[1]; c[1] = c[deepest]; c[deepest] = temp;
  
  // now we can watch the first two literals
  for (size_t j=0; j < 2; j++)
	m_watchers[c[j].variable()].watch_list[c[j].sign()].push_back(c_id);
  
  increment_clause_score(c_id);
  m_clause_score_inc *= CLAUSE_SCORE_INC_FACTOR;

  if (global_vars.use_structure)
	operator[](c_id).initialize_local_search();
  return;
}

Result Pfs::load_unit_literals(Assignment& P)
{
  for (size_t i=0; i < size() && operator[](i).size() <= 1; i++) {
	vector<Literal> unit_lits = operator[](i).universe();
	for (size_t j=0; j < unit_lits.size(); j++) {
	  Clause instance(vector<Literal>(1,unit_lits[j]));
	  if (!P.extend(AnnotatedLiteral(unit_lits[j],Reason(i,instance))))
		return CONTRADICTION;
	}
  }
  return SUCCESS;
}


void Pfs::update_pointers(Assignment& P, size_t max_id)
{
  // leave a forwarding address
  vector<int> indexes(max_id,-1);
  for (size_t i=0; i < size()-1; i++) {
    indexes[operator[](i).id] = i;
    operator[](i).id = i;
  }

//   // update the watched literal pointers
  for (size_t i=1; i < m_watchers.size(); i++) {
	for (size_t j=0; j < 2; j++) {
	  vector<Watcher>& w = m_watchers[i].watch_list[j];
	  for (size_t k=0; k < w.size(); k++) {
		if (indexes[w[k]] == -1) { // we deleted this clause
		  w[k--] = w.back();
		  w.pop_back();
		}
		else 
		  w[k] = indexes[w[k]];
	  }
	}
  }
  P.update_reasons(indexes);
}




void Pfs::remove_irrelevant_clauses(const Assignment& P)
{
  size_t j = m_end_original_clauses;
  for (size_t i=m_end_original_clauses; i < size()-1; i++) {
    PfsClause& c = operator[](i);
    if ((c.size() > global_vars.length_bound) && !is_a_reason(P,i))  m_num_clauses--;
    else {
	  size_t relevance = 0;
	  for (size_t k=0; k < c.size(); k++) {
		//	if (P.value(c[k].variable()) == UNKNOWN) ++relevance;
		if (P.value(c[k].variable()) == c[k].sign()) ++ relevance;
	  }
	  if ((relevance > global_vars.relevance_bound) && !is_a_reason(P,i)) m_num_clauses--;
	  else
		operator[](j++) = operator[](i);
	}
  }
  
  while (size() > m_num_clauses + 1) pop_back();
  back() = PfsClause();
  m_max_clauseset_size *= CLAUSESET_SIZE_MULTIPLIER;
//   if (size() > 1000 && global_vars.length_bound > 30) 
//   	global_vars.length_bound -= 20;
}



void Pfs::reduce_knowledge_base(Assignment& P)
{
  size_t old_clauseset_size = size();
  bool pointers_need_update = false;
  
  if (global_vars.number_branch_decisions >= m_max_clauseset_size) {
   	pointers_need_update = true;
	remove_irrelevant_clauses(P);
   }
  
   if (pointers_need_update) update_pointers(P,old_clauseset_size);
   return;
}


// this is really counter intuitive.  It needs to be fixed
bool Pfs::is_a_reason(const Assignment& P,ClauseID id) const
{
  // this assumes we're doing this during reduce_knowledge_base
  // we don't check the unit list and assume it's empty
  const PfsClause& c = operator[](id);
  for (size_t i=0; i < P.size(); i++) {
	Literal l = P[i];
	Reason r = P.get_reason(l);
	if (r.id() == c.id) return true;
  }
  return false;
//   const PfsClause& c = operator[](id);
//   ClauseID real_id = c.id;
//   Reason r1 = P.get_reason(c[0]);
//   Reason r2 = P.get_reason(c[1]);
//   bool old = (r1.id() == real_id || r2.id() == real_id);

// //  bool thorough = false;
//   for (size_t i=0; i < c.size(); i++) {
//     Reason r = P.get_reason(c[i]);
//     if (r.id() == real_id) {
//       thorough = true;
//       break;
//     }
//   }

//   if (old != thorough) cout << "id " << real_id << " old method " << old << " different from " << thorough << endl;
//  return old;
}


  
const Clause& Pfs::clause(ClauseID c) const
{
  return operator[](c);
}

  
bool Pfs::valid(const Assignment& P) const { return true; }
bool Pfs::closed(const Assignment& P) const { return true; }
bool Pfs::decision_minimal(const Assignment& P) const { return true; }

bool PfsClause::k_transporter(Assignment& P, Literal l)
{
  if (global_vars.test_local_search_up) {
// 	cout << "local search testing " << endl;
// 	cout << "clause: " << Clause(*this) << endl;
// 	cout << "Group: " ;
// 	for (size_t i=0; i < m_transports.size(); i++) cout << m_transports[i] << endl;
// 	cout << "Assignment: " << P << endl;
// 	cout << "Literal: " << l << endl;
	Assignment P_copy = P;
	size_t initial_size = P.unit_list_size();
	long long unsigned touches_before = global_vars.clauses_touched;
	bool watch_tree_result = k_transport_watch_tree(P,l);
	long long unsigned watch_tree_touches = global_vars.clauses_touched - touches_before;
	bool local_search_result = k_transport_local_search(P_copy,l);
	size_t ups_watch_tree = P.unit_list_size() - initial_size;
	size_t ups_local_search = P_copy.unit_list_size() - initial_size;
// 	cout << "Correct: " << P << endl;
//	cout << "Local:   " << P_copy << endl;
//	cout << "*////////////////////  UP Comparison /////////////////////////*" << endl;
	cout << watch_tree_result << "\t" << ups_watch_tree << "\t" << watch_tree_touches << "\t"
		 << local_search_result << "\t" << ups_local_search << "\t" << m_local_search.get_touches() << endl;

// 	if (ups_watch_tree != ups_local_search) {
// 	  cout << *this << endl;
// 	  cout << "propagating  " << l << endl;
// 	  cout << "watch tree " << P << endl;
// 	  cout << "local search " << P_copy << endl;
// 	  exit(1);
// 	}
	return watch_tree_result;
  }
  else return k_transport_watch_tree(P,l);
}

bool PfsClause::k_transport_local_search(Assignment& P, Literal l)
{
  return m_local_search.k_transporter(P,l,id);
}

bool PfsClause::k_transport_watch_tree(Assignment& P, Literal l)
{
  if (l.variable() >= m_watch_index.size()) return true; // the clause doesn't have this literal
  map<size_t,WatchNode*>& watchers = m_watch_index.get_index(l);
  map<size_t,WatchNode*>::iterator it = watchers.begin();
  vector<BackWatcher> to_remove;
  int new_watcher = -1;
  while (it != watchers.end()) {
	pair<size_t,bool> result = it->second->check(P,to_remove,new_watcher,id,m_watch_index);
	if (!result.second) {
	  for (size_t i=0; i < to_remove.size(); i++)
		m_watch_index.remove_watch(to_remove[i]);
	  return false; // contradiction
	}
	size_t skip = result.first;
	if ((skip > 0) && (new_watcher < 0)) it = watchers.upper_bound(skip);
	else { // either skip is 0 or new_watcher >= 0
	  if (skip > 0) {
		++it;
		while (it != watchers.end() && it->first <= skip) {
		  pair<size_t,bool> dummy = it->second->check(P,to_remove,new_watcher,id,m_watch_index);
		  if (!dummy.second) {
			for (size_t i=0; i < to_remove.size(); i++)
			  m_watch_index.remove_watch(to_remove[i]);
			return false; // contradiction
		  }
		  ++it;
		}
		if (it == watchers.end()) break;
		new_watcher = -1;
	  }
	  else {
		new_watcher = -1;
		++it;
	  }
	}
  }
  
  for (size_t i=0; i < to_remove.size(); i++)
	m_watch_index.remove_watch(to_remove[i]);
  
  return true;
}



} // end namespace zap
