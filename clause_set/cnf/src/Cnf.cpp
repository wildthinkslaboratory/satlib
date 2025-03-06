#include "Cnf.h"

namespace zap
{

Cnf::Cnf(const InputTheory& input) : m_num_clauses(0),
									 m_score_inc(1),
									 m_clause_score_inc(1)
{
  string error("Cnf constructor called on structured input");
 
  size_t number_unit_lits = 0;
  for (size_t i=0; i < input.m_clauses.size(); i++) {
    CnfClause c;
	const InputClause& ic = input.m_clauses[i];

	if (ic.gid().size() || ic.universal().size()) quit(error);
	
    for (size_t j=0; j < ic.lits().size(); j++) {
	  const InputLiteral& il = ic.lit(j);
	  size_t atom_id = global_vars.atom_name_map.lookup(il.to_string());
	  c.push_back(Literal(atom_id,il.sign()));
	}
	
    if (c.size() == 1 && size() > number_unit_lits) { // keep all the unit lits at the front of the vector
      push_back(operator[](number_unit_lits)); 
      operator[](number_unit_lits) = c;
    }
    else push_back(c);
	if (c.size() == 1) number_unit_lits++;
    m_num_clauses++;
  }

  m_num_vars = global_vars.atom_name_map.size();
  m_vsids_counts.insert(m_vsids_counts.end(),m_num_vars+1,0);
  m_score_set = FastSet(m_num_vars+1);
  m_watchers.insert(m_watchers.end(),number_variables()+1,VariableWatch());

  for (size_t i=0; i < size(); i++) operator[](i).id = i;  // set the internal clause ids

  m_end_original_clauses = m_num_clauses;
  m_max_clauseset_size = m_num_clauses + (m_num_clauses/3);
    
  // set up the watched literal indexes
  for (size_t i=0; i < size(); i++) {
    if (operator[](i).size() > 1) {  // watch the first two literals
      for (size_t j=0; j < 2; j++)
	m_watchers[operator[](i)[j].variable()].watch_list[operator[](i)[j].sign()].push_back(i);
    }
  }
  push_back(CnfClause()); // temporary storage for learned clause
  back().id = size()-1;
}



Cnf::Cnf(const vector<Clause>& clauses) : m_num_clauses(0),
									 m_score_inc(1),
									 m_clause_score_inc(1)
{
  size_t number_unit_lits = 0;
  for (size_t i=0; i < clauses.size(); i++) {
    CnfClause c(clauses[i]);
 
    if (c.size() == 1 && size() > number_unit_lits) { // keep all the unit lits at the front of the vector
      push_back(operator[](number_unit_lits)); 
      operator[](number_unit_lits) = c;
    }
    else push_back(c);
	if (c.size() == 1) number_unit_lits++;
    m_num_clauses++;
  }

  m_num_vars = global_vars.atom_name_map.size();
  m_vsids_counts.insert(m_vsids_counts.end(),m_num_vars+1,0);
  m_score_set = FastSet(m_num_vars+1);
  m_watchers.insert(m_watchers.end(),number_variables()+1,VariableWatch());

  for (size_t i=0; i < size(); i++) operator[](i).id = i;  // set the internal clause ids

  m_end_original_clauses = m_num_clauses;
  m_max_clauseset_size = m_num_clauses + (m_num_clauses/3);
    
  // set up the watched literal indexes
  for (size_t i=0; i < size(); i++) {
    if (operator[](i).size() > 1) {  // watch the first two literals
      for (size_t j=0; j < 2; j++)
	m_watchers[operator[](i)[j].variable()].watch_list[operator[](i)[j].sign()].push_back(i);
    }
  }
  push_back(CnfClause()); // temporary storage for learned clause
  back().id = size()-1;
}
   

void Cnf::initialize_clause_set(const vector<Clause>& clauses)
{
   m_num_clauses = 0;
   m_score_inc = 1;
   m_clause_score_inc = 1;
   size_t number_unit_lits = 0;
   for (size_t i=0; i < clauses.size(); i++) {
      CnfClause c(clauses[i]);
 
      if (c.size() == 1 && size() > number_unit_lits) { // keep all the unit lits at the front of the vector
         push_back(operator[](number_unit_lits)); 
         operator[](number_unit_lits) = c;
      }
      else push_back(c);
      if (c.size() == 1) number_unit_lits++;
      m_num_clauses++;
   }
   
   m_num_vars = global_vars.atom_name_map.size();
   m_vsids_counts.insert(m_vsids_counts.end(),m_num_vars+1,0);
   m_score_set = FastSet(m_num_vars+1);
   m_watchers.insert(m_watchers.end(),number_variables()+1,VariableWatch());
   
   for (size_t i=0; i < size(); i++) operator[](i).id = i;  // set the internal clause ids
   
   m_end_original_clauses = m_num_clauses;
   m_max_clauseset_size = m_num_clauses + (m_num_clauses/3);
   
   // set up the watched literal indexes
   for (size_t i=0; i < size(); i++) {
      if (operator[](i).size() > 1) {  // watch the first two literals
         for (size_t j=0; j < 2; j++)
            m_watchers[operator[](i)[j].variable()].watch_list[operator[](i)[j].sign()].push_back(i);
      }
   }
   push_back(CnfClause()); // temporary storage for learned clause
   back().id = size()-1;
}
   


//  This is the major computational loop of the solver.  80-90% of execution time will be spent here.
Result Cnf::get_implications(Assignment& P, Literal a)
{
  vector<Watcher>& w = m_watchers[a.variable()].watch_list[!a.sign()]; 
  for (size_t i=0; i < w.size(); i++) {
	global_vars.clauses_touched++; 
    CnfClause& c = operator[](w[i]);
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
    if (!P.extend(AnnotatedLiteral(other_watcher, Reason(w[i])))) 
      return CONTRADICTION; 
  }
  return SUCCESS;
}


void Cnf::increment_variable_score(size_t v)
{
  if (global_vars.up_queue_pop_heuristic_on)     //  we turn the up queue pop heuristic off here using global variable 
	m_vsids_counts[v] += m_score_inc;
  if (m_vsids_counts[v] > SCORE_LIMIT)
    rescale_variable_scores();
}


void Cnf::rescale_variable_scores()
{
  for (size_t i=1; i < m_vsids_counts.size(); i++)
    m_vsids_counts[i] *= SCORE_DIVIDER;
  m_score_inc *= SCORE_DIVIDER;
}

void Cnf::increment_clause_score(ClauseID id)
{
  operator[](id).score += m_clause_score_inc;
  if (operator[](id).score > CLAUSE_SCORE_LIMIT)
    rescale_clause_scores();
}


void Cnf::rescale_clause_scores()
{
  for (size_t i=m_end_original_clauses; i < m_num_clauses; i++)
    operator[](i).score *= CLAUSE_SCORE_DIVIDER;
  m_clause_score_inc *= CLAUSE_SCORE_DIVIDER;
}


Result Cnf::load_unit_literals(Assignment& P)
{
  for (size_t i=0; i < size(); i++) {
    if (operator[](i).size() > 1) break;
    if (!P.extend(AnnotatedLiteral(operator[](i)[0],Reason(i))))
      return CONTRADICTION;
  }
  return SUCCESS;
}



bool Cnf::get_symmetric_unit_lits(Assignment& P, ClauseID c_id, Literal unit_lit)
{
  return P.extend(AnnotatedLiteral(unit_lit,c_id));
}




int top;
size_t local_lits_this_level;


size_t Cnf::add_to_analysis(const CnfClause& c, const Assignment& P) {

  size_t lits_this_level = 0;
  for (size_t i=1; i < c.size(); i++) {
	int  level = P.decision_level(c[i].variable());
	if (!seen[c[i].variable()] && (level >= 0)) {
	  seen[c[i].variable()] = true;
	  increment_variable_score(c[i].variable());
      if (level == P.current_level()) ++lits_this_level;
      if (level < P.current_level())
         back().push_back(c[i]);
	}
  }
  return lits_this_level;
}


ClauseID Cnf::resolve(const Reason& r1, const Reason& r2, const Assignment& P, size_t& lits_this_level, Literal& unit_lit)
{
  if (seen.size() == 0) {
	seen.insert(seen.end(),number_variables() + 1, false);
	first_round = true;
  }
  
  CnfClause& c1 = operator[](r1.id());
  CnfClause& c2 = operator[](r2.id());
  
  if (first_round) {
     back().push_back(Literal());
	first_round = false;
	top = P.size()-1;
	local_lits_this_level = 0;
	local_lits_this_level = add_to_analysis(c1,P);
	local_lits_this_level += add_to_analysis(c2,P);
    resolve_clause = boolean_resolve(c1,c2);
  }
  else {
     local_lits_this_level += add_to_analysis((r1.id() < (int)(size()-1) ? c1 : c2),P);
     resolve_clause = boolean_resolve(resolve_clause,(r1.id() < (int)(size()-1) ? c1 : c2));
  }                       

  // now calculate the next unit lit
  for ( ; top >= 0; top--) {
	if (seen[P[top].variable()]) {
	  unit_lit = P[top].negate();
	  break;
	}
  }
  
  if (top == -1 ) {
	back().clear();
	return size()-1;
  }
 

  seen[unit_lit.variable()] = false;
  local_lits_this_level--;
  
  if (local_lits_this_level == 0) {
	back()[0] = unit_lit;
  }

  lits_this_level = local_lits_this_level + 1;
  return size()-1;
}



void Cnf::add_learned_clause(ClauseID c_id, const Assignment& P)
{
  first_round = true; // reset this backup flag
  seen.clear();
  
  if (c_id != (int)(size()-1)) quit(string("adding clause that isn't temp clause"));
  m_num_clauses++;
  push_back(CnfClause());
  back().id = size()-1;
  CnfClause& c = operator[](c_id);

  if (c.size() <= 1) return;

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
}


/////////////////////////////////////  REDUCING LEARNED CLAUSE SET  ////////////////////////////////////



void Cnf::reduce_knowledge_base(Assignment& P)
{
  size_t old_clauseset_size = size();
  bool pointers_need_update = false;
  if (false && P.current_level() == 0) {
    pointers_need_update = true;
    remove_permanently_sat_clauses(P);
  }
  
  if (global_vars.number_branch_decisions >= m_max_clauseset_size) {
    pointers_need_update = true;
    remove_irrelevant_clauses(P);
  }

  if (pointers_need_update) update_pointers(P,old_clauseset_size);
}



void Cnf::remove_permanently_sat_clauses(const Assignment& P)
{
  size_t j = 0;
  size_t original_clauses_removed = 0;
  for (size_t i=0; i < size(); i++) {
    if (satisfied_at_level_0(P,i) && operator[](i).size() > 1) {
      if (i < m_end_original_clauses) ++original_clauses_removed;
      m_num_clauses--;
    }
    else 
      operator[](j++) = operator[](i);
  }

  while (size() > m_num_clauses + 1) pop_back();
  
  m_end_original_clauses -= original_clauses_removed;
}



void Cnf::remove_irrelevant_clauses(const Assignment& P)
{
  double score_req = (double)(m_clause_score_inc)/(size()-m_end_original_clauses);
  // first we sort clauses by their hit rates
  vector<CnfClause>::iterator start_learned_clauses = begin() + m_end_original_clauses;
  vector<CnfClause>::iterator end_learned_clauses = end() - 1;
  sort(start_learned_clauses,end_learned_clauses);
  
  // in the first half we keep binary clauses, high score clauses and those used as reasons
  size_t mid_point = m_end_original_clauses + (size() - m_end_original_clauses)/2;
  size_t j = m_end_original_clauses;
  for (size_t i=m_end_original_clauses; i < mid_point; i++) {
    CnfClause& c = operator[](i);
    if ((c.size() > 2) && (c.score < score_req) && !is_a_reason(P,i)) {
      m_num_clauses--;
    }
    else 
      operator[](j++) = operator[](i);
  }
  
  // in the second half we keep those used as reasons and binary clauses
  for (size_t i=mid_point; i < size(); i++) {
    CnfClause& c = operator[](i);
    if ((c.size() > 2) && !is_a_reason(P,i)) {
      m_num_clauses--;
    }
    else 
      operator[](j++) = operator[](i);
  }
  
  while (size() > m_num_clauses + 1) pop_back();
  m_max_clauseset_size *= CLAUSESET_SIZE_MULTIPLIER;
}



void Cnf::update_pointers(Assignment& P, size_t max_id)
{
  // leave a forwarding address
  vector<int> indexes(max_id,-1);
  for (size_t i=0; i < size(); i++) {
    indexes[operator[](i).id] = i;
    operator[](i).id = i;
  }

  // update the watched literal pointers
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



bool Cnf::satisfied_at_level_0(const Assignment& P, ClauseID id)
{
  CnfClause& c = operator[](id);
  for (size_t i=0; i < c.size(); i++) {
    if (P.value(c[i].variable()) != UNKNOWN &&
	P.decision_level(c[i].variable()) == 0 &&
	P.value(c[i].variable()) == c[i].sign()) {
      return true;
    }
  }
  return false;
}



// this is really counter intuitive.  It needs to be fixed
bool Cnf::is_a_reason(const Assignment& P,ClauseID id) const
{
  const CnfClause& c = operator[](id);
  ClauseID real_id = c.id;
  Reason r1 = P.get_reason(c[0]);
  Reason r2 = P.get_reason(c[1]);
  bool old = (r1.id() == real_id || r2.id() == real_id);

//   bool thorough = false;
//   for (size_t i=0; i < c.size(); i++) {
//     Reason r = P.get_reason(c[i]);
//     if (r.id() == real_id) {
//       thorough = true;
//       break;
//     }
//   }

//   if (old != thorough) cout << "id " << real_id << " old method " << old << " different from " << thorough << endl;
  return old;
}



/////////////////////////////  TEST FOR INVARIANT PROPERTIES  //////////////////////////////////



bool Cnf::valid(const Assignment& P) const
{
  for (size_t i=0; i < number_clauses(); i++) {
    size_t j=0;
    for ( ; j < operator[](i).size(); j++)
      if (P.watchable(operator[](i)[j])) break;

    if (j >= operator[](i).size()) {
      cerr << "unsatisfied clause: " << operator[](i) << endl;
      return false;
    }
  }
  
  return true;
}


bool Cnf::closed(const Assignment& P) const
{
  for (size_t i=0; i < number_clauses(); i++) {
    const CnfClause& c = operator[](i);
    size_t number_unvalued = 0;
    size_t j=0;
    for ( ; j < c.size(); j++) {
      if (P.watchable(c[j])) {
	if (P.value(c[j].variable()) == UNKNOWN) {
	  number_unvalued++;
	}
	else break;
      }
      if (j == c.size() && number_unvalued <= 1) {
	cerr << "unclosed clause " << c << endl;
	return false;
      }
    } 
  }
  return true;
}


bool Cnf::decision_minimal(const Assignment& P) const
{
  return true;
}

} // end namespace zap
