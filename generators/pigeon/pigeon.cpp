#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
  if (2 != argc) {
    cout << "Usage: pigeon <numberPigeons>" << endl;
    exit(1);
  }
  int num = atoi(argv[1]);

  if (num < 3) {
	cout << num << " is not enough pigeons " << endl;
	exit(1);
  }

  cout << "SORT pigeon " << num << " ;" << endl;
  cout << "SORT hole " << num - 1 << " ;" << endl << endl;

  cout << "PREDICATE in( pigeon hole ) ; " << endl << endl;
  cout << "GROUP PRED pigeon hole ; " << endl << endl;

  cout << "// only one pigeon allowed per hole" << endl;
  cout << "-in[1 1] -in[2 1] GROUP PRED ; " << endl << endl;
  
  cout << "// every pigeon gets a hole " << endl;
  for (size_t i=0; i < num-1; i++) cout << " in[1 " << i+1 << "] ";
  cout << "GROUP PRED ; " << endl << endl;

  return 0;
}





