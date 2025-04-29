/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "StripDefectsEmulatorCondAlg.h"

namespace InDet{
  StatusCode StripDefectsEmulatorCondAlg::initialize(){
    // There are no group defects defined for strips
    m_groupDefectHistNames.clear();
    m_maxNGroupDefects.clear();
    return DefectsEmulatorCondAlgImpl<StripDefectsEmulatorCondAlg>::initialize();
  }

}
