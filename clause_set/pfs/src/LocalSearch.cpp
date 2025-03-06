#include "LocalSearch.h"
#include "Counter.h"

namespace zap 
{

void LocalSearch::initialize(PfsTransportVector* t) 
{
  transports = t;
  if (transports->empty()) return;
  base_solution.clause = Clause(transports->clause_rep());
  lits_to_choices = vector<vector<size_t> >(base_solution.clause.size(),vector<size_t>());
  for (size_t i=0; i < base_solution.clause.size(); i++) {    
	for (size_t j=1; j < transports->size(); j++) { 
	  if (!transports->operator[](j).action().fixes(base_solution.clause[i])) { 
		lits_to_choices[i].push_back(j); 
		//	choices_to_lits[j].push_back(i); 
	  } 
	} 
  } 

  for (size_t i=0; i < transports->size(); i++) 
	base_solution.choices.push_back((*transports)[i].first_rep()); 
  best_solution = base_solution; 

  Clause c = base_solution.clause;
  sort(c.begin(),c.end());
  vector<Literal> universe = transports->universe();
  fixed_lit_index = vector<FixedLitReps>(universe.back().variable() + 1);
  vector<vector<Literal> > orbits = transports->orbits();


  for (size_t i=0; i < orbits.size(); i++) {
	set<Literal> c_int_orbit; 
	set_intersection(c.begin(),c.end(),orbits[i].begin(),orbits[i].end(),inserter(c_int_orbit,c_int_orbit.begin())); 
	for (size_t j=0; j < orbits[i].size(); j++) { 
	  Literal l = orbits[i][j];
	  set<Literal>::iterator it = c_int_orbit.begin(); 
	  for ( ; it != c_int_orbit.end(); it++) { 
		Solution s =  map_solution(*it,l,base_solution);
		fixed_lit_index[l.variable()].get_reps(l.sign()).push_back(s); 
	  } 
	} 
  }

}

// shouldn't this be void?
Column map_and_fix_choice(Choice& choice,const vector<Column>& before,const vector<Column>& after)
{
  for (size_t k=0; k < choice.size(); k++) {
	Column& c = choice[k];
	
	// specialized code
	if (before.size() == 1) {
	  if (before[0] == c) {
		c = after[0];
		choice.fixed[k] = true;
	  }
	  else
		if (after[0] == c) c = before[0];
	  continue;
	}

	bool done = false;
	for (size_t i=0; i < before.size(); i++) 
	  if (before[i] == c) {
		c = after[i];
		choice.fixed[k] = true;
		done = true;
		break;
	  }
	if (done) continue;

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
		c = pre;
	  }
	}
  }
}


Solution LocalSearch::map_solution(Literal from, Literal to, const Solution& base)
{
  Solution answer = base;
 
  for (size_t i=1; i < transports->size(); i++) {
	vector<Column> columns_from = (*transports)[i].action().get_columns(from);
	vector<Column> columns_to = (*transports)[i].action().get_columns(to);
	Choice& choice = answer.choices[i];
	if (columns_from.empty() || columns_to.empty()) continue;

	map_and_fix_choice(choice,columns_from,columns_to);
	
	for (size_t j=0; j < answer.clause.size(); j++)
	  answer.clause[j] = (*transports)[i].action().image(answer.clause[j],choice,base_solution.choices[i]);
  }

  return answer;
}

void LocalSearch::map_clause(Solution& s)
{
  for (size_t l=0; l < s.clause.size(); l++) {
	s.clause[l] = base_solution.clause[l];
	for (size_t i=1; i < s.choices.size(); i++) {
	  s.clause[l] = (*transports)[i].action().image(s.clause[l],base_solution.choices[i],s.choices[i]);
	}
  }
}

// Literal LocalSearch::map_literal(size_t index, size_t choice_index, Choice& choice)
// {
//   return (*transports)[i].action().image(base_solution.clause[index],base_solution.choices[choice_index],choice);
// }

long unsigned LocalSearch::touches = 0;

