/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <EventLoop/BatchJob.h>

#include <TChain.h>
#include <EventLoop/Algorithm.h>
#include <EventLoop/BatchDriver.h>
#include <EventLoop/BatchSample.h>
#include <EventLoop/BatchSegment.h>
#include <EventLoop/OutputStream.h>

//
// method implementations
//

ClassImp(EL::BatchJob)

namespace EL
{
  BatchJob ::
  BatchJob ()
  {
  }



  BatchJob ::
  ~BatchJob ()
  {
  }
}
