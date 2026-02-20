/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonIdHelpers/MuonIdHelperSvc.h"

#include <iostream>
#include <format>
#include <ranges>


namespace Muon {
    using namespace MuonStationIndex;

    MuonIdHelperSvc::MuonIdHelperSvc(const std::string& name, ISvcLocator* svc) :
        base_class(name, svc) {}

    StatusCode MuonIdHelperSvc::initialize() {
        ATH_CHECK(m_detStore.retrieve());
        if (m_hasMDT) ATH_CHECK(m_detStore->retrieve(m_mdtIdHelper));
        if (m_hasRPC) ATH_CHECK(m_detStore->retrieve(m_rpcIdHelper));
        if (m_hasTGC) ATH_CHECK(m_detStore->retrieve(m_tgcIdHelper));
        if (m_hasCSC) ATH_CHECK(m_detStore->retrieve(m_cscIdHelper));       
        if (m_hasSTGC) ATH_CHECK(m_detStore->retrieve(m_stgcIdHelper));
        if (m_hasMM) ATH_CHECK(m_detStore->retrieve(m_mmIdHelper));
       
        /// Find an id helper that is not a nullptr
        using AllHelperArray = std::array<const MuonIdHelper*, 6>; 
        const AllHelperArray allHelpers{m_mdtIdHelper, m_rpcIdHelper, m_tgcIdHelper,
                                         m_cscIdHelper, m_stgcIdHelper, m_mmIdHelper};
        AllHelperArray::const_iterator itr = std::ranges::find_if(allHelpers,
                                                          [](const MuonIdHelper* h){return h != nullptr;});
        if (itr == allHelpers.end()){
            ATH_MSG_WARNING("No MuonIdHelper has been created before. Please do not setup the service if no muon layout is loaded");
            return StatusCode::SUCCESS;
        }                
        m_primaryHelper = (*itr);

        std::stringstream techStr{};
        
        for (int tech = 0; tech <= m_primaryHelper->technologyNameIndexMax(); ++tech) {
            std::string name = m_primaryHelper->technologyString(tech);
            
            if (name == "MDT") m_technologies.push_back(TechIdx::MDT);
            if (name == "CSC") m_technologies.push_back(TechIdx::CSC);
            if (name == "RPC") m_technologies.push_back(TechIdx::RPC);
            if (name == "TGC") m_technologies.push_back(TechIdx::TGC);
            if (name == "STGC") m_technologies.push_back(TechIdx::STGC);
            if (name == "MM") m_technologies.push_back(TechIdx::MM);
            techStr<< ", " << tech << " " << name;
        }
         ATH_MSG_DEBUG(" Technologies: size " << m_primaryHelper->technologyNameIndexMax()<<" "<<techStr.str());
       
        unsigned int nstationsNames = m_primaryHelper->stationNameIndexMax() + 1;
        m_stationNameData.resize(nstationsNames);
        for (int i = 0; i <= m_primaryHelper->stationNameIndexMax(); ++i) {
            std::string name = m_primaryHelper->stationNameString(i);
            if (name.compare(MuonIdHelper::BAD_NAME) == 0) continue;

            StationNameData& data = m_stationNameData[i];

            data.stationName = std::move(name);
            data.isEndcap = m_primaryHelper->isEndcap(i);
            data.isSmall = m_primaryHelper->isSmall(i);

            if (data.isEndcap) {
                if (data.stationName[1] == '1')
                    data.chIndex = ChIdx::EML;
                else if (data.stationName[1] == '2')
                    data.chIndex = ChIdx::EML;
                else if (data.stationName[1] == '3')
                    data.chIndex = ChIdx::EML;
                else if (data.stationName[1] == '4')
                    data.chIndex = ChIdx::EIL;

                if (data.stationName[1] == 'O') {
                    if (data.stationName[2] == 'L')
                        data.chIndex = ChIdx::EOL;
                    else
                        data.chIndex = ChIdx::EOS;
                } else if (data.stationName[1] == 'M') {
                    if (data.stationName[2] == 'L')
                        data.chIndex = ChIdx::EML;
                    else
                        data.chIndex = ChIdx::EMS;
                } else if (data.stationName[1] == 'I') {
                    if (data.stationName[2] == 'L')
                        data.chIndex = ChIdx::EIL;
                    else
                        data.chIndex = ChIdx::EIS;
                } else if (data.stationName[1] == 'E') {
                    if (data.stationName[0] == 'B') {
                        data.chIndex = ChIdx::BEE;
                    } else {
                        if (data.stationName[2] == 'L')
                            data.chIndex = ChIdx::EEL;
                        else
                            data.chIndex = ChIdx::EES;
                    }
                } else if (data.stationName[0] == 'C') {
                    if (data.stationName[2] == 'L')
                        data.chIndex = ChIdx::CSL;
                    else
                        data.chIndex = ChIdx::CSS;
                }
                if (data.stationName[0] == 'S' || data.stationName[0] == 'M') {
                    if (data.isSmall)
                        data.chIndex = ChIdx::EIS;
                    else
                        data.chIndex = ChIdx::EIL;
                }

            } else {
                if (data.stationName[1] == 'O') {
                    if (data.stationName[2] == 'L')
                        data.chIndex = ChIdx::BOL;
                    else
                        data.chIndex = ChIdx::BOS;
                } else if (data.stationName[1] == 'M') {
                    if (data.stationName[2] == 'L' || data.stationName[2] == 'E')
                        data.chIndex = ChIdx::BML;
                    else
                        data.chIndex = ChIdx::BMS;
                } else if (data.stationName[1] == 'I') {
                    if (data.stationName[2] == 'L' || data.stationName[2] == 'M' || data.stationName[2] == 'R')
                        data.chIndex = ChIdx::BIL;
                    else
                        data.chIndex = ChIdx::BIS;
                }
            }
            if (data.chIndex == ChIdx::ChUnknown) {
               ATH_MSG_ERROR("data.chIndex is negative in MuonIdHelperSvc::initialize ");
               return StatusCode::FAILURE;
            }
            data.stIndex = toStationIndex(data.chIndex);

            if (msgLvl(MSG::DEBUG)) {
                msg(MSG::DEBUG) << "Adding station " << i << " " << data.stationName << " ";
                if (data.isEndcap)
                    msg(MSG::DEBUG) << " Endcap, ";
                else
                    msg(MSG::DEBUG) << " Barrel, ";
                if (data.isSmall)
                    msg(MSG::DEBUG) << " Small, ";
                else
                    msg(MSG::DEBUG) << " Large, ";

                msg(MSG::DEBUG) << chName(data.chIndex) << "  " << stName(data.stIndex) << endmsg;
            }
        }
        /// Cache the sMDT stations
        // now, let's check if we are in the inner barrel layer, and if there are RPCs installed
        // if yes, the MDT chambers must be sMDTs
        if (m_mdtIdHelper && m_rpcIdHelper) {
            m_BIS_stat = m_mdtIdHelper->stationNameIndex("BIS");
            for (int eta = MdtIdHelper::stationEtaMin(true); eta <= MdtIdHelper::stationEtaMax(true); ++eta) {
                for (int phi = 1; phi <= 8; ++phi) {
                    // now, let's check if we are in the inner barrel layer, and if there are RPCs installed
                    // if yes, the MDT chambers must be sMDTs
                    // now try to retrieve RPC identifier with the same station name/eta/phi and check if it is valid
                    bool isValid = false;
                    m_rpcIdHelper->elementID(m_BIS_stat, eta, phi, 1, isValid);
                    // last 4 arguments are: doubletR, check, isValid
                    // there is a BI RPC in the same station, thus, this station was already upgraded and sMDTs are present
                    if (!isValid) continue;
                    m_smdt_stat.emplace(m_mdtIdHelper->elementID(m_BIS_stat, eta, phi));
                }
            }
        }

        std::ranges::for_each(allHelpers, [this](const MuonIdHelper* idHelper){
            if (!idHelper) return;
            const TechIdx techIdx = technologyIndex(*idHelper->module_begin());
            for (auto itr = idHelper->module_begin(); itr != idHelper->module_end(); ++itr) {
                const auto idx = toStationIndex(chamberIndex(*itr));
                if (idx == StIndex::StUnknown) continue;
                m_techPerStation[toInt(idx)].insert(techIdx);
            }
        }); 

        ATH_MSG_DEBUG("Configured the service with the following flags --- hasMDT: "<< hasMDT()<<" hasRPC: "<<hasRPC()
                      <<" hasTGC"<< hasTGC() << " hasCSC: "<< hasCSC() << " hasSTGC: " << hasSTGC() << " hasMM: " << hasMM() );
        return StatusCode::SUCCESS;
    }

