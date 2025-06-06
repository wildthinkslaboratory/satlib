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
  cout << "PREDICATE inPlane( objects planes time ) ; " << endl << endl;

  cout << "GROUP  GLOBAL objects locations planes ; " << endl << endl;

  cout << "// frame axioms " << endl;

  for (size_t t=0; t < (times-1); t++) {
	cout << "~objectAt[1 1 " << t+1 << "]  objectAt[1 1 " << t+2 << "] ";
	for (size_t i=0; i < planes; i++) cout << " inPlane[1 " << i+1 << " " << t+2 << "] ";
	cout << " GROUP GLOBAL ;" << endl;
  }
  cout << endl;
  for (size_t t=0; t < (times-1); t++) {
	cout << "~inPlane[1 1 " << t+1 << "]  inPlane[1 1 " << t+2 << "] ";
	for (size_t i=0; i < locations; i++) cout << " objectAt[1 " << i+1 << " " << t+2 << "] ";
	cout << " GROUP GLOBAL ;" << endl;
  }
  cout << endl;
  
  cout << "// change of state axioms " << endl;
  for (size_t t=0; t < (times-1); t++) {
	cout << "~objectAt[1 1 " << t+1 << "]  ~inPlane[1 1 " << t+2 << "]  planeAt[1 1 " << t+1 << "] " << " GROUP GLOBAL ;" << endl;
	cout << "~objectAt[1 1 " << t+1 << "]  ~inPlane[1 1 " << t+2 << "]  planeAt[1 1 " << t+2 << "] " << " GROUP GLOBAL ;" << endl;
	cout << "~inPlane[1 1 " << t+1 << "]  ~objectAt[1 1 " << t+2 << "]  planeAt[1 1 " << t+1 << "] " << " GROUP GLOBAL ;" << endl;
	cout << "~inPlane[1 1 " << t+1 << "]  ~objectAt[1 1 " << t+2 << "]  planeAt[1 1 " << t+2 << "] " << " GROUP GLOBAL ;" << endl << endl;
  }

  
  cout << "// consistency of state " << endl;
  for (size_t t=0; t < times; t++) {
	if (locations > 1) {
	  cout << "~planeAt[1 1 " << t+1 << "]  ~planeAt[1 2 " << t+1 << "] " << " GROUP GLOBAL ;" << endl;
	  cout << "~objectAt[1 1 " << t+1 << "] ~objectAt[1 2 " << t+1 << "]" << " GROUP GLOBAL ;" << endl;
	}
	if (planes > 1)
	  cout << "~inPlane[1 1 " << t+1 << "] ~inPlane[1 2 " << t+1 << "]" << " GROUP GLOBAL ;" << endl;
	cout << "~objectAt[1 1 " << t+1 << "] ~inPlane[1 1 " << t+1 << "] " << " GROUP GLOBAL ;" << endl;
	for (size_t i=0; i < locations; i++) cout << " objectAt[1 " << i+1 << " " << t+1 << "] ";
	for (size_t i=0; i < planes; i++) cout << " inPlane[1 " << i+1 << " " << t+1 << "] ";
	cout << " GROUP GLOBAL ;" << endl << endl;
  }
  

  cout << "// helper axioms" << endl;
  for (size_t t=1; t < (times-1); t++) {
	cout << "~objectAt[1 1 " << t << "] objectAt[1 1 " << t+2 << "] ";
	for (size_t i=0; i < planes; i++) cout << " planeAt[" << i+1 << " 1 " << t+1 << "] ";
	cout << " GROUP GLOBAL ; " << endl;
  }
	


  cout << endl << "// initial conditions " << endl;
  for (size_t o=1; o <= objects; o++)
    cout << "objectAt[" << o << " " << o << " 1] ; " << endl;

  for (size_t p=1; p <= planes; p++)
    cout << "planeAt[" << p << " " << locations << " 1] ; " << endl;


  
  cout << endl << "// final conditions " << endl;
  for (size_t o=1; o < objects; o++)
    cout << "objectAt[" << o << " " << o+1 << " " << times << "] ; " << endl;

  cout << "objectAt[" << objects << " 1 " << times << "] ; " << endl;



  return 0;
}
