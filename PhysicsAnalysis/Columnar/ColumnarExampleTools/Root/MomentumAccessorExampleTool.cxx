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
    if (auto type = objectTypeAcc.staticType())
      resetObjectType (momAcc, *this, *type);
    else
      ANA_MSG_INFO ("ObjectTypeAccessor: no object type set, using default");

    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void MomentumAccessorExampleTool ::
  callSingleEvent (columnar::ParticleRange<CMode> particles) const
  {
    for (columnar::ParticleId<CMode> particle : particles)
    {
      selectionDec(particle) = momAcc.e(particle) > m_energyCut.value();
    }
  }



  void MomentumAccessorExampleTool ::
  callEvents (columnar::EventContextRange<CMode> events) const
  {
    // loop over all events and particles.  note that this is
    // deliberately looping by value, as the ID classes are very small
    // and can be copied cheaply.  this could have also been written as
    // a single loop over all particles in the event range, but I chose
    // to split it up into two loops as most tools will need to do some
    // per-event things, e.g. retrieve `EventInfo`.
    for (columnar::EventContextId<CMode> event : events)
    {
      callSingleEvent (particlesHandle(event));
    }
  }
}
