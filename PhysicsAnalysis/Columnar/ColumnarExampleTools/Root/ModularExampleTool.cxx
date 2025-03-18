/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/ModularExampleTool.h>

//
// method implementations
//

namespace columnar
{
  ModularExampleTool ::
  ModularExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode ModularExampleTool ::
  initialize ()
  {
    m_subtoolPt = std::make_unique<SubtoolPt> (m_ptCut.value());

    // all subtools need to be connected to the base class, so that the
    // accessors can be properly connected in columnar mode.
    addSubtool (*m_subtoolPt);

    /// the second subtool does the connection via the constructor,
    /// which simplifies some things
    m_subtoolEta = std::make_unique<SubtoolEta> (this, m_etaCut.value());

    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void ModularExampleTool ::
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
        selectionDec(particle) = m_subtoolPt->select (particle) && m_subtoolEta->select (particle);
      }
    }
  }



  ModularExampleTool::SubtoolPt ::
  SubtoolPt (float val_cutValue)
    : m_cutValue (val_cutValue)
  {}



  bool ModularExampleTool::SubtoolPt ::
  select (ParticleId particle) const
  {
    return ptAcc(particle) > m_cutValue;
  }



  ModularExampleTool::SubtoolEta ::
  SubtoolEta (ColumnarTool<> *val_parent, float val_cutValue)
    : ColumnarTool<> (val_parent), m_cutValue (val_cutValue)
  {}



  bool ModularExampleTool::SubtoolEta ::
  select (ParticleId particle) const
  {
    return std::abs(etaAcc(particle)) < m_cutValue;
  }
}
