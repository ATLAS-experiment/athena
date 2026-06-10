/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

//protect
#include <RootCoreUtils/ExceptionMsg.h>

#include <memory>
#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace RCU
{
  void ExceptionMsg ::
  testInvariant () const
  {
    //RCU_INVARIANT (this != 0);
    RCU_INVARIANT (!m_message.empty());
  }



  ExceptionMsg ::
  ExceptionMsg (const char *const /*val_file*/, const unsigned /*val_line*/,
		const std::string& val_message)
    : m_message (val_message)
  {
    RCU_NEW_INVARIANT (this);
  }



  ExceptionMsg ::
  ~ExceptionMsg () throw ()
  {
    RCU_DESTROY_INVARIANT (this);
  }



  const char *ExceptionMsg ::
  what () const throw ()
  {
    RCU_READ_INVARIANT (this);
    return m_message.c_str();
  }
}
