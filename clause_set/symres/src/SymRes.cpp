#include "SymRes.h"


namespace zap
{

SymRes::SymRes(const GlobalProductGroup& gpg)
  :  m_global_group(gpg), m_num_asserting_clauses(0)
{

}

void SymRes::initialize_clause_set(vector<PfsClause>& clauses)
{
   vector<Clause> cnf;
   for (size_t i=0; i < clauses.size()-1; i++) {
      vector<Clause> clause_cnf;
      clauses[i].get_ground_clauses(clause_cnf);
      for (size_t j=0; j < clause_cnf.size(); j++) {
         cnf.push_back(clause_cnf[j]);
         cnf.back().group_identifier = clauses[i].group_identifier;
      }
   }
   
   Cnf::initialize_clause_set(cnf);
}

ClauseID SymRes::resolve(const Reason& r1, const Reason& r2, const Assignment& P, size_t& lits_this_level,
					  Literal& unit_lit)
{
  //  cout << "clause id " << r1.id() << " , " << r2.id() << "   size " << size() << endl;
  if (r1.id() == NULL_ID) {
    CnfClause& c = operator[](r2.id());
    // for (size_t i=0; i < c.size(); i++) {
    //   cout << c[i] << ":" << P.value(c[i].variable()) << ":" << P.decision_level(c[i].variable()) << ' ';
    // }
    for (size_t i=0; i < c.size(); i++) 
      if (c[i].variable() == 0 || P.value(c[i].variable()) == UNKNOWN)
	exit(1);
  }
   if (r1.id() == NULL_ID) return r2.id();
   if (r2.id() == NULL_ID) return r1.id();

   CnfClause& c1 = operator[](r1.id());
   CnfClause& c2 = operator[](r2.id());
   // cout << "clause id " << r1.id() << " , " << r2.id() << "   size " << size() << endl;
   // cout << "resolving " << endl << c1 << endl << c2 << endl;
   // cout << "id 1 " << c1.group_identifier << endl;
   // cout <<  "id 2  " << c2.group_identifier << endl;
   // for (size_t i=0; i < c1.size(); i++) {
   //    cout << c1[i] << ":" << P.value(c1[i].variable()) << " ";
   // }
   // cout << endl;
   // for (size_t i=0; i < c2.size(); i++) {
   //    cout << c2[i] << ":" << P.value(c2[i].variable()) << " ";
   // }
   // cout << endl;

   ClauseID id = Cnf::resolve(r1,r2,P,lits_this_level,unit_lit);
   // cout << "resolvent " << resolve_clause << endl;
   // cout << "resolvent ";

   // for (size_t i=0; i < resolve_clause.size(); i++) {
   //   cout << resolve_clause[i] << ":" << P.value(resolve_clause[i].variable()) << ":" << P.current_level() - P.decision_level(resolve_clause[i].variable()) << " ";
   // }
   for (size_t i=0; i < resolve_clause.size(); i++) 
     if (P.value(resolve_clause[i].variable()) == resolve_clause[i].sign()) {
       cout << "sat resolvent" << endl;
       exit(1);
     }

   //   cout << endl << endl;;
   // cout << "back in resolve " << back() << endl;
   if (c1.group_identifier == c2.group_identifier)
      operator[](id).group_identifier = c1.group_identifier;
   return id;
}

bool is_unsat(const Assignment& P, const CnfClause& c) 
{
   for (size_t i=0; i < c.size(); i++) {
      if (P.value(c[i].variable()) == UNKNOWN) return false;
      if (P.value(c[i].variable()) == c[i].sign()) return false;
   }
   return true;
}

size_t SymRes::position(Literal l, const Assignment& P) const
{
  int pos = P.position(l.variable());
  if (pos == NULL_ID) return m_num_vars + 1;
  return pos;
}

size_t SymRes::decision_level(Literal l, const Assignment& P) const
{
  int level = P.decision_level(l.variable());
  if (level == NULL_ID) return m_num_vars + 1;
  return level;
}

bool better_watcher(Literal l1, Literal l2, const Assignment& P)
{
  if (P.value(l1.variable()) == l1.sign()) {
    if (P.value(l2.variable()) == l2.sign()) 
      return P.position(l1.variable()) < P.position(l2.variable());
    return true;
  }
  if (P.value(l2.variable()) == l2.sign()) return false;
  if (P.value(l1.variable()) == UNKNOWN) return true;
  if (P.value(l2.variable()) == UNKNOWN) return false;
  return P.position(l1.variable()) > P.position(l2.variable());
}


