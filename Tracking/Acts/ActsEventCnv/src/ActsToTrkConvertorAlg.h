/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSEVENTCNV_ACTSTOTRK_CONVERTER_ALG_H
#define ACTSEVENTCNV_ACTSTOTRK_CONVERTER_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkTrack/TrackCollection.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"

namespace ActsTrk
{

  class ActsToTrkConvertorAlg: public AthReentrantAlgorithm
  {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~ActsToTrkConvertorAlg() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    PublicToolHandle<IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", ""};
    SG::ReadHandleKey<TrackContainer> m_tracksContainerKey{this, "ACTSTracksLocation", "",  "Output track collection (ActsTrk variant)"};
    SG::WriteHandleKey<::TrackCollection> m_tracksKey{this, "TracksLocation", "", "Output track collection"};

  };

}

#endif
