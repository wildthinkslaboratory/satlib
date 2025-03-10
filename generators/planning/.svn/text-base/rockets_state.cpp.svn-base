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

  int objects = m*n + 1;
  int locations = m*n + 2;
  int planes = n;
  int times = (m+2)*2;

  cout << "SORT objects " << objects << ";" << endl;
  cout << "SORT locations " << locations << ";" << endl;
  cout << "SORT planes " << planes << ";" << endl;
  cout << "SORT time " << times << ";" << endl << endl;
  
  cout << "PREDICATE objectAt( objects locations time ) ; " << endl;
  cout << "PREDICATE planeAt( planes locations time ) ; " << endl;
  cout << "PREDICATE inPlane( objects planes time ) ; " << endl;
  cout << "PREDICATE next( time time ) ;" << endl << endl;

  cout << "// frame axioms " << endl;


  cout << "FORALL(o l t1 t2) EXISTS(p)"
       << " ~next[t1 t2] ~objectAt[o l t1]  objectAt[o l t2]  inPlane[o p t2] " << endl;
  cout << "\t {" << endl;
  cout << "\t\t t1 != t2" << endl;
  cout << "\t } ;" << endl;
  cout << "FORALL(o p t1 t2) EXISTS(l)"
       << " ~next[t1 t2] ~inPlane[o p t1]  inPlane[o p t2]  objectAt[o l t2] " << endl;
  cout << "\t {" << endl;
  cout << "\t\t t1 != t2" << endl;
  cout << "\t } ;" << endl << endl;
  
  cout << "// change of state axioms " << endl;
  cout << "FORALL(o l p t1 t2) ~next[t1 t2] ~objectAt[o l t1]  ~inPlane[o p t2]  planeAt[p l t1] ;" << endl;
  cout << "FORALL(o l p t1 t2) ~next[t1 t2] ~objectAt[o l t1]  ~inPlane[o p t2]  planeAt[p l t2] ;" << endl;
  cout << "FORALL(o l p t1 t2) ~next[t1 t2] ~inPlane[o p t1]  ~objectAt[o l t2]  planeAt[p l t1] ;" << endl;
  cout << "FORALL(o l p t1 t2) ~next[t1 t2] ~inPlane[o p t1]  ~objectAt[o l t2]  planeAt[p l t2] ;" << endl << endl;

  cout << "// consistency of state " << endl;
  cout << "FORALL(p t l m) ~planeAt[p l t]  ~planeAt[p m t] " << endl;
  cout << "\t {" << endl;
  cout << "\t\t l != m" << endl;
  cout << "\t } ;" << endl << endl;

  cout << "FORALL(o t l m) ~objectAt[o l t] ~objectAt[o m t]" << endl;
  cout << "\t {" << endl;
  cout << "\t\t l != m" << endl;
  cout << "\t } ;" << endl;
 
  cout << "FORALL(o t p q) ~inPlane[o p t] ~inPlane[o q t]" << endl;
  cout << "\t {" << endl;
  cout << "\t\t p != q" << endl;
  cout << "\t } ;" << endl;
  
  cout << "FORALL(o t l p) ~objectAt[o l t] ~inPlane[o p t] ;" << endl; 
  cout << "FORALL(o t) EXISTS(p l)  objectAt[o l t]  inPlane[o p t] ; " << endl << endl;




  cout << endl << "// initial conditions " << endl;
  for (size_t o=1; o <= objects; o++)
    cout << "objectAt[" << o << " " << o << " 1] ; " << endl;

  for (size_t p=1; p <= planes; p++)
    cout << "planeAt[" << p << " " << locations << " 1] ; " << endl;


  
  cout << endl << "// final conditions " << endl;
  for (size_t o=1; o < objects; o++)
    cout << "objectAt[" << o << " " << o+1 << " " << times << "] ; " << endl;

  cout << "objectAt[" << objects << " 1 " << times << "] ; " << endl;

  for (int t = 1; t <= (times-1); t++) {
    cout << "next[" << t << " " << t+1 << "] ; " ;
    cout << endl;
  }

  return 0;
}
