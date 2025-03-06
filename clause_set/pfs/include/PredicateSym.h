#ifndef _PREDICATESYM_H
#define _PREDICATESYM_H

#include "common.h"
#include "MultiArray.h"

namespace zap 
{

typedef vector<size_t> ArrayIndex;

class ArrayLiteral {
public:
  size_t          predicate;
  ArrayIndex     index;

  ArrayLiteral(size_t p, const ArrayIndex& i) : predicate(p), index(i) { }
};

class ArgPtr
{
public:
  size_t  predicate;
  size_t  index;
  
  ArgPtr(size_t l, size_t i) : predicate(l), index(i) { }
};

class GlobalAction : public vector<ArgPtr>
{
public:
  size_t  domain_size;
  GlobalAction() : domain_size(0) { }
  GlobalAction(const vector<ArgPtr>& ap, size_t ds) : vector<ArgPtr>(ap), domain_size(ds) { }
};

class LocalSearchGroup : public vector<GlobalAction>
{
public:
  vector<vector<size_t> >     arg_action_map;
  size_t get_action(const ArgPtr& ap) const
  {
	return arg_action_map[ap.predicate][ap.index];
  }
};


class PredicateSym {
  vector<MultiArray<size_t> >    arrays;
  vector<ArrayLiteral>           reverse_map;
  vector<string>                 names;
  vector<vector<string> >        domains;
  LocalSearchGroup               group;
  
  size_t                         lit_count;

public:

  PredicateSym() : lit_count(0) { }

  size_t size() const { return arrays.size(); }

  Variable     lookup(const ArrayLiteral& al) const { return arrays[al.predicate].at(al.index); }
  ArrayLiteral lookup(Variable v) const { return reverse_map[v-1]; }
  const LocalSearchGroup& get_group() const { return group; }
  const MultiArray<size_t>& get_predicate_array(size_t predicate_id) const { return arrays[predicate_id]; }
  
  void add_predicate(const string& predicate, const vector<string>& dom, const vector<size_t>& dims)
  {
	names.push_back(predicate);
	domains.push_back(dom);
	group.arg_action_map.push_back(vector<size_t>(dims.size(),0));
	arrays.push_back(MultiArray<size_t>(dims));

	size_t sz = 1;
	vector<size_t> multipliers(dims.size(),1);
	for (size_t i=0; i < dims.size(); i++) {
	  multipliers[i] = sz;
	  sz *= dims[i];
	}
	

	MultiArray<size_t>& a = arrays.back();
	size_t lc = lit_count;
 	for (size_t i=0; i < sz; i++, lit_count++) {
	  a[i] = lc + i+1;
	  vector<size_t> index(dims.size(),0);
	  size_t remainder = i;
	  for (size_t j=0; j < multipliers.size(); j++) {
		index[multipliers.size() - 1 - j] =  remainder / multipliers[multipliers.size() - 1 - j];
		remainder = remainder % multipliers[multipliers.size() - 1 - j];
	  }
	  reverse_map.push_back(ArrayLiteral(arrays.size() - 1,index));
	}
  }

//   void print_and_verify() const {
// 	for (size_t i=0; i < arrays.size(); i++) {
// 	  const MultiArray<size_t>& A = arrays[i];
// 	  for (size_t j=0; j < A.size(); j++) {
// 		size_t v = A[j];
// 		ArrayLiteral al = reverse_map[v-1];
// 		if (v != A.at(al.index)) cout << "bad mapping in PredicateSym " << v << " and " << A.at(index) << endl;
// 	  }
// 	}
//   }
  
  void build_groups()
  {
	map<string,vector<ArgPtr> > actions;
	for (size_t i=0; i < domains.size(); i++) {
	  for (size_t j=0; j < domains[i].size(); j++) {
		map<string,vector<ArgPtr> >::iterator it = actions.find(domains[i][j]);
		if (it == actions.end()) actions[domains[i][j]] = vector<ArgPtr>(1,ArgPtr(i,j));
		else it->second.push_back(ArgPtr(i,j));
	  }
	}

	map<string,vector<ArgPtr> >::iterator it = actions.begin();
	for (size_t j=0; it != actions.end(); it++, j++) {
	  group.push_back(GlobalAction(it->second,0));
	  for (size_t i=0; i < it->second.size(); i++) {
		if (i == 0) {
		  group.back().domain_size = arrays[it->second[i].predicate].size(it->second[i].index);
		}
		const ArgPtr& ap = it->second[i];
		group.arg_action_map[ap.predicate][ap.index] = j;
	  }
	}
  }
  
};




} // end namespace zap 

#endif
