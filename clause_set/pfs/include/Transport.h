#ifndef __TRANSPORT__
#define __TRANSPORT__

#include <list>
#include "common.h"
#include "FullSym.h"
#include "PfsCounter.h"
#include "Action.h"
#include "Assignment.h"
#include "SmartPointer.h"


namespace zap
{

class Tanswer;
class WatchIndex;


//**************************************************************************************************/
/*
 * PfsTransport:
 * 
 * Represents a single action from a QPROP clause and its effect on
 * the representative ground clause.  The member m_columns contains all
 * the possible domain values for this action and the rep is the set of domain values
 * that appear in the representative ground clause and are moved by this action.
 * The rep is stored in the beginning of m_columns.
 * 
 */
//**************************************************************************************************/
class PfsTransport
{
  Ptr<Action>            m_action;  
  size_t                 m_rep_size; // 
  vector<Column>         m_columns;  // organized with initial rep in the begining
  
public:
  PfsTransport() : m_rep_size(0) { }
  PfsTransport(const vector<Column>& rep, const Ptr<Action>& ap);

  // For generating all the images under the action
  PfsCounter              initial_counter() const { return PfsCounter(m_rep_size,m_columns.size(),&m_columns); }
  bool                    next_image(PfsCounter& counter, vector<Literal>& clause_rep) const;

  
  vector<Column>          first_rep() const;                       
  const Action&           action() const { return *m_action; }
  const vector<Column>&   columns() const { return m_columns; }
  size_t                  number_of_images() const;
  
  friend ostream& operator << (ostream& os, const PfsTransport& pgt);
};



class OrbitCache : public vector<vector<Literal> >
{
  vector<size_t> variable_index;
public:
  OrbitCache() : vector<vector<Literal> >(2,vector<Literal>()) { }
  void add_orbit(const vector<Literal>& o);
  const vector<Literal>& get_orbit(Literal l) const;
  bool empty() const { return size() <= 2; }
  
  
};


inline const vector<Literal>& OrbitCache::get_orbit(Literal l) const
{
  if (l.variable() >= variable_index.size()) return operator[](0);
  return operator[](variable_index[l.variable()] + (l.sign() ? 0 : 1));
}

//**************************************************************************************************/
/* PfsTransportVector : This is everything you need to build the k-transporter search tree.
 *
 *
 */
//**************************************************************************************************/
class PfsTransportVector : public vector<PfsTransport> 
{
  vector<Literal>          m_clause_rep;
  OrbitCache               m_orbit_cache;

public:
  PfsTransportVector() { }
  PfsTransportVector(const vector<Literal>& clause) : m_clause_rep(clause) { }
  PfsTransportVector(const vector<Literal>& clause,const vector<PfsTransport>& t) :
    vector<PfsTransport>(t), m_clause_rep(clause) { }

  
  const vector<Literal>&    clause_rep() const { return m_clause_rep; }
  
  vector<vector<Literal> >  orbits() const { return orbits(m_clause_rep,0); }
  vector<Literal>           universe() const { return orbit(m_clause_rep,0); }

  vector<vector<Literal> >  orbits(const vector<Literal>& c, size_t depth) const;
  vector<Literal>           orbit(const vector<Literal>& c, size_t depth) const;  // used by universe
  void                      build_orbits();
  const vector<Literal>&    orbit(Literal l);
  
  bool                      fixes(Literal l, size_t depth) const;
  size_t                    depth_fixed(Literal l) const;
  size_t                    number_ground_clauses() const;
  bool                      empty() const { return size() <= 1; }
  void                      cnf(size_t depth, vector<Literal> clause, vector<Clause>& clauses) const;
  

