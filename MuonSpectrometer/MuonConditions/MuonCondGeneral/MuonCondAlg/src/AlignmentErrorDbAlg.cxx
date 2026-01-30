/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AlignmentErrorDbAlg.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include <GaudiKernel/EventIDRange.h>
#include <format>
#include <fstream>
#include <iterator>

namespace Muon{
StatusCode AlignmentErrorDbAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_idTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode AlignmentErrorDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());

    // Write Cond Handle

    SG::WriteCondHandle writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    auto writeCdo{std::make_unique<MuonAlignmentErrorData>()};

    std::string clobContent;
    EventIDRange rangeW;
    std::tie(clobContent, rangeW) = m_clobFileOverride.value().empty() ? getDbClobContent(ctx) : getFileClobContent();

    if (clobContent.empty()) {
        ATH_MSG_ERROR("Cannot retrieve alignment error CLOB");
        return StatusCode::FAILURE;
    }

    std::istringstream indata(clobContent);
    if (!indata) {
        ATH_MSG_ERROR("Alignment error configuration invalid");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("***********************************");
    ATH_MSG_DEBUG("PARSING LIST OF DEVIATIONS...");

    std::string line;
    std::vector<MuonAlignmentErrorData::MuonAlignmentErrorRule> MuonAlignmentErrorRuleVec;
    MuonAlignmentErrorData::MuonAlignmentErrorRule aDev;
    while (getline(indata, line)) {
        // READING COMMENTS
        if (line.compare(0, 1,"#") == 0) {
            // ATH_MSG_DEBUG("Reading a commented line saying " << line);
            continue;
        }

        // READING FROM INPUT FILE:                                //
        std::string flag("");
        std::string name_sstring("");
        std::string multilayer_sstring("");
        double translation(0.);
        double rotation(0.);

        // GET INPUT FILE VERSION
        if (line.compare(0, 7, "version") == 0) {
            std::string clobVersion;
            std::istringstream(line) >> flag >> clobVersion;
            ATH_MSG_INFO("*****************************************");
            ATH_MSG_INFO("Input file version " << clobVersion);
            ATH_MSG_INFO("*****************************************");
            writeCdo->setClobVersion(std::move(clobVersion));
            continue;
        }

        // get the flag has_nsw_hits
        if (line.compare(0, 12, "has_nsw_hits") == 0) {
            bool hasNswHits{false};
            std::istringstream(line) >> flag >> hasNswHits;
            writeCdo->setHasNswHits(hasNswHits);
            continue;
        }

        // A FLAG CHARACTERIZING THE TYPE OF ERROR
        // A REGULAR EXPRESSION FOR THE STATION NAME (EX: BIL.*)   //
        // TWO DOUBLES FOR THE TRANSLATION AND ROTATION DEVIATIONS //
        if (std::istringstream(line) >> flag >> name_sstring >> multilayer_sstring >> translation >> rotation) {
            ATH_MSG_DEBUG(flag << " " << name_sstring << " " << multilayer_sstring << " " << translation << " " << rotation);

            // SAVING THE STATION DEVIATIONS IN THE STRUCT //
            aDev.stationName = name_sstring;
            aDev.multilayer = multilayer_sstring;
            aDev.translation = translation;
            aDev.rotation = rotation;

            // FILLING THE VECTOR OF STRUCT CONTAINING THE STATION DEVIATIONS //
            MuonAlignmentErrorRuleVec.emplace_back(std::move(aDev));

        }  // check stream is not at the end

    }  // end of loop on input file lines

    std::vector<MuonAlignmentErrorData::MuonAlignmentErrorRuleCache> MuonAlignmentErrorRuleCacheVec;
    MuonAlignmentErrorData::MuonAlignmentErrorRuleCache aDev_cache;

    // Check the presence of detector elements in current reco and generate multimap to deviationVec
    if (writeCdo->hasNswHits() == 1){
        ATH_MSG_DEBUG("CLOB HAS NSW errors");
        if(m_idHelperSvc->hasSTGC()){
            ATH_MSG_DEBUG("HAS STGC");
            generateMap(m_idHelperSvc->stgcIdHelper(), m_idTool, aDev_cache, MuonAlignmentErrorRuleVec);
        }
        if(m_idHelperSvc->hasMM()){
            ATH_MSG_DEBUG("HAS MM");
            generateMap(m_idHelperSvc->mmIdHelper(), m_idTool, aDev_cache, MuonAlignmentErrorRuleVec);
        }
    }
    if (m_idHelperSvc->hasMDT()){
        ATH_MSG_DEBUG("HAS MDT");
        generateMap(m_idHelperSvc->mdtIdHelper(), m_idTool, aDev_cache, MuonAlignmentErrorRuleVec);
    }
    if (m_idHelperSvc->hasRPC()){
        ATH_MSG_DEBUG("HAS RPC");
        generateMap(m_idHelperSvc->rpcIdHelper(), m_idTool, aDev_cache, MuonAlignmentErrorRuleVec);
    }
    if (m_idHelperSvc->hasTGC()){
        ATH_MSG_DEBUG("HAS TGC");
        generateMap(m_idHelperSvc->tgcIdHelper(), m_idTool, aDev_cache, MuonAlignmentErrorRuleVec);
    }
    if (m_idHelperSvc->hasCSC()){
        ATH_MSG_DEBUG("HAS CSC");
        generateMap(m_idHelperSvc->cscIdHelper(), m_idTool, aDev_cache, MuonAlignmentErrorRuleVec);
    }

    MuonAlignmentErrorRuleCacheVec.emplace_back(std::move(aDev_cache));
    if (MuonAlignmentErrorRuleVec.empty()) {
        ATH_MSG_WARNING("Could not read any alignment error configuration");
        return StatusCode::FAILURE;
    }
    
    writeCdo->setAlignmentErrorRules(std::move(MuonAlignmentErrorRuleVec));
    writeCdo->setMuonAlignmentErrorRuleCache(std::move(MuonAlignmentErrorRuleCacheVec));

    if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record MuonAlignmentErrorData " << writeHandle.key() << " with EventRange " << rangeW
                                                                 << " into Conditions Store");
        return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

    return StatusCode::SUCCESS;
}

