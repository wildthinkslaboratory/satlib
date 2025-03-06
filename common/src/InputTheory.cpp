#include <set>
#include "InputTheory.h"

namespace zap
{
/* Generate the set of domain points associated with a set of arguments.  Arguments
   may be variables or constants, so we want the set of all domain points minus
   the values of any constants int the set of arguments.  
 */

set<size_t> domain_points(const vector<string>& a, size_t domain_sz)
{
  set<size_t> domain_pts;
  for (size_t i=1; i <= domain_sz; i++) domain_pts.insert(i-1);
  for (size_t i=0; i < a.size(); i++) {
    if (is_constant(a[i])) {
      size_t constant = get_constant_value(a[i]);
      domain_pts.erase(constant-1);
    }
  }
  return domain_pts;
}



void InputClause::bind_variables_to_domains(PredicateSpec& P)
{
  m_varDomBindings.clear();	// map from var name to domain name
  for (size_t i = 0 ; i < size() ; ++i)
    for (size_t j = 0 ; j < (*this)[i].size() ; ++j)
	  if (is_var((*this)[i][j]))
		m_varDomBindings[(*this)[i][j]] = P[(*this)[i].name()][j];
}


bool InputClause::check_restrictions(map<string,size_t>& b)
{
  for (size_t i=0; i < m_restrictions.size(); i++)
    if (b[m_restrictions[i].first] == b[m_restrictions[i].second]) return false;
  return true;
}


ClauseSetType InputClause::type() const
{
  if (m_universals.size() || m_existentials.size()) return PFS;
  if (m_gid.size()) return PFS;
  if (m_op != GEQ && m_op != 0) return GROUP_BASED;
  if (m_value != 1 && m_value != 0) return GROUP_BASED;
  return CNF;
}

ClauseSetType InputTheory::type() const
{
  vector<size_t> type_count(4,0);
  for (size_t i=0; i < m_clauses.size(); i++) 
	type_count[m_clauses[i].type()]++;
  if (type_count[PFS] && type_count[GROUP_BASED]) quit("Hybrid constraint set: can't manage this yet");
  if (type_count[PFS]) return PFS;
  if (type_count[GROUP_BASED]) return GROUP_BASED;
  return CNF;
 
}


/* Handle existentials by building up the disjunction one variable at
a time.  */

InputClause InputTheory::handle_existentials(const InputClause &ic)
{
  InputClause ans = ic;
  for (size_t i = 0 ; i < ic.existential().size() ; ++i)
    handle_existential(ans,ic.existential()[i]);
  return ans;
}

/* Handle existential quantification over a single variable.  For each
literal in the clause, see if the variable appears.  If not, it's
easy.  If so, disjoin each bound instance of the literal in
question.  */

void InputTheory::handle_existential(InputClause &ic, const string &var)
{ 
  vector<InputLiteral> newlits;
  for (size_t i = 0 ; i < ic.lits().size() ; ++i)
    if (find(ic.lit(i).begin(),ic.lit(i).end(),var) != ic.lit(i).end()) {
      int lim = m_domainSpecs[ic.variableDomainBindings()[var]];
      for (int j = 0 ; j < lim ; ++j) {
		InputLiteral newlit = ic.lit(i);
		for (size_t k = 0 ; k < newlit.size() ; ++k) {
		  ostringstream s;
		  s << '#' << (j + 1);
		  if (newlit[k] == var) newlit[k] = s.str();
		}
		newlits.push_back(newlit);
      }
    }
    else newlits.push_back(ic.lit(i));
  ic.lits() = newlits;
}

/* Here we check all the things that can be messed up in a clause:

1. Existential quantifiers may be mixed with an equality operator.
   (Not handled because = becomes a conjunction.)
2. A variable may appear under the scope of multiple quantifiers. 
2. A predicate may have the wrong number of arguments.
3. A parameter may appear more than once with different sorts.
4. A parameter may not be in the scope of any quantifier.
5. A variable appearing under a quantifier may not appear in any
   predicate expression, making it impossible to determine its sort. */
enum { MULTIPLE_SCOPE, NUM_ARGS, SORT, FREE, UNSORTED, BAD_EXISTS, VALID };

template <class It1, class It2>
bool meets(It1 f1, It1 l1, It2 f2, It2 l2) {
  while (f1 != l1 && f2 != l2)
    if (*f1 == *f2) return true;
    else if (*f2 < *f1) ++f2;
    else ++f1;
  return false;
}

void InputClause::valid_quantification(PredicateSpec& predicateSpec)
{
  static string qerror[] = {
    "variable appears in scope of multiple quantifiers",
    "incorrect number of arguments to relation symbol",
    "variable appears multiple times in expression with different sorts",
    "variable in expression is not quantified",
    "quantified variable does not appear in predicate symbol (undefined sort)",
    "existential quantifier with RHS = or =%" };
  
  const vector<string> & u = universal();
  const vector<string> & e = existential();
  if (e.size() && (mod() || op() == EQ)) {
    cerr << "In clause: " << *this << endl;
    quit(qerror[BAD_EXISTS]);
  }
  const set<string> us(u.begin(),u.end());
  const set<string> es(e.begin(),e.end());
  if (us.size() != u.size() || es.size() != e.size()) {
    cerr << "In clause: " << *this << endl;
    quit(qerror[MULTIPLE_SCOPE]);
  }
  if (meets(us.begin(),us.end(),es.begin(),es.end())) {
    cerr << "In clause: " << *this << endl;
    quit(qerror[MULTIPLE_SCOPE]);
  }
  for (size_t l = 0 ; l < size() ; ++l) {
    const InputLiteral & lit = operator[](l);
    if (predicateSpec[lit.name()].size() != lit.size()) {
      cerr << "In clause: " << *this << endl;
      quit(qerror[NUM_ARGS]);
    }
    for (size_t p = 0 ; p < lit.size() ; ++p) if (is_var(lit[p])) {
      if (m_varDomBindings[lit[p]] != predicateSpec[lit.name()][p]) {
	cerr << "In clause: " << *this << endl;
	quit(qerror[SORT]);
      }
      if (find(u.begin(),u.end(),lit[p]) == u.end() &&
	  find(e.begin(),e.end(),lit[p]) == e.end()) {
	cerr << "In clause: " << *this << endl;
	quit(qerror[FREE]);
      }
    }
  }
  for (size_t i = 0 ; i < u.size() ; ++i) 
    if (m_varDomBindings.find(u[i]) == m_varDomBindings.end()) {
      cerr << "In clause: " << *this << endl;
      quit(qerror[UNSORTED]);
    }
  for (size_t i = 0 ; i < e.size() ; ++i) 
    if (m_varDomBindings.find(e[i]) == m_varDomBindings.end()) {
      cerr << "In clause: " << *this << endl;
      quit(qerror[UNSORTED]);
    }
}

/* String associated with a bound predicate symbol.  Just stick it all
together.  */
string get_ground_string(const string& predicate,
				       const vector<size_t>& values)
{
  ostringstream ground_name;
  ground_name << predicate;
  if (!values.empty()) {
    ground_name << '[';
    for (size_t i = values.size() - 1 ; i > 0 ; --i)
      ground_name << values[i] << ' ';
    ground_name << values[0] << ']';
  }
  return ground_name.str();
}


/* ID of an atom corresponding to an instance of a literal.  Build up
the string, and then make the correspondingly sized atom.  */
Literal InputLiteral::id(const vector<size_t>& values) const
{
  string s = get_ground_string(name(),values);
  return Literal(global_vars.atom_name_map.lookup(s),sign());
}


/* Get the literal associated with an input literal.  The InputLiteral
   is assumed to be ground (i.e. all of its arguments are constants).
*/
Literal InputLiteral::id() const
{
  vector<size_t> values(size());
  for (size_t j = 0 ; j < size() ; ++j)
    if (is_constant(operator[](j))) values[j] = get_constant_value(operator[](j));
    else quit("unknown value in get_lit");
  return id(values);
}

/* Build a ground clause (i.e. a set of Literals) from a set of grounded
   InputLiterals (i.e. the arguments to all InputLiterals are constants).
 */ 
Clause InputClause::get_ground_clause() const
{
  Clause c;
  for (size_t j=0; j < size(); j++) {
    c.push_back(operator[](j).id());
  }
  return c;
}



/* Build a ground clause (i.e. a set of Literals) from an InputClause and a
   set of variable value bindings.
 */
Clause InputClause::get_ground_clause(map<string,size_t>& var_bindings) const
{
  set<Literal> c;
  for (size_t i=0; i < size(); i++) {
    string pred = operator[](i).name();
    vector<size_t> values(operator[](i).size(),0);
    for (size_t a=0; a < operator[](i).size(); a++) {
      string var = operator[](i)[a];
      if (is_constant(var)) values[a] = get_constant_value(var);
      else {
		map<string,size_t>::iterator it = var_bindings.find(var);
		if (it == var_bindings.end()) quit("Bad variable value binding");
		values[a] =it->second;
      }
    }
    string atom = get_ground_string(pred,values);
    size_t id = global_vars.atom_name_map.lookup(atom);
    Literal l = Literal(id,operator[](i).sign());
    c.insert(l);
  }
  
  Clause clause;
  set<Literal>::iterator it = c.begin();
  for (; it != c.end(); it++) clause.push_back(*it);
  return clause;
}

} // end namespace zap