    int MuonIdHelperSvc::gasGap(const Identifier& id) const {
        switch (technologyIndex(id)) {
            using enum TechnologyIndex;
            case RPC: {
                return m_rpcIdHelper->gasGap(id);
            } case TGC: {
               return m_tgcIdHelper->gasGap(id);
            } case CSC: {
                return m_cscIdHelper->wireLayer(id);
            } case STGC: {
                return m_stgcIdHelper->gasGap(id);
            } case MM: {
                return m_mmIdHelper->gasGap(id);
            } case MDT: {
                return m_mdtIdHelper->channel(id);
            } default:
                break;
        }
        return 0;
    }

    bool MuonIdHelperSvc::isMuon(const Identifier& id) const { 
        return m_primaryHelper && m_primaryHelper->is_muon(id); 
    }
    bool MuonIdHelperSvc::isMdt(const Identifier& id) const {
        return m_mdtIdHelper && m_mdtIdHelper->is_mdt(id);
    }
    bool MuonIdHelperSvc::isMM(const Identifier& id) const {
        return m_mmIdHelper && m_mmIdHelper->is_mm(id);
    }
    bool MuonIdHelperSvc::isCsc(const Identifier& id) const {
        return m_cscIdHelper && m_cscIdHelper->is_csc(id);
    }
    bool MuonIdHelperSvc::isRpc(const Identifier& id) const {
        return m_rpcIdHelper && m_rpcIdHelper->is_rpc(id);
    }

