/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include <fstream>
#include <utility>

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
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

namespace MuonGM {

    MuonDetectorManager::MuonDetectorManager(): AthMessaging{"MGM::MuonDetectorManager"} { 
        setName("Muon");
        if (m_idHelperSvc.retrieve().isFailure()) {
            throw std::runtime_error("MuonDetectorManager() - No IdHelper svc is available");
        }
        loadStationIndices();
    }

    MuonDetectorManager::~MuonDetectorManager()  = default;
    
    template <typename read_out, size_t N> void MuonDetectorManager::clearCache(std::array<std::unique_ptr<read_out>, N>& array) {
        for (std::unique_ptr<read_out>& ele : array) {
            if (ele) ele->clearCache();
        }
    }
    template <typename read_out, size_t N> void MuonDetectorManager::fillCache(std::array<std::unique_ptr<read_out>, N>& array) {
        for (std::unique_ptr<read_out>& ele : array) {
            if (ele) ele->fillCache();
        }
    }

    void MuonDetectorManager::clearCache() {
        clearCache(m_mdtArray);
        clearCache(m_rpcArray);
        clearCache(m_tgcArray);
        if (nCscRE())   clearCache(m_cscArray);
        if (nMMRE())    clearCache(m_mmcArray);
        if (nsTgcRE())  clearCache(m_stgArray);
    }
    void MuonDetectorManager::fillCache() {       
        ATH_MSG_INFO( "Filling cache" );
        fillCache(m_mdtArray);
        fillCache(m_rpcArray);
        fillCache(m_tgcArray);
        if (nCscRE()) fillCache(m_cscArray);
        if (nMMRE()) fillCache(m_mmcArray);
        if (nsTgcRE()) fillCache(m_stgArray);
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

    void MuonDetectorManager::addRpcReadoutElement(std::unique_ptr<RpcReadoutElement>&& x) {
        const Identifier id = x->identify();
        // add RE to map by RE hash
        const IdentifierHash Idhash = x->detectorElementHash();
        if (Idhash >= RpcRElMaxHash) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" with hash Id"
                                  <<Idhash<<" exceeding the allowed boundaries 0-"<<RpcRElMaxHash);
            throw std::runtime_error("Invalid hash assignment");
        } else {
            if (m_rpcArrayByHash[Idhash]) {
                ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been added before by"<<
                              m_idHelperSvc->toStringDetEl(m_rpcArrayByHash[Idhash]->identify()));
                throw std::runtime_error("Double assignment of the Hash");
            }
            m_rpcArrayByHash[Idhash] = x.get();
        }
        int dbz_index{-1};
        int idx = rpcIdentToArrayIdx(id, dbz_index);
        if (m_rpcArray[idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment");               
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
            throw std::runtime_error("Double readout element assignment"); 
        }
        m_mmcArray[array_idx] = std::move(x);
        ++m_n_mmcRE;
    }
    
    void MuonDetectorManager::addsTgcReadoutElement(std::unique_ptr<sTgcReadoutElement>&& x) {       
        const int array_idx = stgcIdentToArrayIdx(x->identify());
        if (m_stgArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(x->identify())<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment"); 
        }
        m_stgArray[array_idx] = std::move(x);
        ++m_n_stgRE;
    }

    void MuonDetectorManager::addMdtReadoutElement(std::unique_ptr<MdtReadoutElement>&& x) {       
       const Identifier id = x->identify();
        // add here the MdtReadoutElement to the array by RE hash
        // use already known RE hash
        const IdentifierHash Idhash = x->detectorElementHash();
        if (Idhash >= MdtRElMaxHash) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" with hash Id"
                                  <<Idhash<<" exceeding the allowed boundaries 0-"<<MdtRElMaxHash);
            throw std::runtime_error("Invalid hash assignment");
        }     
        const int arrayIdx = mdtIdentToArrayIdx(id);
        if (m_mdtArray[arrayIdx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment");
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
        // add here RE to array by hash
        const IdentifierHash Idhash = x->detectorElementHash();
        if (Idhash >= CscRElMaxHash) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" with hash Id"
                                  <<Idhash<<" exceeding the allowed boundaries 0-"<<CscRElMaxHash);
            throw std::runtime_error("Invalid hash assignment");
        }
        const int array_idx = cscIdentToArrayIdx(id);
        if (m_cscArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment");
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
        // add RE to array by RE hash
        const IdentifierHash Idhash = x->detectorElementHash();
        if (Idhash >= TgcRElMaxHash) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" with hash Id"
                                  <<Idhash<<" exceeding the allowed boundaries 0-"<<TgcRElMaxHash);
            throw std::runtime_error("Invalid hash assignment");
        }
        const int array_idx = tgcIdentToArrayIdx(id);
        if (m_tgcArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment");
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
    int MuonDetectorManager::mmIdenToArrayIdx(const Identifier& id) const {
        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        IdentifierHash hash{0};
        if (idHelper.get_detectorElement_hash(id,hash)) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
        return static_cast<int>(hash);
    }

    int MuonDetectorManager::stgcIdentToArrayIdx(const Identifier& id) const {
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        IdentifierHash hash{0};
        if (idHelper.get_detectorElement_hash(id,hash)) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
        return static_cast<int>(hash);
    }
    int MuonDetectorManager::rpcIdentToArrayIdx(const Identifier& id) const {
        int dbl_z{-1};
        return rpcIdentToArrayIdx(id, dbl_z);
    }
    int MuonDetectorManager::rpcIdentToArrayIdx(const Identifier& id, int& dbz_index) const {
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        const int stationName = idHelper.stationName(id);
        const int stationEta = idHelper.stationEta(id);
        const int doubletPhi = idHelper.doubletPhi(id);
        const int doubletZ = idHelper.doubletZ(id);
        const int doubletR = idHelper.doubletR(id);
        const int stname_index = rpcStationTypeIdx(stationName);
        const int steta_index = stationEta + NRpcStEtaOffset;
        const int stphi_index = idHelper.stationPhi(id) - 1;
        const int dbr_index = doubletR - 1;
        dbz_index = doubletZ - 1;

        // BMS 5/ |stEta|= 2 / dbR = 1 and 2 / dbZ = 3
        // BMS 6/ |stEta|= 4 / dbR = 2 / dbZ = 3
        // BMS 6/ |stEta|= 4 / dbR = 1 / dbZ = 2
        // these are the special cases where we want the rpc at doubletPhi = 2
        // to be addressed with a dbz_index=dbZ+1
        if (stname_index == RpcStatType::BMS) {
            if (std::abs(stationEta) == 2 && doubletZ == 3 && doubletPhi == 2)
                ++dbz_index;
            else if (std::abs(stationEta) == 4 && doubletR == 2 && doubletZ == 3 && doubletPhi == 2)
                ++dbz_index;
            else if (std::abs(stationEta) == 4 && doubletR == 1 && doubletZ == 2 && doubletPhi == 2)
                ++dbz_index;
        }
#ifndef NDEBUG
        if (stname_index < 0 || stname_index >= NRpcStatType) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" station name index is out of range "<<stname_index<<" allowed 0-"<<(NRpcStatType-1));
            throw std::runtime_error("Out of range station index index");
        }
        if (steta_index < 0 || steta_index >= NRpcStatEta) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" eta index is out of range "<<steta_index<<" allowed 0-"<<(NRpcStatEta-1));
            throw std::runtime_error("Out of range eta index");
        }
        if (stphi_index < 0 || stphi_index >= NRpcStatPhi) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" phi index is out of range "<<stphi_index<<" allowed 0-"<<(NRpcStatPhi-1));
            throw std::runtime_error("Out of range phi index");            
       }
        if (dbr_index < 0 || dbr_index >= NDoubletR) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" doublet R index is out of range "<<dbr_index<<" allowed 0-"<<(NDoubletR-1));
            throw std::runtime_error("Out of doublet R index");
        }
        if (dbz_index < 0 || dbz_index >= NDoubletZ) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" doublet Z index is out of range "<<dbz_index<<" allowed 0-"<<(NDoubletZ-1));
            throw std::runtime_error("Out of doublet Z index");
       }
