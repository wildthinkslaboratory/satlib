
#include <iostream>
#include <string>

#include "Cnf.h"
#include "Pfs.h"
#include "SymRes.h"
#include "common.h"

int main(int argc, char* argv[]) {
  // use my set
  zap::Set<int> mySet;
  mySet.insert(1);
  mySet.insert(2);

  for (auto x : mySet) std::cout << x << " ";

  std::cout << std::endl;

  std::cout << zap::global_vars.clauses_touched << endl;
  return 0;
}