    bool MuonIdHelperSvc::isTgc(const Identifier& id) const {
        return m_tgcIdHelper && m_tgcIdHelper->is_tgc(id);
    }

    bool MuonIdHelperSvc::issTgc(const Identifier& id) const {
        return m_stgcIdHelper && m_stgcIdHelper->is_stgc(id);
    }

    const std::set<TechnologyIndex>& 
        MuonIdHelperSvc::technologiesInStation(StIndex stIndex) const {
        assert(toInt(stIndex) < static_cast<int>(m_techPerStation.size()));
        return m_techPerStation[toInt(stIndex)];
    }
    bool MuonIdHelperSvc::issMdt(const Identifier& id) const {
        if (!isMdt(id)) {
            return false;
        }
        if (stationName(id) == m_BIS_stat) {
            return m_smdt_stat.find(m_mdtIdHelper->elementID(id)) != m_smdt_stat.end();
        }
        return m_mdtIdHelper->isBME(id) || m_mdtIdHelper->isBMG(id);
    }

    bool MuonIdHelperSvc::hasHPTDC(const Identifier& id) const {
        /** NOTE that in Run4, no HPTDCs at all are planned to be present any more,
            so this function should be obsolete from Run4 onwards */
        // the remaining sMDTs (installed in BI in LS1) all have an HPTDC in Run3
        // all BME sMDTs have no HPTDC
        return issMdt(id) && !m_mdtIdHelper->isBME(id);
    }

    bool MuonIdHelperSvc::isTrigger(const Identifier& id) const {
        return isRpc(id) || isTgc(id);
    }

    bool MuonIdHelperSvc::isEndcap(const Identifier& id) const { return m_primaryHelper->isEndcap(id); }
    bool MuonIdHelperSvc::isSmallChamber(const Identifier& id) const { return m_primaryHelper->isSmall(id); }

    ChIndex MuonIdHelperSvc::chamberIndex(const Identifier& id) const {
        if (!id.is_valid() || !isMuon(id)) {
            if (id.is_valid()) ATH_MSG_WARNING("chamberIndex: invalid ID " << m_primaryHelper->print_to_string(id));
            return ChIdx::ChUnknown;
        }
        return m_stationNameData[stationName(id)].chIndex;
    }

    StIndex MuonIdHelperSvc::stationIndex(const Identifier& id) const {
        if (!id.is_valid() || !isMuon(id)) {
            if (id.is_valid()) ATH_MSG_WARNING("stationIndex: invalid ID " << m_primaryHelper->print_to_string(id));
            return StIdx::StUnknown;
        }
        return m_stationNameData[stationName(id)].stIndex;
    }

