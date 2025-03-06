#ifndef __PRODUCT_FULL_SYM__
#define __PRODUCT_FULL_SYM__
#include "Transport.h"
#include "SmartPointer.h"

using namespace std;

namespace zap
{



class ProductSubgroup // maybe change this name to just ProductGroup
{
  const GlobalProductGroup*        m_parent_group;
  vector<Ptr<Action> >                   m_actions;
  OrbitCache                       m_orbit_cache;


public:

  ///////////////////////////  FINISHED  ////////////////////////////////////
  ProductSubgroup() : m_parent_group(0) { }
  ProductSubgroup(const GlobalProductGroup* g) : m_parent_group(g) { }
  
  friend ostream& operator << (ostream& os, const ProductSubgroup& ps);
  ///////////////////////////  TODO  ////////////////////////////////////////


  // make these inherited?
  size_t size() const { return m_actions.size(); }
  Action& operator[](size_t i) { return *m_actions[i]; }
  
  void add_action(const Action& a);
  void build_orbits();
  ///////////////////////////  REVIEW  //////////////////////////////////////
    
//   // the actions have a pointer to GlobalProductGroup so it doesn't need to be a member of ProductSubgroup
//   ProductSubgroup(const GlobalProductGroup* s, const vector<Action>& a) : m_parent_group(s), m_actions(a) {}
//   const GlobalProductGroup* parent_group() const { return m_parent_group; }
  PfsTransportVector build_transports(const vector<Literal>& clause, const vector<bool>& background) const;
  vector<set<Literal> > orbits(const vector<Literal>& c);
  const vector<Literal>&      orbit(Literal l); 
	
//   bool empty() const { return m_actions.size() == 0; }
  double total_size() const ;
  

//   ///////////////////////////  RESOLUTION  //////////////////////////////////
  Ptr<ProductSubgroup> intersection(const ProductSubgroup& ps2,const Clause& c1,const Clause& c2) const;
  void add_background_symmetry(const vector<Literal>& universe);
  void add_clause_stabilizer(const Clause& c) { return; }
  void identify_background_symmetry(const Clause& c, vector<bool>& b) const;
//   void add_background_action(const vector<Literal>& c);
//   void add_interclause_background_action(const vector<Literal>& c);
};




//////////////////////////////////  INLINES  ////////////////////////////////////////

inline const vector<Literal>& ProductSubgroup::orbit(Literal l)
{
  if (m_orbit_cache.empty()) {
	build_orbits();
  }
  return m_orbit_cache.get_orbit(l); 
}

} // end namespace zap
#endif










