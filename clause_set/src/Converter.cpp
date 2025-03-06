#include "Converter.h"
#include "Counter.h"
#include "Set.h"
#include "PredicateSym.h"
// #include "SubgroupLite.h"

namespace zap
{

PfsConverter::PfsConverter(InputTheory& input) : m_input(input)
{
  fill_symbol_table();
  vector<PfsClause> clauses;
  for (size_t i=0; i < m_input.m_clauses.size(); i++) {
	vector<PfsClause> c = build_augmented(m_input.m_clauses[i]);
	clauses.insert(clauses.end(),c.begin(),c.end());
	
  }
  //  for (size_t i=0; i < clauses.size(); i++) cout << clauses[i] << endl;
  m_theory.initialize_clause_set(clauses);
}



ClauseSetBuilder::ClauseSetBuilder(const InputTheory& input) : m_input(input), m_converter(NULL)
{
  switch (m_input.type()) {
  case CNF: m_converter = new CnfConverter(m_input); break;
  case PFS: m_converter = new PfsConverter(m_input); break;
  case GROUP_BASED: quit("can't manage GROUP_BASED input type"); break;
  default: quit("can't manage input type");
  }
}


ClauseSet* PfsConverter::convert_to_cnf()
{
  vector<Clause> ground_clauses;
  m_theory.get_ground_clauses(ground_clauses);
  m_cnf_conversion = Cnf(ground_clauses);  // this is ugly and inefficient but keeps the Pfs class clean of Cnfs
  return &m_cnf_conversion;                // maybe I can fix this at lesat partially with a copy constructor
}

ClauseSet* PfsConverter::convert_to_symres()
{
   m_symres_conversion = SymRes(m_theory.global_group());
   m_symres_conversion.set_global_groups(m_theory.global_subgroups());
   m_symres_conversion.initialize_clause_set(m_theory);
  return &m_symres_conversion;                
}



/* A function to iterate through the set of all number partitions
   for a natural number n.

   EXAMPLE: The set of number partitions for 5 are:
            {5}, {4,1}, {3,2}, {3,1,1}, {2,1,1,1}, {1,1,1,1,1}

   The vector partition must be initialized to the partition {n}.
   This is represented by the array [n,0,0,...,0] where the
   first entry is n followed by n-1 zeros.

   Given a partition sorted in decreasing order, the next partition
   is created by subtracting 1 from the rightmost part greater than
   1 and distributing the remainder as quickly as possible.
 */

bool next_int_partition(vector<size_t> & partition)
{
  int i = partition.size() - 1;
  size_t sum = 0;
  for ( ; i >= 0 ; --i) {               // find the rightmost number
    if (partition[i] > 1) break;	// greater than 1
    else {
      sum += partition[i];
      partition[i] = 0;
    }
  }
  
  if (i < 0) return false;              // last partition, we're done
  
  --partition[i];                       // subtract 1 from rightmost
  ++sum;
  
  while (true) {                        // distribute the remaining sum
    if (sum > partition[i]) {
      partition[i + 1] = partition[i];
      sum -= partition[i];
      i++;
    }
    else {
      partition[i + 1] = sum;
      break;
    }
  }
  
  return true;
}

bool lex_least(const vector<size_t>& v)
{ 
  vector<bool> in(v.size()+1,false);
  for (size_t i=0; i < v.size(); i++) {
	if (!in[v[i]]) {
	  in[v[i]].flip();
	  for (size_t j=v[i]+1; j < in.size(); j++)
		if (in[j]) return false;
	}
  }
  return true;
}


/*
  This function takes an integer partition {n_1,n_2,...,n_k},
  and converts it to the multiset {n_1*1, n_1*2, ... , n_k*k} and then returns
  the set of all unique permutations of the multiset.
  
  EXAMPLE: Given the integer partition {2,2} we generate the multiset
           {2*1, 2*2} which is {1,1,2,2} and has the set of permutations
	   [1,1,2,2], [1,2,1,2], [2,1,1,2], [2,1,2,1], [1,2,2,1], [2,2,1,1]

	   and we only want the lex least ones
	   [1,1,2,2], [1,2,1,2], [1,2,2,1]

   We just do it the stupid way.   

 */

vector<vector<size_t> > get_arrangements(const vector<size_t> &partition)
{
  size_t sum = 0;
  vector<size_t> mapping;
  for (size_t i=0; i < partition.size(); i++) {
	sum += partition[i];
	for (size_t j=0; j < partition[i]; j++) mapping.push_back(i+1);
  }

  PermutationCounter counter(sum,sum);
  set<vector<size_t> > arrangements;
  do {
	vector<size_t> arrangement(counter.size(),0);
	for (size_t i=0; i < counter.size(); i++) arrangement[i] = mapping[counter[i]];
	if (lex_least(arrangement)) 
	  arrangements.insert(arrangement);
  } while (++counter);

  
  vector<vector<size_t> > answer;
  set<vector<size_t> >::iterator it = arrangements.begin();
  for ( ; it != arrangements.end(); it++) answer.push_back(*it);  // copy into a vector
	
  return answer;
}




/* Make sure that there is a string associated with every lit.  Just
iterate through the possible bindings of each predicate, calling
lookup to get the associated literal.  We discard the answer, but the
fact that we computed it stores it in the relevant map for use
later.

We also build a global group G that is a product group built from the
following factors: for each argument of every predicate, we take the full
symmetry group over the domain associated with that argument.  We store this
group in m_predicateSym which provides us with a mapping between elements
of G and the isomorphic action on the set of ground literals.  We use this
group and mapping to help build the groups associated with the first-order
clauses.

*/

void PfsConverter::fill_symbol_table()
{
  map<string,vector<string> >::iterator i;

  PredicateSpec& predicate_spec = m_input.m_predicateSpecs;
  DomainSpec& domain_spec = m_input.m_domainSpecs;
  AtomNameMap& ids = global_vars.atom_name_map;
  GlobalProductGroup global_product;
  PredicateSym predicate_sym;
  LiteralDomainValueMap literal_dv_map;
  
  for (i = predicate_spec.begin() ; i != predicate_spec.end() ; ++i) {
    string pred = i->first;
	vector<string> doms;
    vector<string> & args = i->second;
    if (args.empty()) continue;
	vector<size_t> dimensions;
    for (size_t a=0; a < args.size(); a++) {
	  dimensions.push_back(domain_spec[args[a]]);
	  doms.push_back(args[a]);
      vector<size_t> idx(args.size(),1);
      string arg_name = pred + to_string(a);
      vector<FSOrbit> orbits;
      //      idx[(a == 0 ? 1 : 0)] = 1;
      for ( ;; ) {
		FSOrbit o;
		for (size_t d=1; d <= domain_spec[args[a]]; d++) {
		  idx[a] = d;
		  o.push_back(ids.lookup(get_ground_string(pred,idx)));
		}
		orbits.push_back(o);
		size_t j = (a == 0 ? 1 : 0);
		if (j == args.size()) break;
		++idx[j];
		while (idx[j] > domain_spec[args[j]]) {
		  idx[j] = 1;
		  j = ((j+1) == a ? j+2 : j+1);
		  if (j == args.size()) break;
		  ++idx[j];
		}	
		if (j == args.size()) break;
      }
      global_product.add_symmetry(FullSym(orbits, arg_name,args[a]));
    }
	predicate_sym.add_predicate(pred,doms,dimensions);
  }

  m_theory = Pfs(global_product);
  predicate_sym.build_groups();
  m_theory.set_predicate_sym(predicate_sym);
  build_global_groups();

}

void PfsConverter::build_global_groups()
{
  GroupSpec& group_spec = m_input.m_groupSpecs;
  map<string,Ptr<ProductSubgroup> > groups;
  map<string,igroup>::iterator it = group_spec.begin();
  for ( ; it != group_spec.end(); it++) {
	if (it->second.size())  quit("we aren't reading in groups from permutations yet");
	if (it->second.m_ic.size()) quit("we aren't reading in groups from clauses yet" );
	vector<string>& domains = it->second.domains;
	ProductSubgroup g(&m_theory.global_group());
	for (size_t i=0; i < domains.size(); i++) {
	  size_t domain_sz = m_input.m_domainSpecs.lookup(domains[i]);
	  set<size_t> domain_pts;
	  for (size_t j=0; j < domain_sz; j++) domain_pts.insert(j);
	  vector<size_t> indexes;
	  for (size_t j=0; j < m_theory.global_group().size();  j++) {
		if (m_theory.global_group()[j].domain() == domains[i]) {
		  indexes.push_back(j);
		}
	  }
	  g.add_action(Action(indexes,domain_pts,&m_theory.global_group()));
	}
	groups[it->first] = Ptr<ProductSubgroup>(g);
  }

  m_theory.set_global_groups(groups);
}


/* The argument/variable sets are the basis for generating both the group and
   ground instances associated with a first-order clause.  The construction is
   quite simple.  We construct a directed graph G = (V,E) with V being the set
   of predicate arguments together with the set of all argument values.

   EXAMPLE:  FORALL(x y z)  F(x y) F(z #2) ;

   The set of arguments are { F0, F1 } and the set of argument values are
   { x, y, z, #2 } where #2 refers to a constant.

   The set of edges E = { (a, b) | arg value b appears as argument a,
                                   variable a appears as argument b }

   Note that constants are always sinks. Once the graph is constructed we simply
   identify connected components and the nodes in the component become an argument/
   variable set.  Each arg/var set defines a product in the product group associated
   with the clause.  Again, note that because constants are sinks, they may be
   included in more than one component without providing a link between the
   components.  
 */
vector<vector<vector<string> > > PfsConverter::build_argument_sets(InputClause& ic)
{
  map<string,set<string> > graph;
  Set<string> closed, open;
  for (size_t i=0; i < ic.size(); i++) {        // build the graph
    string pred = ic[i].name();
    for (size_t a=0; a < ic[i].size(); a++) {
      string arg_name = 'a' + pred + to_string(a);
      string var_name = (is_constant(ic[i][a]) ?  'c' : 'v' ) + ic[i][a];
      open.insert(arg_name);
      map<string,set<string> >::iterator p = graph.find(arg_name);
      if (p != graph.end()) p->second.insert(var_name);
      else { graph[arg_name] = set<string>(); graph[arg_name].insert(var_name);}
      if (var_name[0] == 'c') continue;
      open.insert(var_name);
      if ((p = graph.find(var_name)) != graph.end()) p->second.insert(arg_name);
      else { graph[var_name] = set<string>(); graph[var_name].insert(arg_name);}
    }
  }

  const vector<pair<string,string> >& restrictions = ic.restrictions();
  for (size_t i=0; i < restrictions.size(); i++)   // add restrictions to graph
    {
      string f1 = 'v' + restrictions[i].first;
      string f2 = 'v' + restrictions[i].second;
      map<string,set<string> >::iterator p = graph.find(f1);
      map<string,set<string> >::iterator q = graph.find(f2);
      if (p != graph.end() && q != graph.end()) {
	p->second.insert(f2);
	q->second.insert(f1);
      }
    }

  vector<vector<vector<string> > > arg_var_sets(2,vector<vector<string> >());
  while (!open.empty()) {                  // identify components
    set<string>::iterator it = open.begin();
    Set<string> local_open;
    vector<string> arg_set, var_set;
    string c;
    do { c = open.pop(); } while (closed.contains(c) && !open.empty());
    if (closed.contains(c)) continue;
    closed.insert(c);
    local_open.insert(graph[c].begin(), graph[c].end());
    if (c[0] == 'a') { c.erase(c.begin()); arg_set.push_back(c); }
    else { c.erase(c.begin()); var_set.push_back(c);}
    while (!local_open.empty()) {
      string n = local_open.pop();
      if (closed.contains(n)) continue;
      if (n[0] != 'c') closed.insert(n);
      local_open.insert(graph[n].begin(), graph[n].end());
      if (n[0] == 'a') { n.erase(n.begin()); arg_set.push_back(n);}
      else { n.erase(n.begin()); var_set.push_back(n);}
    }
    arg_var_sets[0].push_back(arg_set);  // add the component to the 
    arg_var_sets[1].push_back(var_set);  // arg/var sets
  }
  
  return arg_var_sets;
}



/*
  Welcome to combinatorial hell.  Here we generate a set of variable assignments
  for a variable set that can be used to generate the set of representative ground
  clauses for a group-based encoding of a first-order clause.  Each variable in
  var_set ranges over the same domain with size domain_size.  Consider the action
  of the group Sym(domain_size) on the variable assignments.  The set of all
  possible assignments is partitioned by Sym(domain_size) into one or more orbits.
  The set of assignments generated here will contain exactly one element from
  each orbit.  Applying the group Sym(domain_size) to this set of orbit reps
  we will be able to generate all assignments.

  Here's how we do it.  We start by generating all integer partitions of domain_size.
  For each partition we generate all unique arrangements.  (See documentation
  for next_int_partition, and get_arrangements in integer.cpp)  Then for each
  arrangement, we construct a real assignment.  We walk through the var_set
  and assign each variable a value based on the arrangement.  If vars x and y
  have the same value in the arrangement, then they need to have the same value
  in the assignment, similarly if they are different in the arrangement, they
  must be different in the assignment.  Some of the elements of var_set may be
  constants so their values must be fixed appropriately.  If they can't because
  the value is already taken or they are supposed to match a different value
  then the arrangement is abandoned and we move on to the next.

  This whole procedure is terribly inefficient, but at least it is clean and
  correct (I think).  An easy pruning technique that could be added would be to check
  each integer partition generated to see if its compatible with constants
  in var_set before we bother to generate all its arrangements.
 */
vector<vector<size_t> > PfsConverter::get_assignments(const vector<string>& var_set,
						      size_t domain_size)
{
  size_t i =0;
  for (; i < var_set.size(); i++)
    if (var_set[i][0] != '#') break;
  
  if (i == var_set.size())
    {
      vector<size_t> a;
      for (size_t j=0; j < var_set.size(); j++)
	a.push_back(get_constant_value(var_set[j]));
      return vector<vector<size_t> >(1,a);
    }
  
  vector<size_t> partition(var_set.size(),0);
  partition[0] = partition.size();
  
  vector<vector<size_t> > arrangements;     // for each partition generate 
  do {                                      // all unique arrangements
    vector<vector<size_t> > a = get_arrangements(partition);
    arrangements.insert(arrangements.end(),a.begin(),a.end());
  } while (next_int_partition(partition));
  
  Set<size_t> values;                       // build a Set of the domain values
  for (size_t i=1; i <= domain_size; i++) values.insert(i);
  
  vector<vector<size_t> > assignments;      // build an assignment for each arrangment
  for (size_t i=0; i < arrangements.size(); i++) {
    map<size_t,size_t> number_bindings;
    Set<size_t> available_values = values;
    vector<size_t> a(var_set.size(),0);
    bool works = true;
    for (size_t j=0; j < var_set.size(); j++) {
      size_t constant = 0, binding = 0;
      if (var_set[j][0] == '#') constant = get_constant_value(var_set[j]);
      map<size_t,size_t>::iterator it = number_bindings.find(arrangements[i][j]);
      if (it != number_bindings.end()) binding = it->second;
      
      if (constant) {
	a[j] = constant;
	if (constant != binding) {
	  if (!binding) {
	    available_values.erase(constant);
	    number_bindings[arrangements[i][j]] = constant;
	  }
	  else { works = false; break; }
	}
      }
      else {
	if (binding) a[j] = binding;
	else {
	  if (!available_values.empty()) a[j] = available_values.pop();
	  else { works = false; break; }
	  number_bindings[arrangements[i][j]] = a[j];
	}
      }
    }
    if (works) assignments.push_back(a);
  }
  
  return assignments;
}


/* Just a simple counter to iterate through a set of indexes. */
bool next_assignment(vector<size_t>& idx, const vector<size_t>& limits)
{
  if (!idx.size()) return false; 
  ++idx[0];
  size_t j = 0;
  while (idx[j] >= limits[j]) {
    idx[j] = 0;
    if (++j == limits.size()) break;
    ++idx[j];
  }	
  if (j == limits.size()) return false;
  return true;
}

/*********************************************************************************/
/*
              FUNCTIONS THAT HELP TRANSLATE A SINGLE INPUT CLAUSE
*/
/*********************************************************************************/
/*
  Here we build a set of augmented clauses that is equivalent to the InputClause ic.
  If the group was specified by the user the task is easy.  We just translate the
  InputLiterals to Literals, lookup the group and we're done.

  Otherwise, we begin by building argument/variable sets for the InputClause.
  These are explained in depth in documentation for function build_argument_sets
  defined below.  This data structure is the foundation that allows us to construct
  both the group and the set of representative clauses.

  Then we build the group by iteratively adding the symmetry of each
  argument set.  In the same loop we do some prep work for building the
  set of representative clauses (but only if representatives haven't been
  specified by the user).

  The group G constructed is a permutation group on the set of ground literals
  that describes the symmetry of the set of ground clauses.  G may not act
  transitively on the set of ground clauses.  Instead it may partition
  the set of ground clauses into a set of disjoint orbits.  If we want to
  generate all the ground clauses we will need, in addition to the group G,
  a representative from every orbit.  These representatives may be provided
  by the user who may only be interested in a subset of the orbits.

  EXAMPLE:
           Consider the pigeonhole constraint:

	   FORALL (p1 p2 h) -in(p1 h) v -in(p2 h)

	   by specifying the representative  -in(1 1) v -in(2 1)
	   we generate only the orbit with (p1 != p2) and eliminate
	   clauses with p1 = p2.

	  
  Building the set of representative clauses is easy if the reps were specified
  by the user.  If no representatives were specified, we build all
  representatives.  We iteratively generate the variable bindings for
  each representative, each time generating a ground clause, checking to make
  sure it's not a tautology, and finally combining it with the group G to
  make an augmented clause.
  
  */

vector<PfsClause> PfsConverter::build_augmented(InputClause& ic)
{
  if (ic.gid().size() >= 1) return build_augmented_from_group(ic);
  
  ic.bind_variables_to_domains(m_input.m_predicateSpecs);
  ic.valid_quantification(m_input.m_predicateSpecs);                  // error check
  InputClause e = m_input.handle_existentials(ic);

  vector<vector<vector<string> > > arg_var_sets(2,vector<vector<string> >());
  if (e.universal().size()) arg_var_sets = build_argument_sets(e);
  else {                                     // its a ground clause
    Clause c(e.get_ground_clause());
	return vector<PfsClause>(1,PfsClause(c,Ptr<ProductSubgroup>(ProductSubgroup(&m_theory.global_group()))));
  }

  const GlobalProductGroup& global_group = m_theory.global_group();
  ProductSubgroup P(&global_group);
  vector<vector<vector<size_t> > > assignments;
  for (size_t i=0; i < arg_var_sets[0].size(); i++) {   // build the group
	size_t index = global_group.index(arg_var_sets[0][i][0]);
	size_t domain_size = global_group[index].orbit_size();
	vector<size_t> indexes;
	for (size_t j=0; j < arg_var_sets[0][i].size(); j++)
	  indexes.push_back(global_group.index(arg_var_sets[0][i][j]));
    set<size_t> domain_pts = domain_points(arg_var_sets[1][i],domain_size);
    if (domain_pts.size() > 1) P.add_action(Action(indexes,domain_pts,&global_group));
	sort(arg_var_sets[1][i].begin(), arg_var_sets[1][i].end());
    assignments.push_back(get_assignments(arg_var_sets[1][i],domain_size));
  }

  // build the ground clauses
  vector<PfsClause> clauses;
  vector<size_t> idx(assignments.size(),0), limits(assignments.size(),0);
  for (size_t i=0; i < assignments.size(); i++) limits[i] = assignments[i].size();
  do {
	map<string,size_t> varValBindings;  // bind the variables to given assignment
	for (size_t i=0; i < arg_var_sets[1].size(); i++) {
	  for (size_t k=0; k < arg_var_sets[1][i].size(); k++) {
  		varValBindings[arg_var_sets[1][i][k]] = assignments[i][idx[i]][k];
	  }
	}
	if (e.check_restrictions(varValBindings)) {
	  // if its not a tautology add an new AugClause
	  Clause c = e.get_ground_clause(varValBindings);
	  if (!is_tautology(c)) {  // add the clause
		clauses.push_back(PfsClause(c,Ptr<ProductSubgroup>(P)));
	  }
	}
  } while (next_assignment(idx,limits));
  return clauses;
}

vector<PfsClause> PfsConverter::build_augmented_from_group(InputClause& ic) {
  const vector<string>& gid = ic.gid();
  if (gid.size() != 1) quit("only one group id per clause right now");
  Clause c(ic.get_ground_clause());
  c.group_identifier = gid[0];
  return vector<PfsClause>(1,PfsClause(c,m_theory.lookup_group(gid[0])));

  // look up the group in the group specs
}

} // end namespace zap
