/** \file VtErrors.cpp

 * Copyright (c) 2002 by
 * All Rights Reserved
 *
 */


//*********************************************************************
// INCLUDES
//*********************************************************************
#include <time.h>
#include "VtSysdefs.h"
#include "VtErrors.h"


using namespace Vt;


//*********************************************************************
// Constructor
//*********************************************************************
ContractViolation::ContractViolation(char const * prefix, char const * message, 
                                     char const * file, int line) : 
  std::exception()
{
  time_t Time;
  time( &Time );
  struct tm* GMT = gmtime( &Time );
 
  std::string ErrorMessage(asctime( GMT ));
  ErrorMessage.append(prefix);
  ErrorMessage.append(" ");
  ErrorMessage.append(message);
  ErrorMessage.append(" ");
  ErrorMessage.append(file);
  ErrorMessage.append(", Line: ");
  vt_char Buffer[16];
  itoa(line, Buffer, 10);
  ErrorMessage.append(Buffer);

  if (ErrorMessage.length() + 1 < ContractViolation::bufsize_ )
  {
	strncpy( what_, ErrorMessage.c_str(), ErrorMessage.length() + 1 );
  }
  else
  {
	strncpy( what_, ErrorMessage.c_str(), ContractViolation::bufsize_ );
  }
}


//*********************************************************************
// Constructor
//*********************************************************************
ContractViolation::ContractViolation(char const * prefix, char const * message):
  std::exception()
{
  time_t Time;
  time( &Time );
  struct tm* GMT = gmtime( &Time );
 
  std::string ErrorMessage(asctime( GMT ));
  ErrorMessage.append(prefix);
  ErrorMessage.append(" ");
  ErrorMessage.append(message);
}