#endif
        /// Unfold the array by
        /// [A][B][C][D][E]
        /// a * BxCxDxE + b * CxDxE + c*DxE +d*E +e
        constexpr int E = NDoubletZ;
        constexpr int DxE = NDoubletR * E;
        constexpr int CxDxE = NRpcStatPhi * DxE;
        constexpr int BxCxDxE = NRpcStatEta * CxDxE;
        const int arrayIdx = stname_index * BxCxDxE + steta_index * CxDxE + stphi_index * DxE + dbr_index * E + dbz_index;
        return arrayIdx;
    }
    int MuonDetectorManager::mdtIdentToArrayIdx(const Identifier& id) const {
       const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
       IdentifierHash hash{0};
       if (idHelper.get_detectorElement_hash(id,hash)) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
       }
       return static_cast<int>(hash);
    }
    int MuonDetectorManager::tgcIdentToArrayIdx(const Identifier& id) const {
        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        IdentifierHash hash{0};
        if (idHelper.get_detectorElement_hash(id,hash)) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
        return static_cast<int>(hash);
    }    
    int MuonDetectorManager::cscIdentToArrayIdx(const Identifier& id) const {
        const CscIdHelper& idHelper{m_idHelperSvc->cscIdHelper()};
        IdentifierHash hash{0};
        if (idHelper.get_detectorElement_hash(id,hash)) {
           ATH_MSG_WARNING("Failed to retrieve a proper hash for "<<m_idHelperSvc->toString(id));
           return -1;
        }
        return static_cast<int>(hash);
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

    const MdtReadoutElement* MuonDetectorManager::getMdtReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= MdtRElMaxHash) {           
            ATH_MSG_WARNING(" try to getMdtReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << MdtRElMaxHash - 1 );
            return nullptr;
        }
#endif
        return m_mdtArray[id].get();
    }

    const RpcReadoutElement* MuonDetectorManager::getRpcReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= RpcRElMaxHash) {           
            ATH_MSG_WARNING(" try to getRpcReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << RpcRElMaxHash - 1 );
            return nullptr;
        }
#endif
        return m_rpcArrayByHash[id];
    }

    const TgcReadoutElement* MuonDetectorManager::getTgcReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= TgcRElMaxHash) {           
            ATH_MSG_WARNING(" try to getTgcReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << TgcRElMaxHash - 1 );
            return nullptr;
        }
#endif
        return m_tgcArray[id].get();
    }

    const CscReadoutElement* MuonDetectorManager::getCscReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= CscRElMaxHash) {           
            ATH_MSG_WARNING(" try to getCscReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << CscRElMaxHash - 1 );
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
        
        if (m_idHelperSvc->hasMDT()) {
            const MdtIdHelper& mdtHelper{m_idHelperSvc->mdtIdHelper()};
            m_mdt_EIS_stName = mdtHelper.stationNameIndex("EIS");
            m_mdt_BIM_stName = mdtHelper.stationNameIndex("BIM");
            m_mdt_BME_stName = mdtHelper.stationNameIndex("BME");
            m_mdt_BMG_stName = mdtHelper.stationNameIndex("BMG");
        }
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
    void MuonDetectorManager::setGenericTgcDescriptor(const GenericTGCCache& tc) {
        m_genericTGC.frame_h = tc.frame_h;
        m_genericTGC.frame_ab = tc.frame_ab;
        m_genericTGC.nlayers = tc.nlayers;
        for (unsigned int i = 0; i < (tc.materials).size(); i++) {
            m_genericTGC.materials[i] = tc.materials[i];
            m_genericTGC.positions[i] = tc.positions[i];
            m_genericTGC.tck[i] = tc.tck[i];
        }
    }

}  // namespace MuonGM