inline void AlignmentErrorDbAlg::generateMap(const auto & helper_obj, const auto & idTool, 
                                MuonAlignmentErrorData::MuonAlignmentErrorRuleCache& adev_new, 
                                std::vector<MuonAlignmentErrorData::MuonAlignmentErrorRule>& devVec) const {

    for(auto itr = helper_obj.detectorElement_begin(); itr!= helper_obj.detectorElement_end(); ++itr){
        // GATHERING INFORMATION TO PUT TOGETHER THE STATION NAME //
        MuonCalib::MuonFixedLongId calibId = idTool->idToFixedLongId(*itr);
        if (!calibId.isValid()) continue;
        int multilayer = 1;
        if (calibId.is_mdt()) {
            multilayer = calibId.mdtMultilayer();
        } else if (calibId.is_mmg()) {
            multilayer = calibId.mmgMultilayer();
        } else if (calibId.is_stg()) {
            multilayer = calibId.stgMultilayer();
        }
        std::string multilayerName = std::to_string(multilayer);
        std::string alignStationName = hardwareName(calibId);
        MuonAlignmentErrorData::MuonAlignmentErrorRuleIndex i = 0;
        // try to regexp-match the station name and the multilayer name then fill multimap
        for (auto& iDev : devVec){
            if (!boost::regex_match(alignStationName, iDev.stationName)) {
                ++i;
                continue;
            }
            if (!boost::regex_match(multilayerName, iDev.multilayer)) {
                ++i;
                continue;
            }
            adev_new.id_rule_map.insert({*itr, i});
            ++i;
        }
    }
}

std::tuple<std::string, EventIDRange> AlignmentErrorDbAlg::getDbClobContent(const EventContext& ctx) const {
    // Read Cond Handle
    SG::ReadCondHandle readCdo{m_readKey, ctx};
    if (!readCdo.isValid()) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return {};
    }

    EventIDRange rangeW;
    if (!readCdo.range(rangeW)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << m_readKey.key());
        return {};
    }

    ATH_MSG_INFO("Size of CondAttrListCollection " << m_readKey.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_INFO("Range of input is " << rangeW);

    // like MuonAlignmentErrorDbTool::loadAlignmentError() after retrieving atrc (readCdo)

    std::string clobContent, dbClobVersion;
    CondAttrListCollection::const_iterator itr;
    for (itr = readCdo->begin(); itr != readCdo->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;
        clobContent = *(static_cast<const std::string*>((atr["syserrors"]).addressOfData()));
        dbClobVersion = *(static_cast<const std::string*>((atr["version"]).addressOfData()));
    }
    return std::make_tuple(std::move(clobContent), rangeW);
}

std::tuple<std::string, EventIDRange> AlignmentErrorDbAlg::getFileClobContent() const {
    ATH_MSG_INFO("Retrieving alignment error CLOB from file override " << m_clobFileOverride.value());

    std::ifstream in(m_clobFileOverride.value());
    if (!in) {
        ATH_MSG_ERROR("Failed to read CLOB file " << m_clobFileOverride.value());
        return std::make_tuple(std::string(), EventIDRange());
    }
    return std::make_tuple(std::string(std::istreambuf_iterator<char>(in), {}), IOVInfiniteRange::infiniteTime());
}
////////////////////////////
// RECOGNIZE STATION NAME //
////////////////////////////

