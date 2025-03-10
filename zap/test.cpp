#include "FullSym.h"
#include "GlobalProductGroup.h"
#include "Action.h"
#include "ProductSubgroup.h"
#include "Counter.h"
#include "MultiArray.h"
#include "FOAssigner.h"

using namespace zap;

int main(){

  FOAssigner foa(5,6);

  foa.add_assignment(0,1);
  foa.add_assignment(1,1);
  foa.add_assignment(2,1);
  foa.add_assignment(3,1);
  foa.add_assignment(4,1);
  foa.add_assignment(2,3);
  foa.add_assignment(3,3);
  foa.add_assignment(3,4);
  foa.add_assignment(4,4);
  foa.add_assignment(0,4);
  foa.add_assignment(0,5);

  cout << foa << endl;

  list<NodePtr>::iterator min = foa.get_min_node();
  cout << "min node " << min->row << "," << min->col << endl;
  foa.set_assignment(min);
  cout << foa << endl;
  
//   vector<size_t> dims;
//   dims.push_back(3);
//   dims.push_back(2);
//   dims.push_back(3);
  
//   MultiArray<size_t> array(dims);
//   vector<size_t> index(dims.size(),0);
//   for (size_t i=0; i < 18; i++) {
// 	index[0] = i / 6;
// 	index[1] = (i % 6) / 3;
// 	index[2] = (i % 6) % 3;
// 	array.at(index) = i+1;
//   }

  
//   index = vector<size_t>(3,0);
//   for (size_t i = 0; i < 3; i++) {
// 	index[2] = i;
// 	cout << array.at(index) << ' ';
//   }
//   cout << endl;
  
//   PermCounterWithFixing counter(3,4);
//   counter.fix(1);
//   do {
// 	cout << counter << endl;
//   } while (++counter);
  
//   Counter count(2,3);
//   do {
// 	cout << count << endl;
//   } while (++count);
  
  
//   size_t sz = 4;
  
//   vector<FSOrbit> h;

//   for (size_t i=0; i < sz; i++) {
// 	FSOrbit o;
// 	for (size_t j=0; j < sz-1; j++) o.push_back(j + i*(sz-1) + 1);
// 	h.push_back(o);
//   }
  
//   FullSym holes(h,string("in"),string("hole"));

//   cout << "hole symmetry " << endl << holes << endl;

//   vector<FSOrbit> p;

//   for (size_t j=0; j < sz-1; j++) {
// 	FSOrbit o;
// 	for (size_t i=0; i < sz; i++) o.push_back(j + i*(sz-1) + 1);
// 	p.push_back(o);
//   }

//   FullSym pigeons(p,string("in"),string("pigeon"));

//   cout << "pigeon symmetry " << endl << pigeons << endl;

//   vector<double> counts(13,0.0);
//   Assignment P(12,&counts);
//   P.extend(AnnotatedLiteral(Literal(4,true),Reason()));
//   P.extend(AnnotatedLiteral(Literal(1,true),Reason()));
//   P.extend(AnnotatedLiteral(Literal(5,false),Reason()));
  
//  //  set<Column> columns;
//   for (size_t i=0; i < sz; i++) columns.insert(i);
//   vector<set<Column> > set_stab = pigeons.set_stabilize(P,columns);

//   for (size_t i=0; i < set_stab.size(); i++) {
// 	set<Column>::iterator it = set_stab[i].begin();
// 	for ( ; it != set_stab[i].end(); it++) cout << *it << ", ";
// 	cout << endl;
//   }

//   GlobalProductGroup gpg;
//   gpg.add_symmetry(holes);
//   gpg.add_symmetry(pigeons);

//   cout << "global product group " << endl << gpg << endl;


  


//   cout << g1 << endl << g2 << endl;
//   ProductSubgroup intersection = g1.intersection(g2);
//   cout << "intersection : " << endl << intersection << endl;

  return 0;
}

