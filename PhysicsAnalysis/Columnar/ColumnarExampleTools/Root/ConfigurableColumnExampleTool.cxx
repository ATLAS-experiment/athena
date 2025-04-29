/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/ConfigurableColumnExampleTool.h>

//
// method implementations
//

namespace columnar
{
  ConfigurableColumnExampleTool ::
  ConfigurableColumnExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode ConfigurableColumnExampleTool ::
  initialize ()
  {
    if (m_ptVar.empty())
    {
      ATH_MSG_ERROR ("no pt variable specified");
      return StatusCode::FAILURE;
    }
    // set the accessor for the pt variable the user configured
    resetAccessor (ptAcc, *this, m_ptVar.value());

    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void ConfigurableColumnExampleTool ::
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
        selectionDec(particle) = (ptAcc(particle) > m_ptCut.value());
      }
    }
  }
}
