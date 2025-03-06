
#ifndef __SYMRES__
#define __SYMRES__

#include <vector>
#include "Cnf.h"
#include "GlobalProductGroup.h"
#include "ProductSubgroup.h"
#include "Pfs.h"

using namespace std;

namespace zap
{

class BackupClause : public CnfClause 
{
 public:
   size_t assertion_level;
 BackupClause(const CnfClause& c) : CnfClause(c), assertion_level(-1) { }
};

   inline bool operator<(const BackupClause& bc1, const BackupClause& bc2) {
      return bc1.assertion_level < bc2.assertion_level;
   }
/////////////////////////   SYMRES CLAUSE SET   //////////////////////////////////

/*  
 */

/*
  TODO:
  *  The back up that doesn't fully resolve is finding unit clauses that don't exist I think?
*/

class SymRes : public Cnf
{
   GlobalProductGroup                 m_global_group;  
   map<string,Ptr<ProductSubgroup> >  m_global_groups;
   size_t                             m_num_asserting_clauses;

   size_t position(Literal l, const Assignment& P) const;
   size_t decision_level(Literal l, const Assignment& P) const;
  
public:

 SymRes() : m_num_asserting_clauses(0) { }
   SymRes(const GlobalProductGroup& gpg);
   GlobalProductGroup& global_group() {  return m_global_group; }
   void set_global_groups(const map<string,Ptr<ProductSubgroup> >& g) { m_global_groups = g; }
   
   void initialize_clause_set(vector<PfsClause>& clauses);

   ClauseID resolve(const Reason& r1, const Reason& r2, const Assignment& P, 
		    size_t& lits_this_level, Literal& unit_lit);
   void add_learned_clause(ClauseID c_id, const Assignment& P);
   bool get_symmetric_unit_lits(Assignment& P, ClauseID c_id, Literal unit_lit);
  Ptr<ProductSubgroup> lookup_group(const string& group_id)
  {
	map<string,Ptr<ProductSubgroup> >::iterator it = m_global_groups.find(group_id);
	if (it == m_global_groups.end()) quit("Pfs::lookup_group failed");
	return it->second;
  }

};



//////////////////////////   INLINES   ///////////////////////////////////////





} // end namespace zap
#endif

