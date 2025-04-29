/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/SimpleSelectorExampleTool.h>

//
// method implementations
//

namespace columnar
{
  SimpleSelectorExampleTool ::
  SimpleSelectorExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode SimpleSelectorExampleTool ::
  initialize ()
  {
    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void SimpleSelectorExampleTool ::
  callSingleEvent (ParticleRange particles) const
  {
    for (ParticleId particle : particles)
    {
      selectionDec(particle) = ptAcc(particle) > m_ptCut.value();
    }
  }



  void SimpleSelectorExampleTool ::
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
      callSingleEvent (particlesHandle(event));
    }
  }
}