  // is the assertion level related to the best watchers?

void SymRes::add_learned_clause(ClauseID c_id, const Assignment& P)
{
  m_num_asserting_clauses = 1;
  // cout << P << endl << operator[](c_id) << endl;
   if (operator[](c_id).size() > global_vars.symres_bound || operator[](c_id).group_identifier == "") {
      Cnf::add_learned_clause(c_id,P);
      return;
   }
   first_round = true;
   seen.clear();
   
   // cout << "adding learned clause with symmetry " << operator[](c_id) << endl;

   if (c_id != (int)(size()-1)) return;

   CnfClause c = operator[](c_id);
   pop_back(); 
   PfsClause pfsClause(c,lookup_group(c.group_identifier)); 
   pfsClause.build_transports();
   vector<Clause> ground_clauses;
   pfsClause.get_ground_clauses(ground_clauses);
   
   vector<BackupClause> backups;
   if (c.size() > 1) {
      for (size_t j=0; j < ground_clauses.size(); j++) {
         backups.push_back(BackupClause(ground_clauses[j]));
         BackupClause& bc = backups.back();

	 size_t watcher = 0;
	 size_t num_sat = 0;
	 for (size_t i=0; i < bc.size(); i++) { // find the first watcher
	   if (P.value(bc[i].variable()) == bc[i].sign()) ++num_sat;
	   if (position(bc[i],P) > position(bc[watcher],P)) watcher = i;    
	 }
	 Literal temp = bc[0]; bc[0] = bc[watcher]; bc[watcher] = temp; // put it in the first array position
         
	 watcher = 1;
	 for (size_t i=1; i < bc.size(); i++)  // find the second watcher
	   if (position(bc[i],P) > position(bc[watcher],P)) watcher = i;
	 temp = bc[1]; bc[1] = bc[watcher]; bc[watcher] = temp;
         
	 // now compute the assertion level
	 if (num_sat > 1) 
	   bc.assertion_level =  m_num_vars + 1;
	 else {
	   if (num_sat == 1 && !(P.value(bc[0].variable()) == bc[0].sign()))
	     bc.assertion_level = m_num_vars + 1;
	   else
	     bc.assertion_level = decision_level(bc[1],P);
	 }
      }
      sort(backups.begin(), backups.end()); // sort based on backtrack point
   }
   else 
      for (size_t j=0; j < ground_clauses.size(); j++) backups.push_back(BackupClause(ground_clauses[j]));
   
   //   cout << P << endl;
   // for (size_t i=0; i < backups.size(); i++) {
   //   for (size_t j=0; j < backups[i].size(); j++) 
   //     cout << backups[i][j] << ":" << P.value(backups[i][j].variable()) << ':' << P.decision_level(backups[i][j].variable()) << ' ';
   //   cout << " assertion level " << " " << backups[i].assertion_level << endl;
   // }
   // cout << endl;
   
   m_num_asserting_clauses = 0;
   size_t base_assertion_level = backups[0].assertion_level;
   for (size_t i=0; i < backups.size(); i++) {
      push_back(backups[i]);
      back().group_identifier = c.group_identifier;
      back().id = size()-1;
      increment_clause_score(size()-1);
      
      if (c.size() > 1) {
         for (size_t j=0; j < 2; j++) // now we can watch the first two literals of each clause
            m_watchers[back()[j].variable()].watch_list[back()[j].sign()].push_back(size()-1);
      }
      if (backups[i].assertion_level == base_assertion_level) m_num_asserting_clauses++;
   }
   //   cout << "number unit clauses " << m_num_asserting_clauses << endl;
   m_clause_score_inc *= CLAUSE_SCORE_INC_FACTOR;
   push_back(CnfClause());
   
   // cout << "num asserting clauses " << m_num_asserting_clauses << endl;
  
}




bool SymRes::get_symmetric_unit_lits(Assignment& P, ClauseID c_id, Literal unit_lit)
{
  //  cout << "number unit clauses " << m_num_asserting_clauses << endl;
  //  cout << "getting symmetric lits " << endl << P << endl;
   // cout << "primary clause " << operator[](c_id) << endl;
  for (size_t i=c_id; (i < c_id + m_num_asserting_clauses) && (i < size()-1); i++) { // walk through all the learned clauses
      CnfClause& c = operator[](i);
      //if (c_id != i) cout << "additional sym lit " << c[0] << endl;
      //      cout << i << " extending " << c[0] << " with reason " << i << " instance " << operator[](i) << endl;
      if (!P.extend(AnnotatedLiteral(c[0],i))) {
	cout << "extend failed" << endl;
         return false;
      }
   }
   
   return true;
}



} // end namespace zap

