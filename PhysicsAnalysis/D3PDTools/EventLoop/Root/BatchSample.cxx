/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <EventLoop/BatchSample.h>

//
// method implementations
//

ClassImp(EL::BatchSample)

namespace EL
{
  BatchSample ::
  BatchSample ()
    : begin_segments (0), end_segments (0)
  {
  }



  BatchSample ::
  ~BatchSample ()
  {
  }
}
