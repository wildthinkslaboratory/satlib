#include<iostream>
using namespace std;

int main(int argc, char* argv[])
{
  if ( 5 != argc )
    {
      cout << "Usage: " << argv[0] << " <objects> <locations> <planes> <timesteps>" << endl;
      exit(1);
    }

  int objects = atoi(argv[1]);
  int locations = atoi(argv[2]);
  int planes = atoi(argv[3]);
  int times = atoi(argv[4]);

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
  

//   cout << "// helper axiom" << endl;
//   cout << "~next[1 2] ~next[2 3] ~objectAt[1 1 1] objectAt[1 1 3] ";
//   for (size_t i=0; i < planes; i++) cout << " planeAt[" << i+1 << " 1 2] ";
//   cout << " GROUP GLOBAL ; " << endl << endl;
	


  cout << endl << "// initial conditions " << endl;
  for (size_t o=1; o <= objects; o++)
    cout << "objectAt[" << o << " " << (rand() % locations) + 1 << " 1] ; " << endl;

  for (size_t p=1; p <= planes; p++)
    cout << "planeAt[" << p << " " << (rand() % locations) + 1 << " 1] ; " << endl;


  
  cout << endl << "// final conditions " << endl;
  for (size_t o=1; o <= objects; o++)
    cout << "objectAt[" << o << " " << (rand() % locations) + 1 << " " << times << "] ; " << endl;




  return 0;
}
