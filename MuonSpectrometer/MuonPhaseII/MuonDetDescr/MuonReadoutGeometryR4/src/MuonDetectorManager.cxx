/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include <limits>

namespace {
    template <class T>
    using ElementStorage = MuonGMR4::MuonDetectorManager::ElementStorage<T>;
    /// Helper function to copy the radout elements from a technology into the 
    /// vector of all readout elements.
    template <class ReadOutEleStoreType,
              class ReadoutEleReturnType> void insert(const ElementStorage<ReadOutEleStoreType>& eleStore,
                                                      std::vector<ReadoutEleReturnType>& returnVec) {
        returnVec.reserve(returnVec.capacity() + eleStore.size());
        for (const auto& ele : eleStore) {
            if (ele) returnVec.push_back(ele.get());
        }
    }
    template <class ReadOutEleType,
              class ReadOutEleReturnType> void insert(std::vector<ReadOutEleType*>&& eleStore,
                                                      std::vector<ReadOutEleReturnType*>& returnVec) {
        returnVec.insert(returnVec.end(), 
                        std::make_move_iterator(eleStore.begin()),
                        std::make_move_iterator(eleStore.end()));
    }
}

#define WRITE_SETTER(ELE_TYPE, SETTER, STORAGE_VEC)                                 \
    StatusCode MuonDetectorManager::SETTER(ElementPtr<ELE_TYPE> element) {          \
        if (!element) {                                                             \
            ATH_MSG_FATAL(__func__ << " -- nullptr is given.");                     \
            return StatusCode::FAILURE;                                             \
        }                                                                           \
        ATH_CHECK(element->initElement());                                          \
        element->releaseUnAlignedTrfs();                                            \
        size_t idx = static_cast<unsigned int>(element->identHash());               \
        if (idx >= STORAGE_VEC.size())                                              \
            STORAGE_VEC.resize(idx + 1);                                            \
        std::unique_ptr<ELE_TYPE>& new_element = STORAGE_VEC[idx];                  \
        if (new_element) {                                                          \
            ATH_MSG_FATAL("The detector element "                                   \
                          << m_idHelperSvc->toStringDetEl(element->identify())      \
                          << " has already been added before "                      \
                          <<m_idHelperSvc->toStringDetEl(new_element->identify())); \
            return StatusCode::FAILURE;                                             \
        }                                                                           \
        new_element = std::move(element);                                           \
        return StatusCode::SUCCESS;                                                 \
    }
#define ADD_DETECTOR(ELE_TYPE, STORAGE_VEC)                                         \
    WRITE_SETTER(ELE_TYPE, add##ELE_TYPE, STORAGE_VEC)                              \
                                                                                    \
    std::vector<const ELE_TYPE*> MuonDetectorManager::getAll##ELE_TYPE##s() const { \
         std::vector<const ELE_TYPE*> allElements{};                                \
         insert(STORAGE_VEC, allElements);                                          \
         return allElements;                                                        \
    }                                                                               \
                                                                                    \
    std::vector<ELE_TYPE*> MuonDetectorManager::getAll##ELE_TYPE##s() {             \
         std::vector<ELE_TYPE*> allElements{};                                      \
         insert(STORAGE_VEC, allElements);                                          \
         return allElements;                                                        \
    }