    PhiIndex MuonIdHelperSvc::phiIndex(const Identifier& id) const {
#ifndef NDEBUG
        if (!id.is_valid() || !isMuon(id)) {
            if (id.is_valid()) ATH_MSG_WARNING("phiIndex: invalid ID " << m_primaryHelper->print_to_string(id));
            return PhiIdx::PhiUnknown;
        }
        if (isMdt(id) || isMM(id)) {
            ATH_MSG_WARNING("phiIndex: not supported for " << toString(id));
            return PhiIdx::PhiUnknown;
        }
#endif
        switch (stationIndex(id)) {
            case StIndex::BI:{
                return m_rpcIdHelper->doubletR(id) == 1 ?  PhiIdx::BI1 : PhiIdx::BI2;
            } case StIndex::BM: {
                return m_rpcIdHelper->doubletR(id) == 1 ? PhiIdx::BM1 : PhiIdx::BM2;
            } case StIndex::BO: {
                return m_rpcIdHelper->doubletR(id) == 1 ? PhiIdx::BO1 : PhiIdx::BO2;
            } case StIndex::EI: {
                switch (technologyIndex(id)) {
                    case TechIdx::CSC:
                        return PhiIdx::CSC;
                    case TechIdx::TGC:
                        return PhiIdx::T4;
                    case TechIdx::STGC:
                        return m_stgcIdHelper->multilayer(id) == 1 ? PhiIdx::STGC1 : PhiIdx::STGC2;
                    default:
                        break;
                }
                break;
            } case StIndex::EM: {
                const std::string chamberName = stationNameString(id);
                if (chamberName[1] == '1') {
                    return PhiIdx::T1;
                } else if (chamberName[1] == '2') {
                    return PhiIdx::T2;
                } else {
                    return PhiIdx::T3;
                }
            } default:
                break;
        }
        return PhiIdx::PhiUnknown;
    }

    DetectorRegionIndex MuonIdHelperSvc::regionIndex(const Identifier& id) const {
        if (isEndcap(id)) return stationEta(id) < 0 ? DetectorRegionIndex::EndcapC : DetectorRegionIndex::EndcapA;
        return DetectorRegionIndex::Barrel;
    }

    LayerIndex MuonIdHelperSvc::layerIndex(const Identifier& id) const {
        return toLayerIndex(stationIndex(id));
    }

    TechnologyIndex MuonIdHelperSvc::technologyIndex(const Identifier& id) const {
        if (isMdt(id)) return TechIdx::MDT;   
        else if (issTgc(id)) return TechIdx::STGC;
        else if (isMM(id)) return TechIdx::MM;     
        else if (isTgc(id)) return TechIdx::TGC;
        else if (isRpc(id)) return TechIdx::RPC;
        else if (isCsc(id)) return TechIdx::CSC;
        return TechIdx::TechnologyUnknown;
    }
    std::string MuonIdHelperSvc::toString(const Identifier& id) const {
        if (!id.is_valid()) return " Invalid Identifier";
        switch (technologyIndex(id)) {
            using enum TechnologyIndex;
            case MDT:{
                return toStringGasGap(id);
            } case RPC: {
                return std::format("{:} {:} channel {:2d}", toStringGasGap(id), 
                                   measuresPhi(id) ? "phi" : "eta", m_rpcIdHelper->channel(id));
            } case TGC: {
                return std::format("{:} {:} channel {:2d}", toStringGasGap(id), 
                                   measuresPhi(id) ? "phi" : "eta",  m_tgcIdHelper->channel(id));
            } case CSC: {
                return std::format("{:} {:} channel {:2d}", toStringGasGap(id), 
                                   measuresPhi(id) ? "phi" : "eta",  m_cscIdHelper->channel(id));
            } case STGC: {
                using sTgcType = sTgcIdHelper::sTgcChannelTypes;
                const int channelType = m_stgcIdHelper->channelType(id);
                return std::format("{:} {:} channel {:3d}", toStringGasGap(id), 
                                    channelType == sTgcType::Strip ? "eta" : channelType == sTgcType::Wire ? "phi" : "pad" ,
                                    m_stgcIdHelper->channel(id));
            } case MM: {
                return std::format("{:} channel {:4d}",toStringGasGap(id),  m_mmIdHelper->channel(id));
            } default:
                break;
        }
        return {};
    }

