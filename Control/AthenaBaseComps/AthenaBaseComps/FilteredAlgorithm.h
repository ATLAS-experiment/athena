/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENABASECOMPS_FILTEREDALGORTITHM_H
#define ATHENABASECOMPS_FILTEREDALGORTITHM_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaKernel/IDecisionSvc.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"


/** @class FilteredAlgorithm
  * @brief Base class for stream algorithms using DecisionSvc
  * 
  * @author srinir@bnl.gov
  */
class FilteredAlgorithm : public AthAlgorithm
{
public:
  using AthAlgorithm::AthAlgorithm;

  virtual StatusCode initialize() override;

  /// Test whether this event should be output
  bool isEventAccepted(const EventContext& ctx) const;

protected:
  /// Vector of names of Algorithms that this stream accepts
  StringArrayProperty      m_acceptNames{this, "AcceptAlgs", {},
    "Filters which if any are passed enable output"};
  /// Vector of names of Algorithms that this stream requires
  StringArrayProperty      m_requireNames{this, "RequireAlgs", {},
    "Filters which must all be passed to enable output"};
  /// Vector of names of Algorithms that this stream is vetoed by
  StringArrayProperty      m_vetoNames{this, "VetoAlgs", {},
    "Filters which if any are passed disable output"};

  ServiceHandle<IDecisionSvc> m_decSvc{this, "decSvc", "DecisionSvc/DecisionSvc",
    "Handle to DecisionSvc"};
};

#endif // !ATHENABASECOMPS_FILTEREDALGORITHM_H
