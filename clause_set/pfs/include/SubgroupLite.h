#ifndef __SUBGROUP_LITE__
#define __SUBGROUP_LITE__
#include "GlobalProductGroup.h"
#include <set>
#include <vector>
using namespace std;

/*
  1. Get the Literal Domain Value Map filled in correctly
  2. Test the fix function
  3. 
 */
namespace zap
{
  typedef vector<vector<pair<size_t,size_t> > > LiteralDomainValueMap;

  /*
    This is a light weight class 
   */
class SubgroupLite : public vector<set<size_t> >
{
  const GlobalProductGroup*        m_parent_group;
  const LiteralDomainValueMap*     m_literal_dv_map;
 public:
  SubgroupLite(const GlobalProductGroup* g, const LiteralSymMap& lsm);
  void fix(const Literal& l);
  vector<Literal> orbit(const Literal& l);
  
};

/////////////////////////  INLINES  ////////////////////////////////////////

  SubgroupLite::SubgroupLite(const GlobalProductGroup* g, const LiteralSymMap& lsm) 
    : vector<set<size_t> >(g->size(),set<size_t>()), 
    m_parent_group(g),
    m_literal_sym_map(lsm)
    {
      for (size_t i=0; i < g->size(); i++) {
	for (size_t j=0; j < g->operator[](i).orbit_size(); j++) operator[](i).insert(j);
      }
    }

  void SubgroupLite::fix(const Literal& l)
  {
    const vector<pair<size_t,size_t> >& dv_pairs = literal_dv_map->get_domain_value_pairs(l);
    for (size_t i=0; i < dv_pairs.size(); i++) {
      operator[](dv_pairs[i].first).remove(dv_pairs[i].second));
    }
  }


} // end namespace zap
#endif
