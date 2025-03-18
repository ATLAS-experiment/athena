/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/VectorExampleTool.h>

//
// method implementations
//

namespace columnar
{
  VectorExampleTool ::
  VectorExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode VectorExampleTool ::
  initialize ()
  {
    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void VectorExampleTool ::
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
      for (ParticleId particle : particlesHandle(event))
      {
        // in pactical terms we should find the index of the primary
        // vertex, but for purposes of the example we just use the first
        // vertex instead
        const std::size_t index = 0;

        // it is actually safe to copy the return value of the
        // accessors, as those are ranges that are cheap to copy
        auto trknum = trknumAcc(particle);
        auto trksumpt = trksumptAcc(particle);

        // this is probably not a meaningful selection, but it hopefully
        // illustrates how to use the vector accessors
        selectionDec(particle) = ptAcc(particle) > m_ptCut.value() && trknum.size() > index && trknum[index] > 2 && trksumpt.size() > index && trksumpt[index] > 2e3;
      }
    }
  }
}
