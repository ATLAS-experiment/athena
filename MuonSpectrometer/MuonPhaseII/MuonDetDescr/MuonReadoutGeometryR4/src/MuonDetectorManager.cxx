/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include <limits>
#include <cassert>

#ifndef SIMULATIONBASE
#include "Acts/Utilities/Helpers.hpp"
#endif
namespace {
    using ChIndex = Muon::MuonStationIndex::ChIndex;
    template <class T>
    using ElementStorage_t = MuonGMR4::MuonDetectorManager::ElementStorage_t<T>;
    /// Helper function to copy the radout elements from a technology into the 
    /// vector of all readout elements.
    template <class ReadOutEleStoreType,
              class ReadoutEleReturnType> 
        void insert(const ElementStorage_t<ReadOutEleStoreType>& eleStore,
                    std::vector<ReadoutEleReturnType>& returnVec) {
        returnVec.reserve(returnVec.capacity() + eleStore.size());
        for (const auto& ele : eleStore) {
            if (ele) returnVec.push_back(ele.get());
        }
    }
    template <class ReadOutEleType,
              class ReadOutEleReturnType> 
            void insert(std::vector<ReadOutEleType*>&& eleStore,
                        std::vector<ReadOutEleReturnType*>& returnVec) {
        returnVec.insert(returnVec.end(), 
                        std::make_move_iterator(eleStore.begin()),
                        std::make_move_iterator(eleStore.end()));
    }
#ifndef SIMULATIONBASE
    inline unsigned msSectorIdHash(const ChIndex chIndex, const int sector, const int side) {
        using namespace Muon::MuonStationIndex;
        constexpr unsigned chIdxMax = Acts::toUnderlying(StIndex::StIndexMax);
        constexpr unsigned secMax = Muon::MuonStationIndex::numberOfSectors();
        return sector +  secMax* Acts::toUnderlying(toStationIndex(chIndex))* secMax + 
               (Acts::copySign(1,side) == 1) * chIdxMax * secMax;
    }
#endif
}

