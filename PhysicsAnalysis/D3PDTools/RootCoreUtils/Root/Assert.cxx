/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <RootCoreUtils/Assert.h>

#include <cstdlib>
#include <string>
#include <RootCoreUtils/Message.h>
#include <RootCoreUtils/MessageCheck.h>

//
// method implementations
//

namespace RCU
{
  namespace Check
  {
    const char * const typeLiteral [typeNum] =
      {
	"assertion failed",
	"assertion failed",
	"assertion failed",

	"requirement failed",
	"requirement failed",
	"requirement failed",

	"postcondition violated",
	"postcondition violated",
	"postcondition violated",

	"invariant violated"
      };



    const bool typeAbort [typeNum] =
      {
	false,
	true,
	true,

	false,
	true,
	true,

	false,
	true,
	true,

	true,
      };



    void fail (const char *file, unsigned line,
	       Type type, const char *error)
    {
      RCU_REQUIRE (file != 0);
      RCU_REQUIRE (line != 0);
      RCU_REQUIRE (type < typeNum);
      RCU_REQUIRE (error != 0);

      Message msg;
      msg.file = file;
      msg.line = line;
      msg.type = typeAbort[type] ? MESSAGE_ABORT : MESSAGE_EXCEPTION;

      std::string mymessage = typeLiteral[type];
      mymessage += ": ";
      mymessage += error;
      msg.message = mymessage.c_str();

      msg.send();
    }



    TestInvariant ::
    TestInvariant (TestInvariantFunction function,
		   const void *object)
      : m_function (function), m_object (object)
    {
      (*m_function) (m_object);
    }



    TestInvariant ::
    ~TestInvariant ()
    {
      (*m_function) (m_object);
    }



    bool testInvariantPrint (TestInvariantFunction function,
			     const void *object)
    {
      using namespace msgRootCoreUtils;

      try
      {
	(*function) (object);
	return false;
      } catch (std::exception& e)
      {
        ANA_MSG_ERROR ("caught exception in invariant test: " << e.what());
        return true;
      };
    }
  }
}
