/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/
							       

#ifndef _LITERAL_H
#define _LITERAL_H

#include <iostream>

/*****************************************************************/
/*

   CLASS: Literal

   PURPOSE: A literal is a variable in either a positive or negated
   form.

   IMPLEMENTATION: we store a literal in a single int, with first bit
   used to represent the sign of the literal (positive or negative).
   The remaining bits store the variable identifier.

********************************************************************/

class Literal {
protected:
  int m_value;
public:                   
						
  Literal() : m_value(0) {}
  Literal(std::size_t atom, bool sign) : m_value((atom<<1) + sign) {}

  /******************* READ *************************/

  unsigned int getAtom() const { return m_value >> 1; }
  bool getSign() const { return m_value & 1; }
  Literal getNegation() const { return Literal(getAtom(),!getSign()); }

  void print(std::ostream& os = std::cout) const
    {
      os << ((m_value & 1) ? " " : "-") << (m_value >> 1) << " " << std::flush;
    }
  
  /******************* WRITE *************************/

};

inline std::ostream& operator<<(std::ostream& os, const Literal& lit)
{
  lit.print(os);
  return os;
}



/*****************************************************************/
/*

   CLASS: PBLiteral

   PURPOSE: A PBLiteral is a variable in either a positive or negated
   form with a coefficient weight.

   IMPLEMENTATION: A Literal with an added int field called m_weight.
   
********************************************************************/

class PBLiteral : public Literal
{
protected:
  std::size_t m_weight;
public:                   
						
  PBLiteral() : m_weight(0) {}
  PBLiteral(std::size_t atom, bool sign, std::size_t weight) :
    Literal(atom,sign),
    m_weight(weight) {}

  /******************* READ *************************/

  std::size_t getWeight() const { return m_weight; }
  void print(std::ostream& os = std::cout) const
    {
      Literal::print();
      os << ":" << m_weight << std::flush;
    }
  
  /******************* WRITE *************************/

};

inline std::ostream& operator<<(std::ostream& os, const PBLiteral& lit)
{
  lit.print(os);
  return os;
}





/***************************************************************/
/* 

   CLASS: PoolLiteral

   PURPOSE: Represent a literal, or a clauseID, or a null literal.
   The main data structure of LazyClauseSet classes is a
   large array of PoolLiterals called the pool.  This is where
   all the clauses are stored.  A clause is a series of
   PoolLiterals with the last PoolLiteral containing the ID of the
   clause.  The ID marks the end of the clause with the following
   PoolLiteral being the first literal in a different clause.
   When a clause is deleted, the literals in that clause are
   turned into null literals.  Therefore a PoolLiteral may be
   either a literal, an ID, or a null.

   
   INTERFACE:
           isLiteral, isID, isNull - these functions test what
	   kind of PoolLiteral an object is.

	   getID, setID - functions for managing an ID PoolLiteral

	   getAtom, getSign, setLiteral - functions for managing
	   a literal PoolLiteral

	   isWatched, getDirection, setWatch, unsetWatch - These
	   are used for the chaff lazy indexing scheme for literals.
	   They tell us if a literal is being watched in the
	   literal index of the LazyClauseSet.  A literal can
	   be watched UP, or watched DOWN and hence has a direction.  

	   clear - make the PoolLiteral null.
	   

   IMPLEMENTATION: A PoolLiteral is an int and 3 bools, but 
   they are represented in a single int with the help of some
   bit manipulation.

   literal - atom - positive integer greater than zero. stored in
                    bits 3 and up.

		    NOTE: the number 0 is not a
		    valid atom identifier.

		    
             sign - bool stored in bit 2
	     
	  watched - true if the literal is being watched in
	            LazyClauseSet literal index. stored in bit 1
		    
        direction - direction of the watched literal, 1 for UP
	            and 0 for DOWN stored in bit 0;

   ID - negative number NOTE: the number 0 is not a valid ID number

   null - zero

********************************************************************/


class PoolLiteral
{
  
protected:
  int m_value;
  
public:
  
  PoolLiteral() : m_value(0) {}

  // ************************** READ ********************************* 
  
  bool isWatched() const { return m_value & 2; }
  bool isLiteral() const { return m_value > 0; }
  bool isNull() const { return m_value == 0; }
  bool isID() const { return m_value < 0; }
  int getAtom() const { return m_value >> 3; }
  bool getSign() const { return m_value & 4; }  
  int getID() const { return -m_value; }
  bool getDirection() const { return m_value & 1; }
  inline void print(std::ostream& os = std::cout) const;
  inline void printDetail(std::ostream& os = std::cout) const;

  //*************************** WRITE ****************************

  void setLiteral(int var, bool sign) {m_value = (((var << 1) + sign) << 2);}  
  void setID(int id) { m_value = - id; }
  void clear() { m_value = 0; }
  void setWatch(int direction) { m_value += direction + 2; }
  void unsetWatch() { m_value &= (-4); }
  
};





/***************************************************************/
/*

   CLASS: PoolPBLiteral

   PURPOSE: This class is the same as PoolLiteral with the
   addition of a field called m_weight. The weight field allows us
   to represent a weighted Literal which is needed for
   full Pseudo-Boolean constraints.

   INTERFACE: same as above with the addition of functions
   to manage the weight field.

   IMPLEMENTATION: same as above.
   
********************************************************************/


class PoolPBLiteral : public PoolLiteral
{
  
protected:
  std::size_t m_weight;
  
public:
  
  PoolPBLiteral() : m_weight(0) {}

  // ************************** READ ********************************* 
  
  std::size_t getWeight() const { return m_weight; }
  inline void print(std::ostream& os = std::cout) const;
  inline void printDetail(std::ostream& os = std::cout) const;


  //*************************** WRITE ****************************

  inline void setLiteral(int v, bool s, int w);  
  void clear() { m_value = 0; m_weight = 0; }
  
};



//*************************** INLINE FUNCTIONS ************************



inline void PoolLiteral::print(std::ostream& os) const
{
  os << ((m_value & 4) ? " " : "-")
     << (m_value >> 3) << " " << std::flush;   
}


inline void PoolLiteral::printDetail(std::ostream& os) const
{
  print(os);
  if (isWatched())
    os << "watched " << ( getDirection() ? "UP  " : "DOWN  ") << std::flush;
}

inline std::ostream& operator<<(std::ostream& os, const PoolLiteral& plit)
{
  plit.print(os);
  return os;
}


inline void PoolPBLiteral::setLiteral(int v, bool s, int w)
{
  m_value = (((v << 1) + s) << 2);
  m_weight = w;
}  


inline void PoolPBLiteral::print(std::ostream& os) const
{
  os << ((m_value & 4) ? " " : "-")
     << (m_value >> 3);
  if (m_weight == 1)
     os << " " << std::flush;
  else
    os << ":" << m_weight << " " << std::flush;
}


inline void PoolPBLiteral::printDetail(std::ostream& os) const
{
  print(os);
  if (isWatched())
    os << "watched " << ( getDirection() ? "UP  " : "DOWN  ") << std::flush;
}

inline std::ostream& operator<<(std::ostream& os, const PoolPBLiteral& plit)
{
  plit.print(os);
  return os;
}


inline bool compareWeight(const PBLiteral& left, const PBLiteral& right)
{
  return left.getWeight() > right.getWeight();
}


#endif

   




