#ifndef _INPUT_THEORY
#define _INPUT_THEORY

/***  Please see copyright notice in common.h  ***/

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iostream>
#include <set>
#include "common.h"
using namespace std;

namespace zap
{
inline ostream& operator<< (ostream& os, const vector<string>& vs) {
  for (size_t i=0; i < vs.size(); i++) os << vs[i] << " ";
  return os;
}


/*****************************************************************/
/*  
 Support functions for the parser; use for diagnostics and the like.
 Not intended for use outside the parser.
*/
/****************************************************************/

enum { LT = -5, LEQ, EQ, GEQ, GT }; /* comparators */

/* Utility to see if a parameter name is a constant. */

inline bool   is_constant(const string s) { return s[0] == '#'; }
inline bool   is_var     (const string s) { return s[0] != '#'; }
inline size_t get_constant_value(const string& name) { return atoi(name.substr(1).c_str()); }
string get_ground_string(const string& predicate, const vector<size_t>& values);
set<size_t> domain_points(const vector<string>& a, size_t domain_sz);


/* An ATOM is a (possibly empty) vector of parameter names along with
a symbol name.  */

class InputAtom : public vector<string> {
public:
  string	 m_name;

  InputAtom() { }
  InputAtom(const char* n) : m_name(n) { }
  InputAtom(const string& n, const vector<string>& p) : 
    vector<string>(p), m_name(n) { }
  InputAtom(const char* n, const vector<string>& p) :
    vector<string>(p), m_name(n) { }

  string to_string() const {
	stringstream s;
	s << m_name << "(";
	for (size_t i=0; i < size(); i++) {
	  s << operator[](i);
	  if (i != size()-1) s << ",";
	}
	s << ")";
	return string(s.str());
  }

  /****************************** WRITE ******************************/
  
  void clear() { m_name = ""; vector<string>::clear(); }

  /****************************** PRINT ******************************/

  friend ostream& operator << (ostream& os, const InputAtom& ia) {
    os << ia.m_name;
    if (ia.empty()) os << ' ';
    else {
      os << '[';
      for (size_t i = 0 ; i < ia.size() ; ++i) {
	if (i) os << ' ';
	os << ia[i];
      }
      os << ']';
    }
    return os;
  }
};

/* A LITERAL is an atom and a sign. */

class InputLiteral : public InputAtom {
  bool m_sign;
  
public:
  /****************************** READ ******************************/
  
  bool sign() const	          { return m_sign; }
  const string& name() const      { return m_name; }
  void set_name(const string & s) { m_name = s; }
  Literal id(const vector<size_t>& values) const;
  Literal id() const;
  
  /****************************** WRITE ******************************/
 
  void set_literal(bool w, const InputAtom& a) { 
    m_sign = w;
    InputAtom::operator=(a);
  }
  void clear() { InputAtom::clear(); }
  
  /****************************** PRINT ******************************/

  friend ostream& operator << (ostream& os, const InputLiteral& il) {
    if (!il.sign()) os << '-';
    os << (InputAtom)(il);
    return os;
  }
};

inline bool operator<(const InputLiteral &l1, const InputLiteral &l2) 
{ return l1.name() < l2.name(); }


/* A PREDICATE SPECIFICATION is a map from predicate symbols to
vectors of argument types.  */

class PredicateSpec : public map<string,vector<string> >
{
 public:
  /****************************** READ ******************************/

  vector<string>& lookup(const string& name) {
    map<string,vector<string> >::iterator p = find(name);
    if (p == end()) quit("Predicate lookup failed!");
    return p->second;
  }

  /****************************** PRINT ******************************/

  friend ostream& operator << (ostream& os, const PredicateSpec& p) {
    map<string,vector<string> >::const_iterator b;
    for (b = p.begin() ; b != p.end() ; b++) {
      os << "PREDICATE " << b->first << "( ";
      for (size_t i = 0 ; i < b->second.size() ; ++i) os << b->second[i] << ' ';
      os << ')' << endl;
    }
    return os;
  }
};

/* A DOMAIN SPECIFICATION is a map from domain symbols to sizes.  */

class DomainSpec : public map<string,size_t> {
 public:
  /****************************** READ ******************************/

  size_t lookup(const string& name) {
    map<string,size_t>::iterator p = find(name);
    if (p == end()) quit("Domain lookup failed");
    return p->second;
  }

  /****************************** PRINT ******************************/

  friend ostream& operator << (ostream& os, const DomainSpec& d) {
    map<string,size_t>::const_iterator b;
    for (b = d.begin() ; b != d.end() ; b++) 
      os << "SORT " << b->first << "  " << b->second << "  ";
    return os;
  }
};

/* A CLAUSE is vectors of symbols quantified over universally or
existentially, and a flag indication that the universal quantification
is noteq.  We've got a vector of included literals, and then an
operator < <= = >= >, a modulus (0 if no modulus) and a value for the
RHS.  Finally, there's a vector of group names if we're describing the
group that way.  */

class InputClause {
  vector<string>       m_universals;
  vector<string>       m_existentials;
  vector<InputLiteral> m_lits;
  vector<pair<string,string> > m_restrictions;
  int		       m_op;	/* LT, LEQ, EQ, GEQ, GT */
  unsigned	       m_mod;	/* 0 or modulus */
  unsigned	       m_value;
  vector<string>       m_gid;
  
public:

  map<string,string> m_varDomBindings;
  
  InputClause() : m_op(0), m_mod(0), m_value(0) { }

  /****************************** READ ******************************/

  bool is_ground() const 
    { return m_universals.empty() && m_existentials.empty(); }
  size_t size() const { return m_lits.size(); }
  const InputLiteral& operator[](size_t i) const { return m_lits[i]; }
  ClauseSetType type() const;

