#ifndef _PFS_H
#define _PFS_H
#include <sstream>
#include "GlobalProductGroup.h"
#include "ProductSubgroup.h"
#include "Assignment.h"
#include "ClauseSet.h"
#include "Transport.h"
#include "FastSet.h"
#include "LocalSearch2.h"
#include "LocalSearch3.h"
#include "PredicateSym.h"

namespace zap
{
  
  ////////////////////////////////////////   PFS CLAUSE   ////////////////////////////////////////
  /*
	
   */
  
  class PfsClause : public Clause {
	mutable Ptr<ProductSubgroup>     m_subgroup;
	mutable PredicateSym*        m_predicate_sym;
	PfsTransportVector       m_transports;  // this should be a pointer to a transport vector
	WatchIndex               m_watch_index;
	Ptr<WatchNode>           m_root;
	LocalSearch2             m_local_search;
	LocalSearch3             m_local_search3;
	vector<Literal>          m_universe;  /// maybe this cache should be in m_transport since that is who creates it.
	vector<bool>             m_background_symmetry;

	public:
	PfsClause() : m_root(WatchNode(Clause())) { }
	PfsClause(const Clause& c) : Clause(c), m_root(c) { }
	PfsClause(const Clause& c, const Ptr<ProductSubgroup>& ps);
	//	PfsClause(const Clause& c, const ProductSubgroup& ps, PredicateSym* psp );
	PfsClause(const Clause& c, const Ptr<ProductSubgroup>& pps,PredicateSym* psp);
	PfsClause(const PfsClause& c);

	
	const ProductSubgroup& group() const { return *m_subgroup; }
	ProductSubgroup& group() { return *m_subgroup; }
	Ptr<ProductSubgroup>& group_ptr() const { return m_subgroup; }
	void build_transports();
	void build_watch_tree();
	void initialize_local_search() { m_local_search3.initialize(Clause(*this),m_predicate_sym); }
	size_t number_ground_clauses() { return m_transports.number_ground_clauses(); }
	void get_ground_clauses(vector<Clause>& ground_clauses);
	bool k_transporter(Assignment& P, Literal l);
	bool get_symmetric_unit_lits(Assignment& P, Literal unit_lit);
	bool get_cluster(Assignment& P, Literal unit_lit);

	
	bool k_transport_watch_tree(Assignment& P, Literal l);
	bool k_transport_local_search(Assignment& P, Literal l);

	const vector<Literal>& universe();
	friend ostream& operator<<(ostream& os, const PfsClause& c);
	
  };
  

////////////////////////////////////////   PFS   ////////////////////////////////////////
/*

*/
class Pfs : public vector<PfsClause>, public virtual ClauseSet
{
  size_t                 m_num_vars;
  size_t                 m_num_clauses;
  size_t                 m_end_original_clauses; // marks end of original input and points to first learned clause
  vector<VariableWatch>  m_watchers;

  vector<double>         m_vsids_counts;
  double                 m_score_inc;
  double                 m_clause_score_inc;
  FastSet                m_score_set;
  double                 m_max_clauseset_size;
  
  GlobalProductGroup     m_global_group;
  map<string,Ptr<ProductSubgroup> >  m_global_groups;
  PredicateSym           m_predicate_sym;
  
  void adjust_counts(size_t index);
  Result    get_implications_ground(Assignment& P, Literal l);
    void      increment_variable_score(size_t v);
  void      rescale_variable_scores();
  void      increment_clause_score(ClauseID id);
  void      rescale_clause_scores();
  void      add_to_analysis(const Clause& c, const Assignment& P);

    // these functions are all for bounding/reducing the size of the learned clause set
  void      remove_permanently_sat_clauses(const Assignment& P);
  void      remove_irrelevant_clauses(const Assignment& P);
  void      update_pointers(Assignment& P, size_t max_id);
  bool      satisfied_at_level_0(const Assignment& P, ClauseID id);
  bool      is_a_reason(const Assignment& P, ClauseID id) const;


public:

