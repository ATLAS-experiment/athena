/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MdtCalibData/MdtCalibDataContainer.h>
namespace MuonCalib{

MdtCalibDataContainer::MdtCalibDataContainer(const Muon::IMuonIdHelperSvc* idHelperSvc,
                              const RegionGranularity granularity):
        m_idHelperSvc{idHelperSvc},
        m_granularity{granularity} {
}
void MdtCalibDataContainer::setInversePropSpeed(const float speed) {m_invPropSpeed = speed; }
float MdtCalibDataContainer::inversePropSpeed() const { return m_invPropSpeed; }

MdtCalibDataContainer::RegionGranularity MdtCalibDataContainer::granularity() const { return m_granularity; }
inline std::optional<unsigned int>  MdtCalibDataContainer::containerIndex(const Identifier& measId, MsgStream& msg) const {
     IdentifierHash hash{0};
      
    if (m_granularity == RegionGranularity::OnePerMultiLayer && 
        (m_idHelper.get_detectorElement_hash(measId, hash) ||
         m_idHelper.detectorElement_hash_max() < static_cast<unsigned int>(hash))) {
        msg<<MSG::ERROR <<__FILE__<<":"<<__LINE__<<" Failed to look up a proper detector element hash for "
                        <<m_idHelperSvc->toString(measId)<<endmsg;
        return std::nullopt;
    } else if (m_granularity == RegionGranularity::OnePerChamber &&
               (m_idHelper.get_module_hash(measId, hash) ||
                m_idHelper.module_hash_max() < static_cast<unsigned int>(hash))) {
        msg<<MSG::ERROR <<__FILE__<<":"<<__LINE__<<" Failed to look up a proper detector element hash for "
                        <<m_idHelperSvc->toString(measId)<<endmsg;
        return std::nullopt;
    }
    return std::make_optional<unsigned int>(static_cast<unsigned int>(hash));
}

bool MdtCalibDataContainer::hasDataForChannel(const Identifier& measId, MsgStream& msg) const {
    std::optional<unsigned int > index = containerIndex(measId, msg);
    return !(!index || (*index) >= m_dataCache.size() || !m_dataCache[*index]);
}

const MdtFullCalibData* MdtCalibDataContainer::getCalibData(const Identifier& measId, MsgStream& msg) const {
    std::optional<unsigned int> index = containerIndex(measId, msg);
    if (!index) return nullptr;
    if ((*index ) < m_dataCache.size()) {
        const MdtFullCalibData& data = m_dataCache[*index];
        if (data) {return &data;}
    }
    msg<<MSG::WARNING<<__FILE__<<":"<<__LINE__<<" No Mdt calibration data is stored for "
                     <<m_idHelperSvc->toString(measId)<<endmsg;
    return nullptr;
}
bool MdtCalibDataContainer::storeData(const Identifier& mlID, CorrectionPtr corrFuncSet, MsgStream& msg) {
    /// Check for a valid index
    std::optional<unsigned int> index = containerIndex(mlID, msg);
    if (!index) return false;
    if (!corrFuncSet) {
        msg << MSG::ERROR<<__FILE__<<":"<<__LINE__<<" No correction functional set parsed for multilayer "
            <<m_idHelperSvc->toString(mlID) << endmsg;
        return false;
    }
    /// Resize the container if neccessary
    if (m_dataCache.size() <= (*index)) m_dataCache.resize(*index + 1);
    MdtFullCalibData& cache = m_dataCache[*index];
    if (cache.corrections && cache.corrections != corrFuncSet) {
        msg << MSG::ERROR<<__FILE__<<":"<<__LINE__<<" There already exist a rt relation object for multilayer "
            <<m_idHelperSvc->toString(mlID) << endmsg;
        return false;
    }
    cache.corrections = std::move(corrFuncSet);
    
    if (msg.level() <= MSG::DEBUG) {
         msg << MSG::DEBUG<<__FILE__<<":"<<__LINE__<<" Added successfully the rt corrections for "
             << m_idHelperSvc->toString(mlID)<<endmsg;
    }
    return true;
}
bool MdtCalibDataContainer::storeData(const Identifier& mlID, RtRelationPtr rtRelation, MsgStream& msg) {
    /// Check for a valid index
    std::optional<unsigned int> index = containerIndex(mlID, msg);
    if (!index) return false;
    if (!rtRelation) {
        msg << MSG::ERROR<<__FILE__<<":"<<__LINE__<<" No rt relation parsed for multilayer "
            <<m_idHelperSvc->toString(mlID) << endmsg;
        return false;
    }
    /// Resize the container if neccessary
    if (m_dataCache.size() <= (*index)) m_dataCache.resize(*index + 1);
    MdtFullCalibData& cache = m_dataCache[*index];
    if (cache.rtRelation && cache.rtRelation != rtRelation) {
        msg << MSG::ERROR<<__FILE__<<":"<<__LINE__<<" There already exist a rt relation object for multilayer "
            <<m_idHelperSvc->toString(mlID) << endmsg;
        return false;
    }
    cache.rtRelation = std::move(rtRelation);

    if (msg.level() <= MSG::DEBUG) {
         msg << MSG::DEBUG<<__FILE__<<":"<<__LINE__<<" Added successfully the rt relations for "
             << m_idHelperSvc->toString(mlID)<<endmsg;
    }    
    return true;
}
bool MdtCalibDataContainer::storeData(const Identifier& mlID, TubeContainerPtr tubeContainer, MsgStream& msg){
    /// Check for a valid index
    std::optional<unsigned int> index = containerIndex(mlID, msg);
    if (!index) return false;
    if (!tubeContainer) {
        msg << MSG::ERROR<<__FILE__<<":"<<__LINE__<<" No tube ontainer parsed for multilayer "
            <<m_idHelperSvc->toString(mlID) << endmsg;
        return false;
    }

    /// Resize the container if neccessary
    if (m_dataCache.size() <= (*index)) m_dataCache.resize(*index + 1);
    MdtFullCalibData& cache = m_dataCache[*index];
    if (cache.tubeCalib && cache.tubeCalib != tubeContainer) {
        msg << MSG::ERROR<<__FILE__<<":"<<__LINE__<<" There already exist a tube calibration container for multilayer "
            <<m_idHelperSvc->toString(mlID) << endmsg;
        return false;
    }
    if (msg.level() <= MSG::DEBUG) {
         msg << MSG::DEBUG<<__FILE__<<":"<<__LINE__<<" Added successfully the tube calibrations for "
             << m_idHelperSvc->toString(mlID)<<endmsg;
    }
    cache.tubeCalib = tubeContainer;
    if (m_granularity == RegionGranularity::OneRt || 
        m_idHelper.multilayer(mlID) == 2 || 
        m_idHelper.numberOfMultilayers(mlID) == 1) return true;
    return storeData(m_idHelper.multilayerID(mlID, 2), tubeContainer, msg);
}



}