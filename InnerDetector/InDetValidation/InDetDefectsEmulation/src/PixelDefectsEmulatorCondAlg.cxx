/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// Silicon trackers includes
#include "PixelDefectsEmulatorCondAlg.h"

#include "InDetIdentifier/PixelID.h"

namespace InDet{

  StatusCode PixelDefectsEmulatorCondAlg::initialize(){

    m_groupDefectHistNames.clear();
    m_groupDefectHistNames.push_back("core-column");
    m_groupDefectHistNames.push_back("circuit");
    // set upper edge of the 1D number of group defects (core-column, circuit) histograms
    m_maxNGroupDefects.clear();
    m_maxNGroupDefects.push_back(10);
    m_maxNGroupDefects.push_back(4);
    return DefectsEmulatorCondAlgImpl<PixelDefectsEmulatorCondAlg>::initialize();
  }

}// namespace closure