  void                      order_tree(size_t depth, vector<Literal> still_to_fix);
};




//**************************************************************************************************/
class BackWatcher
{
public:
  Literal          lit;
  size_t           id; 
  size_t           local_index; // where it is in the clause
  BackWatcher(Literal l, size_t i, size_t li) : lit(l), id(i), local_index(li) { }
  friend ostream& operator<<(ostream& os, const BackWatcher& bw) {
	os << "[" << bw.lit << ", " << bw.id << ", " << bw.local_index << "]";
	return os;
  }
};




//**************************************************************************************************/
/* WatchNode : This is a node in the k-transporter search tree
 *
 *
 */
//**************************************************************************************************/
class WatchNode
{
  static size_t                     m_next_id;               // for search statistics
  
  size_t                            m_depth;                 
  size_t                            m_child;
  size_t                            m_level_id;
  size_t                            m_right_leaf;            // ID of the right most leaf node in subtree
  WatchNode*                        m_parent;
  vector<WatchNode*>                m_children;
  Clause                            m_clause;
  vector<BackWatcher>               m_back_watchers;
  vector<bool>                      m_fixed;                
  
  WatchNode* expand(const PfsTransportVector& transports);
  bool next_child(PfsCounter& counter, const PfsTransport& transport);
  size_t right_most_leaf() const;
  size_t get_skip(size_t index) const;
  
public:
  WatchNode(const vector<Literal>& clause) : m_depth(0), m_child(0),
    m_level_id(0), m_right_leaf(0), m_parent(0), m_clause(clause), m_fixed(clause.size(),false) { }
  WatchNode(const WatchNode& n) : m_depth(n.m_depth),
    m_child(n.m_child), m_level_id(n.m_level_id), m_right_leaf(0), m_parent(n.m_parent),
    m_clause(n.m_clause), m_fixed(n.m_fixed) { }
  // need a copy constructor here.  Shouldn't copy m_children or the destructor will seg fault
  ~WatchNode() { for (size_t i=0; i < m_children.size(); i++) delete m_children[i]; }

  
  void       set_clause(const vector<Literal>& c);
  void       build(const PfsTransportVector& transports, WatchIndex& watch_index);
  void       build_right_leaf();
  pair<size_t,bool>     check(Assignment& P,vector<BackWatcher>& to_remove,int& new_watcher,ClauseID id,WatchIndex& watch_index);
  
