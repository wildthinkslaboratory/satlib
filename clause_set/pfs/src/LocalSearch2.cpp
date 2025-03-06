#include "LocalSearch2.h"


namespace zap 
{
long unsigned LocalSearchStatistics::touches = 0;

bool LocalSearchAction::can_be_mapped(size_t index, const vector<Column>& mapping)
{
  vector<size_t>& p = lit_position_map[index];    // get the positions for index
  for (size_t i=0; i < p.size(); i++) {
	if (!PermCounterWithFixing::can_set_value(p[i],column_value_map[mapping[i]])) {
	  return false;
	}
  }
  return true;
}


bool LocalSearchAction::can_be_mapped2(size_t index, const vector<Column>& mapping)
{
  vector<size_t>& p = lit_position_map[index];    // get the positions for index
  for (size_t i=0; i < p.size(); i++) {
	if (!PermCounterWithFixing::can_set_value2(p[i],column_value_map[mapping[i]])) {
	  return false;
	}
  }
  return true;
}


void LocalSearchAction::apply_mapping(size_t index,const vector<Column>& mapping)
{
  vector<size_t>& p = lit_position_map[index]; // get the positions for the index

  for (size_t i=0; i < p.size(); i++) {
	bool success = PermCounterWithFixing::set_value(p[i],column_value_map[mapping[i]]);
	if (!success) quit("failure to set value in apply_mapping");
  }



  for (size_t j=0; j < size(); j++) current_columns[j] = operator[](j);

 
}

// void LocalSearch2::apply_mapping(size_t index, Literal l)
// {
//   vector<size_t>& a = s.lit_action_map[index];
//   for (size_t i=0; i < a.size(); i++) {
// 	vector<Column> c = (*transports)[a[i]].action().get_columns(l);   // get the columns
// 	s.actions[a[i]].apply_mapping(index,c);                           // apply mapping to each column
//   }
// }

void LocalSearch2::apply_mapping(size_t index, Literal l)
{ 
  for (size_t i=1; i < transports->size(); i++) {
	vector<Column> c = (*transports)[i].action().get_columns(l);   // get the columns
	s.actions[i].apply_mapping(index,c);                           // apply mapping to each column
  }
}


void LocalSearch2::initialize(PfsTransportVector* t, const Ptr<ProductSubgroup>& pps)
{
  group = pps;
  transports = t;
  s.clause = Clause(transports->clause_rep());
  s.image  = Clause(transports->clause_rep());
  s.lit_action_map = vector<vector<size_t> >(s.clause.size(),vector<size_t>());
  s.fixed_lits = vector<bool>(s.clause.size(),false);
  if (transports->empty()) return;
  for (size_t i=0; i < s.clause.size(); i++) {
	for (size_t j=1; j < transports->size(); j++) {
	  if (!transports->operator[](j).action().fixes(s.clause[i])) { 
		s.lit_action_map[i].push_back(j); 
	  } 
	} 
  } 

  for (size_t i=0; i < transports->size(); i++) {
	if (i == 0) s.actions.push_back(LocalSearchAction());
	else {
	  vector<vector<Column> > lit_column_map(s.clause.size());
	  for (size_t j=0; j < s.clause.size(); j++) {
		lit_column_map[j] = (*transports)[i].action().get_columns(s.clause[j]);
	  }
	  s.actions.push_back(
		LocalSearchAction((*transports)[i].first_rep().size(),i,(*transports)[i].columns(),lit_column_map)
						  );
	}
  }

//   for (size_t i=0; i < s.clause.size(); i++) {
// 	lit_orbit_map.push_back(transports->orbit(vector<Literal>(1,s.clause[i]),0));
//   }
//   cout << "orbit map size " << lit_orbit_map.size() << endl;
//   for (size_t i=0; i < s.clause.size(); i++) {
// 	cout << s.clause[i] << "    " << Clause(lit_orbit_map[i]) << endl;
//   }
}




bool LocalSearch2::try_to_map(size_t index, Literal l)
{
  for (size_t i=1; i < transports->size(); i++) {
	vector<Column> c = (*transports)[i].action().get_columns(l);
	if (c.empty()) continue;
	if (!s.actions[i].can_be_mapped(index,c)) return false;
  }
  apply_mapping(index,l);
  return true;
}

Literal LocalSearch2::map(size_t index)
{
  Literal l = s.clause[index];
  for (size_t i=1; i < s.actions.size(); i++)
	l = (*transports)[i].action().image(l,s.actions[i].from(),s.actions[i].to());
  
  return l;
}


void LocalSearch2::map_clause()
{
  for (size_t i=0; i < s.clause.size(); i++) {
	s.image[i] = s.clause[i];
	for (size_t j=1; j < s.actions.size(); j++)
	  s.image[i] = (*transports)[j].action().image(s.image[i],s.actions[j].from(),s.actions[j].to());
  }
}


Literal LocalSearch2::score_clause(const Assignment& P)
{
  s.score.clear();
  Literal deepest_lit = s.image[0];
  Literal unit_lit;
  for (size_t i=0; i < s.image.size(); i++) {
	if (P.position(deepest_lit.variable()) < P.position(s.image[i].variable())) deepest_lit = s.image[i];
	if (P.value(s.image[i].variable()) == UNKNOWN) {
	  s.score[UNVALUED]++;
	  unit_lit = s.image[i];
	}
	else
	  if (P.value(s.image[i].variable()) == s.image[i].sign()) s.score[SATISFIED]++;
	  else s.score[UNSATISFIED]++;
  }

  if (s.score.is_unsat()) return deepest_lit;
  return unit_lit;
}


bool LocalSearch2::try_to_map_negated_lits(size_t index, vector<vector<Literal> > lit_orbit_map, Assignment& P)
{
  if (index >= s.clause.size()) return true;
  if (s.is_fixed(index)) {
	Literal l = map(index);
	if (P.value(l.variable()) == UNSATISFIED) return try_to_map_negated_lits(index+1,lit_orbit_map,P);
	else return false;
  }

  do {
	size_t random_index = rand() % lit_orbit_map[index].size();        // pick a random literal and
	Literal r = lit_orbit_map[index][random_index];                    // remove it from the list
	lit_orbit_map[index][random_index] = lit_orbit_map[index].back();
	lit_orbit_map[index].pop_back();
	
	bool mapping_found = try_to_map(index,r); // see if there is a mapping
	if (mapping_found) {
	  s.fix_literal(index);
	  return try_to_map_negated_lits(index+1,lit_orbit_map,P);
	}
  } while (!lit_orbit_map[index].empty());
  
  return false;
}

// size_t number_iterations = 5;
// bool LocalSearch2::get_symmetric_unit_lits(Assignment& P, ClauseID id) {

// //   cout << P << endl << s.clause  << endl;
//   s.reset();
//   int unit_lit_index = -1;
//   vector<size_t> negated_lits;
//   size_t deepest = 0;
//   for (size_t i=0; i < s.clause.size(); i++) {
// 	if (P.value(s.clause[i].variable()) == UNKNOWN) 
// 	  unit_lit_index = i;
// 	else if (!s.clause[i].sign()) negated_lits.push_back(i);
// 	if (P.position(s.clause[i].variable()) > P.position(s.clause[deepest].variable())) deepest = i;
//   }
//   if (unit_lit_index == -1) unit_lit_index = deepest;

//   vector<vector<Literal> > lit_orbit_map_reduced(s.clause.size());
//   for (size_t k =0; k < number_iterations; k++) {
// 	// fix negated lits (unless it is the unit lit)
// 	for (size_t i=0; i < negated_lits.size(); i++) s.fix_liter(P.value(s.image[i].variable()) == s.image[i].sign()al(i);
// 	Neighborhood neighborhood = s.neighborhood(unit_lit_index);
// 	do {
// 	  // check the unit lit, if it is SAT then continue
// 	  Literal l = map(unit_lit_index);
// 	  cout << "unit lit " << s.clause[unit_lit_index] << " mapped to " << l << endl;
// 	  if (P.value(l.variable()) == (size_t)(l.sign())) continue;
	  
// 	  // go ahead and map the whole clause
// 	  map_clause();
//  	  cout << s.image << endl;
	  
// 	  // if it's unit then add it to the partial assignment
// 	  Literal u = score_clause(P);
// 	  stats.touches++;
// 	  if (s.score.is_unit() || s.score.is_unsat()) {
// 		cout << "                                                      IS UNIT " << endl;
// 		stats.unit_clauses_found++;
// 		bool extend_succeeded = P.extend(AnnotatedLiteral(u,Reason(id,s.image)));
// 		if (!extend_succeeded) return false;
// 	  }
	  
// 	} while (++neighborhood);
	
// 	if (negated_lits.empty()) break;
// 	else {

// 	  if (k==0) {
// 		for (size_t i=0; i < lit_orbit_map.size(); i++) {
// 		  for (size_t j=0; j < lit_orbit_map[i].size(); i++) {
// 			if (lit_orbit_map[i][j].sign()) break; // ignore orbits of positive literals
// 			if (P.value(lit_orbit_map[i][j].variable()) == SATISFIED)
// 			  lit_orbit_map_reduced[i].push_back(lit_orbit_map[i][j]);
// 		  }
// 		}
// 	  }
	  
// 	  for (size_t i=0; i < negated_lits.size(); i++) s.unfix_literal(i);  	  // unfix the negated literals
// 	  bool mapping_found = try_to_map_negated_lits(0,lit_orbit_map_reduced,P);
// 	}
//   }
  
//   return true;
// }


bool LocalSearch2::get_symmetric_unit_lits(Assignment& P, ClauseID id) {

//   cout << P << endl << "clause " << s.clause  << endl;

  s.reset();                                           // find the unit literal
  int unit_lit_index = -1;
  vector<size_t> negated_lits;
  size_t deepest = 0;
  for (size_t i=0; i < s.clause.size(); i++) {
	if (P.value(s.clause[i].variable()) == UNKNOWN) 
	  unit_lit_index = i;
	else if (!s.clause[i].sign()) negated_lits.push_back(i);
	if (P.position(s.clause[i].variable()) > P.position(s.clause[deepest].variable())) deepest = i;
  }
  if (unit_lit_index == -1) {  // clause is unsat we need to keep backing up
	unit_lit_index = deepest;
	stats.unit_clauses_found++;
	bool extend_succeeded = P.extend(AnnotatedLiteral(s.clause[unit_lit_index],Reason(id,s.clause)));
	if (!extend_succeeded) return false;
  }

  vector<vector<Literal> > candidates(s.clause.size());
  for (size_t i=0; i < s.clause.size(); i++) {       // reduce orbits to good candidate mappings
	Literal l = s.clause[i];                              // using the partial assignment
	const vector<Literal>& o = group->orbit(l);
	for (size_t j=0; j < o.size(); j++) {
	  if (i == unit_lit_index) {
		if (P.value(o[j].variable()) == UNKNOWN)
		  candidates[i].push_back(o[j]);
	  }
	  else if (P.value(o[j].variable()) != l.sign() && P.value(o[j].variable()) != UNKNOWN)
		candidates[i].push_back(o[j]);
	}
  }

//   for (size_t i=0; i < candidates.size(); i++) {
// 	cout << "candidates for " << s.clause[i] <<  " : " << Clause(candidates[i]) << endl;
//   }

//   cout << "orbit for unit lit " << Clause(group->orbit(s.clause[unit_lit_index])) << endl;
//   cout << "candidates for unit lit " << Clause(candidates[unit_lit_index]) << endl;
  Literal l = s.clause[unit_lit_index];
  bool first_time_through = true;
  do
  {
	if (!first_time_through) {
	  l = candidates[unit_lit_index].back();          // map and fix the unit lit to one of it's candidates
	  candidates[unit_lit_index].pop_back();                  // remove that lit from the candidate list
	}
	else first_time_through = false;
	
	s.reset();
	apply_mapping(unit_lit_index,l);
	s.fix_literal(unit_lit_index);
// 	cout << "mapping " << s.clause[unit_lit_index] << " to " << l << endl;
	
	for (size_t j=0; j < global_vars.fix_attempts; j++) {
	  map_clause();

// 	  cout << "Clause " << s.image << endl;
	  
	  Literal u = score_clause(P);	  
	  stats.touches++;
	  
	  if (s.score.is_unit() || s.score.is_unsat()) {
// 		cout << "                                                      IS UNIT " << endl;
		stats.unit_clauses_found++;
		bool extend_succeeded = P.extend(AnnotatedLiteral(u,Reason(id,s.image)));
		if (!extend_succeeded) return false;

// 		// try and refine the search near this solution.
// // 		cout << "Searching neighborhood " << endl;
// 		s.unfix_literal(unit_lit_index);
// 		for (size_t i=0; i < negated_lits.size(); i++) {
// 		  if (negated_lits[i] != unit_lit_index) s.fix_literal(negated_lits[i]);
// 		}
// 		Neighborhood n (&s,transports,u,unit_lit_index);
// 		do {
// 		  map_clause();
// // 		  cout << "Clause " << s.image << endl;
// 		  Literal h = score_clause(P);
// 		  stats.touches++;
// 		  if (s.score.is_unit() || s.score.is_unsat()) {
// // 			cout << "                                                      IS UNIT " << endl;
// 			stats.unit_clauses_found++;
// 			// remove h from the candidate list
// 			for (size_t j=0; j < candidates[unit_lit_index].size(); j++) {
// 			  if (candidates[unit_lit_index][j] == h) {
// 				candidates[unit_lit_index][j] = candidates[unit_lit_index].back();
// 				candidates[unit_lit_index].pop_back();
// 				break;
// 			  }
// 			}
// 			bool extend_succeeded = P.extend(AnnotatedLiteral(h,Reason(id,s.image)));
// 			if (!extend_succeeded) return false;
// 		  }
		  
// 		} while(++n);
		break;
	  }
	  else {

		vector<size_t> broken;                              // try to fix the mapping
		for (size_t i=0; i < s.image.size(); i++)           // figure out what's broken
		  if (i != unit_lit_index)
			if (P.value(s.image[i].variable()) != (size_t)(!s.image[i].sign())) broken.push_back(i);

		for (size_t j=0; j < global_vars.map_attempts; j++) {
		  size_t b = broken[rand() % broken.size()];                 // randomly pick a broken lit
		  Literal l = candidates[b][rand() % candidates[b].size()];  // randomly pick a candidate mapping
// 		  cout << "try to map " << s.image[b] << " to " << l << endl;
		  if (try_to_map(b,l)) break;
		}
	  }
	  
	}

  }  while (!candidates[unit_lit_index].empty());

  return true;
}


} // end namespace zap 

