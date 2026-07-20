/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <EventLoop/BatchSegment.h>

//
// method implementations
//

ClassImp(EL::BatchSegment)

namespace EL
{
  BatchSegment ::
  BatchSegment ()
    : sample (0), job_id (0),
      begin_file (0), begin_event (0),
      end_file (0), end_event (0)
  {
  }



  BatchSegment ::
  ~BatchSegment ()
  {
  }
}
