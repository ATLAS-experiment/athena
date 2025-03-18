/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/OptionalColumnExampleTool.h>

//
// method implementations
//

namespace columnar
{
  OptionalColumnExampleTool ::
  OptionalColumnExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode OptionalColumnExampleTool ::
  initialize ()
  {
    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void OptionalColumnExampleTool ::
  callEvents (EventContextRange events) const
  {
    // loop over all events and particles.  note that this is
    // deliberately looping by value, as the ID classes are very small
    // and can be copied cheaply.  this could have also been written as
    // a single loop over all particles in the event range, but I chose
    // to split it up into two loops as most tools will need to do some
    // per-event things, e.g. retrieve `EventInfo`.
    for (columnar::EventContextId event : events)
    {
      for (columnar::ParticleId particle : particlesHandle(event))
      {
        // check if the corrected pt is available, and use it if it is
        // and the default pt otherwise
        if (ptCorrAcc.isAvailable(particle))
          selectionDec(particle) = (ptCorrAcc(particle) > m_ptCut.value());
        else
          selectionDec(particle) = (ptAcc(particle) > m_ptCut.value());
      }
    }
  }
}
