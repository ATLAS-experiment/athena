/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include <fstream>
#include <utility>

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoModelKernel/throwExcept.h"
#include "MuonAlignmentData/ALinePar.h"
#include "MuonAlignmentData/BLinePar.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "MuonReadoutGeometry/GlobalUtilities.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

namespace {
    template <typename read_out> void clearCache(std::vector<std::unique_ptr<read_out>>& array) {
        for (std::unique_ptr<read_out>& ele : array) {
            if (ele) ele->clearCache();
        }
    }
    template <typename read_out> void fillCache(std::vector<std::unique_ptr<read_out>>& array) {
        for (std::unique_ptr<read_out>& ele : array) {
            if (ele) ele->fillCache();
        }
    }
}

namespace MuonGM {

    MuonDetectorManager::MuonDetectorManager(): AthMessaging{"MGM::MuonDetectorManager"} { 
        setName("Muon");
        if (m_idHelperSvc.retrieve().isFailure()) {
           THROW_EXCEPTION("MuonDetectorManager() - No IdHelper svc is available");
        }
        loadStationIndices();
        if (m_idHelperSvc->hasMDT()){
            m_mdtArray.resize(m_idHelperSvc->mdtIdHelper().detectorElement_hash_max());
        }
        if (m_idHelperSvc->hasCSC()){
            m_cscArray.resize(m_idHelperSvc->cscIdHelper().detectorElement_hash_max());
        }
        if (m_idHelperSvc->hasTGC()){
            m_tgcArray.resize(m_idHelperSvc->tgcIdHelper().detectorElement_hash_max());
        }
        if (m_idHelperSvc->hasRPC()){
            m_rpcArray.resize(m_idHelperSvc->rpcIdHelper().detectorElement_hash_max());
        }
        if (m_idHelperSvc->hasMM()){
            m_mmcArray.resize(m_idHelperSvc->mmIdHelper().detectorElement_hash_max());
        }
        if (m_idHelperSvc->hasSTGC()){
            m_stgArray.resize(m_idHelperSvc->stgcIdHelper().detectorElement_hash_max());
        }
    }
    void MuonDetectorManager::setMMPassivation(const NswPassivationDbData* passiv) {
        m_mmPassivation = passiv;
    }

    MuonDetectorManager::~MuonDetectorManager()  = default;

    void MuonDetectorManager::clearCache() {
        ::clearCache(m_mdtArray);
        ::clearCache(m_rpcArray);
        ::clearCache(m_tgcArray);
        ::clearCache(m_cscArray);
        ::clearCache(m_mmcArray);
        ::clearCache(m_stgArray);
    }
    void MuonDetectorManager::fillCache() {       
        ATH_MSG_INFO( "Filling cache" );
        ::fillCache(m_mdtArray);
        ::fillCache(m_rpcArray);
        ::fillCache(m_tgcArray);
        ::fillCache(m_cscArray);
        ::fillCache(m_mmcArray);
        ::fillCache(m_stgArray);
    }
    const MuonReadoutElement* MuonDetectorManager::getReadoutElement(const Identifier& id) const {
        const MuonReadoutElement* reEle{nullptr};
        using TechIndex = Muon::MuonStationIndex::TechnologyIndex;
        switch (m_idHelperSvc->technologyIndex(id)){
            case TechIndex::MDT:
                reEle = getMdtReadoutElement(id);
                break;
            case TechIndex::RPC:
                reEle = getRpcReadoutElement(id);
                break;
            case TechIndex::TGC:
                reEle = getTgcReadoutElement(id);
                break;
            case TechIndex::CSC:
                reEle = getCscReadoutElement(id);
                break;
            case TechIndex::MM:
                reEle = getMMReadoutElement(id);
                break;
            case TechIndex::STGC:
                reEle = getsTgcReadoutElement(id);
                break;
            default:
                ATH_MSG_WARNING("Invalid technology ");
        };
        if (!reEle) ATH_MSG_WARNING("No readout element retrieved "<<m_idHelperSvc->toString(id));
        return reEle;
    }

    unsigned int MuonDetectorManager::getNumTreeTops() const { return m_envelope.size(); }