  Pfs() { }
  Pfs(const GlobalProductGroup& gpg);
  // don't really like this type of access to a ClauseSet.  Should be a tighter interface.
  GlobalProductGroup& global_group() {  return m_global_group; }
  void set_global_groups(const map<string,Ptr<ProductSubgroup> >& g) { m_global_groups = g; }
  const map<string,Ptr<ProductSubgroup> >& global_subgroups() const { return m_global_groups; }
  void set_predicate_sym(const PredicateSym& ps) { m_predicate_sym = ps; }
  Ptr<ProductSubgroup> lookup_group(const string& group_id)
  {
	map<string,Ptr<ProductSubgroup> >::iterator it = m_global_groups.find(group_id);
	if (it == m_global_groups.end()) quit("Pfs::lookup_group failed");
	return it->second;
  }
  
  void initialize_clause_set(const vector<PfsClause>& clauses);
  
  // void add_clause(const Clause& c) { push_back(PfsClause(c,ProductSubgroup(&m_global_group),&m_predicate_sym)); }

  void get_ground_clauses(vector<Clause>& ground_clauses);
  
  // the ClauseSet interface functions
  size_t    number_variables()                         const { return m_num_vars; }
  Result    get_implications(Assignment& P, Literal l);
  bool      get_symmetric_unit_lits(Assignment& P, ClauseID c_id, Literal unit_lit);
  bool      get_cluster(Assignment& P, ClauseID c_id, Literal unit_lit);
  ClauseID  resolve(const Reason& r1, const Reason& r2, const Assignment& P,
					size_t& lits_this_level, Literal& unit_lit);
  void      add_learned_clause(ClauseID c_id, const Assignment& P);
  Result    load_unit_literals(Assignment& P);
  void      reduce_knowledge_base(Assignment& P);
  
  const Clause&          clause(ClauseID c) const;
  const vector<double>&  VSIDS_counts()     const { return m_vsids_counts; }
  
  bool      valid(const Assignment& P) const;  // for checking the invariants
  bool      closed(const Assignment& P) const;
  bool      decision_minimal(const Assignment& P) const;

  friend ostream& operator<<(ostream& os, const Pfs& t);
};


inline ostream& operator<<(ostream& os, const PfsClause& c) {
  os << "(";
  c.Clause::print(os);
  os << "," << c.m_subgroup << ")" << endl;
  return os;
}

inline ostream& operator<<(ostream& os, const Pfs& t) {
  os << "GLOBAL GROUP: " << endl << t.m_global_group << endl << "CLAUSES : " << endl;
  for (size_t i=0; i < t.size(); i++) os << t[i] << endl;
  return os;
}

inline PfsClause::PfsClause(const PfsClause& c)
  : Clause(Clause(c)), m_subgroup(c.m_subgroup), m_predicate_sym(c.m_predicate_sym), m_transports(c.m_transports),
  m_watch_index(c.m_watch_index), m_root(c.m_root), m_local_search(c.m_local_search),
  m_local_search3(c.m_local_search3), m_universe(c.m_universe)
{
  m_local_search.update_transport_pointer(&m_transports);
}



inline void PfsClause::build_watch_tree() {
  if (m_transports.empty()) build_transports();
  
  const vector<Literal>& U = universe();
  m_watch_index = WatchIndex(vector<PfsVariableWatch>(U.back().variable()+1));
  m_root->set_clause(vector<Literal>(*this));
  m_root->build(m_transports,m_watch_index);
  m_root->build_right_leaf();
}

inline void PfsClause::build_transports()
{
  if (!m_subgroup) return;
  
  m_subgroup->identify_background_symmetry(*this,m_background_symmetry);
//   cout << "background symmetry vector " << m_background_symmetry << endl;
  m_transports = m_subgroup->build_transports(*this,m_background_symmetry);
  //  m_transports.build_orbits();
  //  m_subgroup.add_background_symmetry(universe());
//   m_subgroup.add_clause_stabilizer(c);
}

inline void PfsClause::get_ground_clauses(vector<Clause>& ground_clauses) {
  if (m_transports.empty()) build_transports();
  m_transports.cnf(0,*this,ground_clauses);
}

inline const vector<Literal>& PfsClause::universe()
{
  if (m_universe.empty()) m_universe = m_transports.universe();
  return m_universe;
}




} // end namespace zap
#endif