    std::string MuonIdHelperSvc::toStringTech(const Identifier& id) const {
        return  technologyName(technologyIndex(id));
    }

    std::string MuonIdHelperSvc::chamberNameString(const Identifier& id) const {
        return m_primaryHelper->stationNameString(stationName(id));
    }

    std::string MuonIdHelperSvc::toStringStation(const Identifier& id) const {
        return std::format("{:} {:} eta {:2d} phi {:2d}", toStringTech(id),stationNameString(id),
                                                          stationEta(id), stationPhi(id));
    }
    std::string MuonIdHelperSvc::toStringChamber(const Identifier& id) const {
        if (!id.is_valid()) {
            return " Invalid Identifier";
        }
        if (isRpc(id)) {
            return std::format("{:} dbR {:1}", toStringStation(id), m_rpcIdHelper->doubletR(id));
        }
        return toStringStation(id);
    }

    std::string MuonIdHelperSvc::toStringDetEl(const Identifier& id) const {
        switch (technologyIndex(id)) {
            using enum TechnologyIndex;
            case MDT: {
                return std::format("{:} ml {:1d}", toStringChamber(id), m_mdtIdHelper->multilayer(id));
            } case RPC: {
                return std::format("{:} dbZ {:1d} dbPhi {:1d}", toStringChamber(id), m_rpcIdHelper->doubletZ(id), m_rpcIdHelper->doubletPhi(id));
            } case TGC: {
                return toStringChamber(id);
            } case CSC: {
                return std::format("{:} chlay {:1d}",toStringChamber(id), m_cscIdHelper->chamberLayer(id));
            } case STGC: {
                return std::format("{:} chlay {:1d}",toStringChamber(id), m_stgcIdHelper->multilayer(id));
            } case MM: {
                return std::format("{:} chlay {:1d}",toStringChamber(id), m_mmIdHelper->multilayer(id));
            } default:
                break;
        }
        return "Invalid detector element";
    }

    std::string MuonIdHelperSvc::toStringGasGap(const Identifier& id) const {
        switch (technologyIndex(id)) {
            using enum TechnologyIndex;
            case MDT: {
                return std::format("{:} lay {:1d} tube {:1d}", toStringDetEl(id),
                                   m_mdtIdHelper->tubeLayer(id), m_mdtIdHelper->tube(id));
            } case RPC:
              case TGC: {
                return std::format("{:} gap {:1d}", toStringDetEl(id), gasGap(id)); 
            } case STGC:
              case MM:
              case CSC: {
                return std::format("{:} lay {:1d}", toStringDetEl(id), gasGap(id));
            } default:
                break;
        }
        return "invalid gas gap Id";
    }

    Identifier MuonIdHelperSvc::chamberId(const Identifier& id) const {
        // use phi hits on segment
        switch (technologyIndex(id)) {
            using enum TechnologyIndex;
            case MDT: {
                return m_mdtIdHelper->elementID(id);
            } case RPC: {
                return m_rpcIdHelper->elementID(id);
            } case TGC: {
                return m_tgcIdHelper->elementID(id);
            } case CSC: {
                return m_cscIdHelper->elementID(id);
            } case STGC: {
                return m_stgcIdHelper->elementID(id);
            } case MM: {
                return m_mmIdHelper->elementID(id);
            } default:
                break;
        }
        return Identifier{};
    }

    Identifier MuonIdHelperSvc::detElId(const Identifier& id) const {
        // use phi hits on segment
        switch (technologyIndex(id)) {
            using enum MuonStationIndex::TechnologyIndex;
            case MDT: {
                return m_mdtIdHelper->channelID(id, m_mdtIdHelper->multilayer(id), 1, 1);;
            } case RPC: {
                return m_rpcIdHelper->channelID(id, m_rpcIdHelper->doubletZ(id), m_rpcIdHelper->doubletPhi(id), 1, false, 1);
            } case TGC: {
                return m_tgcIdHelper->elementID(id);   
            } case CSC: {
                return m_cscIdHelper->channelID(id, 2, 1, 1, 1);
            } case MM: {
                return m_mmIdHelper->channelID(id, m_mmIdHelper->multilayer(id), 1, 1);   
            } case STGC: {
                return m_stgcIdHelper->channelID(id, m_stgcIdHelper->multilayer(id), 1, sTgcIdHelper::sTgcChannelTypes::Strip, 1);
            } default:
                 break;
        }
        return id;
    }