    PVConstLink MuonDetectorManager::getTreeTop(unsigned int i) const { return m_envelope[i]; }
    PVLink MuonDetectorManager::getTreeTop(unsigned int i) { return m_envelope[i]; }
    void MuonDetectorManager::addTreeTop(PVLink pV) {
        m_envelope.push_back(pV);
    }

    void MuonDetectorManager::addMuonStation(std::unique_ptr<MuonStation>&& mst) {
        std::string key = muonStationKey(mst->getStationType(), mst->getEtaIndex(), mst->getPhiIndex());
        m_MuonStationMap[key] = std::move(mst);
    }

    std::string MuonDetectorManager::muonStationKey(const std::string& stName, int statEtaIndex, int statPhiIndex) {
        std::string key;
        if (statEtaIndex < 0)
            key = stName.substr(0, 3) + "_C_zi" + MuonGM::buildString(std::abs(statEtaIndex), 2) + "fi" +
                  MuonGM::buildString(statPhiIndex, 2);
        else
            key = stName.substr(0, 3) + "_A_zi" + MuonGM::buildString(std::abs(statEtaIndex), 2) + "fi" +
                  MuonGM::buildString(statPhiIndex, 2);
        return key;
    }

    const MuonStation* MuonDetectorManager::getMuonStation(const std::string& stName, int stEtaIndex, int stPhiIndex) const {
        std::string key = muonStationKey(stName, stEtaIndex, stPhiIndex);

        std::map<std::string, std::unique_ptr<MuonStation>>::const_iterator it = m_MuonStationMap.find(key);
        if (it != m_MuonStationMap.end())
            return (*it).second.get();
        else
            return nullptr;
    }

    MuonStation* MuonDetectorManager::getMuonStation(const std::string& stName, int stEtaIndex, int stPhiIndex) {
        std::string key = muonStationKey(stName, stEtaIndex, stPhiIndex);

        std::map<std::string, std::unique_ptr<MuonStation>>::const_iterator it = m_MuonStationMap.find(key);
        if (it != m_MuonStationMap.end())
            return (*it).second.get();
        else
            return nullptr;
    }

    const std::vector<const MuonStation*> MuonDetectorManager::getMuonStations() const { 

        std::vector<const MuonStation*> stationList;
        stationList.reserve(m_MuonStationMap.size());
        for (const auto & [name,ptr]: m_MuonStationMap){
            stationList.push_back(ptr.get());
        }    
        return stationList;
    } 


    void MuonDetectorManager::addRpcReadoutElement(std::unique_ptr<RpcReadoutElement>&& x) {
        const Identifier id = x->identify();        
        int idx = rpcIdentToArrayIdx(id);
        if (m_rpcArray[idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            THROW_EXCEPTION("Double readout element assignment");               
        }
        m_rpcArray[idx] = std::move(x);
        ++m_n_rpcRE;
    }

    const RpcReadoutElement* MuonDetectorManager::getRpcReadoutElement(const Identifier& id) const {
        int idx = rpcIdentToArrayIdx(id);
        return m_rpcArray[idx].get();
    }

    void MuonDetectorManager::addMMReadoutElement(std::unique_ptr<MMReadoutElement>&& x) {
        const int array_idx = mmIdenToArrayIdx(x->identify());
        if (m_mmcArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(x->identify())<<" which has been already added.");
            THROW_EXCEPTION("Double readout element assignment"); 
        }
        m_mmcArray[array_idx] = std::move(x);
        ++m_n_mmcRE;
    }
    
