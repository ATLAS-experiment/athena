/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/Meta.h>

#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace SH
{
  std::string dbg (const Meta& /*obj*/, unsigned /*verbosity*/)
  {
    return "meta-object";
  }



  void Meta ::
  testInvariant () const
  {
  }



  Meta ::
  ~Meta ()
  {
    RCU_DESTROY_INVARIANT (this);
  }



  Meta ::
  Meta (const std::string& name)
    : TNamed (name.c_str(), name.c_str())
  {
    RCU_NEW_INVARIANT (this);
  }
}
