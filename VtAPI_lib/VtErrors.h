/** \file VtErrors.h

 The main exception handling routines.
 
   $Author: david $
   $Revision: 1.2 $
   $Date: 2013/05/27 12:04:18 $
 
   \par REQUIREMENTS: 
   A set of classes to deal with the iXAPI error handling requirements.
 
   \par SPECIFICATIONS: 
   To provide functionality to handle error messages, error locations and error types.
 
   \par DESIGN NOTES: 
   None.
 
  Copyright (c) 2013 by
  All Rights Reserved
 
 */

#ifndef __VT_ERRORS_H__
#define __VT_ERRORS_H__


#include <stdexcept>
#include <stdio.h>


namespace Vt 
{

class ContractViolation : public std::exception
{
public:

  /**
   * Constructor 1
   */
  ContractViolation(char const * prefix, char const * message, 
                    char const * file, int line);
  
  /**
   * Constructor 2
   */
  ContractViolation(char const * prefix, char const * message);

  /**
   * Destructor
   */
  virtual ~ContractViolation() {}
  
  /**
   * returns the details of the violation
   */
  virtual const char * what() const throw() { return what_; }

private:

  // Parameters
  enum { bufsize_ = 1100 };
  char what_[bufsize_];
};


/**
 *  $Author: david $
 *  $Revision: 1.2 $
 *  $Date: 2013/05/27 12:04:18 $
 *
 *  \par REQUIREMENTS: 
 *  A set of classes to deal with the iXAPI error handling requirements.
 *
 *  \par SPECIFICATIONS: 
 *  To provide functionality to handle error messages, error locations and error types.
 *
 *  \par DESIGN NOTES: 
 *  None.
 *
 */
class PreconditionViolation : public ContractViolation
{
public:

  /**
   * Constructor 1
   */
  PreconditionViolation(char const * message, const char * file, int line)
  : ContractViolation("Precondition violation!", message, file, line)
  {}
  
  /**
   * Constructor 2
   */
  PreconditionViolation(char const * message)
  : ContractViolation("Precondition violation!", message)
  {}
};


/**
 *  $Author: david $
 *  $Revision: 1.2 $
 *  $Date: 2013/05/27 12:04:18 $
 *
 *  \par REQUIREMENTS: 
 *  A set of classes to deal with the iXAPI error handling requirements.
 *
 *  \par SPECIFICATIONS: 
 *  To provide functionality to handle error messages, error locations and error types.
 *
 *  \par DESIGN NOTES: 
 *  None.
 *
 */
class  PostconditionViolation : public ContractViolation
{
public:

  /**
   * Constructor 1
   */
  PostconditionViolation(char const * message, const char * file, int line)
  : ContractViolation("Postcondition violation!", message, file, line)
  {}
  
  /**
   * Constructor 2
   */
  PostconditionViolation(char const * message)
  : ContractViolation("Postcondition violation!", message)
  {}
};


/**
 *  $Author: david $
 *  $Revision: 1.2 $
 *  $Date: 2013/05/27 12:04:18 $
 *
 *  \par REQUIREMENTS: 
 *  A set of classes to deal with the iXAPI error handling requirements.
 *
 *  \par SPECIFICATIONS: 
 *  To provide functionality to handle error messages, error locations and error types.
 *
 *  \par DESIGN NOTES: 
 *  None.
 *
 */
class  InvariantViolation : public ContractViolation
{
public:

  /**
   * Constructor 1
   */
  InvariantViolation(char const * message, const char * file, int line)
  : ContractViolation("Invariant violation!", message, file, line)
  {}
  
  /**
   * Constructor 2
   */
  InvariantViolation(char const * message)
  : ContractViolation("Invariant violation!", message)
  {}
};


#ifndef NDEBUG


//*********************************************************************
// throw_invariant_error
//*********************************************************************
inline void throw_invariant_error(bool predicate, char const * message, char const * file, int line)
{
  if(!predicate)
    throw Vt::InvariantViolation(message, file, line); 
}


//*********************************************************************
// throw_precondition_error
//*********************************************************************
inline void throw_precondition_error(bool predicate, char const * message, char const * file, int line)
{
  if(!predicate)
    throw Vt::PreconditionViolation(message, file, line); 
}


//*********************************************************************
// throw_postcondition_error
//*********************************************************************
inline void throw_postcondition_error(bool predicate, char const * message, char const * file, int line)
{
  if(!predicate)
    throw Vt::PostconditionViolation(message, file, line); 
}


//*********************************************************************
// throw_runtime_error
//*********************************************************************
inline void throw_runtime_error(char const * message, char const * file, int line)
{
  char what_[1100];
  sprintf(what_, "\n%.900s\n(%.100s:%d)\n", message, file, line);
  throw std::runtime_error(what_); 
}


//*********************************************************************
// Vt_precondition
//*********************************************************************
#define Vt_precondition(PREDICATE, MESSAGE) Vt::throw_precondition_error((PREDICATE), MESSAGE, __FILE__, __LINE__)


//*********************************************************************
// Vt_postcondition
//*********************************************************************
#define Vt_postcondition(PREDICATE, MESSAGE) Vt::throw_postcondition_error((PREDICATE), MESSAGE, __FILE__, __LINE__)


//*********************************************************************
// Vt_invariant
//*********************************************************************
#define Vt_invariant(PREDICATE, MESSAGE) Vt::throw_invariant_error((PREDICATE), MESSAGE, __FILE__, __LINE__)
            

//*********************************************************************
// Vt_fail
//*********************************************************************
#define Vt_fail(MESSAGE) Vt::throw_runtime_error(MESSAGE, __FILE__, __LINE__)


#else // NDEBUG


//*********************************************************************
// throw_invariant_error
//*********************************************************************
inline void throw_invariant_error(bool predicate, char const * message)
{
  if(!predicate)
    throw Vt::InvariantViolation(message); 
}


//*********************************************************************
// throw_precondition_error
//*********************************************************************
inline void throw_precondition_error(bool predicate, char const * message)
{
  if(!predicate)
    throw Vt::PreconditionViolation(message); 
}


//*********************************************************************
// throw_postcondition_error
//*********************************************************************
inline void throw_postcondition_error(bool predicate, char const * message)
{
  if(!predicate)
    throw Vt::PostconditionViolation(message); 
}


//*********************************************************************
// Vt_precondition
//*********************************************************************
#define Vt_precondition(PREDICATE, MESSAGE) Vt::throw_precondition_error((PREDICATE), MESSAGE)


//*********************************************************************
// Vt_postcondition
//*********************************************************************
#define Vt_postcondition(PREDICATE, MESSAGE) Vt::throw_postcondition_error((PREDICATE), MESSAGE)


//*********************************************************************
// Vt_invariant
//*********************************************************************
#define Vt_invariant(PREDICATE, MESSAGE) Vt::throw_invariant_error((PREDICATE), MESSAGE)
         

//*********************************************************************
// Vt_fail
//*********************************************************************   
#define Vt_fail(MESSAGE) throw std::runtime_error(MESSAGE)
 

#endif // NDEBUG


} // end Vt namespace
#endif // __VT_ERRORS_H__