bool LocalSearch::k_transporter(Assignment& P, Literal l, ClauseID id)
{
  return true;
  return k_transporter_incomplete(P,l,id);
  Assignment copy_P = P;
  size_t initial_size = P.unit_list_size();
  double start_time, complete_time, incomplete_time;
  start_time = get_cpu_time();
  bool complete_result = k_transporter_complete(P,l,id);
  complete_time = get_cpu_time() - start_time;
  start_time = get_cpu_time();
  bool incomplete_result = k_transporter_incomplete(copy_P,l,id);
  incomplete_time = get_cpu_time() - start_time;
  size_t complete_ups = copy_P.unit_list_size() - initial_size;
  size_t incomplete_ups = P.unit_list_size() - initial_size;
  cout << "*////////////////////  UP Comparison /////////////////////////*" << endl;
  cout << id << "\t" << complete_result << "\t" << complete_ups << "\t" << complete_time << "\t"
  	   << incomplete_result << "\t" << incomplete_ups  << "\t" << incomplete_time << endl;
  
  return complete_result;
}


bool LocalSearch::k_transporter_incomplete(Assignment& P, Literal l, ClauseID id) 
{
  double start_time = get_cpu_time();
  touches = 0;
  stats.clear();
  best_solution = base_solution; // reset the best solution
  if (fixed_lit_index.size() == 0) {
	cerr << "uninitialized index " << endl;
	exit(1);
  }
  if (l.variable() > fixed_lit_index.size()) return true;
  vector<Solution> candidate_list = fixed_lit_index[l.variable()].get_reps(l.sign());

  //   cout << "incomplete k-transport " << l << endl;
//   cout << "all candidates " << endl;
//   for (size_t i=0; i < candidate_list.size(); i++) cout << candidate_list[i] << endl;
   
  for (size_t i=0; i < parameters.number_iterations; i++) { 
 	for (size_t j=0; j < candidate_list.size(); j++) {
	  Solution& candidate = candidate_list[j];  // look at all neighbors
 	  for (size_t k=1; k < candidate.choices.size(); k++) {
		PermCounterWithFixing counter(candidate.choices[k].size(),(*transports)[k].columns().size());
		for (size_t l=0; l < candidate.choices[k].size(); l++) 
		  if (candidate.choices[k].fixed[l]) { counter.fix(l,candidate.choices[k][l]);}
		do {
		  if ((get_cpu_time() - start_time) > 0.5) return true; // time out
		  stats.clauses_touched++;
		  Solution neighbor = candidate; 
		  neighbor.score.clear(); 
		  for (size_t m=0; m < counter.size(); m++) neighbor.choices[k][m] = (*transports)[k].columns()[counter[m]]; 
		  map_clause(neighbor);
		  Literal l = score(neighbor,P);
		  ++touches;
// 		  cout << "\t" << neighbor << endl;
		  if (neighbor.score.is_unit() || neighbor.score.is_unsat()) {
			stats.unit_clauses_found++;
// 			cout << "unit prop in local search k-trans " << l <<  " from instance " << neighbor.clause << endl;
			bool extend_succeeded = P.extend(AnnotatedLiteral(l,Reason(id,neighbor.clause)));
			if (!extend_succeeded) return false;
// 			cout << neighbor << endl;
// 			cout << P << endl;
		  }
		  if (candidate.score < neighbor.score) candidate = neighbor;  
		} while (++counter); 
	  }	 
	} 
  } 
  return true;
} 

double complete_start_time;
bool LocalSearch::get_symmetric_unit_lits(Assignment& P,ClauseID id)
{
//     cout <<"transpors " << endl;
//   for (size_t i=0; i < transports->size(); i++) cout << transports->operator[](i) << endl;

  Assignment copy_P = P;
  size_t initial_size = P.unit_list_size();
  double start_time, complete_time, incomplete_time;
  start_time = get_cpu_time();
  complete_start_time = get_cpu_time();

//   cout << "base clause      ";
//   for (size_t i=0; i < base_solution.choices.size(); i++) cout << base_solution.choices[i] << "  ";
//   cout << "/t"<< base_solution.clause << endl << endl;

  
  bool complete_result = expand_node(P,id,1,base_solution);
  complete_time = get_cpu_time() - start_time;
  start_time = get_cpu_time();
  bool incomplete_result = symmetric_lits_inner_call2(copy_P,id);
  incomplete_time = get_cpu_time() - start_time;
  size_t complete_ups = P.unit_list_size() - initial_size;
  size_t incomplete_ups = copy_P.unit_list_size() - initial_size;
//   cout << "complete " << P << endl << "incomplete " << copy_P << endl;
  cout << "#///////////////////  Sym Lits Comparison /////////////////////////*" << endl;
  cout << id << "\t" << complete_result << "\t" << complete_ups << "\t"  << complete_time << "\t"
  	   << incomplete_result << "\t" << incomplete_ups  << "\t" << incomplete_time << endl;
  
  return complete_result; 

}

