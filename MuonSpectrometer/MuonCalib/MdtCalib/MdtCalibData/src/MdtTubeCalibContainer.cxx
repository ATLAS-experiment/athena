/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MdtCalibData/MdtTubeCalibContainer.h>
namespace MuonCalib {
    MdtTubeCalibContainer::MdtTubeCalibContainer(const Muon::IMuonIdHelperSvc* idHelperSvc,
                                                 const Identifier& moduleID):
        m_moduleID{idHelperSvc->chamberId(moduleID)},
        m_idHelperSvc{idHelperSvc}{

    m_nMl = m_idHelper.numberOfMultilayers(moduleID);
    const Identifier secondMl = m_idHelper.multilayerID(m_moduleID, m_nMl);
    m_nLayers = std::max(m_idHelper.tubeLayerMax(m_moduleID),
                         m_idHelper.tubeLayerMax(secondMl));
    m_nTubes = std::max(m_idHelper.tubeMax(m_moduleID),
                        m_idHelper.tubeMax(secondMl));
    m_data.resize(m_nLayers * m_nTubes * m_nMl);
}
bool MdtTubeCalibContainer::setCalib(SingleTubeCalibPtr val, const Identifier& tubeId, MsgStream& msg) {
    /// Make enough space for the calibration constants
    const unsigned int index = vectorIndex(tubeId);
    if (m_moduleID != m_idHelperSvc->chamberId(tubeId)) {
        msg << MSG::ERROR<<__FILE__<<":"<<__LINE__<<" The channel "<<m_idHelperSvc->toString(tubeId)
            <<" does not correspond to chamber "<<m_idHelperSvc->chamberNameString(tubeId) <<endmsg;
        return false;
    }
    if (!val) {
        msg << MSG::ERROR<<__FILE__<<":" <<__LINE__<<" No data is parsed for "
            << m_idHelperSvc->toString(tubeId)<<endmsg;
        return false;
    }
    if (index >= m_data.size()) {
        msg << MSG::WARNING<<__FILE__<<":"<<__LINE__<<" The channel "<<m_idHelperSvc->toString(tubeId)
            <<"does not seem to match the anticipated chamber sizes of "
            <<m_idHelperSvc->chamberNameString(m_moduleID)<<endmsg;
        m_data.resize(index +1);
    }
    SingleTubeCalibPtr& store = m_data[index];
    if (store && store !=val) {
        msg << MSG::ERROR<< __FILE__ <<":"<< __LINE__<< " Data has already been stored for channel "
            << m_idHelperSvc->toString(tubeId) 
            <<", layer max: "<<m_nLayers<<", tube max: "<<m_nTubes
            << endmsg;
        return false;
    }
    store = std::move(val);
    if (msg.level() <= MSG::VERBOSE) {
        msg << MSG::VERBOSE<<" Succesfully stored calibration data for channel "<<m_idHelperSvc->toString(tubeId) << endmsg;
    }
    return true;
}
const Muon::IMuonIdHelperSvc* MdtTubeCalibContainer::idHelperSvc() const {
    return m_idHelperSvc;
}
}