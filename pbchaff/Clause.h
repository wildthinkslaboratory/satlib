/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/



#ifndef _CLAUSE_H
#define _CLAUSE_H

#include "PartialAssignment.h"
#include "ImplicationList.h"
#include "FastClause.h"


#define STARTUP_LIT_POOL_SIZE 0X1000
#define UP               1
#define DOWN             0






/*****************************************************************/
/**

   CLASS: Clause
   
   PURPOSE: Provide access to a clause in the literal pool.  It
   provides functions for accessing and manipulating pool literals
   in a clause.  A clause is currently a cardinality constraint.
   A set of literals and a positive integer which determines the
   number of literals that must be sat to satisfy the whole constraint.
   
   INTERFACE:
	     size - return the number of literals in a clause.

	     inUse - true if clause is considered part of the clauseSet.

	     getRequired() - return the number of literals that must
	     be satisfied.
	     
	     getAtom(int), getSign(int), getReadLiteral(int),
	     getWriteLiteral(int) - provide access to literals by index.

	     initialize - Initialize clause to point to a given clause.

	     markUnused - mark a clause as deleted.

	     **********************************************************
	     
	     These functions are used in memory management when clauses
	     are copied to different memory.
	     
	     setFirst - Set the m_firstLiteral*

	     incFirst - Increment the m_firstLiteral* by an amount.


   IMPLEMENTATION: A Clause is basically a pointer into the
   literal pool.  The pointer m_firstLiteral points to the first
   literal in the clause.

   m_sizeAndFlags store the number of values.

             size - bits 2 and up.
	     inUse - bit 1
	     Unused flag - bit 0.

	     
**********************************************************************/

class Clause
{
protected:
  PoolLiteral * m_firstLiteral;
  std::size_t m_required;
  std::size_t m_sizeAndFlags;
  
public:

  Clause() : m_firstLiteral(0),
		 m_required(0),
		 m_sizeAndFlags(0) {}
  
  //************************ READ *********************************
  
  std::size_t size() const { return m_sizeAndFlags >> 2; }
  bool inUse() const { return m_sizeAndFlags & 2; }
  std::size_t getRequired() const { return m_required; }
  const PoolLiteral& getReadLiteral(int i) const { return m_firstLiteral[i]; }
  int getAtom(int i) const { return m_firstLiteral[i].getAtom(); }
  bool getSign(int i) const { return m_firstLiteral[i].getSign(); }
  void print(std::ostream& os = std::cout) const;
  void printDetail(std::ostream& os = std::cout) const;  

  //*********************** WRITE *********************************

  inline void initialize(PoolLiteral *p, std::size_t r, std::size_t n); 
  void markUnused() { m_sizeAndFlags &= ~2 ; }
  PoolLiteral& getWriteLiteral(int i) { return m_firstLiteral[i]; }
  void setFirst(PoolLiteral *plp) { m_firstLiteral = plp; }
  void incFirst(int i) { m_firstLiteral += i; }
  PoolLiteral * getFirst() { return m_firstLiteral; }
};





/***************************************************************/
/**

   CLASS: PBClause

   PURPOSE: This class is the same as Clause except it uses
   PoolPBLiterals instead of PoolLiterals.  It can therefore
   represent a Pseudo-Boolean constraint.

   INTERFACE: same as above with the addition of being to
   access a literal's weight (its coefficient).

   IMPLEMENTATION: same as above.
   
********************************************************************/



class PBClause 
{
protected:
  PoolPBLiteral * m_firstLiteral;
  std::size_t m_required;
  std::size_t m_sizeAndFlags;
  
public:

  PBClause() : m_firstLiteral(0),
		 m_required(0),
		 m_sizeAndFlags(0) {}
  
  //************************ READ *********************************
  
  std::size_t size() const { return m_sizeAndFlags >> 2; }
  bool inUse() const { return m_sizeAndFlags & 2; }
  std::size_t getRequired() const { return m_required; }
  const PoolPBLiteral& getReadLiteral(int i) const { return m_firstLiteral[i]; }
  int getAtom(int i) const { return m_firstLiteral[i].getAtom(); }
  bool getSign(int i) const { return m_firstLiteral[i].getSign(); }
  int getWeight(int i) const { return m_firstLiteral[i].getWeight(); }
  void print(std::ostream& os = std::cout) const;
  void printDetail(std::ostream& os = std::cout) const;  

