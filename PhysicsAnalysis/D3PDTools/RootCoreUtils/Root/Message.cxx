/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <RootCoreUtils/Message.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/MessageCheck.h>

//
// method implementations
//

namespace RCU
{
  Message ::
  Message ()
    : file (nullptr), line (0), type (MESSAGE_UNSPECIFIED),
      message (nullptr)
  {
  }



  void Message ::
  send () const
  {
    using namespace msgRootCoreUtils;

    MessageType mytype = type;
    if (mytype < 0 || mytype > MESSAGE_UNSPECIFIED)
      mytype = MESSAGE_UNSPECIFIED;

    std::ostringstream str;

    if (file != nullptr)
    {
      if (strncmp (file, "../", 3) == 0)
	str << (file+3) << ":";
      else
	str << file << ":";
    }
    if (line != 0)
      str << line << ":";

    if (mytype != MESSAGE_UNSPECIFIED)
    {
      static const char * const type_names[MESSAGE_UNSPECIFIED] =
	{"message", "warning", "error", "exception", "abort"};
      str << type_names[mytype] << ":";
    }

    if (!str.str().empty())
      str << " ";
    if (message != nullptr)
      str << message;
    else
      str << "(null)";

    const char *envname = nullptr;
    if (mytype == MESSAGE_ABORT)
      envname = "ROOTCOREUTILS_ABORT";
    else if (mytype == MESSAGE_EXCEPTION)
      envname = "ROOTCOREUTILS_EXCEPTION";
    if (envname)
    {
      const char *abort_type = getenv (envname);
      const MessageType def_type = MESSAGE_EXCEPTION;

      if (abort_type == nullptr)
      {
	mytype = def_type;
      } else if (strcmp (abort_type, "abort") == 0)
      {
	mytype = MESSAGE_ABORT;
      } else if (strcmp (abort_type, "exception") == 0)
      {
	mytype = MESSAGE_EXCEPTION;
      } else
      {
	mytype = def_type;

        ANA_MSG_WARNING (std::string ("unknown value for ") << envname << " " << abort_type);
      }
    }

    std::cout << str.str() << std::endl;
    if (mytype == MESSAGE_EXCEPTION)
      throw std::runtime_error (str.str());
    if (mytype == MESSAGE_ABORT)
      std::abort ();
  }
}