bool LocalSearch::symmetric_lits_inner_call2(Assignment& P, ClauseID id)
{
  // find the index of the unit lit
  Clause& c = base_solution.clause;
  size_t unit_lit_index = 0;
  for (size_t i=0; i < c.size(); i++) {
	if (P.value(c[i].variable()) == UNKNOWN) {
	  unit_lit_index = i;
	  break;
	}
  }

  vector<size_t> active = lits_to_choices[unit_lit_index];
//   for (size_t i=0; i < active.size(); i++) {
// 	Solution neighbor = base_solution;
// 	neighbor.score.clear();
// 	size_t k = active[i];
// 	PermCounterWithFixing counter(base_solution.choices[k].size(),(*transports)[k].columns().size());
// 	++counter;
// 	for (size_t m=0; m < counter.size(); m++) neighbor.choices[k][m] = (*transports)[k].columns()[counter[m]]; 
// 	map_clause(neighbor);
// 	Literal l = score(neighbor,P);
// 	++touches;
// 	if (!neighbor.score.is_unit() && !neighbor.score.is_unsat()) {
// 	  active[i] = active.back();
// 	  active.pop_back();
// 	  --i;
// 	}
//   }

//   cout << "active  ";
//   for (size_t i=0; i < active.size(); i++) cout << active[i] << ' ';
//   cout << endl;
  return expand_node(P,id,0,base_solution,active);
  
  
}



bool LocalSearch::symmetric_lits_inner_call(Assignment& P, ClauseID id)
{
  //cout << endl << "symmetric lit attempt " << endl;
  vector<Solution> candidate_list = vector<Solution>(1,base_solution);

  double start_time = get_cpu_time();
  
  for (size_t i=0; i < 1; i++) { 
 	for (size_t j=0; j < candidate_list.size(); j++) {
	  Solution& candidate = candidate_list[j];  // look at all neighbors
 	  for (size_t k=1; k < candidate.choices.size(); k++) { 
		PermCounterWithFixing counter(candidate.choices[k].size(),(*transports)[k].columns().size());
// 		for (size_t l=0; l < candidate.choices[k].size(); l++) 
// 		  if (candidate.choices[k].fixed[l]) { cout << " shouldn't be fixing " << endl; counter.fix(l,candidate.choices[k][l]);}
		do {
 		  if ((get_cpu_time() - start_time) > 5) return true; // time out
		  stats.clauses_touched++;
		  Solution neighbor = candidate; 
		  neighbor.score.clear(); 
		  for (size_t m=0; m < counter.size(); m++) neighbor.choices[k][m] = (*transports)[k].columns()[counter[m]]; 
		  map_clause(neighbor);
		  Literal l = score(neighbor,P);
		  ++touches;
//  		  cout << "symmetric lits " << neighbor << endl;
		  if (neighbor.score.is_unit() || neighbor.score.is_unsat()) {
			stats.unit_clauses_found++;
			bool extend_succeeded = P.extend(AnnotatedLiteral(l,Reason(id,neighbor.clause)));
// 			cout << "get sym lits extend ";
// 			for (size_t i=0; i < neighbor.choices.size(); i++) cout << neighbor.choices[i] << "  ";
// 			cout << "lit " << l << "  ";
// 			cout << neighbor.clause << endl;
			if (!extend_succeeded) return false;
// 			cout << "unit prop " << l <<  " from instance " << neighbor.clause << endl;
		  }
		  //		  if (candidate.score < neighbor.score) candidate = neighbor;  
		} while (++counter); 
	  }	 
	} 
  }


  return true;
}



