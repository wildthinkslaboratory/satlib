
#include <iostream>
#include <set>
#include <vector>

using namespace std;

int main(int argc, char* argv[] ) {
  if ( 4 != argc )
    {
      cerr << "Usage: quasigroup <gridsize> <percent complete> <seed>" << endl;
      exit(1);
    }
  size_t grid_size = atoi(argv[1]);
  size_t seed = atoi(argv[3]);
  srand(seed);
  
  cout << "// domain specs" << endl;
  cout << "SORT row " << grid_size << " ;" << endl;
  cout << "SORT column " << grid_size << " ;" << endl;
  cout << "SORT color " << grid_size << " ;" << endl << endl;

  cout << "// predicate specs" << endl;
  cout << "PREDICATE Colored ( row column color ) ;" << endl << endl;
  cout << "GROUP PRED row column color ; " << endl << endl;

  cout << "// first order constraints " << endl;
  cout << "// every square gets a color" << endl;
  for (size_t i=0; i < grid_size; i++) 
	cout << "Colored[ 1 1 " << i+1  << " ] " ;
  cout << " GROUP PRED ; " << endl << endl;
  
  cout << "// a color appears at most once per row" << endl;
  cout << "~Colored[ 1 1 1 ] ~Colored[ 2 1 1 ] GROUP PRED ; " << endl << endl;
  cout << "// a color appears at most once per column" << endl;
  cout << "~Colored[ 1 1 1 ] ~Colored[ 1 2 1 ] GROUP PRED ; " << endl << endl;


  int percent_complete = atoi(argv[2]);
  vector<vector<size_t> > grid(grid_size,vector<size_t>(grid_size,0));
  vector<size_t> grid_squares;
  int num_squares = grid_size * grid_size;
  int num_complete = num_squares * percent_complete / 100;
  for (size_t i=0; i < num_squares; i++) grid_squares.push_back(i);

  
  while (num_complete > 0)
    {
      // randomly pick one of the remaining grid squares
      size_t index = rand() & (grid_squares.size()-1);
      size_t square = grid_squares[index];
      grid_squares.erase(remove(grid_squares.begin(),grid_squares.end(),square),grid_squares.end());
      --num_complete;

      // translate square number into i,j indices
      size_t i = square / grid_size;
      size_t j = square % grid_size;
      
      // determine allowed range of colors
      set<size_t> exclude;
      for (size_t k=0; k < grid.size(); k++) exclude.insert(grid[i][k]);
      for (size_t k=0; k < grid.size(); k++) exclude.insert(grid[k][j]);

      vector<size_t> allowed;
      for (size_t k=1; k <= grid_size; k++) if (exclude.find(k) == exclude.end()) allowed.push_back(k);

      if (allowed.size() == 0)
	{
	  cerr << "Could not create a valid grid.  Try a different random seed" << endl;
	  exit(1);
	}
      
      // randomly pick a color
      index = rand() & (allowed.size() - 1);
      
      // set the color in the grid
      grid[i][j] = allowed[index];
    }


  
  cout << "// partial coloring for grid " << percent_complete << "% complete with random seed " << seed << endl;

  for (size_t i=0; i < grid.size(); i++)
    {
      cout << "// \t";
      for (size_t j=0; j < grid[i].size(); j++)
	{
	  if (grid[i][j] == 0) cout << "* \t";
	  else cout << grid[i][j] << "\t" ;
	}
      cout << endl;
    }
  for (size_t i=0; i < grid.size(); i++)
    {
      for (size_t j=0; j < grid[i].size(); j++)
	{
	  if (grid[i][j] == 0 ) continue;
	  cout << "Colored[ " << i+1 << ' ' << j+1 << ' ' << grid[i][j] << " ]  ; " << endl;
	}
    }
  
  return 1;
  
}




