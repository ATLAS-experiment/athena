/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_TRACKMERGER_ALG_H
#define ACTSTRK_TRACKMERGER_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/TrackContainerHandlesHelper.h" 

namespace ActsTrk {

  class TrackMergerAlg :
    public AthReentrantAlgorithm  {
  public:
    TrackMergerAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TrackMergerAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    
  private:
    ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper;
    SG::ReadHandleKeyArray< ActsTrk::TrackContainer > m_inputTrackCollections {this, "InputTrackCollections", {},
	"Input Acts Tracks to be merged"};
    SG::WriteHandleKey< ActsTrk::TrackContainer > m_outputTrackCollection {this, "OutputTrackCollection", "",
      "Output Acts Tracks obtained from the merging of the input collections"};
  }; 
  
}

#endif //> !ACTSTRACKRECONSTRUCTION_PROTOTRACKREPORTINGALG_H