    Identifier MuonIdHelperSvc::layerId(const Identifier& id) const {
        switch (technologyIndex(id)) {
            using enum TechnologyIndex;
            case TGC: {
                return m_tgcIdHelper->channelID(id, m_tgcIdHelper->gasGap(id), m_tgcIdHelper->measuresPhi(id), 1);
            } case RPC: {
                return m_rpcIdHelper->channelID(id, m_rpcIdHelper->doubletZ(id), m_rpcIdHelper->doubletPhi(id), 
                                                m_rpcIdHelper->gasGap(id), m_rpcIdHelper->measuresPhi(id), 1); 
            } case CSC: {
                return m_cscIdHelper->channelID(id, m_cscIdHelper->chamberLayer(id), m_cscIdHelper->wireLayer(id), 
                                                m_cscIdHelper->measuresPhi(id), 1);
            } case MM: {
                return m_mmIdHelper->channelID(id, m_mmIdHelper->multilayer(id), m_mmIdHelper->gasGap(id), 1);
            } case STGC: {
                return m_stgcIdHelper->channelID(id, m_stgcIdHelper->multilayer(id), 
                                                 m_stgcIdHelper->gasGap(id), m_stgcIdHelper->channelType(id), 1);
            } default:
                break;
        }
        /// Return Identifier for the Mdts 
        return id;
    }

    Identifier MuonIdHelperSvc::gasGapId(const Identifier& id) const {
        switch (technologyIndex(id)) {
            using enum TechnologyIndex;
            case RPC: {
                return m_rpcIdHelper->channelID(id, m_rpcIdHelper->doubletZ(id), m_rpcIdHelper->doubletPhi(id), 
                                                m_rpcIdHelper->gasGap(id), false, 1);
            } case TGC: {
               return m_tgcIdHelper->channelID(id, m_tgcIdHelper->gasGap(id), false, 1); 
            } case CSC: {
                return m_cscIdHelper->channelID(id, m_cscIdHelper->chamberLayer(id), m_cscIdHelper->wireLayer(id), 1, 1);
            } case STGC: {
                return m_stgcIdHelper->channelID(id, m_stgcIdHelper->multilayer(id), m_stgcIdHelper->gasGap(id), 
                                                 sTgcIdHelper::sTgcChannelTypes::Strip, 1);
            } case MM: {
                return m_mmIdHelper->channelID(id, m_mmIdHelper->multilayer(id), m_mmIdHelper->gasGap(id), 1);
            } default: 
                break;
        }
        return id;
    }
    int MuonIdHelperSvc::sector(const Identifier& id) const {
        // TGC has different segmentation, return 0 for the moment
        if (isTgc(id)) {
            auto initTgcSectorMapping = [&]() -> std::vector<int>* {
                std::vector<int>* mapping = nullptr;
                StatusCode sc = m_detStore->retrieve(mapping, "TGC_SectorMapping");
                if (sc.isFailure() || !mapping) {
                    ATH_MSG_WARNING("sector: failed to retrieve TGC sector mapping");
                    return nullptr;
                }
                ATH_MSG_DEBUG("sector: retrieve TGC sector mapping " << mapping->size());
                return mapping;
            };
            static const std::vector<int> tgcSectorMapping = *initTgcSectorMapping();

            IdentifierHash hash;
            m_tgcIdHelper->get_module_hash(id, hash);
            if (hash >= tgcSectorMapping.size()) {
                ATH_MSG_WARNING("sector: TGC not yet supported");
                return 0;
            }
            return tgcSectorMapping[hash];
        }
        int sect = 2 * stationPhi(id);
        if (!isSmallChamber(id)) --sect;
        return sect;
    }

