/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MdtCalibData/MdtTubeFitContainer.h>

namespace MuonCalib{
using SingleTubeFit = MdtTubeFitContainer::SingleTubeFit;
MdtTubeFitContainer::MdtTubeFitContainer(const Muon::IMuonIdHelperSvc* idHelperSvc,
                                         const Identifier& moduleID): 
        MdtTubeCalibContainer(idHelperSvc, moduleID) {
    m_info.resize(size());
}

void MdtTubeFitContainer::setImplementation(const std::string& impl) { 
    m_implementation = impl; 
}

const std::string& MdtTubeFitContainer::name() const { 
    return m_name; 
}

const std::string& MdtTubeFitContainer::implementation() const { 
    return m_implementation; 
}

bool MdtTubeFitContainer::setFit(SingleTubeFit&& val, const Identifier& tubeId, MsgStream& log) {
    unsigned int idx = vectorIndex(tubeId);
    if (idx >= m_info.size()) {
        log<<MSG::WARNING<<__FILE__<<":"<<__LINE__<<" Index "<<idx
           <<" exceeds range "<<m_info.size()<<endmsg;
        return false;
    }
    m_info[idx] = std::move(val);
    return true;
}

void MdtTubeFitContainer::setGroupBy(const std::string& group_by) {
    for (SingleTubeFit& fit : m_info) {
        fit.group_by = group_by;
    }
}
std::string MdtTubeFitContainer::GroupBy() const {
    if (!m_info.size()) return m_group_by;
    return m_info.begin()->group_by;
}

const SingleTubeFit* MdtTubeFitContainer::getFit(const Identifier& tubeId) const {
    unsigned int idx = vectorIndex(tubeId);
    if (idx >= m_info.size()) return nullptr;
    return &m_info[idx];
}

SingleTubeFit* MdtTubeFitContainer::getFit(const Identifier& tubeId) {
    unsigned int idx = vectorIndex(tubeId);
    if (idx >= m_info.size()) return nullptr;
    return &m_info[idx];
}

}
