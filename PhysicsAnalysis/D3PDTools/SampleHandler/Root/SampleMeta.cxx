/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/SampleMeta.h>

#include <stdexcept>
#include <RootCoreUtils/Assert.h>
#include <SampleHandler/SampleLocal.h>

//
// method implementations
//

ClassImp (SH::SampleMeta)

namespace SH
{
  void SampleMeta :: 
  testInvariant () const
  {
  }



  SampleMeta ::
  SampleMeta ()
    : Sample ("unnamed")
  {
    RCU_NEW_INVARIANT (this);
  }



  SampleMeta :: 
  SampleMeta (const std::string& name)
    : Sample (name)
  {
    RCU_NEW_INVARIANT (this);
  }



  std::size_t SampleMeta ::
  getNumFiles () const
  {
    RCU_READ_INVARIANT (this);
    // rationale: this is just so that print() will work
    return 0;
  }



  std::string SampleMeta ::
  getFileName (std::size_t /*index*/) const
  {
    RCU_READ_INVARIANT (this);
    throw std::runtime_error ("fileName() should not be called on SampleMeta");
  }



   std::unique_ptr<SampleLocal> SampleMeta ::
  doMakeLocal () const
  {
    RCU_READ_INVARIANT (this);
    throw std::runtime_error ("makeLocal() should not be called on SampleMeta");
  }



  std::vector<std::string> SampleMeta ::
  doMakeFileList () const
  {
    RCU_READ_INVARIANT (this);

    std::vector<std::string> result;
    return result;
  }
}