    bool MuonIdHelperSvc::hasRPC() const { return m_rpcIdHelper != nullptr; }
    bool MuonIdHelperSvc::hasTGC() const { return m_tgcIdHelper != nullptr; }
    bool MuonIdHelperSvc::hasMDT() const { return m_mdtIdHelper != nullptr; }
    bool MuonIdHelperSvc::hasCSC() const { return m_hasCSC; }
    bool MuonIdHelperSvc::hasSTGC() const { return m_hasSTGC; }
    bool MuonIdHelperSvc::hasMM() const { return m_hasMM; }
    

    inline IdentifierHash MuonIdHelperSvc::moduleHash(const MuonIdHelper& idHelper, const Identifier& id) const{
        IdentifierHash hash{};
        if (idHelper.get_module_hash(id, hash) ||
            static_cast<unsigned int>(hash) >= idHelper.module_hash_max()){
            ATH_MSG_VERBOSE("Failed to deduce module hash "<<toString(id));            
        }
        return hash;
    }
    inline IdentifierHash MuonIdHelperSvc::detElementHash(const MuonIdHelper& idHelper, const Identifier& id) const{
        IdentifierHash hash{};
        if (idHelper.get_detectorElement_hash(id, hash) ||
            static_cast<unsigned int>(hash)>= idHelper.detectorElement_hash_max()) {
            ATH_MSG_VERBOSE("Failed to deduce detector element hash "<<toString(id));
        }
        return hash;
    }
    IdentifierHash MuonIdHelperSvc::moduleHash(const Identifier& id) const {
        switch (technologyIndex(id)){
            using enum MuonStationIndex::TechnologyIndex;
            case MDT: {
                return moduleHash(*m_mdtIdHelper, id);
            }
            case RPC: {
                return moduleHash(*m_rpcIdHelper, id);
            }
            case TGC: {
                return moduleHash(*m_tgcIdHelper, id);
            }
            case CSC: {
                return moduleHash(*m_cscIdHelper, id);
            }
            case STGC: {
                return moduleHash(*m_stgcIdHelper, id);
            }
            case MM: {
                return moduleHash(*m_mmIdHelper, id);
            }
            default:
                break;
        }

        ATH_MSG_WARNING("moduleHash(): No muon Identifier "<<id);
        return IdentifierHash{};
    }
    IdentifierHash MuonIdHelperSvc::detElementHash(const Identifier& id) const {
       switch (technologyIndex(id)){
            using enum MuonStationIndex::TechnologyIndex;
            case MDT: {
                return detElementHash(*m_mdtIdHelper, id);
            }
            case RPC: {
                return detElementHash(*m_rpcIdHelper, id);
            }
            case TGC: {
                return detElementHash(*m_tgcIdHelper, id);
            }
            case CSC: {
                return detElementHash(*m_cscIdHelper, id);
            }
            case STGC: {
                return detElementHash(*m_stgcIdHelper, id);
            }
            case MM: {
                return detElementHash(*m_mmIdHelper, id);
            }
            default:
                break;
        }
        ATH_MSG_WARNING("detElementHash(): No muon Identifier "<<id);
        return IdentifierHash{};
    }
    #define IMPL_FIELDGETTER(fieldName, retType) \
    retType MuonIdHelperSvc::fieldName(const  Identifier& id) const { \
        switch (technologyIndex(id)) {                                \
            using enum TechnologyIndex;                               \
            case MDT: {                                               \
                return m_mdtIdHelper->fieldName(id);                  \
            } case RPC: {                                             \
                return m_rpcIdHelper->fieldName(id);                  \
            } case TGC: {                                             \
                return m_tgcIdHelper->fieldName(id);                  \
            } case CSC: {                                             \
                return m_cscIdHelper->fieldName(id);                  \
            } case STGC: {                                            \
                return m_stgcIdHelper->fieldName(id);                 \
            } case MM: {                                              \
                return m_mmIdHelper->fieldName(id);                   \
            } default:                                                \
                break;                                                \
        }                                                             \
        return {};                                                    \
    } 
    IMPL_FIELDGETTER(stationPhi, int);
    IMPL_FIELDGETTER(stationEta, int);
    IMPL_FIELDGETTER(stationName, int);
    IMPL_FIELDGETTER(stationRegion, int);
    IMPL_FIELDGETTER(measuresPhi, bool);
    IMPL_FIELDGETTER(stationNameString, std::string);
    #undef IMPL_FIELDGETTER
}  // namespace Muon
