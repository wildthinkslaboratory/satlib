#include "ProductSubgroup.h"
// #include "memory.h"
// #include "Set.h"

namespace zap
{

ostream& operator << (ostream& os, const ProductSubgroup& ps) {
  for (size_t i=0; i < ps.m_actions.size(); i++) {
	os << *ps.m_actions[i] << endl;
  }
  return os;
}


// void ProductGroup::dump(const Group &, ostream& os) const
// {
//   os << "Product Group: ";
//   for (size_t i=0; i < m_products.size(); i++)
//     {
//       os << Group(m_products[i]) << "  ";
//       if (i < m_products.size() - 1) os << "X ";
//     }
// }

// bool ProductGroup::k_transporter(const AugClause &c, int k, const AtomValueMap& PA, Literal in, 
// 			     const Termination &t, Consequences *p, 
// 			     watchlist *w) const
// {
// //   if (c.products().size() == 0) 
// //     // should always have this if empty group
// //     return c.k_transporter_easy(k,PA,p);
  
//   if (in) {
//     map<size_t,WatchNode*>& watchers = c.watch_index().get_index(in);
//     map<size_t,WatchNode*>::iterator it = watchers.begin();
//     vector<BackWatcher> to_remove;
//     int new_watcher = -1;
//     while (it != watchers.end()) {
//       size_t skip = it->second->check(PA,to_remove,p,new_watcher);

// #ifdef SUBSEARCH
      
//       if ((skip > 0) && (new_watcher < 0)) it = watchers.upper_bound(skip);
//       else { // either skip is 0 or new_watcher >= 0
// 	if (skip > 0) {
// 	  ++it;
// 	  while (it != watchers.end() && it->first <= skip) {
// 	    size_t dummy = it->second->check(PA,to_remove,p,new_watcher);
// 	    ++it;
// 	  }
// 	  if (it == watchers.end()) break;
// 	  new_watcher = -1;
// 	}
// 	else {
// 	  new_watcher = -1;
// 	  ++it;
// 	}
//       }
// #endif

// #ifdef GROUND
//       ++it;
// #endif
//     }
    
//     for (size_t i=0; i < to_remove.size(); i++)
//       c.watch_index().remove_watch(to_remove[i]);
//   }
//   else {
//     cout << "cant do this yet " << endl;
//     exit(1);
//   }


//   return false;
// }




// AugClause::AugClause(const vector<Literal>& c, const ProductSubgroup& g, bool learned, bool ground) :
//   m_lits(c), m_first_lit(NULL), m_products(g), m_transport_new(c), m_root(c), m_learned(learned), m_active(true),
//   m_backtrack_num(0), m_depth_learned(0)
// {
//   if (ground) m_group = GrpPtr(Group());
//   else {
//     Cycle c;
//     c.push_back(literal(1,true));
//     c.push_back(literal(2,true));
//     vector<Cycle> cycles(1,c);
//     Pdata pdat(cycles);
//     vector<Permutation> p(1,pdat);
//     m_group = GrpPtr(Group(RawGroup(p)));
//     //m_group = GrpPtr(Group(m_products.group()));
//   }

//   m_transport_new = m_products.build_transports(vector<Literal>(*this));
  
//   //  vector<Literal> c_image = group()->orbit(vector<Literal>(*this));
  
//   //  m_products.add_background_action(c_image);  
//   //  m_products.add_interclause_background_action(c);
// }


// void AugClause::build_product_structure()
// {
//   ProductGrp p;
  
// //   for (size_t i=0; i < m_products.size(); i++) {
// //     if (!m_products[i].background) {
      
// //       vector<Permutation> gens = m_products.get_generators(i);

// //       p.push_back(vector<Permutation>());
// //       p.back().insert(p.back().end(),gens.begin(),gens.end());
// //     }
// //   }
  
//   group()->type() = PRODUCT;
//   group()->set_structure(new ProductGroup(p));
  
//   //  build_transports();
// }


double ProductSubgroup::total_size() const {
  double sz = 1;
  for (size_t i=0; i < m_actions.size(); i++) {
    unsigned n = (unsigned)m_actions[i]->columns[0].size();
    sz *= factorial(n);
  }
  return sz;
}


// vector<Permutation> ProductSubgroup::group() const
// {
//   vector<Permutation> p;
//   for (size_t i=0; i < m_actions.size(); i++) {
//     vector<Permutation> gens = get_generators(i);
//     p.insert(p.end(), gens.begin(), gens.end());
//   }
//   return p;
// }


// void ProductSubgroup::add_action(const vector<Permutation>& perms, const vector<Literal>& c, bool background)
// {
//   if (!perms.size()) return;

//   const ProductPerm& p = perms[0].product_perm();
//   vector<size_t> moved;
//   for (size_t i=0; i < p.size(); i++) {
//     if (!p[i].istrivial()) {
//       moved.push_back(i);
//     }
//   }

//   set<size_t> set_c;
//   for (size_t i=0; i < c.size(); i++) set_c.insert(get_atom(c[i])-1);
//   m_actions.push_back(Action(moved,set_c,m_parent_group,background));
// }


void ProductSubgroup::add_action(const Action& a)
{
  if (a.empty()) return;
  m_actions.push_back(Ptr<Action>(a));
}


// // void SymmetryGroup::dump(const Group &, ostream& os) const
// // {
// //   os << m_sym << endl;
// // }

// it's not clear who should own this  shouldn't this be part of AugClause constructor?
// maybe it isn't because of all the weird copying stuff for cached groups.
PfsTransportVector ProductSubgroup::build_transports(const vector<Literal>& clause,const vector<bool>& background) const
{
  PfsTransportVector t(clause);
  t.push_back(PfsTransport());

  for (size_t i=0; i < m_actions.size(); i++) {
	if (!background[i]) {
      vector<Column> rep = m_actions[i]->get_columns(clause);
      t.push_back(PfsTransport(rep,m_actions[i]));
	}
  }
//   t.order_tree(1,clause);
//   cout << "tree order" << endl;
//   for (size_t i=1; i < t.size(); i++) cout << t[i] << endl;
//   cout << endl;
  return t;
}





/////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////  RESOLUTION  /////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////






void restrict(vector<int> map1,vector<int> map2,vector<set<Column> >& c1,const vector<set<Column> >& c2)
{
  vector<set<Column> > answer;
  for (size_t i=0; i < map1.size(); i++) {
	if (map1[i] >= 0 && map2[i] >= 0) {
	  set<Column>& set1 = c1[map1[i]];
	  const set<Column>& set2 = c2[map2[i]];
	  set<Column>::iterator it = set1.begin();
	  for (; it != set1.end(); it++) map1[*it] = -1;   // remove the elements of both sets from their associated map
	  it = set2.begin();
	  for (; it != set2.end(); it++) map2[*it] = -1; 
	  set<Column> set_int; ;                          // do the intersection
	  set_intersection(set1.begin(),set1.end(),set2.begin(),set2.end(),inserter(set_int,set_int.begin()));
	  if (set_int.size() > 1) answer.push_back(set_int);  // if the intersection has size bigger than 1 we keep it
	}
  }
  // load the data back into c1 and map1
  for (size_t i=0; i < map1.size(); i++) map1[i] = -1;
  for (size_t i=0; i < answer.size(); i++) {
	set<Column>::iterator it = answer[i].begin();
	for ( ; it != answer[i].end(); it++) { 
	  if (*it >= map1.size()) map1.insert(map1.end(),*it+1-map1.size(),-1);
	  map1[*it] = i;
	}
  }
  c1 = answer;
}


Ptr<ProductSubgroup> ProductSubgroup::intersection(const ProductSubgroup& ps, const Clause& c1,const Clause& c2) const
{
  // we want to project one (set of actions)/(vector space) onto the other to get
  // the intersection of the vector spaces.

//   cout << "intersection " << *this << endl << "with " << ps << endl;
//   cout << "clauses to stabilize " << c1 << " /t " << c2 << endl;
  
  ProductSubgroup answer(m_parent_group);
  
  set<Literal> s2(c1.begin(),c1.end());
  set<Literal> s1(c2.begin(),c2.end());
  // copy the ProductGroups
  vector<Ptr<Action> > a2 = m_actions;
  vector<Ptr<Action> > a1 = ps.m_actions;
  map<IndexVector,vector<set<Column> > > basis;

  for (size_t i=0; i < a1.size(); i++) {
	if (a1[i]->stabilizes_set(s2)) {
	  bool is_independent = true;
	  for (size_t j=0; j < a2.size(); j++) {
		if (!a1[i]->independent(*a2[j])) {
		  is_independent = false;
		  break;
		}
	  }
 	  if (is_independent)
	  {
		basis[a1[i]->index_vector] = a1[i]->columns;
	  }
	}
  }
  
  for (size_t i=0; i < a2.size(); i++) {
	if (a2[i]->stabilizes_set(s1)) {
	  bool is_independent = true;
	  for (size_t j=0; j < a1.size(); j++) {
		if (!a2[i]->independent(*a1[j])) {
		  is_independent = false;
		  break;
		}
	  }
 	  if (is_independent)
	  {
		basis[a2[i]->index_vector] = a2[i]->columns;
	  }
	}
  }
  
  size_t vsize = m_parent_group->size();

  // we're going to map U + W -> W
  // first map U onto W
  for (size_t i=0; i < a1.size(); i++) {
	IndexVector v(vsize,false);
	//	v = v + a1[i]->index_vector;
	vector<int> column_map = a1[i]->column_map;
	vector<set<Column> > columns = a1[i]->columns;
	for (size_t j=0; j < a2.size(); j++) {
	  size_t dot_product = a1[i]->index_vector.dot_product(a2[j]->index_vector);
// 	  cout << a1[i]->index_vector << " * " << a2[j]->index_vector << " = " << dot_product << endl;
	  if (dot_product > 0) {
		v = v + a2[j]->index_vector;
		// these actions overlap so make them agree on columns
		restrict(column_map,a2[j]->column_map,columns,a2[j]->columns);  
	  }
	}
	if (basis.find(v) == basis.end())  // need to fix this so it keeps the best set of columns
	  basis[v] = columns;  // two of these vectors are independent if they differ in at least one spot
  }

//   // now add  W to our basis
//   for (size_t j=0; j < a2.size(); j++) {
// 	IndexVector& v = a2[j]->index_vector;
// 	vector<set<Column> > columns = a2[j]->columns;
// 	if (basis.find(v) == basis.end())  // need to fix this so it keeps the best set of columns
// 	  basis[v] = columns;  // two of these vectors are independent if they differ in at least one spot
//   }

  // now we map U onto our new basis
//   map<IndexVector,vector<set<Column> > > U_intersect_W;
//   for (size_t i=0; i < a1.size(); i++) {
// 	IndexVector v(vsize,false);
// 	v = v + a1[i]->index_vector;
// 	vector<int> column_map = a1[i]->column_map;
// 	vector<set<Column> > columns = a1[i]->columns;
// 	map<IndexVector,vector<set<Column> > >::iterator it = basis.begin();
// 	for ( ; it != basis.end(); it++) {
// 	  size_t dot_product = a1[i]->index_vector.dot_product(it->first);
// 	  if (dot_product > 0) {
// 		v = v + it->first;
// 		//		restrict(column_map,a2[j]->column_map,columns,it->second);   // don't know if this is right
// 	  }
// 	}
// 	if (U_intersect_W.find(v) == U_intersect_W.end())  // need to fix this so it keeps the best set of columns
// 	  U_intersect_W[v] = columns;  // two of these vectors are independent if they differ in at least one spot
//   }
  
  map<IndexVector,vector<set<Column> > >::iterator it = basis.begin();
  for ( ; it != basis.end(); it++) {
	// 	cout << "\t" << it->first << endl;
	answer.add_action(Action(it->first,it->second,m_parent_group));
  }

//   cout << "answer " << answer << endl;
  return Ptr<ProductSubgroup>(answer);
}





void ProductSubgroup::add_background_symmetry(const vector<Literal>& universe)
{

//   cout << "computing background sym " << endl << *this << endl;
//   cout << Clause(universe) << " parent group " << m_parent_group << endl;
  if (!m_parent_group) return;
  const GlobalProductGroup& global_group = *m_parent_group;
//   for (size_t i=0; i < global_group.size(); i++) {
// 	set<Column> pt_stab = global_group[i].point_stabilizer(universe);
// 	if (pt_stab.size() > 1)
// 	  add_action(Action(vector<size_t>(1,i),pt_stab,m_parent_group,true));
//   }

//   cout << "with background sym " << endl;
//   cout << *this << endl << endl << endl;
}

// 	// now we need to add any symmetries that stabilize the clause
// 	set<Column> clause_stab = global_group[i].clause_stabilizer(c);
// 	if (clause_stab.size() > 1)
// 	  add_action(Action(vector<size_t>(1,i),clause_stab,m_parent_group,true));


void ProductSubgroup::build_orbits()
{ 
  vector<Literal> working_orbit;
  Set<Literal> open, closed;
  vector<Literal> c = m_parent_group->get_all_literals();
  
  for (size_t i=0; i < c.size(); i++) open.insert(c[i]);
  
  while (!open.empty()) {
    Literal point = open.pop();
    working_orbit.push_back(point);
    closed.insert(point);

    for (size_t k=0; k < working_orbit.size(); k++) {
      point = working_orbit[k];
      for (size_t i=0; i < size(); i++) {
		vector<Literal> orbit = m_actions[i]->orbit(point);
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


vector<set<Literal> > ProductSubgroup::orbits(const vector<Literal>& c)
{ 
  vector<set<Literal> > answer;
  set<Literal> working_orbit;
  Set<Literal> open, closed;

  for (size_t i=0; i < c.size(); i++) open.insert(c[i]);
  
  while (!open.empty()) {
    Literal point = open.pop();
    working_orbit.insert(point);
    closed.insert(point);

	set<Literal>::iterator it = working_orbit.begin();
    for ( ; it != working_orbit.end(); it++) {
      point = *it;
      for (size_t i=0; i < size(); i++) {
		vector<Literal> orbit = m_actions[i]->orbit(point);
		for (size_t j=0; j < orbit.size(); j++) {
		  if (!closed.contains(orbit[j])) {
			working_orbit.insert(orbit[j]);
			open.remove(orbit[j]);
			closed.insert(orbit[j]);
		  }
		}
      }
    }
	
    answer.push_back(working_orbit);
    working_orbit.clear();
  }
  return answer;
}

void ProductSubgroup::identify_background_symmetry(const Clause& c, vector<bool>& b) const
{
  b = vector<bool>(m_actions.size(),false);
  for (size_t i=0; i < m_actions.size(); i++) {
	bool moves = false;
	for (size_t j=0; j < c.size(); j++) {
	  if (!m_actions[i]->fixes(c[j])) {
		moves = true;
		break;
	  }
	}
	b[i] = !moves;    // first just the background sym
  }

  set<Literal> c_set(c.begin(),c.end());   // this assumes all the actions are independent
  for (size_t i=0; i < m_actions.size(); i++) {
	if (m_actions[i]->stabilizes_set(c_set)) b[i] = true;
  }
}

} // end namespace zap