#define WRITE_SETTER(ELE_TYPE, SETTER, STORAGE_VEC)                                 \
    StatusCode MuonDetectorManager::SETTER(ElementPtr_t<ELE_TYPE> element) {        \
        if (!element) {                                                             \
            ATH_MSG_FATAL(__func__ << " -- nullptr is given.");                     \
            return StatusCode::FAILURE;                                             \
        }                                                                           \
        ATH_CHECK(element->initElement());                                          \
        element->releaseUnAlignedTrfs();                                            \
        size_t idx = static_cast<size_t>(element->identHash());                     \
        if (idx >= STORAGE_VEC.size()) {                                            \
            STORAGE_VEC.resize(idx + 1);                                            \
        }                                                                           \
        std::unique_ptr<ELE_TYPE>& new_element = STORAGE_VEC[idx];                  \
        if (new_element) {                                                          \
            ATH_MSG_FATAL("The detector element "                                   \
                          << m_idHelperSvc->toStringDetEl(element->identify())      \
                          << " has already been added before "                      \
                          <<m_idHelperSvc->toStringDetEl(new_element->identify())); \
            return StatusCode::FAILURE;                                             \
        }                                                                           \
        linkElements(STORAGE_VEC, element.get());                                   \
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
#define WRITE_ALLGETTER(CONSTNESS) \
    std::vector<CONSTNESS MuonReadoutElement*> \
        MuonDetectorManager::getAllReadoutElements(const ActsTrk::DetectorType type) CONSTNESS { \
        std::vector<CONSTNESS MuonReadoutElement*> allEles{};  \
        using enum ActsTrk::DetectorType;                      \
        if (type == UnDefined || type == Mdt) {                \
            insert(getAllMdtReadoutElements(), allEles);       \
        }                                                      \
        if (type == UnDefined || type == Rpc) {                \
            insert(getAllRpcReadoutElements(), allEles);       \
        }                                                      \
        if (type == UnDefined || type == Tgc) {                \
            insert(getAllTgcReadoutElements(), allEles);       \
        }                                                      \
        if (type == UnDefined || type == Mm) {                 \
            insert(getAllMmReadoutElements(), allEles);        \
        }                                                      \
        if (type == UnDefined || type == sTgc) {               \
            insert(getAllsTgcReadoutElements(), allEles);      \
        }                                                      \
        return allEles;                                        \
    }                                                          \
                                                               \
    CONSTNESS MuonReadoutElement*                              \
        MuonDetectorManager::getReadoutElement(const Identifier& id) CONSTNESS {          \
        switch(m_idHelperSvc->technologyIndex(id)) {                                      \
            using enum Muon::MuonStationIndex::TechnologyIndex;                           \
            case MDT:                                                                     \
                return getMdtReadoutElement(id);                                          \
            case RPC:                                                                     \
                return getRpcReadoutElement(id);                                          \
            case TGC:                                                                     \
                return getTgcReadoutElement(id);                                          \
            case STGC:                                                                    \
                return getsTgcReadoutElement(id);                                         \
            case MM:                                                                      \
                return getMmReadoutElement(id);                                           \
            default:                                                                      \
                ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Not a muon detector element "  \
                    <<m_idHelperSvc->toString(id));                                       \
        }                                                                                 \
        return nullptr;                                                                   \
    }


namespace MuonGMR4 {

#ifndef SIMULATIONBASE
    using MuonSectorSet = MuonDetectorManager::MuonSectorSet;
    using MuonChamberSet = MuonDetectorManager::MuonChamberSet;

    bool MuonDetectorManager::MSEnvelopeSorter::operator()(const SpectrometerSector* a, const SpectrometerSector* b) const {
        return (*a) < (*b);
    }
    bool MuonDetectorManager::MSEnvelopeSorter::operator()(const Chamber* a, const Chamber* b) const {
        return (*a) < (*b);
    }
#endif
MuonDetectorManager::~MuonDetectorManager() {
#ifndef SIMULATIONBASE
    m_secEnvelopes.clear();
#endif    
}
MuonDetectorManager::MuonDetectorManager()
    : AthMessaging{"MuonDetectorManagerR4"} {
    if (!m_idHelperSvc.retrieve().isSuccess()) {
        THROW_EXCEPTION(__func__<< "()  -- Failed to retrieve the Identifier service");
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
    assert(i < m_treeTopVector.size());
    return m_treeTopVector[i];
}
void MuonDetectorManager::clearTreeTops() { m_treeTopVector.clear(); }

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
    void MuonDetectorManager::addSpectrometerSector(ElementPtr_t<SpectrometerSector>&& chSector) {
        const unsigned hash = msSectorIdHash(chSector->chamberIndex(), chSector->sector(), chSector->side());
        ATH_MSG_DEBUG("Add new sector "<<(*chSector)<<", hash: "<<hash);
        const auto [element, isNew] = m_envelopesById.insert(std::make_pair(hash, chSector.get()));
        if (!isNew) {
            THROW_EXCEPTION("Conflicting hash: "<<hash<<", inserted: \n"<<(*element->second)
                           <<",\n tried to insert:\n "<<(*chSector));
        }
        m_secEnvelopes.push_back(std::move(chSector));
    }
    const SpectrometerSector* MuonDetectorManager::getSectorEnvelope(const Muon::MuonStationIndex::ChIndex chIdx,
                                                                     const unsigned sector,
                                                                     const int side) const {
        const unsigned hash = msSectorIdHash(chIdx, sector, side);
        EnvelopeMap_t::const_iterator itr = m_envelopesById.find(hash);
        if (itr != m_envelopesById.end()) {
            return itr->second;
        }
        ATH_MSG_WARNING("Failed to fetch valid envelope for "<<Muon::MuonStationIndex::chName(chIdx)
                        <<", sector: "<<sector<<", side: "<<side);
        return nullptr;
    }
    const SpectrometerSector* MuonDetectorManager::getSectorEnvelope(const Identifier& channelId) const {
        return getReadoutElement(channelId)->msSector();
    }
    /** @brief Retrieves the chamber enclosing the channel's readout element
      *  @param channelId: Identifier of a muon channel of interest*/
    const Chamber* MuonDetectorManager::getChamber(const Identifier& channelId) const {
        return getReadoutElement(channelId)->chamber();
    }
    /// @brief: Returns all MuonChambers associated with the readout geometry
    MuonSectorSet MuonDetectorManager::getAllSectors() const{
        return Acts::unpackConstSmartPointers(m_secEnvelopes);
    }
    MuonChamberSet MuonDetectorManager::getAllChambers() const {
        MuonChamberSet chambers{};
        std::ranges::for_each(m_secEnvelopes,
                             [&chambers](const ElementPtr_t<SpectrometerSector>& ms){
                                std::ranges::transform(ms->chambers(), std::back_inserter(chambers),
                                                       [](const SpectrometerSector::ChamberPtr& ch){
                                                            return ch.get();
                                                       });
                            });
        return chambers;
    }
#endif

template <class MuonDetectorType> void MuonDetectorManager::linkElements(ElementStorage_t<MuonDetectorType>& detStore,
                                                                         MuonDetectorType* reEle) {
        ATH_MSG_VERBOSE("No inter-linking for "<<ActsTrk::to_string(reEle->detectorType())<<" "<<detStore.size());
}
template <> void MuonDetectorManager::linkElements(ElementStorage_t<MdtReadoutElement>& detStore,
                                                   MdtReadoutElement* refEle) {
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        const int complMl = refEle->multilayer() == 2 ? 1 : idHelper.multilayerMax(refEle->identify());
        const Identifier complMlId = idHelper.multilayerID(refEle->identify(), complMl);
        const unsigned mlHash =static_cast<unsigned>(m_idHelperSvc->detElementHash(complMlId));
        /// The complementary detector element has not yet been parsed.
        if (mlHash >= detStore.size() || !detStore[mlHash]) {
            return;
        }
        detStore[mlHash]->setComplementaryReadoutEle(refEle);
        refEle->setComplementaryReadoutEle(detStore[mlHash].get());

}


}  // namespace MuonGMR4
#undef WRITE_SETTER
#undef ADD_DETECTOR
#undef WRITE_ALLGETTER