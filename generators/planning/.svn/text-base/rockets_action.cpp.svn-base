#include<iostream>
using namespace std;

int main(int argc, char* argv[])
{
  if ( 3 != argc )
    {
      cout << "Usage: " << argv[0] << " <planes/holes> <multiplier> " << endl;
      exit(1);
    }

  int n = atoi(argv[1]);
  int m = atoi(argv[2]);

  int object = m*n + 1;
  int location = m*n + 2;
  int plane = n;
  int time = (m+2)*2;

  cout << "// ZAP encoding for classic rockets/planes domain." << endl
       << "// The issue of fuel is ignored so planes can make multiple flights." << endl
       << "// Based on graph plan based encoding" << endl << endl;
  
  cout << "SORT object " << object << ";" << endl
       << "SORT location " << location << ";" << endl
       << "SORT plane " << plane << ";" << endl
       << "SORT time " << time << ";" << endl << endl;
  
  cout << "PREDICATE objectAt( object location time ) ; " << endl
       << "PREDICATE planeAt( plane location time ) ; " << endl
       << "PREDICATE inPlane( object plane time ) ; " << endl
       << "PREDICATE load( object plane location time ) ; " << endl
       << "PREDICATE unload( object plane location time ) ; " << endl
       << "PREDICATE fly( plane location location time ) ; " << endl
       << "PREDICATE next( time time ) ;" << endl << endl;

  cout << "// load preconditions " << endl
       << "FORALL( o p l t ) ~load[ o p l t ] objectAt[o l t] ; " << endl
       << "FORALL( o p l t ) ~load[ o p l t ] planeAt[p l t] ; " << endl << endl

       << "// unload preconditions" << endl
       << "FORALL( o p l t ) ~unload[ o p l t ] inPlane[o p t] ; " << endl
       << "FORALL( o p l t ) ~unload[ o p l t ] planeAt[p l t] ; " << endl << endl

       << "// fly preconditions " << endl
       << "FORALL( p l1 l2 t ) ~fly[ p l1 l2 t ] planeAt[p l1 t] ; " << endl << endl;

  

  cout << "// facts imply disjunction of add effect " << endl
       << "FORALL( o p t2 t1 ) EXISTS(l) ~next[t1 t2] ~inPlane[o p t2] inPlane[o p t1] load[ o p l t1 ] " << endl
       << "\t{" << endl
       << "\t\t t1 != t2 " << endl
       << "\t} ;" << endl;

  cout << "FORALL( o l t2 t1 ) EXISTS(p) ~next[t1 t2] ~objectAt[o l t2] objectAt[o l t1] unload[ o p l t1 ] " << endl
       << "\t{" << endl
       << "\t\t	t1 != t2 " << endl
       << "\t} ; " << endl;

  cout << "FORALL(p l2 t1 t2) EXISTS(l1) ~next[t1 t2] ~planeAt[p l2 t2] planeAt[p l2 t1] fly[p l1 l2 t1]" << endl
       << "\t{" << endl
       << "\t\t	t1 != t2 " << endl
       << "\t} ; " << endl << endl;
  

  cout << "// conflicting actions are mutually exclusive " << endl
       << "FORALL( o p l1 l2 t) ~load[ o p l1 t ] ~fly[ p l1 l2 t ] ; " << endl
       << "FORALL( o p l1 l2 t) ~unload[ o p l1 t ] ~fly[ p l1 l2 t ] ; " << endl << endl;
  

  cout << endl << "// initial conditions " << endl;
  for (size_t o=1; o <= object; o++)
    cout << "objectAt[" << o << " " << o << " 1] ; " << endl;

  for (size_t p=1; p <= plane; p++)
    cout << "planeAt[" << p << " " << location << " 1] ; " << endl;


  
  cout << endl << "// final conditions " << endl;
  for (size_t o=1; o < object; o++)
    cout << "objectAt[" << o << " " << o+1 << " " << time << "] ; " << endl;

  cout << "objectAt[" << object << " 1 " << time << "] ; " << endl;

  for (int t = 1; t <= (time-1); t++) {
    cout << "next[" << t << " " << t+1 << "] ; " ;
    cout << endl;
  }

  return 0;
}