  //*********************** WRITE *********************************

  inline void initialize(PoolPBLiteral *p, std::size_t r, std::size_t n);
  void markUnused() { m_sizeAndFlags &= ~2 ; }
  PoolPBLiteral& getWriteLiteral(int i) { return m_firstLiteral[i]; }
  void setFirst(PoolPBLiteral *plp) { m_firstLiteral = plp; }
  void incFirst(int i) { m_firstLiteral += i; }
  PoolPBLiteral * getFirst() { return m_firstLiteral; }
};



class Mod2Clause
{
protected:
  PoolLiteral * m_firstLiteral;
  bool m_sumMod2; // this could be added to the flags
  std::size_t m_sizeAndFlags;
  
public:

  Mod2Clause() : m_firstLiteral(0),
		 m_sumMod2(false),
		 m_sizeAndFlags(0) {}
  
  //************************ READ *********************************
  
  std::size_t size() const { return m_sizeAndFlags >> 2; }
  bool inUse() const { return m_sizeAndFlags & 2; }
  bool getSumMod2() const { return m_sumMod2; }
  const PoolLiteral& getReadLiteral(int i) const { return m_firstLiteral[i]; }
  int getAtom(int i) const { return m_firstLiteral[i].getAtom(); }
  bool getSign(int i) const { return m_firstLiteral[i].getSign(); }
  void print(std::ostream& os = std::cout) const;
  void printDetail(std::ostream& os = std::cout) const;  

  //*********************** WRITE *********************************

  inline void initialize(PoolLiteral *p, bool r, std::size_t n); 
  void markUnused() { m_sizeAndFlags &= ~2 ; }
  PoolLiteral& getWriteLiteral(int i) { return m_firstLiteral[i]; }
  void setFirst(PoolLiteral *plp) { m_firstLiteral = plp; }
  void incFirst(int i) { m_firstLiteral += i; }
  PoolLiteral * getFirst() { return m_firstLiteral; }
};







// these are some extra classes that are used by all the ClauseSet classes.
// simple struct for conflicts
class Conflict
{
public:
  int atom;
  int reason1;
  int reason2;
};

class ClauseSetStatistics
{
public:
  std::size_t literalCount;
  bool outOfMemory;
  std::size_t outOfMemoryCount;
  std::size_t initialClauseCount;
  std::size_t addedClauseCount;
  std::size_t deletedClauseCount;

  ClauseSetStatistics() :
    literalCount(0),
    outOfMemory(false),
    outOfMemoryCount(0),
    initialClauseCount(0),
    addedClauseCount(0),
    deletedClauseCount(0) {}
    
};

class ClauseSetSettings
{
public:
  std::size_t memoryLimit;
  std::size_t relevanceBound;
  std::size_t lengthBound;
  std::size_t maxLength;
  bool strengthenOn;

  ClauseSetSettings() :
    memoryLimit(1024*1024*256), // thats .25 GIG
    relevanceBound(20),
    lengthBound(100),
    maxLength(5000),
    strengthenOn(false) {}
};




//********************* INLINE FUNCTIONS *********************************


inline void Clause::initialize(PoolLiteral *p, std::size_t r, std::size_t n)
{
  m_firstLiteral = p;
  m_required = r;
  m_sizeAndFlags = (n << 2) + 2;
}

inline std::ostream& operator << (std::ostream& os, const Clause& c)
{
  c.print(os);
  return os;
}


inline void PBClause::initialize(PoolPBLiteral *p, std::size_t r, std::size_t n)
{
  m_firstLiteral = p;
  m_required = r;
  m_sizeAndFlags = (n << 2) + 2;
}


inline std::ostream& operator << (std::ostream& os, const PBClause& c)
{
  c.print(os);
  return os;
}



inline void Mod2Clause::initialize(PoolLiteral *p, bool r, std::size_t n)
{
  m_firstLiteral = p;
  m_sumMod2 = r;
  m_sizeAndFlags = (n << 2) + 2;
}

inline std::ostream& operator << (std::ostream& os, const Mod2Clause& c)
{
  c.print(os);
  return os;
}



#endif