    void MuonDetectorManager::addsTgcReadoutElement(std::unique_ptr<sTgcReadoutElement>&& x) {       
        const int array_idx = stgcIdentToArrayIdx(x->identify());
        if (m_stgArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(x->identify())<<" which has been already added.");
            THROW_EXCEPTION("Double readout element assignment"); 
        }
        m_stgArray[array_idx] = std::move(x);
        ++m_n_stgRE;
    }

    void MuonDetectorManager::addMdtReadoutElement(std::unique_ptr<MdtReadoutElement>&& x) {       
        const Identifier id = x->identify();
        const int arrayIdx = mdtIdentToArrayIdx(id);
        if (m_mdtArray[arrayIdx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            THROW_EXCEPTION("Double readout element assignment");
        }
        m_mdtArray[arrayIdx] = std::move(x);
        ++m_n_mdtRE;
    }

    const MdtReadoutElement* MuonDetectorManager::getMdtReadoutElement(const Identifier& id) const {
        const int arrayIdx = mdtIdentToArrayIdx(id);
        return arrayIdx < 0 ? nullptr : m_mdtArray[arrayIdx].get();
    }

     MdtReadoutElement* MuonDetectorManager::getMdtReadoutElement(const Identifier& id) {
        const int arrayIdx = mdtIdentToArrayIdx(id);
        return arrayIdx < 0 ? nullptr :  m_mdtArray[arrayIdx].get();
    }

    void MuonDetectorManager::addCscReadoutElement(std::unique_ptr<CscReadoutElement>&& x) {
        const Identifier id = x->identify();
        const int array_idx = cscIdentToArrayIdx(id);
        if (m_cscArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            THROW_EXCEPTION("Double readout element assignment");
        }
        m_cscArray[array_idx] = std::move(x);
        ++m_n_cscRE;
    }

    const CscReadoutElement* MuonDetectorManager::getCscReadoutElement(const Identifier& id) const {
        const int array_idx = cscIdentToArrayIdx(id);
        return array_idx < 0 ? nullptr :  m_cscArray[array_idx].get();
    }

     CscReadoutElement* MuonDetectorManager::getCscReadoutElement(const Identifier& id) {
        const int array_idx = cscIdentToArrayIdx(id);
        return array_idx < 0 ? nullptr : m_cscArray[array_idx].get();
    }
    
    void MuonDetectorManager::addTgcReadoutElement(std::unique_ptr<TgcReadoutElement>&& x) {
        const Identifier id = x->identify();
        const int array_idx = tgcIdentToArrayIdx(id);
        if (m_tgcArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            THROW_EXCEPTION("Double readout element assignment");
        }

        m_tgcArray[array_idx] = std::move(x);
        ++m_n_tgcRE;
    }

    const TgcReadoutElement* MuonDetectorManager::getTgcReadoutElement(const Identifier& id) const {
        const int array_idx = tgcIdentToArrayIdx(id);
        return array_idx < 0 ? nullptr : m_tgcArray[array_idx].get();
    }
    TgcReadoutElement* MuonDetectorManager::getTgcReadoutElement(const Identifier& id) {
        const int array_idx = tgcIdentToArrayIdx(id);
        return array_idx < 0 ? nullptr : m_tgcArray[array_idx].get();
    }   
    const MMReadoutElement* MuonDetectorManager::getMMReadoutElement(const Identifier& id) const {
        const int array_idx = mmIdenToArrayIdx(id);
        return array_idx < 0 ? nullptr : m_mmcArray[array_idx].get();
    }
    const sTgcReadoutElement* MuonDetectorManager::getsTgcReadoutElement(const Identifier& id) const {
        const int array_idx = stgcIdentToArrayIdx(id);
        return array_idx < 0 ? nullptr : m_stgArray[array_idx].get();
    }
    int MuonDetectorManager::mdtIdentToArrayIdx(const Identifier& id) const {
        const int hash = static_cast<int>(m_idHelperSvc->detElementHash(id));
#ifndef NDEBUG
        if (hash <0) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
#endif 
        return hash;
    }

    int MuonDetectorManager::mmIdenToArrayIdx(const Identifier& id) const {
        const int hash = static_cast<int>(m_idHelperSvc->detElementHash(id));
#ifndef NDEBUG
        if (hash <0) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
#endif 
        return hash;
    }
    int MuonDetectorManager::stgcIdentToArrayIdx(const Identifier& id) const {
        const int hash = static_cast<int>(m_idHelperSvc->detElementHash(id));
#ifndef NDEBUG
        if (hash <0) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
#endif 
        return hash;
    }
    int MuonDetectorManager::rpcIdentToArrayIdx(const Identifier& id) const {
        const int hash = static_cast<int>(m_idHelperSvc->detElementHash(id));
#ifndef NDEBUG
        if (hash <0) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
#endif 
        return hash;
    }
    int MuonDetectorManager::tgcIdentToArrayIdx(const Identifier& id) const {
        const int hash = static_cast<int>(m_idHelperSvc->detElementHash(id));
#ifndef NDEBUG
        if (hash <0) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
#endif 
        return hash;
    }    
    int MuonDetectorManager::cscIdentToArrayIdx(const Identifier& id) const {
        const int hash = static_cast<int>(m_idHelperSvc->detElementHash(id));
#ifndef NDEBUG
        if (hash <0) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
#endif
        return hash;
    }
    
    StatusCode MuonDetectorManager::updateAlignment(const ALineContainer& alineData) {
        if (alineData.empty()) {
            ATH_MSG_DEBUG("Got empty A-line container (expected for MC), not applying A-lines...");
            return StatusCode::SUCCESS;
        }

        using Parameter = ALinePar::Parameter;
        // loop over the container of the updates passed by the MuonAlignmentDbTool
        unsigned int nLines{0}, nUpdates{0};
        for (const ALinePar&  ALine : alineData) {
            nLines++;
            ATH_MSG_DEBUG(ALine << " is new. ID = " << m_idHelperSvc->toString(ALine.identify()));
            const std::string stType = ALine.AmdbStation();
            const int jff = ALine.AmdbPhi();
            const int jzz = ALine.AmdbEta();
            const int job = ALine.AmdbJob();
            //********************
            // NSW Cases
            //********************            
            if (stType[0] == 'M' || stType[0] == 'S') {            
                if (!nMMRE() || !nsTgcRE()) {
                    ATH_MSG_WARNING("Unable to set A-line; the manager does not contain NSW readout elements" );
                    continue;
                }
                if (stType[0] == 'M') {
                    // Micromegas                        
                    const int array_idx  = mmIdenToArrayIdx(ALine.identify());
                    MMReadoutElement* RE = m_mmcArray[array_idx].get();

                    if (!RE) {
                        ATH_MSG_WARNING(ALine << " *** No MM readout element found\n"
                            << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use");
                        return StatusCode::FAILURE;
                    }
                
                    RE->setDelta(ALine);

                } else if (stType[0] == 'S') {
                    // sTGC
                    const int array_idx    = stgcIdentToArrayIdx(ALine.identify());
                    sTgcReadoutElement* RE = m_stgArray[array_idx].get();

                    if (!RE) {
                        ATH_MSG_WARNING(ALine << " *** No sTGC readout element found\n"
                            << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use");
                        return StatusCode::FAILURE;
                    }
                
                    RE->setDelta(ALine);
                }
                continue;
            }
             

            //********************
            // Non-NSW Cases
            //********************
            MuonStation* thisStation = getMuonStation(stType, jzz, jff);
            if (!thisStation) {
                ATH_MSG_WARNING("ALinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << "*** No MuonStation found\n"
                    << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use" );
                continue;
            }

            if (job != 0) {
                // job different than 0 (standard for TGC conditions for Sept 2010 repro.)
                if (stType[0] == 'T') {                    
                    ATH_MSG_DEBUG( "ALinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                        << " has JOB not 0 - this is expected for TGC" );
                } else {
                    ATH_MSG_WARNING("ALinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                        << " has JOB not 0 - this is NOT EXPECTED yet for non TGC chambers - skipping this A-line" );
                    continue;
                }
            }
            if (job == 0) {
                ATH_MSG_DEBUG( "Setting delta transform for Station " << ALine);
                using Parameter = ALinePar::Parameter;
                thisStation->setDelta_fromAline(ALine.getParameter(Parameter::transS), 
                                                ALine.getParameter(Parameter::transZ), 
                                                ALine.getParameter(Parameter::transT), 
                                                ALine.getParameter(Parameter::rotS),
                                                ALine.getParameter(Parameter::rotZ),
                                                ALine.getParameter(Parameter::rotT));
                
                thisStation->clearCache();
                thisStation->fillCache();                
            } else {
                // job different than 0 (standard for TGC conditions for Sept 2010 repro.)
                ATH_MSG_DEBUG( "Setting delta transform for component " << ALine);
                thisStation->setDelta_fromAline_forComp(job, 
                                                ALine.getParameter(Parameter::transS), 
                                                ALine.getParameter(Parameter::transZ), 
                                                ALine.getParameter(Parameter::transT), 
                                                ALine.getParameter(Parameter::rotS),
                                                ALine.getParameter(Parameter::rotZ),
                                                ALine.getParameter(Parameter::rotT));
                
                thisStation->getMuonReadoutElement(job)->refreshCache();
                
            }
            nUpdates++;
        }
        ATH_MSG_INFO( "# of A-lines read from the ALineMapContainer in StoreGate is " << nLines );
        ATH_MSG_INFO( "# of deltaTransforms updated according to A-lines         is " << nUpdates );
        return StatusCode::SUCCESS;
    }

    StatusCode MuonDetectorManager::updateDeformations(const BLineContainer& blineData) {
        ATH_MSG_DEBUG( "In updateDeformations()" );        
        if (blineData.empty()) {
            ATH_MSG_DEBUG( "Got empty B-line container (expected for MC), not applying B-lines..." );
            return StatusCode::SUCCESS;
        } else
            ATH_MSG_INFO( "temporary B-line container with size = " << blineData.size() );

        // loop over the container of the updates passed by the MuonAlignmentDbTool
        unsigned int nLines{0}, nUpdates{0};
        for (const BLinePar& BLine : blineData) {
            ++nLines;
            const std::string stType = BLine.AmdbStation();
            const int jff = BLine.AmdbPhi();
            const int jzz = BLine.AmdbEta();
            const int job = BLine.AmdbJob();
            //********************
            // NSW Cases
            //********************            
            if (stType[0] == 'M' || stType[0] == 'S') {            
                if (!nMMRE() || !nsTgcRE()) {
                    ATH_MSG_WARNING("Unable to set B-line; the manager does not contain NSW readout elements" );
                    continue;
                }
                if (stType[0] == 'M') {
                    // Micromegas                        
                    const int array_idx  = mmIdenToArrayIdx(BLine.identify());
                    MMReadoutElement* RE = m_mmcArray[array_idx].get();

                    if (!RE) {
                        ATH_MSG_WARNING("BlinePar with AmdbId " <<BLine<< " *** No MM readout element found\n"
                            << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use");
                        return StatusCode::FAILURE;
                    }
                    RE->setBLinePar(BLine);
                } else if (stType[0] == 'S') {
                    // sTGC
                    const int array_idx    = stgcIdentToArrayIdx(BLine.identify());
                    sTgcReadoutElement* RE = m_stgArray[array_idx].get();
                    if (!RE) {
                        ATH_MSG_WARNING("BlinePar with AmdbId " << BLine << " *** No sTGC readout element found\n"
                            << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use");
                        return StatusCode::FAILURE;
                    }
                    RE->setBLinePar(BLine);
                }
                continue;
            }
            
            //********************
            // MDT Cases
            //********************
            if (stType.at(0) == 'T' || stType.at(0) == 'C' || (stType.substr(0, 3) == "BML" && std::abs(jzz) == 7)) {                
                ATH_MSG_DEBUG( "BLinePar with AmdbId " << BLine << " is not a MDT station - skipping" );
                continue;
            }
            ATH_MSG_DEBUG( "BLinePar with AmdbId " <<BLine << " is new ID = " << m_idHelperSvc->toString(BLine.identify()) );
            if (job == 0) {
                MuonStation* thisStation = getMuonStation(stType, jzz, jff);
                if (!thisStation) {
                    ATH_MSG_WARNING("BLinePar with AmdbId " << BLine <<
                            " *** No MuonStation found \n PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and "
                           "Geometry Layout in use");
                    continue;
                }
                ATH_MSG_DEBUG( "Setting deformation parameters for Station " << stType << " " << jzz << " " << jff << " ");
                thisStation->setBline(&BLine);                
                nUpdates++;
            } else {
                ATH_MSG_WARNING("BLinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " has JOB not 0 ");
                return StatusCode::FAILURE;
            }
        }
        ATH_MSG_INFO( "# of B-lines read from the ALineMapContainer in StoreGate   is " << nLines );
        ATH_MSG_INFO( "# of deform-Transforms updated according to B-lines         is " << nUpdates );
        return StatusCode::SUCCESS;
    }

    StatusCode MuonDetectorManager::updateCSCInternalAlignmentMap(const ALineContainer& ilineData) {
       
        if (ilineData.empty()) {
            ATH_MSG_WARNING("Empty temporary CSC I-line container - nothing to do here" );
            return StatusCode::SUCCESS;
        } else
            ATH_MSG_INFO( "temporary CSC I-line container with size = " << ilineData.size() );

        // loop over the container of the updates passed by the MuonAlignmentDbTool
        unsigned int nLines{0}, nUpdates{0};
        for (const ALinePar& ILine : ilineData) {
            nLines++;
            const std::string stType = ILine.AmdbStation();
            const int jff = ILine.AmdbPhi();
            const int jzz = ILine.AmdbEta();
            const int job = ILine.AmdbJob();
            ATH_MSG_DEBUG( "CscInternalAlignmentPar with AmdbId " << ILine << " is new ID = " << m_idHelperSvc->toString(ILine.identify()) );
            if (job == 3) {
                MuonStation* thisStation = getMuonStation(stType, jzz, jff);
                if (!thisStation) {
                    ATH_MSG_WARNING("CscInternalAlignmentPar with AmdbId " << ILine
                        << " *** No MuonStation found \n PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and "
                           "Geometry Layout in use");
                    continue;
                }
                ATH_MSG_DEBUG( "Setting CSC I-Lines for Station " <<ILine);
                CscReadoutElement* CscRE = dynamic_cast<CscReadoutElement*>(thisStation->getMuonReadoutElement(job));
                if (!CscRE)
                    ATH_MSG_ERROR( "The CSC I-lines container includes stations which are no CSCs! This is impossible." );
                else {
                    CscRE->setCscInternalAlignmentPar(ILine);
                }               
                thisStation->refreshCache();
                
                nUpdates++;

            } else {
                ATH_MSG_ERROR( "job for CSC I-Lines= " << job << " is not 3 => This is not valid." );
            }
        }
        ATH_MSG_INFO( "# of CSC I-lines read from the ILineMapContainer in StoreGate is " << nLines );
        ATH_MSG_INFO( "# of deltaTransforms updated according to A-lines             is " << nUpdates );
        return StatusCode::SUCCESS;
    }
    StatusCode MuonDetectorManager::updateMdtAsBuiltParams(const MdtAsBuiltContainer& asbuiltData) {
       
        if (asbuiltData.empty()) {
            ATH_MSG_WARNING("Empty temporary As-Built container - nothing to do here" );
            return StatusCode::SUCCESS;
        } else
            ATH_MSG_INFO( "temporary As-Built container with size = " << asbuiltData.size() );

        // loop over the container of the updates passed by the MuonAlignmentDbTool
        unsigned int nLines{0}, nUpdates{0};
        for (const auto& AsBuiltPar : asbuiltData) {
            nLines++;
            const std::string stType = AsBuiltPar.AmdbStation();
            const int jff = AsBuiltPar.AmdbPhi();
            const int jzz = AsBuiltPar.AmdbEta();
            
            ATH_MSG_DEBUG( "MdtAsBuiltPar with AmdbId " << AsBuiltPar
                    << " is new ID = " << m_idHelperSvc->toString(AsBuiltPar.identify()) );

            MuonStation* thisStation = getMuonStation(stType, jzz, jff);
            if (thisStation) {
                
                ATH_MSG_DEBUG( "Setting as-built parameters for Station " << AsBuiltPar );
                thisStation->setMdtAsBuiltParams(&AsBuiltPar);
                nUpdates++;
            } else {
                ATH_MSG_WARNING("MdtAsBuiltPar with AmdbId " <<AsBuiltPar
                    << " *** No MuonStation found \n PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and "
                       "Geometry Layout in use");
                continue;
            }
        }
        ATH_MSG_INFO( "# of MDT As-Built read from the MdtAsBuiltMapContainer in StoreGate is " << nLines );
        ATH_MSG_INFO( "# of deltaTransforms updated according to As-Built                  is " << nUpdates );
        return StatusCode::SUCCESS;
    }
    
    void MuonDetectorManager::setNswAsBuilt(const NswAsBuiltDbData* nswAsBuiltData) {
        m_nswAsBuilt = nswAsBuiltData;
    }

    void MuonDetectorManager::setsTGCAsBuilt(const sTGCAsBuiltData* stgcAsBuilt) {
        m_stgcAsBuildData = stgcAsBuilt;
    }

    const MdtReadoutElement* MuonDetectorManager::getMdtReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= m_idHelperSvc->mdtIdHelper().detectorElement_hash_max()) {           
            ATH_MSG_WARNING(" try to getMdtReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << m_idHelperSvc->mdtIdHelper().detectorElement_hash_max() - 1 );
            return nullptr;
        }
#endif
        return m_mdtArray[id].get();
    }

    const RpcReadoutElement* MuonDetectorManager::getRpcReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= m_idHelperSvc->rpcIdHelper().detectorElement_hash_max()) {           
            ATH_MSG_WARNING(" try to getRpcReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << m_idHelperSvc->rpcIdHelper().detectorElement_hash_max() - 1 );
            return nullptr;
        }
#endif
        return m_rpcArray[id].get();
    }

    const TgcReadoutElement* MuonDetectorManager::getTgcReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= m_idHelperSvc->tgcIdHelper().detectorElement_hash_max()) {           
            ATH_MSG_WARNING(" try to getTgcReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << m_idHelperSvc->tgcIdHelper().detectorElement_hash_max() - 1 );
            return nullptr;
        }
#endif
        return m_tgcArray[id].get();
    }

    const CscReadoutElement* MuonDetectorManager::getCscReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= m_idHelperSvc->cscIdHelper().detectorElement_hash_max()) {           
            ATH_MSG_WARNING(" try to getCscReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << m_idHelperSvc->cscIdHelper().detectorElement_hash_max() - 1 );
            return nullptr;
        }
#endif
        return m_cscArray[id].get();
    }

    unsigned int MuonDetectorManager::rpcStationTypeIdx(const int stationName) const {
        std::map<int, int>::const_iterator itr = m_rpcStatToIdx.find(stationName);
        if (itr != m_rpcStatToIdx.end()) return itr->second;
        return RpcStatType::UNKNOWN;
    }

    int MuonDetectorManager::rpcStationName(const int stationIndex) const {
        std::map<int, int>::const_iterator itr = m_rpcIdxToStat.find(stationIndex);
        if (itr != m_rpcIdxToStat.end()) return itr->second;
        return -1;
    }
    void MuonDetectorManager::loadStationIndices() {
        
        if (!m_idHelperSvc->hasRPC()) return;
        const RpcIdHelper& rpcHelper{m_idHelperSvc->rpcIdHelper()};
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BML"), RpcStatType::BML));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BMS"), RpcStatType::BMS));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BOL"), RpcStatType::BOL));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BOS"), RpcStatType::BOS));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BMF"), RpcStatType::BMF));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BOF"), RpcStatType::BOF));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BOG"), RpcStatType::BOG));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BME"), RpcStatType::BME));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BIR"), RpcStatType::BIR));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BIM"), RpcStatType::BIM));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BIL"), RpcStatType::BIL));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BIS"), RpcStatType::BIS));

        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BML, rpcHelper.stationNameIndex("BML")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BMS, rpcHelper.stationNameIndex("BMS")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BOL, rpcHelper.stationNameIndex("BOL")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BOS, rpcHelper.stationNameIndex("BOS")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BMF, rpcHelper.stationNameIndex("BMF")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BOF, rpcHelper.stationNameIndex("BOF")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BOG, rpcHelper.stationNameIndex("BOG")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BME, rpcHelper.stationNameIndex("BME")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BIR, rpcHelper.stationNameIndex("BIR")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BIM, rpcHelper.stationNameIndex("BIM")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BIL, rpcHelper.stationNameIndex("BIL")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BIS, rpcHelper.stationNameIndex("BIS")));
    }    
    void MuonDetectorManager::set_DBMuonVersion(const std::string& version) { m_DBMuonVersion = version; }
    void MuonDetectorManager::setGeometryVersion(const std::string& version) { m_geometryVersion = version; }
    void MuonDetectorManager::setMinimalGeoFlag(int flag) { m_minimalgeo = flag; }
    void MuonDetectorManager::setCutoutsFlag(int flag) { m_includeCutouts = flag; }
    void MuonDetectorManager::setCutoutsBogFlag(int flag) { m_includeCutoutsBog = flag; }


}  // namespace MuonGM
