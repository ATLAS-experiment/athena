/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/MdtCondDbData.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "GeoModelKernel/throwExcept.h"

using DcsConstants = MdtCondDbData::DcsConstants;
// --- writing identifiers -------
MdtCondDbData::MdtCondDbData(const MdtIdHelper& id_helper):
    m_id_helper(id_helper) {}

void MdtCondDbData::setDeadTube(const Identifier& Id) { m_cachedDeadTubes.insert(Id); }
void MdtCondDbData::setDeadLayer(const Identifier& Id) { m_cachedDeadLayers.insert(Id); }
void MdtCondDbData::setDeadMultilayer(const Identifier& Id) { m_cachedDeadMultilayers.insert(Id); }
void MdtCondDbData::setDeadChamber(const Identifier& Id) { m_cachedDeadChambers.insert(Id); }


const std::set<Identifier>& MdtCondDbData::getDeadTubesId() const{ return m_cachedDeadTubes; }
const std::set<Identifier>& MdtCondDbData::getDeadLayersId() const{ return m_cachedDeadLayers; }
const std::set<Identifier>& MdtCondDbData::getDeadMultilayersId() const{ return m_cachedDeadMultilayers; }
const std::set<Identifier>& MdtCondDbData::getDeadChambersId() const{ return m_cachedDeadChambers; }

bool  MdtCondDbData::isGood(const Identifier & Id) const {
    return (isGoodChamber(Id) && isGoodMultilayer(Id) && isGoodLayer(Id) && isGoodTube(Id));
}

bool MdtCondDbData::isGoodTube(const Identifier & Id) const { return !m_cachedDeadTubes.count(Id); } 
bool MdtCondDbData::isGoodLayer(const Identifier & Id) const { 
    if (m_cachedDeadLayers.empty()) return true;
    const int layer = m_id_helper.tubeLayer(Id);
    const int multiLayer = m_id_helper.multilayer(Id);
    return !m_cachedDeadLayers.count(m_id_helper.channelID(Id,multiLayer, layer, 1)); 
} 
bool MdtCondDbData::isGoodMultilayer(const Identifier & Id) const { 
    return m_cachedDeadLayers.empty() || !m_cachedDeadMultilayers.count(m_id_helper.multilayerID(Id)); 
} 
bool MdtCondDbData::isGoodChamber(const Identifier & Id) const { 
  return m_cachedDeadChambers.empty() || !m_cachedDeadChambers.count(m_id_helper.elementID(Id)); 
} 

void MdtCondDbData::setHvState(const Identifier& multiLayerID, const DcsFsmState state, const float standByVolt, const float readyVolt) {
    
    if (m_dcsStates.empty()) m_dcsStates.resize(m_id_helper.detectorElement_hash_max());
    
    IdentifierHash hash{};
    m_id_helper.get_detectorElement_hash(multiLayerID, hash);
    unsigned int hashIdx = static_cast<unsigned int>(hash);
    if (hashIdx >= m_dcsStates.size()) {
        return;
    }
    DcsConstants& constants = m_dcsStates.at(hashIdx);
    constants.standbyVolt = standByVolt;
    constants.readyVolt = readyVolt;
    constants.fsmState = state;
}
const std::vector<DcsConstants>& MdtCondDbData::getAllHvStates() const { return m_dcsStates; }
const DcsConstants& MdtCondDbData::getHvState(const Identifier& multiLayerID) const {
  IdentifierHash hash{};
  m_id_helper.get_detectorElement_hash(multiLayerID, hash);
  unsigned int hashIdx =static_cast<unsigned int>(hash);
  if (hashIdx >= m_dcsStates.size()) {
     THROW_EXCEPTION("MdtCondDbData::getHvState() - Out of bound access "+
                          std::to_string(hashIdx) + " vs. "+std::to_string(m_dcsStates.size()));
  } 
  return m_dcsStates[hashIdx];
}
bool MdtCondDbData::hasDCS() const {
  return m_dcsStates.size();
}