  /****************************** READ/WRITE ************************/
  
  const vector<string> & universal() const { return m_universals; }
  vector<string> & universal()		   { return m_universals; }

  const vector<string> & existential() const { return m_existentials; }
  vector<string> & existential()	     { return m_existentials; }
  
  const vector<pair<string,string> >& restrictions() const { return m_restrictions; }  
  vector<pair<string,string> >& restrictions() { return m_restrictions; }

  map<string,string>& variableDomainBindings() { return m_varDomBindings; }
  bool check_restrictions(map<string,size_t>& b);
  void valid_quantification(PredicateSpec& predicateSpec);
  Clause get_ground_clause() const;
  Clause get_ground_clause(map<string,size_t>& var_bindings) const;

  /****************************** WRITE ******************************/

  void add_literals(const vector<InputLiteral>& lits) { m_lits = lits; }
  void add_literal(const InputLiteral& l) { m_lits.push_back(l); }
  void set_name(int i,const string & s)   { m_lits[i].set_name(s); }
 
  void clear() { 
    m_universals.clear();
    m_existentials.clear();
    m_lits.clear();
    m_op = GEQ;
    m_mod = 0;
    m_value = 1;
    m_gid.clear();
  }

  void sort() { std::sort(m_lits.begin(),m_lits.end()); }
  void bind_variables_to_domains(PredicateSpec& P);

  /****************************** READ/WRITE ************************/

  int op() const       { return m_op; }
  int & op()	       { return m_op; }

  unsigned mod() const	     { return m_mod; }
  unsigned & mod()	     { return m_mod; }

  unsigned value() const       { return m_value; }
  unsigned & value()	       { return m_value; }

  const vector<string> & gid() const { return m_gid; }
  vector<string> & gid()	     { return m_gid; }

  const InputLiteral & lit(int i) const     { return m_lits[i]; }
  const vector<InputLiteral> & lits() const { return m_lits; }
  vector<InputLiteral> & lits()		    { return m_lits; }

  /****************************** PRINT ******************************/

  friend ostream& operator << (ostream& os, const InputClause& c) {
    if (!c.universal().empty()) os << "FORALL " << c.universal() << ' ';
    if (!c.existential().empty()) os << "EXISTS " << c.existential() << ' ';
    for (size_t i = 0 ; i < c.m_lits.size() ; ++i) os << c.m_lits[i] << ' ';
    if (c.m_gid.empty()) { 
      if (c.m_mod == 0)
	switch (c.m_op) {
	case LT  : os << " < "; break;
	case LEQ : os << " <= "; break;
	case EQ  : os << " = "; break;
	case GEQ : os << " >= "; break;
	case GT  : os << " > "; break;
	default:
	  {
	    os << endl << "restrictions " ;
	    for (size_t i=0; i < c.m_restrictions.size(); i++)
	      os << c.m_restrictions[i].first << " != " << c.m_restrictions[i].second << "  ";
	    
	  }
	}
      else os << " %" << c.m_mod << " = ";
      if (c.m_mod || c.m_op) os << c.m_value << "  ";
    }
    else {
      os << "GROUP";
      for (size_t i = 0 ; i < c.m_gid.size() ; ++i) os << ' ' << c.m_gid[i];
    }
    return os;
  }
};


class icycle : public vector<InputLiteral> {
 public:
  friend ostream & operator << (ostream & os, const icycle & i) {
    os << '(';
    for (size_t j = 0 ; j < i.size() ; ++j) {
      if (j) os << ' ';
      os << i[j];
    }
    os << ')';
    return os;
  }
};

class iperm : public vector<icycle> {
 public:
  friend ostream & operator << (ostream & os, const iperm & i) {
    os << '(';
    for (size_t j = 0 ; j < i.size() ; ++j) {
      if (j) os << ' ';
      os << i[j];
    }
    os << ')';
    return os;
  }
};

class igroup : public vector<iperm> {
 public:
  InputClause m_ic;
  vector<string>    domains;
  friend ostream & operator << (ostream & os, const igroup & i) {
    os << '<';
    for (size_t j = 0 ; j < i.size() ; ++j) os << ' ' << i[j];
    os << " >";
    return os;
  }
};

class GroupSpec : public map<string,igroup> {
 public:
  /****************************** READ ******************************/

  const igroup & lookup(const string& name) {
    map<string,igroup>::iterator p = find(name);
    if (p == end()) quit("Group lookup failed");
    return p->second;
  }

  /****************************** PRINT ******************************/

  friend ostream& operator << (ostream& os, const GroupSpec& d) {
    map<string,igroup>::const_iterator b;
    for (b = d.begin() ; b != d.end() ; b++) 
      os << "GROUP " << b->first << "  " << b->second << "  ";
    return os;
  }
};

/* An INPUT THEORY consists of domain specifications, predicate
specifications, and clauses.  */

class InputTheory
{
 public:
  DomainSpec		m_domainSpecs;
  PredicateSpec		m_predicateSpecs;
  GroupSpec		    m_groupSpecs;
  vector<InputClause>	m_clauses;

  InputClause handle_existentials(const InputClause& ic);
  void handle_existential(InputClause& ic, const string& var);
  ClauseSetType type() const;
  
  /****************************** PRINT ******************************/

  friend ostream& operator << (ostream& os, const InputTheory& it) {
    os << endl << "Domain Specifications : " << endl 
       << it.m_domainSpecs << endl << endl 
       << "Predicate Specifications : " << endl 
       << it.m_predicateSpecs << endl << endl 
       << "Clauses : " << endl;
    for (size_t i = 0 ; i < it.m_clauses.size() ; ++i)
      os << it.m_clauses[i] << endl;
    return os;
  }
};

} // end namsepace zap
#endif // _INPUT_THEORY_H
