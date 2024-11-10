/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_PROASSOCIATION_ALG_H
#define ACTSTRK_PROASSOCIATION_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/PrepRawDataAssociation.h"

namespace ActsTrk {

class PrdAssociationAlg
  : public AthReentrantAlgorithm {
public:
  PrdAssociationAlg(const std::string& name,
		    ISvcLocator* pSvcLocator);
  virtual ~PrdAssociationAlg() override = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;
  
private:
  SG::ReadHandleKey< ActsTrk::TrackContainer > m_inputTrackCollections {this, "InputTrackCollection", "",
      "Input Acts Tracks"};

  SG::ReadHandleKey< ActsTrk::PrepRawDataAssociation > m_inputPrdMap {this, "InputPrdMap", "",
      "Map of used measurements from previous tracking pass"};
  
  SG::WriteHandleKey< ActsTrk::PrepRawDataAssociation > m_outputPrdMap {this, "OutputPrdMap", "",
      "Map of used measurements"};

private:
  enum EStat {
    kNTracks,
    kNPixelMeasurements,
    kNStripMeasurements,
    kNHgtdMeasurements,
    kNStat
  };
  
  mutable std::array<std::atomic<unsigned int>, kNStat> m_stat ATLAS_THREAD_SAFE {}; 
};

}

#endif