bool LocalSearch::k_transporter_complete(Assignment& P, Literal l, ClauseID id) 
{
  complete_start_time = get_cpu_time();
//   cout << "COMPLETE K-TRANSPORT" << endl;
  touches = 0;
  stats.clear();
  best_solution = base_solution; // reset the best solution
  if (fixed_lit_index.size() == 0) {
	cerr << "uninitialized index " << endl;
	exit(1);
  }
  if (l.variable() > fixed_lit_index.size()) return true;
  vector<Solution> candidate_list = fixed_lit_index[l.variable()].get_reps(l.sign());

 
//   for (size_t i=0; i < candidate_list.size(); i++) {
// 	Solution candidate = candidate_list[i];  // 
// 	if (!expand_node(P,id,1,candidate)) return false;
//   }

  size_t r = expand_node(P,id,1,base_solution);
  if (r == 0) return false;  // contradiction
  return true; // success or time out
}


size_t LocalSearch::expand_node(Assignment& P, ClauseID id, size_t depth, Solution candidate) 
{
  if ((get_cpu_time() - complete_start_time) > 5) return 2;
  if (depth == candidate.choices.size()) {
	map_clause(candidate);
	//	cout << "expand node " << candidate.clause << " depth " << depth << endl;
	candidate.score.clear();
	Literal l = score(candidate,P);
	if (candidate.score.is_unit() || candidate.score.is_unsat()) {
	  stats.unit_clauses_found++;
//   	  cout << "        expand node ";
// 	  for (size_t i=0; i < candidate.choices.size(); i++) cout << candidate.choices[i] << "  ";
// 	  cout << "lit " << l << "  ";
// 	  cout << "/t"<< candidate.clause << endl;
	  bool extend_succeeded = P.extend(AnnotatedLiteral(l,Reason(id,candidate.clause)));
	  if (!extend_succeeded) {
		//		cout << "contradiction" << endl;
		return false;
	  }
	}
	return true;
  }

  PermCounterWithFixing counter(candidate.choices[depth].size(),(*transports)[depth].columns().size());
  do {
	//	cout << "counter " << counter << " " << flush;
	for (size_t m=0; m < counter.size(); m++) candidate.choices[depth][m] = (*transports)[depth].columns()[counter[m]];
	size_t r = expand_node(P,id,depth+1,candidate);
	if (r == 0) return false;
	if (r == 2) return 2;
// 	expand_node(P,id,depth+1,candidate);
  } while (++counter);

  return true;
}


size_t LocalSearch::expand_node(Assignment& P, ClauseID id, size_t depth, Solution candidate, vector<size_t>& choices) 
{
  if (depth == choices.size()) {
	map_clause(candidate);
	//	cout << "expand node " << candidate.clause << " depth " << depth << endl;
	candidate.score.clear();
	Literal l = score(candidate,P);
	if (candidate.score.is_unit() || candidate.score.is_unsat()) {
	  stats.unit_clauses_found++;
//   	  cout << "  NEW   expand node ";
// 	  for (size_t i=0; i < candidate.choices.size(); i++) cout << candidate.choices[i] << "  ";
// 	  cout << "lit " << l << "  ";
// 	  cout << "/t"<< candidate.clause << endl;
	  bool extend_succeeded = P.extend(AnnotatedLiteral(l,Reason(id,candidate.clause)));
	  if (!extend_succeeded) {
		//		cout << "contradiction" << endl;
		return false;
	  }
	}
	return true;
  }

  PermCounterWithFixing counter(candidate.choices[choices[depth]].size(),(*transports)[choices[depth]].columns().size());
  do {
	//	cout << "counter " << counter << " " << flush;
	for (size_t m=0; m < counter.size(); m++) candidate.choices[choices[depth]][m] = (*transports)[choices[depth]].columns()[counter[m]];
	size_t r = expand_node(P,id,depth+1,candidate,choices);
	if (r == 0) return false;
// 	expand_node(P,id,depth+1,candidate);
  } while (++counter);

  return true;
}

} // end namespace zap 

