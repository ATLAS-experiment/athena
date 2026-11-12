/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/VariantLinkColumnExampleTool.h>

//
// method implementations
//

namespace ADD_CMODE(columnar)
{
  VariantLinkColumnExampleTool ::
  VariantLinkColumnExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode VariantLinkColumnExampleTool ::
  initialize ()
  {
    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());

    return StatusCode::SUCCESS;
  }



  void VariantLinkColumnExampleTool ::
  callSingleEvent (columnar::MuonRange muons) const
  {
    for (auto muon : muons)
    {
      // retrieve the track linked to the muon.  this is a variant
      // object id, so it can refer to a track in either container.
      // `opt_value` gives us an optional that is empty for invalid
      // links.
      auto track = trackLinkAcc(muon).opt_value();

      // apply the selection.  the variant accessor picks the correct
      // container for the track.
      selectionDec(muon) = track && std::abs(trackQOverPAcc(track.value())) < 1. / m_ptCut.value();
    }
  }



  void VariantLinkColumnExampleTool ::
  callEvents (columnar::EventContextRange events) const
  {
    for (columnar::EventContextId event : events)
    {
      callSingleEvent (muonsHandle(event));
    }
  }
}
