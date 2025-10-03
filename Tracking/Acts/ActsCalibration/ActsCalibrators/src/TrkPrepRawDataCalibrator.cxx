/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibrators/TrkPrepRawDataCalibrator.h"

namespace ActsTrk::detail {
  TrkPrepRawDataCalibrator::TrkPrepRawDataCalibrator(const ActsTrk::IActsToTrkConverterTool* convTool,
                                                     const Trk::IRIO_OnTrackCreator* rotCreator):
      m_convTool{convTool}, m_rotCreator{rotCreator} {}
  
  TrkPrepRawDataCalibrator::SourceLink_t 
        TrkPrepRawDataCalibrator::unpack(const Acts::SourceLink& sl) {
    SourceLink_t prd = sl.template get<SourceLink_t>();
    assert(prd != nullptr);
    return prd;
  }
  Acts::SourceLink TrkPrepRawDataCalibrator::pack(const SourceLink_t prd) {
    return Acts::SourceLink{prd};
  }
}