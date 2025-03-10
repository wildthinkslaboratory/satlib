#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
  

  if (2 != argc) {
    cout << "Usage: pigeonCNF <numberPigeons>" << endl;
    exit(1);
  }
  int num = atoi(argv[1]);
  
  int no_vars = num * (num + 1);
  int no_constraints = (num * num * (num + 1) / 2 ) + num + 1;
  cout << "p cnf " << no_vars << " " << no_constraints << endl;
  for (int i = 0; i < num; i++) {
    for (int j = 1; j <= num; j++) {
      for (int k = j+1; k <= (num + 1); k++ ) {
	cout << "-"
	     << j + (i * (num + 1))
	     << " -"
	     << k + (i * (num + 1))
	     << " " << 0 << endl;
      }
    }
  }

  for (int i = 1; i <= (num + 1); i++) {
    for (int j = 0; j < num; j++)
      cout <<  i + (j * (num + 1)) << " ";
    cout << 0 << endl;
  }
  return 0;
}




