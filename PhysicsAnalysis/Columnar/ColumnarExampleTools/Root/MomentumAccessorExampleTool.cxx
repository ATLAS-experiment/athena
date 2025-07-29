/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/MomentumAccessorExampleTool.h>

//
// method implementations
//

namespace columnar
{
  MomentumAccessorExampleTool ::
  MomentumAccessorExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode MomentumAccessorExampleTool ::
  initialize ()
  {
    // normally one would set this based on a property and pick the
    // right momentum accessor for our particle type. however, this is
    // an example and I don't feel like demonstrating that aspect.
    resetPtEtaPhiReadM (momAcc, *this);

    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void MomentumAccessorExampleTool ::
  callSingleEvent (ParticleRange particles) const
  {
    for (ParticleId particle : particles)
    {
      selectionDec(particle) = momAcc.e(particle) > m_energyCut.value();
    }
  }



  void MomentumAccessorExampleTool ::
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
