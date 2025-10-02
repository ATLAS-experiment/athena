/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/StringExampleTool.h>

//
// method implementations
//

namespace columnar
{
  StringExampleTool ::
  StringExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode StringExampleTool ::
  initialize ()
  {
    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void StringExampleTool ::
  callEvents (EventContextRange events) const
  {
    // loop over all events and met terms.  note that this is
    // deliberately looping by value, as the ID classes are very small
    // and can be copied cheaply.  this could have also been written as
    // a single loop over all mets in the event range, but I chose to
    // split it up into two loops as most tools will need to do some
    // per-event things, e.g. retrieve `EventInfo`.
    for (columnar::EventContextId event : events)
    {
      for (MetId met : metAcc(event))
      {
        selectionDec(met) = nameAcc(met) == "Final";
      }
    }
  }
}
