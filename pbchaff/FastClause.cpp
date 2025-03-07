/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#include "FastClause.h"
#include "Clause.h"
#include "Rational.h"
#include "Literal.h"
#include <iostream>
#include <algorithm>

using namespace std;

void FastClause::initialize(const Clause &c)
{
  clear();
  m_required = c.getRequired();
  for (size_t i=0; i < c.size(); ++i)
    addAtom(c.getAtom(i),(c.getSign(i) ? 1 : -1));
  m_flags = 0;
}


void FastClause::initialize(const PBClause &c)
{
  clear();
  m_required = c.getRequired();
  for (size_t i=0; i < c.size(); ++i)
      addAtom(c.getAtom(i),(c.getSign(i) ? c.getWeight(i) : - c.getWeight(i)));
  m_flags = 0;
}

void FastClause::initialize(const Mod2Clause &c)
{
  clear();
  m_required = c.getSumMod2();
  for (size_t i=0; i < c.size(); ++i)
    addAtom(c.getAtom(i),1);
  m_flags = 1;
}


/** Add FastClauses f and *this.  Assign the result to *this.
    Remember that x + !x = 1, i.e. cancellation. */
void FastClause::add(const FastClause &f) {

  int a,v;
  
  /// add required field
  m_required += f.getRequired();

  /// add literals
  for (size_t i = 0; i < f.size(); ++i) {
    a = f.getAtom(i);
    v = f.getValue(a);
    if (v) {
      if (!(m_value[a])) addAtom(a,v);
      else {
	/// no cancellation
	if ((v < 0) == (m_value[a] < 0)) increment(a,v);
	else {
	  /// complete cancellation
	  if (-v == m_value[a]) {
	    remove(a);
	    m_required -= abs(v);
	  }
	  else {
	    /// partial cancellation
	    if (abs(v) < abs(m_value[a])) m_required -= abs(v);
	    else m_required -= abs(m_value[a]);
	    increment(a,v);
	  }
	}
      }
    }
  }	
}



/** Pseudo-Boolean version of resolution.  Just a special
   linear combination where two functions with opposing
   literals for a given variable are combined to form
   a new function with that variable completely cancelled
   out.
*/
void FastClause::resolve(FastClause &f, int a) {

  int weight1, weight2, g;

  ///check if clause are resolvable
  ASSERT((m_value[a] > 0) != (f.getValue(a) > 0 ));
      
  weight1 = abs(getValue(a));
  weight2 = abs(f.getValue(a));

  ASSERT(weight1 != 0 && weight2 != 0);
  
  // g is the greatest common denominator
  g = gcd(weight1,weight2);
  weight1 /= g;
  weight2 /= g;
  if (weight2 != 1) multiply(weight2);
  if (weight1 != 1) f.multiply(weight1);
  add(f);
}




void FastClause::divideQuick(int divisor)
{
  int weight;
  for (size_t i=0; i < m_size; ++i)
    {
      int atom = m_atoms[i];
      int val = m_value[atom];
      weight = val / divisor;
      if (val % divisor) weight += (val > 0) ? 1 : -1;
      m_value[atom] = weight;
    }
  
  weight = m_required / divisor;
  if (m_required % divisor) weight++;
  m_required = weight;    
}



/* do a little more work and potentially get a stronger constraint.
   We do the division, but we keep track of the result and remainder
   for each new coefficient.  We then walk along the constraint and
   see if we can optimize it a little.  If we have 
   a remainder bigger than one for m_required, then we may be able
   to use it to reduce a few coefficients for literals on the left.
   We only reduce a coefficient if we have enough extra to reduce
   all the coefficients in the constraint that have the same original
   coefficient as well.
 */