#define WRITE_ALLGETTER(TYPE) \
    std::vector<TYPE MuonReadoutElement*> MuonDetectorManager::getAllReadoutElements() TYPE { \
        std::vector<TYPE MuonReadoutElement*> allEles{};                                      \
        insert(getAllMdtReadoutElements(), allEles);                                          \
        insert(getAllRpcReadoutElements(), allEles);                                          \
        insert(getAllTgcReadoutElements(), allEles);                                          \
        insert(getAllMmReadoutElements(), allEles);                                           \
        insert(getAllsTgcReadoutElements(), allEles);                                         \
        return allEles;                                                                       \
    }                                                                                         \
    TYPE MuonReadoutElement* MuonDetectorManager::getReadoutElement(const Identifier& id) TYPE { \
    if (m_idHelperSvc->isMdt(id)) return getMdtReadoutElement(id);                               \
    else if (m_idHelperSvc->isRpc(id)) return getRpcReadoutElement(id);                          \
    else if (m_idHelperSvc->isTgc(id)) return getTgcReadoutElement(id);                          \
    else if (m_idHelperSvc->issTgc(id)) return getsTgcReadoutElement(id);                        \
    else if (m_idHelperSvc->isMM(id)) return getMmReadoutElement(id);                            \
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Not a muon detector element "                     \
                    <<m_idHelperSvc->toString(id));                                              \
    return nullptr;                                                                              \
}


namespace MuonGMR4 {

#ifndef SIMULATIONBASE
bool MuonDetectorManager::ChamberSorter::operator()(const MuonChamber* a, const MuonChamber* b) const {
    if (a->stationName() != b->stationName()) return a->stationName() < b->stationName();
    if (a->stationEta() != b->stationEta()) return a->stationEta() < b->stationEta();
    return a->stationPhi() < b->stationPhi();
}
#endif

MuonDetectorManager::MuonDetectorManager()
    : AthMessaging{"MuonDetectorManagerR4"} {
    if (!m_idHelperSvc.retrieve().isSuccess()) {
        ATH_MSG_FATAL(__func__<< "()  -- Failed to retrieve the Identifier service");
        throw std::runtime_error("MuonIdHelperSvc does not exist");
    }
    setName("MuonR4");
}

WRITE_ALLGETTER(const)
WRITE_ALLGETTER( )
ADD_DETECTOR(MdtReadoutElement, m_mdtEles);
ADD_DETECTOR(TgcReadoutElement, m_tgcEles);
ADD_DETECTOR(RpcReadoutElement, m_rpcEles);
ADD_DETECTOR(MmReadoutElement, m_mmEles);
ADD_DETECTOR(sTgcReadoutElement, m_sTgcEles);

unsigned int MuonDetectorManager::getNumTreeTops() const {
    return m_treeTopVector.size();
}
PVConstLink MuonDetectorManager::getTreeTop(unsigned int i) const {
    return m_treeTopVector[i];
}

void MuonDetectorManager::addTreeTop(PVConstLink pv) {
    m_treeTopVector.push_back(pv);
}
const Muon::IMuonIdHelperSvc* MuonDetectorManager::idHelperSvc() const {
    return m_idHelperSvc.get();
}
std::vector<ActsTrk::DetectorType> MuonDetectorManager::getDetectorTypes() const {
    std::vector<ActsTrk::DetectorType> types{};
    if (!m_mdtEles.empty()) types.push_back(ActsTrk::DetectorType::Mdt);
    if (!m_tgcEles.empty()) types.push_back(ActsTrk::DetectorType::Tgc);
    if (!m_rpcEles.empty()) types.push_back(ActsTrk::DetectorType::Rpc);
    if (!m_sTgcEles.empty()) types.push_back(ActsTrk::DetectorType::sTgc);
    if (!m_mmEles.empty()) types.push_back(ActsTrk::DetectorType::Mm);
    return types;
}

#ifndef SIMULATIONBASE
    const MuonChamber* MuonDetectorManager::getChamber(const Identifier& channelId) const {
        const MuonReadoutElement* re = getReadoutElement(channelId);
        return re ? re->getChamber() : nullptr;
    }
    MuonDetectorManager::MuonChamberSet MuonDetectorManager::getAllChambers() const{
         MuonChamberSet allChambers{};
         std::vector<const MuonReadoutElement*> allREs{getAllReadoutElements()};
         for (const MuonReadoutElement* re : allREs) {
            if (re->getChamber()) {
                allChambers.insert(re->getChamber());
            }
         }
         return allChambers;
    }
#endif


}  // namespace MuonGMR4
#undef WRITE_SETTER
#undef ADD_DETECTOR
#undef WRITE_ALLGETTER