/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/PreselectionExampleTool.h>

//
// method implementations
//

namespace ADD_CMODE(columnar)
{
  PreselectionExampleTool ::
  PreselectionExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode PreselectionExampleTool ::
  initialize ()
  {
    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void PreselectionExampleTool ::
  callEvents (columnar::EventContextRange events) const
  {
    for (columnar::EventContextId event : events)
    {
      for (columnar::ParticleId particle : particlesHandle(event))
      {
        // objects failing the preselection are not processed, but they
        // still need a well-defined output value.  if the preselection
        // column is not present, all objects are processed.
        if (preselectionAcc.isAvailable(particle) && !preselectionAcc(particle))
        {
          selectionDec(particle) = false;
          continue;
        }

        selectionDec(particle) = (ptAcc(particle) > m_ptCut.value());
      }
    }
  }
}