void FastClause::divideSmart(int divisor)
{
  vector<int> remainder(m_size,0);
  vector<int> result(m_size,0);

  // do the division
  for (size_t i=0; i < m_size; ++i)
    {
      int atom = getAtom(i);
      int weight = abs(getValue(atom));
      result[i] = weight / divisor;
      remainder[i] = weight % divisor;
    }

  int reqResult = m_required / divisor;
  int reqRemainder = m_required % divisor;
  int extra = 0;
  
  if (reqRemainder != 1)
    {
      // calculate the extra we have to spend
      if (reqRemainder == 0) extra = divisor - 1;
      else extra = reqRemainder - 1;

      // find a block of literals that all had the same
      // original weight
      int oldWeight = abs(getValue(m_size-1));
      size_t start = m_size-1;
      size_t end = start;
      for (int i=m_size-1; i >= 0; --i)
	{
	  int atom = getAtom(i);
	  int newWeight = abs(getValue(atom));
	  if (newWeight == oldWeight)
	    {
	      start = i;
	      if (i != 0) continue;
	    }
	  // found a block delemited by start and end
	  // is our extra enough to reduce this whole block?
	  int count = end - start + 1;
	  if ((remainder[start] * count) <= extra)
	    {
	      extra -= remainder[start] * count;
	      for (size_t j=start; j <= end; ++j)
		remainder[j] = 0;
	    }
	  
	  if (extra == 0) break;
	  
	  // get set up to find a new block
	  start = i;
	  end = i;
	  oldWeight = newWeight;
	}
    }

  // now fill in the results of the division
  for (size_t i=0; i < m_size; ++i)
    {
      int atom = getAtom(i);
      int value = getValue(atom);
      int weight = (remainder[i] > 0) ? (result[i] + 1) : result[i];
      m_value[atom] = (value >= 0) ? weight : -weight;
    }

  // remove any literals with 0 weights
  for (size_t i=0; i < m_size; ++i)
    {
      if (getWeight(i) == 0)
	{
	  removeByIndex(i);
	  --i;
	}
    }
  
  m_required = (reqRemainder > 0) ? ++ reqResult : reqResult;
}



/* Do our best to reduce the size of coefficients in the constraint
   maintaining logical equivalency. The basic idea is we try
   dividing by every coefficient starting with the largest one.
   We stop when the division produces a logically equivalent constraint.
 */
void FastClause::simplify()
{
  // cut down any coefficients bigger than m_required
  for (size_t i=0; i < m_size; ++i)
    {
      int a = m_atoms[i];
      int v = m_value[a];
      if (abs(v) >= m_required) m_value[a] = (v > 0) ? m_required : -m_required;
    }

  if (m_required == 1) return;

  // sort the literals by weight
  size_t size = m_size;
  PBLiteral* lits = new PBLiteral[size];
  for (size_t i=0; i < size; ++i)
    {
      int atom = getAtom(i);
      int val = getValue(atom);
      lits[i] = PBLiteral(atom,(val > 0),abs(val));
    }
  PBLiteral* end = lits + size;
  sort(lits,end,compareWeight);

  // keep a copy of original values
  int required = m_required;

  int divisor = 1;
  int oldWeight = 0;
  for (size_t i=0; i < size; ++i)
    {
      // find the next new coefficient to try dividing with
      int newWeight = lits[i].getWeight();
      if (oldWeight == newWeight) continue;
      oldWeight = newWeight;

      // load the original lits
      clear();
      for (size_t j=0; j < size; ++j)
	{
	  int atom = lits[j].getAtom();
	  int weight = lits[j].getWeight();
	  int value = lits[j].getSign() ? weight : -weight;
	  addAtom(atom,value);
	}
      m_required = required;
      divideSmart(newWeight);
      multiply(newWeight);

      // now we check to see if the new constraint subsumes
      // the old one.
      
      // calculate extra
      int extra = 0;
      for (size_t j=0; j < size; ++j)
	{
	  int atom = lits[j].getAtom();
	  int weight = lits[j].getWeight();
	  int newWeight = abs(getValue(atom));
	  if (newWeight > weight) extra += newWeight - weight;
	}
      if ((m_required - extra) >= required)
	{
	  divisor = newWeight;
	  break;
	}
    }

  if (divisor != 1) divideQuick(oldWeight);
  else
    {
      // we couldn't find a divisor so put back the old values
      clear();
      for (size_t j=0; j < size; ++j)
	{
	  int atom = lits[j].getAtom();
	  int weight = lits[j].getWeight();
	  int value = lits[j].getSign() ? weight : -weight;
	  addAtom(atom,value);
	}
      m_required = required;
    }
  delete [] lits;
  return;
}