  friend ostream& operator << (ostream& os, const WatchNode& wn);
};




//**************************************************************************************************/
class PfsVariableWatch
{
public:
  map<size_t,WatchNode*> index[2];
  const map<size_t,WatchNode*>& get_index(bool b) const { return (b ? index[1] : index[0]); }
  map<size_t,WatchNode*>& get_index(bool b) { return (b ? index[1] : index[0]); }
};

//**************************************************************************************************/
class WatchIndex : public vector<PfsVariableWatch>
{
public:
  WatchIndex() { }
  WatchIndex(const vector<PfsVariableWatch>& w) : vector<PfsVariableWatch>(w) { }
  map<size_t,WatchNode*>& get_index(Literal l) { return operator[](l.variable()).get_index(l.sign()); }
  void add_watch(const Literal l, WatchNode* w, size_t id) { get_index(l)[id] = w; }
  void remove_watch(const BackWatcher b) {
	if (b.lit.variable() >= size()) quit("bad BackWatcher lit ");
    map<size_t,WatchNode*>& watchers = get_index(b.lit);
    map<size_t,WatchNode*>::iterator it = watchers.find(b.id);
	if (it == watchers.end()) quit("bad BackWatcher id");
    watchers.erase(it);
  }
};



///////////////////////////////  INLINES  ///////////////////////////////////////////

inline void WatchNode::set_clause(const vector<Literal>& c)
{
  m_clause = c; m_fixed = vector<bool>(c.size(),false); 
}


inline ostream& operator << (ostream& os, const WatchNode& wn)
{
  os << "Node:" << wn.m_level_id << " depth:" << wn.m_depth << " child:" << wn.m_child
     << " CLAUSE: " << wn.m_clause << " fixed: " << wn.m_fixed << " right leaf " << wn.m_right_leaf;
  return os;
}

// inline vector<pair<size_t,size_t> > TransportCache::get_orbit_ids(const vector<Literal>& c,size_t depth) const
// {
//   const TOrbit& t = m_orbit_lookup[depth];
  
//   vector<size_t> orbits_seen;
//   for (size_t i=0; i < c.size(); i++) {
//     size_t id = t.get_orbit_id(c[i]);
//     orbits_seen.push_back(id);
//   }
  
//   sort(orbits_seen.begin(),orbits_seen.end());
//   vector<pair<size_t,size_t> > answer;
//   if (orbits_seen.size() == 0) return answer;
//   size_t current_id = orbits_seen[0];
//   size_t overlap = 1;
//   for (size_t i=1; i < orbits_seen.size(); i++) {
//     if (orbits_seen[i] == current_id) {
//       overlap++;
//       continue;
//     }
//     answer.push_back(make_pair(current_id, overlap));
//     current_id = orbits_seen[i];
//     overlap = 1;
//   }
//   answer.push_back(make_pair(current_id,overlap));
//   return answer;
// }


// inline const Orbit& TransportCache::get_orbit(Literal l, size_t depth) const
// {
//   const TOrbit& t = m_orbit_lookup[depth];
//   size_t id = t.get_orbit_id(l);
//   return m_orbit_index->orbit(id);
// }

inline vector<Column> PfsTransport::first_rep() const
{
  vector<Column> rep(m_rep_size);
  for (size_t i=0; i < m_rep_size; i++)
    rep[i] = m_columns[i];
  return rep;
}


inline size_t PfsTransport::number_of_images() const
{
  size_t n = 1;
  for (size_t i=0; i < m_rep_size; i++) n *= m_columns.size() - i;
  return n;
}

inline void OrbitCache::add_orbit(const vector<Literal>& o) // we assume the orbit is in positive phase literals
{
  vector<Literal> negated(o.size());
  push_back(o);
  push_back(negated);
  for (size_t i=0; i < o.size(); i++) {
	if (o[i].variable() >= variable_index.size()) {
	  variable_index.insert(variable_index.end(),o[i].variable() - variable_index.size() + 1,0);
	}
	variable_index[o[i].variable()] = size() - 2;
	operator[](size()-1)[i] = Literal(o[i].variable(),false);
  }
 
}

inline size_t PfsTransportVector::number_ground_clauses() const
{
  if (empty()) return 0;
  size_t answer = 1;
  for (size_t i=1; i < size()-1; i++) answer *= operator[](i).number_of_images();
  return answer;
}


inline bool PfsTransportVector::fixes(Literal l, size_t depth) const
{
  for (size_t i=depth+1; i < size(); i++) 
    if (!operator[](i).action().fixes(l,operator[](i).columns())) return false;
  return true;
}

inline size_t PfsTransportVector::depth_fixed(Literal l) const
{
  int i= size()-1;
  for ( ; i >= 1; i--) 
    if (!operator[](i).action().fixes(l,operator[](i).columns())) break;
  if (i == 0) return 0;
  return size_t(i);
}



inline bool WatchNode::next_child(PfsCounter& counter, const PfsTransport& transport)
{
  if (!counter.next()) return false;
  WatchNode* node = new WatchNode(*m_children.back());  // make a copy of earlier sibling
  //  node->m_id = ++m_next_id;
  node->m_child++;
  node->m_level_id++;
  vector<Literal>& clause = node->m_clause;
  for (size_t i=0; i < clause.size(); i++) 
    clause[i] = transport.action().image(clause[i],counter);
  m_children.push_back(node);
  return true;
}

inline ostream& operator << (ostream& os, const PfsTransport& pgt)
{
  os << "[ Transport : rep [";
  for (size_t i=0; i < pgt.m_rep_size; i++) os << pgt.m_columns[i] << ' ';
  os << "]  remaining columns [";
  for (size_t i=pgt.m_rep_size; i < pgt.m_columns.size(); i++) os << pgt.m_columns[i] << ' ';
  cout << "] {" << flush;
  if (pgt.m_action != 0) os << pgt.action();
  os << "} ]  ";
  return os;
}

} // end namespace zap

#endif