inline std::string AlignmentErrorDbAlg::hardwareName(MuonCalib::MuonFixedLongId calibId) const {
    using StationName = MuonCalib::MuonFixedLongId::StationName;

    // The only exception that cannot be caught by hardwareEta() above
    if (sector(calibId)==13) {
        if (calibId.eta()== 7 && calibId.stationName()==StationName::BOL) return "BOE1A13"; // BOE1A13 not BOL7A13
        if (calibId.eta()==-7 && calibId.stationName()==StationName::BOL) return "BOE1C13"; // BOE1C13 not BOL7C13
        if (calibId.eta()== 8 && calibId.stationName()==StationName::BOL) return "BOE2A13"; // BOE2A13 not BOL8A13
        if (calibId.eta()==-8 && calibId.stationName()==StationName::BOL) return "BOE2C13"; // BOE2C13 not BOL8C13
    }

    std::string ret = std::format("{}{}{}{}", calibId.stationNameString(), std::abs(hardwareEta(calibId)), side(calibId), sectorString(calibId) );
    return ret;
}

inline std::string_view AlignmentErrorDbAlg::side(MuonCalib::MuonFixedLongId calibId) const {
    return calibId.eta()>0 ? "A" : calibId.eta()<0 ? "C" : "B";
}

inline std::string AlignmentErrorDbAlg::sectorString(MuonCalib::MuonFixedLongId calibId) const {
    int sec = sector(calibId);
    if (sec<0 || sec > 99) {
        throw std::runtime_error("Unhandled sector number");
    }
    return std::to_string(sec/10) + std::to_string(sec%10);
}

inline int AlignmentErrorDbAlg::sector(MuonCalib::MuonFixedLongId calibId) const {
    if (calibId.is_tgc()) {
        // TGC sector convention is special
        return calibId.phi();
    } else {
        return isSmallSector(calibId) ? 2*calibId.phi() : 2*calibId.phi()-1;
    }
}

inline bool AlignmentErrorDbAlg::isSmallSector(MuonCalib::MuonFixedLongId calibId) const {
    using StationName = MuonCalib::MuonFixedLongId::StationName;
    switch (calibId.stationName()) {
        case StationName::BIS:
        case StationName::BMS:
        case StationName::BOS:
        case StationName::BEE:
        case StationName::BMF:
        case StationName::BOF:
        case StationName::BOG:
        case StationName::EES:
        case StationName::EMS:
        case StationName::EOS:
        case StationName::EIS:
        case StationName::CSS:
        case StationName::BMG:
        case StationName::MMS:
        case StationName::STS:
            return true;
        default:
            return false;
    }
}

inline int AlignmentErrorDbAlg::hardwareEta(MuonCalib::MuonFixedLongId calibId) const {
    using StationName = MuonCalib::MuonFixedLongId::StationName;
    switch (calibId.stationName()) {
        case StationName::BML:
            {
                if (sector(calibId)==13) {
                    switch (calibId.eta()) {
                        case 4: return 5;
                        case 5: return 6;
                        case 6: return 7;
                        case -4: return -5;
                        case -5: return -6;
                        case -6: return -7;
                    }
                }
                return calibId.eta();
            }
        case StationName::BOL:
            {
                // In sector 13, the BOE chambers are coded as BOL, with eta=7
                if (sector(calibId)==13) {
                    if (calibId.eta()== 7) return 1; // BOE1A13 not BOL7A13
                    if (calibId.eta()==-7) return -1; // BOE1C13 not BOL7C13
                }
                return calibId.eta();
            }
        case StationName::BOF:
            // In sectors 12 and 14 BOF1, BOF3, BOF5 are coded with eta=1,2,3
            return calibId.eta()>0 ? calibId.eta()*2-1 : calibId.eta()*2+1;
        case StationName::BOG:
            // In sectors 12 and 14, BOG2, BOG4, BOG6 are coded with eta=1,2,3
            return calibId.eta()*2;
        case StationName::EIL:
            {
                if ((sector(calibId) == 1) || (sector(calibId) == 9)) {
                    switch (calibId.eta()) {
                        // In sectors 1 and 9 the small chamber is called EIL5, although it comes inward wrt EIL4, so athena indices need to be swapped
                        case 4: return 5;
                        case 5: return 4;
                        case -4: return -5;
                        case -5: return -4;
                    }
                }
                return calibId.eta();
            }
        case StationName::EEL:
            {
                // In sector 5, special EEL chamber called EEL2X05, while athena eta = 1
                if ((sector(calibId) == 5) && (calibId.eta() == 1)) return 2;
                if ((sector(calibId) == 5) && (calibId.eta() == -1)) return -2;
                return calibId.eta();
            }
        default: return calibId.eta();
    }
}
}