/* Determine whether the FastClause f is subsumed (logically implied)
   by clause pointed to by this.
*/
bool FastClause::subsumes(const FastClause &f) const 
{
  
  int rhs = m_required;
  if (f.getRequired() == 0) return true;

  // force coefficients to be less than or equal
  // to coefficients in f.
  for (size_t i = 0; i < m_size; ++i)
    {
      int atom = getAtom(i);
      int val = getValue(atom);
      int fval = f.getValue(atom);

      // if f coefficient is 0 (doesn't appear in constraint
      // or the literals are opposing remove literal from the constraint
      if ((fval == 0) || ((val > 0) == (fval < 0))) rhs -= abs(val);
      if (rhs <= 0) return false;
    }
  
  Rational u(1,1);
  int maxfVal = 0;
  for (size_t i = 0; i < m_size; ++i)
    {
      int atom = getAtom(i);
      int val = getValue(atom);
      int fval = f.getValue(atom);
      int absVal = abs(val);
      int absfVal = abs(fval);

      // round off coefficients greater than right hand side
      if (absVal > rhs) absVal = rhs;

      
      if ((fval != 0) && ((val > 0) == (fval >= 0)))
	{
	  if (maxfVal < absfVal) maxfVal = absfVal;
	  if (absVal > absfVal)
	    {
	      Rational x(absfVal,absVal);
	      if (u > x) u = x;
	    }
	}
    }

  if (u != 1)
    {
      u *= rhs;
      rhs = u.ceiling(); 
      return (rhs >= f.getRequired());
    }

  if (rhs >= f.getRequired()) return true;
  
  int extra = 0;
  for (size_t i=0; i < m_size; ++i)
    {
      int atom = getAtom(i);
      int val = getValue(atom);
      int fval = f.getValue(atom);
      int absVal = abs(val);
      int absfVal = abs(fval);
      if (absVal > rhs) absVal = rhs;
      
      if ((fval != 0) && ((val > 0) == (fval >= 0)))
	{
	  int newVal = absVal * maxfVal;
	  if (newVal > absfVal)
	    {
	      extra += newVal - absfVal;
	    }
	}
    }
  rhs *= maxfVal;
  return ((rhs - extra) >= f.getRequired());
}


/* Weaken the constraint to the strongest possible cardinality constraint.
   
 */
void FastClause::weakenToCardinality(const PartialAssignment &pa, int specialAtom)
{ 
  int sum = sumCoefficients();
  int numberExtras = 0;
  int maxCoefficient = getWeight(0);
  if (maxCoefficient == 1) return;
  bool foundMax = false;

  size_t i=0;
  for (; sum >= m_required && i < m_size; ++i)
    {
      int atom = getAtom(i);
      int weight = getWeight(i);
      bool sign = getSign(i);
      
      if (pa.isUnsat(Literal(atom,sign)) || (atom == specialAtom))
	{
	  if (!foundMax)
	    {
	      foundMax = true;
	      maxCoefficient = weight;
	    }
	  sum -= weight;
	  m_value[atom] = sign ? 1 : -1;
	}
      else
	{
	  if (!foundMax || weight >= maxCoefficient)
	    {
	      m_value[atom] = sign ? 1 : -1;
	      ++numberExtras;
	    }
	  else
	    {
	      remove(atom);
	      --i;
	    }
	}
    } 

  for (; i < m_size; ++i)
    {
      int atom = getAtom(i);
      int weight = getWeight(i);
      bool sign = getSign(i);

      if (weight >= maxCoefficient)
	{
	  m_value[atom] = sign ? 1 : -1;
	  ++numberExtras;
	}
      else
	{
	  remove(atom);
	  --i;
	}
    }
  
  m_required = 1 + numberExtras;
  
}


/* get all literals valued favorably with respect to 
     the PartialAssignment */
void FastClause::getFavorables(AtomValueMap& faves, const PartialAssignment& pa) const
{
  for (size_t i=0; i < m_size; i++)
    {
      int atom = getAtom(i);
      if (pa.isValued(atom) && !pa.isUnsat(getLiteral(i))) 
	faves.addIfAbsent(atom,getSign(i) ? 1 : -1);
    }
}


/* get all literals valued unfavorably with respect to 
     the PartialAssignment */
void FastClause::getUnfavorables (AtomValueMap& unfaves, const PartialAssignment& pa) const
{
  for (size_t i=0; i < m_size; i++)
    {
      if (pa.isUnsat(getLiteral(i))) 
	unfaves.addIfAbsent(getAtom(i),getSign(i) ? 1 : -1);
    }
}



void FastClause::strengthen(AtomValueMap& assumptions) {

  m_required += 1;
  int amount = 1;

  for (size_t i=0,sz=assumptions.size(); i < sz; ++i)
    {
      int a = assumptions.getAtom(i);
      bool s = assumptions.getSign(i);
    

      if (!contains(a))
	// if its not there add it
	addAtom(a, (s ? amount : -amount));
      else {
	bool sign = m_value[a] > 0;
	int weight = abs(m_value[a]);
	
	if (sign == s) {
	  // no cancellation
	  if (sign) m_value[a] += amount;
	  else m_value[a] -= amount;
	}
	else {
	  // complete cancelation
	  if (weight == amount) {
	    remove(a);
	    m_required -= amount;
	  }
	  else {
	    // partial cancellation
	    if (weight < amount) m_required -= weight;
	    else m_required -= amount;
	    if (sign) m_value[a] -= amount;
	    else m_value[a] += amount;
	  }
	}
      }
    }
}

  


void FastClause::print(ostream& os) const
{
  AtomValueMap::print();
  os << (isMod2() ? " mod2 = " : " >= ") << m_required << endl;
}







