#include "Transport.h"
#include "Set.h"

namespace zap
{

//OrbitIndex* TransportCache::m_orbit_index = 0;

void print_columns(const vector<Column> & v) {
  cout << '[';
  for (size_t i=0; i < v.size(); i++) cout << v[i] << ' ';
  cout << "] ";
}


void PfsTransportVector::build_orbits()
{
  const vector<Literal>& c = clause_rep();
  size_t depth = 0;
  vector<Literal> working_orbit;
  Set<Literal> open, closed;

  for (size_t i=0; i < c.size(); i++) open.insert(c[i]);
  
  while (!open.empty()) {
    Literal point = open.pop();
    working_orbit.push_back(point);
    closed.insert(point);

    for (size_t i=0; i < working_orbit.size(); i++) {
      point = working_orbit[i];
      for (size_t i=depth+1; i < size(); i++) {
		vector<Literal> orbit = operator[](i).action().orbit(point);
		for (size_t j=0; j < orbit.size(); j++) {
		  if (!closed.contains(orbit[j])) {
			working_orbit.push_back(orbit[j]);
			open.remove(orbit[j]);
			closed.insert(orbit[j]);
		  }
		}
      }
    }
    sort(working_orbit.begin(),working_orbit.end());
	
    m_orbit_cache.add_orbit(working_orbit);
    working_orbit.clear();
  }
}

const vector<Literal>& PfsTransportVector::orbit(Literal l)
{
  return m_orbit_cache.get_orbit(l);
}


// void PfsTransportVector::order_tree(size_t depth, vector<Literal> still_to_fix)
// {
//   if (size() < 2) return; // there is nothing to do
//   if (still_to_fix.size() == 0) return;
  
//   vector<size_t> arity(still_to_fix.size(),0);
//   for (size_t i=0; i < still_to_fix.size(); i++) {
//     for (size_t j=depth; j < size(); j++)
//       if (!operator[](j).action().fixes(still_to_fix[i],operator[](j).columns())) arity[i]++;
//   }
  
//   // sort clause in terms of arity of predicate
//   for (size_t i=0; i < still_to_fix.size(); i++) {
//     for (size_t j=i+1; j < still_to_fix.size(); j++) {
//       if (arity[j] > arity[i]) { // swap
// 	Literal temp_lit = still_to_fix[i];
// 	size_t temp_arity = arity[i];
// 	still_to_fix[i] = still_to_fix[j];
// 	arity[i] = arity[j];
// 	still_to_fix[j] = temp_lit;
// 	arity[j] = temp_arity;
//       }
//     }
//   }

// //   for (size_t i=0; i < arity.size(); i++) 
// //     cout << display_lit(still_to_fix[i]) << " arity " << arity[i] << endl;
// //   cout << endl;
  
//   while (arity.size() > 0 && arity.back() == 0) {
//     arity.pop_back();
//     still_to_fix.pop_back();
//   }

//   if (arity.empty()) return;
		       
//   // fix the next literal
//   Literal to_fix = still_to_fix.back();
//   still_to_fix.pop_back();

//   // if it moves to_fix, then move it up
//   size_t new_depth = depth;
//   for (size_t i=depth; i < size(); i++) {
//     if (!operator[](i).action().fixes(to_fix,operator[](i).columns())) {
//       PfsTransport temp = operator[](new_depth);
//       operator[](new_depth) = operator[](i);
//       operator[](i) = temp;
//       new_depth++;
//     }
//   }
  
//   // recurse
//   order_tree(new_depth, still_to_fix);
// }




PfsTransport::PfsTransport(const vector<Column>& rep,const Ptr<Action>& ap) :
  m_action(ap), m_rep_size(rep.size()), m_columns(rep)
{
  set<Column> srep;
  for (size_t i=0; i < rep.size(); i++) srep.insert(rep[i]);
  
  set<Column> set_int, set_diff;
  const set<Column>& c = ap->columns[0];
  set_intersection(c.begin(),c.end(),srep.begin(),srep.end(),inserter(set_int,set_int.begin()));
  if (set_int.size() == 0) return;
  set_difference(c.begin(),c.end(),srep.begin(),srep.end(),inserter(set_diff,set_diff.begin()));
  set<Column>::iterator it = set_diff.begin();
  for ( ; it != set_diff.end(); it++) m_columns.push_back(*it);
}



// this must produce the actual ground clause
// bool PfsTransport::next_image(PfsCounter& counter, vector<Literal>& clause_rep) const
// {
//   if (!counter.next()) return false;
//   for (size_t i=0; i < clause_rep.size(); i++)  // this scales with the length of the clause
//     clause_rep[i] = action().image(clause_rep[i],counter);
//   return true;
// }

// shouldn't need this anymore
// Permutation PfsTransport::get_perm(const PfsCounter& counter) const {

//   if (counter.size() == 0) return Permutation();
//   if (action().parent_group == 0) return Permutation();
//   const GlobalProductGroup* pg = action().parent_group;

//   vector<size_t> before = first_rep();
//   vector<size_t> after = counter.current_rep();
//   ColumnPerm2 p(ColumnPerm(before,after));
  
//   vector<RawPerm> product_perm(pg->size(),RawPerm());
//   vector<Index> indexes = action().indexes;
//   for (size_t i=0; i < indexes.size(); i++)
//     product_perm[indexes[i]] = p;
  
//   return pg->ground_perm(product_perm);
  
// }


vector<vector<Literal> > PfsTransportVector::orbits(const vector<Literal>& c, size_t depth) const
{ 
  vector<vector<Literal> > answer;
  vector<Literal> working_orbit;
  Set<Literal> open, closed;

  for (size_t i=0; i < c.size(); i++) open.insert(c[i]);
  
  while (!open.empty()) {
    Literal point = open.pop();
    working_orbit.push_back(point);
    closed.insert(point);

    for (size_t i=0; i < working_orbit.size(); i++) {
      point = working_orbit[i];
      for (size_t i=depth+1; i < size(); i++) {
		vector<Literal> orbit = operator[](i).action().orbit(point);
		for (size_t j=0; j < orbit.size(); j++) {
		  if (!closed.contains(orbit[j])) {
			working_orbit.push_back(orbit[j]);
			open.remove(orbit[j]);
			closed.insert(orbit[j]);
		  }
		}
      }
    }
    sort(working_orbit.begin(),working_orbit.end());
	
    answer.push_back(working_orbit);
    working_orbit.clear();
  }
  return answer;
}

vector<Literal> PfsTransportVector::orbit(const vector<Literal>& c, size_t depth) const
{  
  vector<Literal> answer;
  Set<Literal> open, closed;

  for (size_t i=0; i < c.size(); i++) open.insert(c[i]);
  
  while (!open.empty()) {
    Literal point = open.pop();
    answer.push_back(point);
    closed.insert(point);

    for (size_t i=0; i < answer.size(); i++) {
      point = answer[i];
      for (size_t i=depth+1; i < size(); i++) {
		vector<Literal> orbit = operator[](i).action().orbit(point);
		for (size_t j=0; j < orbit.size(); j++) {
		  if (!closed.contains(orbit[j])) {
			answer.push_back(orbit[j]);
			open.remove(orbit[j]);
			closed.insert(orbit[j]);
		  }
		}
      }
    }
  }
  sort(answer.begin(),answer.end());
  return answer;
}


size_t WatchNode::m_next_id = 1;

void WatchNode::build(const PfsTransportVector& transports, WatchIndex& watch_index)
{
  if (m_depth == 0) {  // root node

    vector<size_t> depth_fixed(m_clause.size(),transports.size()+1);
    for (size_t i=0; i < m_clause.size(); i++)
      depth_fixed[i] = transports.depth_fixed(m_clause[i]);

    // now sort based on depth fixed
    for (size_t i=0; i < m_clause.size()-1; i++) {
      size_t min_i = i;
      for (size_t j=i+1; j < m_clause.size(); j++) 
	if (depth_fixed[j] < depth_fixed[min_i]) min_i = j;

      // swap
      size_t fixed_temp = depth_fixed[i];
      Literal temp = m_clause[i];
      depth_fixed[i] = depth_fixed[min_i];
      m_clause[i] = m_clause[min_i];
      depth_fixed[min_i] = fixed_temp;
      m_clause[min_i] = temp;
    }
  }
  
  for (size_t i=0; i < m_clause.size(); i++) 
    if (!m_fixed[i]) m_fixed[i] = transports.fixes(m_clause[i],m_depth);

//   cout << " building " << *this << endl;
  
  if (m_depth >= transports.size()-1) {
    for (size_t i=0; i < m_clause.size() && i < 2; i++) {
// 	  cout << "adding watcher " << endl << m_clause[i] << endl << *this << endl << m_level_id << endl;
      watch_index.add_watch(m_clause[i],this,m_level_id);
      m_back_watchers.push_back(BackWatcher(m_clause[i],m_level_id,i));
    }
    return;
  }
  
  PfsCounter counter = transports[m_depth+1].initial_counter();
  m_children.push_back(expand(transports));
  do {
    m_children.back()->build(transports,watch_index);
  } while (next_child(counter,transports[m_depth+1]));
  
  return;
  
}

void PfsTransportVector::cnf(size_t depth, vector<Literal> clause, vector<Clause>& clauses) const
{
  if (depth == size()-1) {  // leaf node. get the clause
	clauses.push_back(Clause(clause));
	return;
  }

  PfsCounter counter = operator[](depth+1).initial_counter();
  do {
	for (size_t i=0; i < clause.size(); i++)
	  clause[i] = operator[](depth+1).action().image(clause[i],counter);
	cnf(depth+1,clause,clauses);
  } while (counter.next());
}


void WatchNode::build_right_leaf()
{
  m_right_leaf = right_most_leaf();
  
  if (m_children.empty()) return;
  
  for (size_t i=0; i < m_children.size(); i++)
    m_children[i]->build_right_leaf();
  return;
  
}




size_t WatchNode::get_skip(size_t index) const
{
  size_t skip = 0;
  const WatchNode* node = m_parent;
  while (node && node->m_fixed[index]) {
    skip = node->m_right_leaf;
    node = node->m_parent;
  }
  return skip;
}

//#ifdef SUBSEARCH

pair<size_t,bool> WatchNode::check(Assignment& PA,vector<BackWatcher>& to_remove,int& new_watcher,ClauseID id, WatchIndex& watch_index)
{
  global_vars.clauses_touched++;
//    cout << "checking " << m_clause  << " with old watchers ";
//    cout <<  m_clause[m_back_watchers[0].local_index] << "  and  "
//         << m_clause[m_back_watchers[1].local_index] << endl;
  
  int unsat=-1, unval=-1, deepest=-1;
  for (size_t i=0; i < m_back_watchers.size(); i++) {
	Literal l = m_clause[m_back_watchers[i].local_index];
    size_t value = PA.value(l.variable());
    global_vars.literals_touched++;
    if (value == l.sign()) return make_pair(get_skip(m_back_watchers[i].local_index),true);
    if (value == UNKNOWN) unval = i;
    else {
	  if (PA.decision_level(l.variable()) == NULL_ID) deepest = i;
	  unsat = i;
	}
  }

  if (unsat == -1) {
	cout << "Backwatachers " << m_back_watchers[0] << " , " << m_back_watchers[1] << endl << m_clause << endl << PA << endl;
	exit(1);
  }
  
  if (unval == -1) {
    // there had better be a sat lit in this clause
    for (size_t i=0; i < m_clause.size(); i++) {
      size_t value = PA.value(m_clause[i].variable());
      if (value == m_clause[i].sign()) return make_pair(get_skip(i),true);
    }
	cout << "unsat clause " << endl;
	exit(1);
  	// clause is unsat.  The lit valued deepest is the contradiction
	// this needs to be fixed to manage incomplete search.  We just need to scan the whole clause for the deepest lit.
	//     cout << "watchers " <<  m_clause[m_back_watchers[0].local_index] << "  and  "
// 	 << m_clause[m_back_watchers[1].local_index] ; 
//     cout << "no satisfied lit " << m_clause << endl;
// 	cout << "partial assignment " << PA << endl;
	return make_pair(0,PA.extend(AnnotatedLiteral(m_clause[m_back_watchers[deepest].local_index],Reason(id,m_clause)))); 
  }

  
  if (new_watcher >= 0 && m_back_watchers[unval].local_index != new_watcher) {

//  	cout << "remove watcher 1 " << m_back_watchers[unsat] << endl;
    to_remove.push_back(m_back_watchers[unsat]);  // remove old watcher
    m_back_watchers[unsat] = m_back_watchers.back();
    m_back_watchers.pop_back();
    
    watch_index.add_watch(m_clause[new_watcher],this,m_level_id);  // add the new watcher
    m_back_watchers.push_back(BackWatcher(m_clause[new_watcher],m_level_id,new_watcher));
    return  make_pair(0,true);
  }

  
  size_t unsat_i = m_back_watchers[unsat].local_index;
  size_t unval_i = m_back_watchers[unval].local_index;
  
  for (size_t i=0; i < m_clause.size(); i++) {
    if (i == unsat_i || i == unval_i) continue;
    size_t value = PA.value(m_clause[i].variable());
//  	cout << "value " << value  << " lit " << m_clause[i] << ":" << m_clause[i].sign() << endl;
	//    global_vars.literals_touched++;
    if (value == m_clause[i].sign()) return make_pair(get_skip(i),true);
    if (value == UNKNOWN) {                         // found a new watcher
// 	  cout << "remove watcher 1 " << m_back_watchers[unsat] << endl;
// 	  cout << m_back_watchers.size() << "\t" << unsat << endl;
      to_remove.push_back(m_back_watchers[unsat]);  // remove old watcher
      m_back_watchers[unsat] = m_back_watchers.back();
      m_back_watchers.pop_back();
      watch_index.add_watch(m_clause[i],this,m_level_id);  // add the new watcher
      m_back_watchers.push_back(BackWatcher(m_clause[i],m_level_id,i));

      if (new_watcher >= 0) return make_pair(0,true);
      new_watcher = i;
      return make_pair(get_skip(i),true);
    }
  }

  // clause is unit
//    cout << "unit clause " << m_clause << " lit " << m_clause[unval_i] << endl;
//    cout << "partial assignment " << PA << endl;
  return make_pair(0,PA.extend(AnnotatedLiteral(m_clause[unval_i],Reason(id,m_clause)))); // we need some way to access the clause ID
}
//#endif

// #ifdef GROUND
// size_t WatchNode::check_ground(Assignment& PA,vector<BackWatcher>& to_remove,int& new_watcher)
// {
//   global_vars.clauses_touched++;
// //   cout << "\t\t\tchecking " << m_clause  << " with old watchers " <<  display_lit(m_clause[m_back_watchers[0].local_index]) << "  and  "
// //        << display_lit(m_clause[m_back_watchers[1].local_index]) << endl;
  
//   int unsat=-1, unval=-1;
//   for (size_t i=0; i < m_back_watchers.size(); i++) {
// 	Literal l = m_clause[m_back_watchers[i].local_index];
//     size_t value = PA.value_no_unitlist(l.variable());
// 	global_vars.literals_touched++;
//     if (value == l.sign()) return make_pair(0,true);
//     if (value == UNKNOWN) unval = i;
//     else unsat = i;
//   }

//     // both unsat and unval should be assigned now
//   // we look for a replacement for unsat
//   if (unsat == -1 || unval == -1) {
//     cout << "Watchers " ;
//     for (size_t i=0; i < m_back_watchers.size(); i++)
//     cout << m_clause[m_back_watchers[i].local_index] << ":"
// 	 << m_back_watchers[i].lit  << " ";
//     cout << " in clause " << m_clause << endl;
//     exit(1);
//   }
  
//   size_t unsat_i = m_back_watchers[unsat].local_index;
//   size_t unval_i = m_back_watchers[unval].local_index;
  
//   for (size_t i=0; i < m_clause.size(); i++) {
//     if (i == unsat_i || i == unval_i) continue;
//     size_t value = PA.value_no_unitlist(m_clause[i].variable());
// 	global_vars.literals_touched++;
//     if (value == m_clause[i].sign() || value == UNKNOWN) {   // found a new watcher
//       to_remove.push_back(m_back_watchers[unsat]);  // remove old watcher
//       m_back_watchers[unsat] = m_back_watchers.back();
//       m_back_watchers.pop_back();

//       m_watch_index->add_watch(m_clause[i],this,m_level_id);  // add the new watcher
//       m_back_watchers.push_back(BackWatcher(m_clause[i],m_level_id,i));
//       return make_pair(0,true);
//     }
//   }

//   // clause is unit
//   return make_pair(0,PA.extend(AnnotatedLiteral(m_clause[unval_i],Reason(1,m_clause)))); //
// }

// #endif



WatchNode* WatchNode::expand(const PfsTransportVector& transports)
{
  WatchNode* node = new WatchNode(*this);
  node->m_parent = this;
  node->m_depth++;
  node->m_child = 0;
  node->m_level_id = m_level_id * transports[node->m_depth].number_of_images();
  
  return node;
}

size_t WatchNode::right_most_leaf() const
{
  size_t answer = m_level_id;
  const WatchNode* node = this;
  while (node->m_children.size()) {
    answer = answer * node->m_children.size() + (node->m_children.size() - 1);
    node = node->m_children[0];
  }

  return answer;
}



} // end namespace zap
