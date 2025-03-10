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

  cout << "GROUP PRED objects locations planes time ; " << endl << endl;
  
  cout << "// frame axioms " << endl;


  cout << "~next[1 2] ~objectAt[1 1 1]  objectAt[1 1 2] ";
  for (size_t i=0; i < planes; i++) cout << " inPlane[1 " << i+1 << " 2] ";
  cout << " GROUP PRED ;" << endl;
  
  cout << "~next[1 2] ~inPlane[1 1 1]  inPlane[1 1 2] ";
  for (size_t i=0; i < locations; i++) cout << " objectAt[1 " << i+1 << " 2] ";
  cout << " GROUP PRED ;" << endl << endl;

  cout << "// change of state axioms " << endl;
  cout << "~next[1 2] ~objectAt[1 1 1]  ~inPlane[1 1 2]  planeAt[1 1 1] " << " GROUP PRED ;" << endl;
  cout << "~next[1 2] ~objectAt[1 1 1]  ~inPlane[1 1 2]  planeAt[1 1 2] " << " GROUP PRED ;" << endl;
  cout << "~next[1 2] ~inPlane[1 1 1]  ~objectAt[1 1 2]  planeAt[1 1 1] " << " GROUP PRED ;" << endl;
  cout << "~next[1 2] ~inPlane[1 1 1]  ~objectAt[1 1 2]  planeAt[1 1 2] " << " GROUP PRED ;" << endl << endl;

  cout << "// consistency of state " << endl;

  if (locations > 1) {
	cout << "~planeAt[1 1 1]  ~planeAt[1 2 1] " << " GROUP PRED ;" << endl;
	cout << "~objectAt[1 1 1] ~objectAt[1 2 1]" << " GROUP PRED ;" << endl;
  }
  if (planes > 1)
	cout << "~inPlane[1 1 1] ~inPlane[1 2 1]" << " GROUP PRED ;" << endl;
  cout << "~objectAt[1 1 1] ~inPlane[1 1 1] " << " GROUP PRED ;" << endl;
  for (size_t i=0; i < locations; i++) cout << " objectAt[1 " << i+1 << " 1] ";
  for (size_t i=0; i < planes; i++) cout << " inPlane[1 " << i+1 << " 1] ";
  cout << " GROUP PRED ;" << endl << endl;
  

  cout << "// helper axiom" << endl;
  cout << "~next[1 2] ~next[2 3] ~objectAt[1 1 1] objectAt[1 1 3] ";
  for (size_t i=0; i < planes; i++) cout << " planeAt[" << i+1 << " 1 2] ";
  cout << " GROUP PRED ; " << endl << endl;
	


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
