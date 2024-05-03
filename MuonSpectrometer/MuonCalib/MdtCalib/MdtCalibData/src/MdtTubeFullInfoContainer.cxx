/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibData/MdtTubeFullInfoContainer.h"

namespace MuonCalib {
using SingleTubeFullInfo = MdtTubeFullInfoContainer::SingleTubeFullInfo;
MdtTubeFullInfoContainer::MdtTubeFullInfoContainer(const Muon::IMuonIdHelperSvc* idHelperSvc,
                                                   const Identifier& moduleID): 
        MdtTubeCalibContainer(idHelperSvc, moduleID) {
    m_info.resize(size());
}

bool MdtTubeFullInfoContainer::setFullInfo(const Identifier& tubeId, SingleTubeFullInfo&& val) {
    unsigned int idx = vectorIndex(tubeId);
    if (idx >= m_info.size()) return false;
    m_info[idx] = std::move(val);
    return true;
}     
void MdtTubeFullInfoContainer::setImplementation(const std::string& impl) { 
    m_implementation = impl; 
}
const std::string& MdtTubeFullInfoContainer::name() const { 
    return m_name; 
}
const std::string& MdtTubeFullInfoContainer::implementation() const { 
    return m_implementation; 
}
const SingleTubeFullInfo* MdtTubeFullInfoContainer::getFullInfo(const Identifier& tubeId) const {
    unsigned int idx = vectorIndex(tubeId);
    if (idx >= m_info.size()) return nullptr;
    return &m_info[idx];
}


}  // namespace MuonCalib
