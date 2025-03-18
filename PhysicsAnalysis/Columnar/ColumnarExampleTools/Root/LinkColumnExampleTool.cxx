/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/LinkColumnExampleTool.h>

//
// method implementations
//

namespace columnar
{
  LinkColumnExampleTool ::
  LinkColumnExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode LinkColumnExampleTool ::
  initialize ()
  {
    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void LinkColumnExampleTool ::
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
      for (auto muon : muonsHandle(event))
      {
        // retrieve the track linked to the muon
        OptTrackId track = trackLinkAcc(muon);

        // apply the selection, the OptTrackId tries to (mostly) behave
        // like a std::optional<TrackId>, so we can use it in a similar
        // way.  Here we first check if the track is valid, then do a
        // curvature selection on the track.
        selectionDec(muon) = track && std::abs(trackQOverPAcc(track.value())) < 1. / m_ptCut.value();
      }
    }
  }
}
