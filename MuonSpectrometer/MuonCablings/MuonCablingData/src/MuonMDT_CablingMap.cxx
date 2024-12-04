/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCablingData/MuonMDT_CablingMap.h"

#include <cmath>

#include "GaudiKernel/ISvcLocator.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "CxxUtils/ArrayHelper.h"
#include "StoreGate/StoreGateSvc.h"
#include "Identifier/Identifier.h"
#include "GeoModelKernel/throwExcept.h"

namespace {
/// Four mezzanine channels explicitly break the cabling schema in the legacy
/// database layout structure. The four channels are associated with BIS78 & BMG
/// hedgehog cards.
static const std::set<uint8_t> special_cards{50, 60, 61, 71};
}  // namespace
using MezzCardPtr = MuonMDT_CablingMap::MezzCardPtr;

MuonMDT_CablingMap::MuonMDT_CablingMap() {
    // initialize the message service

    // retrieve the MdtIdHelper
    ISvcLocator* svcLocator = Gaudi::svcLocator();
    SmartIF<StoreGateSvc> detStore{svcLocator->service("DetectorStore")};
    if (!detStore) {
        THROW_EXCEPTION("Could not find the detctor store");
    }
    StatusCode sc = detStore->retrieve(m_mdtIdHelper, "MDTIDHELPER");
    if (sc != StatusCode::SUCCESS) {
        THROW_EXCEPTION("Could not retrieve the MdtIdHelper");
    }
    m_2CSM_cham = m_mdtIdHelper->stationNameIndex("BME") != -1;
}

MuonMDT_CablingMap::~MuonMDT_CablingMap() = default;

bool MuonMDT_CablingMap::convert(const CablingData& cabling_data,
                                 Identifier& id, bool check_valid) const {
    bool valid{!check_valid};
    id = check_valid ? m_mdtIdHelper->channelID(
                           cabling_data.stationIndex, cabling_data.eta,
                           cabling_data.phi, cabling_data.multilayer,
                           cabling_data.layer, cabling_data.tube, valid)
                     : m_mdtIdHelper->channelID(
                           cabling_data.stationIndex, cabling_data.eta,
                           cabling_data.phi, cabling_data.multilayer,
                           cabling_data.layer, cabling_data.tube);
    return valid;
}
bool MuonMDT_CablingMap::convert(const Identifier& module_id,
                                 CablingData& cabling_data) const {
    if (!m_mdtIdHelper->is_mdt(module_id))
        return false;
    cabling_data.stationIndex = m_mdtIdHelper->stationName(module_id);
    cabling_data.eta = m_mdtIdHelper->stationEta(module_id);
    cabling_data.phi = m_mdtIdHelper->stationPhi(module_id);
    cabling_data.tube = m_mdtIdHelper->tube(module_id);
    cabling_data.multilayer = m_mdtIdHelper->multilayer(module_id);
    cabling_data.layer = m_mdtIdHelper->tubeLayer(module_id);
    return true;
}

/** add a new line from the database, describing a mezzanine type */
bool MuonMDT_CablingMap::addMezzanineLine(const int type, const int layer,
                                          const int sequence, MsgStream& log) {
    const bool debug = (log.level() <= MSG::VERBOSE);
    if (special_cards.count(type)) {
        if (debug)
            log << MSG::VERBOSE << "Mezzanine type " << type
                << " breaks the legacy database format. No need to add the "
                   "card if it's hardcoded in C++"
                << endmsg;
        return true;
    }

    int nOfLayers{0}, ntubes{0}, number{sequence};
    std::array<int, 8> newseq{};
    // now decode the sequence, up to 8 tubes per sequence
    int tube = number % 10;

    while (tube != 0) {
        // add the tube to the tube sequence
        if (ntubes > 7) {
            log << MSG::ERROR << "More than 8 tubes in a layer, not possible !"
                << endmsg;
            return false;
        }
        if (debug) {
            log << MSG::VERBOSE << "Adding tube number: " << tube
                << " to the layer " << layer << " of mezzanine type " << type
                << endmsg;
        }

        newseq[ntubes] = tube;

        ++ntubes;
        number = (number - tube) / 10;
        tube = number % 10;
    }

    if (ntubes != 8 && ntubes != 6) {
        log << MSG::ERROR << "in type " << type
            << ": number of tubes per mezzanine layer can be only 6 or 8 ! "
               "what are you doing ???"
            << endmsg;
        return false;
    }
    nOfLayers = 24 / ntubes;
    if (layer > nOfLayers) {
        log << MSG::ERROR
            << "The maximum number of layers for this mezzanine is: "
            << nOfLayers << " so you can't initialize layer: " << layer
            << endmsg;
        return false;
    }

    if (debug) {
        log << MSG::VERBOSE << "Found " << ntubes << " tubes in layer " << layer
            << endmsg;
        log << MSG::VERBOSE << "This is a " << nOfLayers
            << " layers mezzanine - OK, OK..." << endmsg;
    }

    // now swap the sequence to have it as in the DB and create the real layers
    std::array<uint8_t, 8> newLayer{};
    for (int i = 0; i < ntubes; ++i) {
        newLayer[i] = newseq[ntubes - i - 1];
    }
    MezzCardList::iterator itr = std::find_if(
        m_mezzCards.begin(), m_mezzCards.end(),
        [type](const MezzCardPtr& card) { return type == card->id(); });

    using MezzMapping = MdtMezzanineCard::Mapping;
    MezzMapping new_map =
        itr != m_mezzCards.end()
            ? (**itr).tdcToTubeMap()
            : make_array<uint8_t, 24>(MdtMezzanineCard::NOTSET);

    MdtMezzanineCard dummy_card(new_map, nOfLayers, 0);

    for (int i = 0; i < ntubes; ++i) {
        if (layer) {
            const int chStart = ntubes * (layer - 1);
            new_map[chStart + i] = dummy_card.tubeNumber(layer, newLayer[i]);
        } else {
            for (int lay = 1; lay <= nOfLayers; ++lay) {
                const int chStart = ntubes * (lay - 1);
                new_map[chStart + i] = dummy_card.tubeNumber(lay, newLayer[i]);
            }
        }
    }
    /// Overwrite the old mezzanine card pointer
    if (itr != m_mezzCards.end()) {
        if (!layer) {
            log << MSG::ERROR << "The mezzanine type " << type
                << "has been already initialized" << endmsg;
            return false;
        }
        (*itr) = std::make_unique<MdtMezzanineCard>(new_map, nOfLayers, type);
        if (debug)
            log << MSG::VERBOSE << "Updated mezzanine " << (**itr) << endmsg;
    } else {
        m_mezzCards.emplace_back(
            std::make_unique<MdtMezzanineCard>(new_map, nOfLayers, type));
        if (debug)
            log << MSG::VERBOSE << " Added new mezzanine "
                << (*m_mezzCards.back()) << endmsg;
    }
    return true;
}
MezzCardPtr MuonMDT_CablingMap::getHedgeHogMapping(uint8_t mezzCardId) const {
    MezzCardPtr mezzaType{nullptr};
    MezzCardList::const_iterator it = std::find_if(
        m_mezzCards.begin(), m_mezzCards.end(),
        [&](const MezzCardPtr& card) { return card->id() == mezzCardId; });
    return it != m_mezzCards.end() ? (*it) : nullptr;
}
/** Add a new mezzanine to the map */
bool MuonMDT_CablingMap::addMezzanine(CablingData map_data, DataSource source,
                                      MsgStream& log) {
    bool debug = (log.level() <= MSG::VERBOSE);

    MezzCardPtr mezzaType = source == DataSource::LegacyCOOL
                                ? legacyHedgehogCard(map_data, log)
                                : getHedgeHogMapping(map_data.mezzanine_type);
    if (!mezzaType) {
        log << MSG::ERROR
            << "Mezzanine Type: " << static_cast<int>(map_data.mezzanine_type)
            << " not found in the list !" << endmsg;
        return false;
    } else if (source == DataSource::LegacyCOOL && !mezzaType->checkConsistency(log)){
        return false;
    }
    auto newTdc = std::make_unique<MdtTdcMap>(mezzaType, map_data);
    if (debug) {
        log << MSG::VERBOSE << " Added new readout channel " << map_data<< endmsg;
    }
    MdtOffChModule& offModule = m_toOnlineConv[map_data];
    offModule.cards.emplace(newTdc.get());
    if (!offModule.csm[0]) {
        offModule.csm[0] = map_data;
        if (debug){
            log<< MSG::VERBOSE<<"Assign first CSM "<<map_data<<endmsg;
        }
        /// If the card is mounted on ML1 check that the first CSM on the second ML is the same
        if (map_data.multilayer == 1) {
            CablingData secondMl = map_data;
            secondMl.multilayer = 2;
            MdtOffChModule& secondModule{m_toOnlineConv[secondMl]};
            if (!secondModule.csm[0]){
                secondModule.csm[0] = map_data;
            } else if (secondModule.csm[0] != map_data && !secondModule.csm[1]) {
                secondModule.csm[1] = map_data;
                std::swap(secondModule.csm[1], secondModule.csm[0]);
            }
        }
    } else if (offModule.csm[0] != map_data) {
        if (!offModule.csm[1]) {
            offModule.csm[1] = map_data;
            if (debug) {
                log << MSG::VERBOSE << " Add second CSM for " << map_data << endmsg;
            }
        } else if (offModule.csm[1] != map_data) {
            log << MSG::ERROR << "The mulit layer " << map_data
                << " has already associated the CSMs " << std::endl
                << "   *** " << offModule.csm[0] << std::endl
                << "   *** " << offModule.csm[1] << std::endl
                << ", while this one is a third one and not supported"
                << endmsg;
            return false;
        }
    }
    TdcOnlSet& attachedTdcs = m_toOfflineConv[map_data].all_modules;
    if (attachedTdcs.size() <= map_data.tdcId)
        attachedTdcs.resize(map_data.tdcId + 1);
    attachedTdcs[map_data.tdcId] = MdtTdcOnlSorter{newTdc.get()};
    m_tdcs.push_back(std::move(newTdc));

    if (!addChamberToROBMap(map_data, log) && debug) {
        log << MSG::VERBOSE << "Station already in the map !" << endmsg;
    }

    return true;
}
bool MuonMDT_CablingMap::getOfflineId(CablingData& cabling_map,
                                      MsgStream& log) const {
    OnlToOffMap::const_iterator module_itr = m_toOfflineConv.find(cabling_map);
    if (module_itr == m_toOfflineConv.end()) {
        log << MSG::WARNING
            << "Could not find a cabling module to recieve offline Id for "
            << cabling_map << endmsg;
        return false;
    }
    /// if it's the dummy TDC (i.e. the 0xff used to convert the full station)
    if (cabling_map.tdcId == 0xff && cabling_map.channelId == 0xff) {
        cabling_map.channelId = 0;
        if (!module_itr->second.zero_module) {
            log << MSG::WARNING << " No tdc with channel zero found for "
                << module_itr->first << endmsg;
            return false;
        }
        if (!module_itr->second.zero_module->offlineId(cabling_map, log)) {
            log << MSG::WARNING << "MdtTdMap::getOfflineId() -- Channel: "
                << static_cast<unsigned>(cabling_map.channelId)
                << " Tdc: " << static_cast<unsigned>(cabling_map.tdcId)
                << " not found in "
                << static_cast<const MdtCablingOnData&>(cabling_map) << endmsg;
            return false;
        }
    } else {
        const TdcOnlSet& attachedTdcs = module_itr->second.all_modules;
        if (attachedTdcs.size() <= cabling_map.tdcId) {
            log << MSG::WARNING << "getOfflineId() -- Tdc: "
                << static_cast<unsigned>(cabling_map.tdcId)
                << " is not part of " << module_itr->first << ". Maximally "
                << attachedTdcs.size() << " Tdcs were attached. " << endmsg;
            return false;
        }
        const MdtTdcOnlSorter& TdcItr = attachedTdcs.at(cabling_map.tdcId);
        if (!TdcItr) {
            log << MSG::WARNING << "getOfflineId() -- Tdc: "
                << static_cast<unsigned>(cabling_map.tdcId) << " not found in "
                << static_cast<const MdtCablingOnData&>(cabling_map) << endmsg;
            return false;
        }
        if (!TdcItr->offlineId(cabling_map, log)) {
            log << MSG::WARNING << "MdtTdMap::getOfflineId() -- channel: "
                << static_cast<unsigned>(cabling_map.channelId)
                << " Tdc: " << static_cast<unsigned>(cabling_map.tdcId)
                << " not found in "
                << static_cast<const MdtCablingOnData&>(cabling_map) << endmsg;
            return false;
        }
    }
    if (log.level() <= MSG::VERBOSE) {
        log << MSG::VERBOSE
            << "Channel: " << static_cast<unsigned>(cabling_map.channelId)
            << " Tdc: " << static_cast<unsigned>(cabling_map.tdcId) << " "
            << static_cast<const MdtCablingOnData&>(cabling_map) << endmsg;

        log << MSG::VERBOSE << "Mapped to  "
            << static_cast<const MdtCablingOffData&>(cabling_map)
            << " layer: " << cabling_map.layer << " tube: " << cabling_map.tube
            << endmsg;
    }
    return true;
}

/** get the online id from the offline id */
bool MuonMDT_CablingMap::getOnlineId(CablingData& cabling_map,
                                     MsgStream& log) const {
    OffToOnlMap::const_iterator module_itr = m_toOnlineConv.find(cabling_map);
    if (module_itr == m_toOnlineConv.end()) {
        log << MSG::WARNING
            << "getOnlineId() --- Could not find a cabling CSM set recieve "
               "online Id for "
            << static_cast<MdtCablingOffData&>(cabling_map) << endmsg;
        return false;
    }
    const TdcOffSet& attachedTdcs = module_itr->second.cards;
    TdcOffSet::const_iterator tdc_itr = attachedTdcs.find(cabling_map);
    if (tdc_itr == attachedTdcs.end()) {
        log << MSG::WARNING << "No matching Tdc channel was found for "
            << cabling_map << endmsg;
    } else if ((*tdc_itr)->onlineId(cabling_map, log))
        return true;
    /// May be we missed it?
    TdcOffSet::const_iterator control_itr =
        std::find_if(attachedTdcs.begin(), attachedTdcs.end(),
                     [&cabling_map, &log](const MdtTdcOffSorter& tdc) {
                         return tdc->onlineId(cabling_map, log);
                     });
    if (control_itr == attachedTdcs.end()) {
        log << MSG::WARNING
            << "Second trial to find a valid cabling channel for "
            << cabling_map << " failed as well. " << endmsg;
        return false;
    }
    return true;
}
const MuonMDT_CablingMap::OnlToOffMap& MuonMDT_CablingMap::getOnlineConvMap()
    const {
    return m_toOfflineConv;
}
const MuonMDT_CablingMap::OffToOnlMap& MuonMDT_CablingMap::getOfflineConvMap()
    const {
    return m_toOnlineConv;
}

bool MuonMDT_CablingMap::addChamberToROBMap(const CablingData& map_data,
                                            MsgStream& log) {
    bool debug = (log.level() <= MSG::VERBOSE);
    IdentifierHash chamberId, multiLayerId{0};
    Identifier ml{0};
    if (!getStationCode(map_data, chamberId, log)) {
        log << MSG::ERROR << "Could not find hashId for station: " << map_data
            << endmsg;
        return false;
    }
    if (!getMultiLayerCode(map_data, ml, multiLayerId, log)) {
        log << MSG::ERROR << "Could not find hashId for multi layer: " << map_data << endmsg;
        return false;
    }
    int sub = map_data.subdetectorId;
    int rod = map_data.mrod;

    uint32_t hardId = (sub << 16) | rod;
    if (debug) {
        log << MSG::VERBOSE << "Adding the chamber with Id: " << chamberId
            << " and subdetector+rod ID: " << hardId << endmsg;
    }

    // check if the chamber has already been put into the map
    ChamberToROBMap::const_iterator it = m_multilayerToROB.find(multiLayerId);
    if (it != m_multilayerToROB.end()) {
        return false;
    }
    m_multilayerToROB.insert(std::make_pair(multiLayerId, hardId));
    m_chamberToROB.insert(std::make_pair(chamberId, hardId));
    // new function to do the opposite of the above
    m_ROBToMultiLayer[hardId].push_back(multiLayerId);
    // now check if the ROB is already in the list of ROB vector
    const bool robInitialized =
        std::find(m_listOfROB.begin(), m_listOfROB.end(), hardId) !=
        m_listOfROB.end();
    if (!robInitialized) {
        if (debug) {
            log << MSG::VERBOSE << "Adding the ROB " << hardId << " to the list"
                << endmsg;
        }
        m_listOfROB.push_back(hardId);
    }
    return true;
}
unsigned int MuonMDT_CablingMap::csmNumOnChamber(const CablingData& map_data,
                                                 MsgStream& log) const {
    /// Look up whether the station corresponds to the first or the second CSM
    OffToOnlMap::const_iterator off_itr = m_toOnlineConv.find(map_data);
    if (off_itr == m_toOnlineConv.end()) {
        log << MSG::ERROR << "csmNumOnChamber() -- Nothing is saved under  "
            << map_data << endmsg;
        return 0;
    }
    return (1 * (off_itr->second.csm[0] == map_data)) +
           (2 * (off_itr->second.csm[1] == map_data));
}
bool MuonMDT_CablingMap::getStationCode(const CablingData& map_data,
                                        IdentifierHash& mdtHashId,
                                        MsgStream& log) const {
    const Identifier elementId = m_mdtIdHelper->elementID(
        map_data.stationIndex, map_data.eta, map_data.phi);
    if (m_mdtIdHelper->get_module_hash(elementId, mdtHashId)) {
        log << MSG::ERROR
            << "getstationCode() -- Could not find HashId for module: "
            << map_data << endmsg;
        elementId.show();
        return false;
    }
    return true;
}
bool MuonMDT_CablingMap::has2CsmML() const {
    return m_2CSM_cham;
}
bool MuonMDT_CablingMap::getMultiLayerCode(const CablingData& map_data,
                                           Identifier& elementId,
                                           IdentifierHash& mdtHashId,
                                           MsgStream& log) const {
    const unsigned int ml = m_2CSM_cham ? csmNumOnChamber(map_data, log) : 1;
    if (!ml) {
        log << MSG::ERROR
            << "getMultiLayerCode() -- Could not determine the detector layer "
            << map_data << endmsg;
        return false;
    }
    /// create the station identifier
    elementId = m_mdtIdHelper->channelID(map_data.stationIndex, map_data.eta,
                                         map_data.phi, ml, 1, 1);
    /// Layouts with BMEs use the detector element hashes to cover the chambers
    /// with 2 mounted CSMs
    if (m_2CSM_cham &&
        m_mdtIdHelper->get_detectorElement_hash(elementId, mdtHashId)) {
        log << MSG::ERROR
            << "getMultiLayerCode() -- Could not find HashId for module: "
            << map_data << endmsg;
        elementId.show();
        return false;
    }
    /// In Run1, all chambers had only one CSM. To maintain backward
    /// compbability use the module hash
    else if (!m_2CSM_cham &&
             m_mdtIdHelper->get_module_hash(elementId, mdtHashId)) {
        log << MSG::ERROR
            << "getMultiLayerCode() -- Could not find HashId for module: "
            << map_data << endmsg;
        elementId.show();
        return false;
    }
    return true;
}
uint32_t MuonMDT_CablingMap::getROBId(const IdentifierHash& stationCode,
                                      MsgStream& log) const {
    ChamberToROBMap::const_iterator it = m_chamberToROB.find(stationCode);
    if (it != m_chamberToROB.end()) {
        return it->second;
    }
    log << MSG::WARNING << "ROB ID " << stationCode << " not found !" << endmsg;
    return 0;
}
// get the robs corresponding to a vector of hashIds, copied from Svc before the
// readCdo migration
std::vector<uint32_t> MuonMDT_CablingMap::getROBId(
    const std::vector<IdentifierHash>& mdtHashVector, MsgStream& log) const {
    std::vector<uint32_t> robVector;
    bool debug = (log.level() <= MSG::VERBOSE);
    for (unsigned int i = 0; i < mdtHashVector.size(); ++i) {
        int robId = getROBId(mdtHashVector[i], log);
        if (!robId) {
            log << MSG::ERROR
                << "ROB id not found for Hash Id: " << mdtHashVector[i]
                << endmsg;
        } else if (debug) {
            log << MSG::VERBOSE << "Found ROB id " << robId << " for hashId "
                << mdtHashVector[i] << endmsg;
        }
        robVector.push_back(robId);
    }
    if (debug) {
        log << MSG::VERBOSE << "Size of ROB vector is: " << robVector.size()
            << endmsg;
    }
    return robVector;
}

const std::vector<IdentifierHash>& MuonMDT_CablingMap::getMultiLayerHashVec(
    const uint32_t ROBId, MsgStream& log) const {
    ROBToChamberMap::const_iterator Rob_it = m_ROBToMultiLayer.find(ROBId);
    if (Rob_it != m_ROBToMultiLayer.end()) {
        return Rob_it->second;
    }

    log << MSG::WARNING << "Rod ID not found !" << endmsg;
    static const std::vector<IdentifierHash> emptyIdHashVec{};
    return emptyIdHashVec;
}

std::vector<IdentifierHash> MuonMDT_CablingMap::getMultiLayerHashVec(
    const std::vector<uint32_t>& ROBId_list, MsgStream& log) const {
    std::vector<IdentifierHash> HashVec;
    for (unsigned int i = 0; i < ROBId_list.size(); ++i) {
        ROBToChamberMap::const_iterator Rob_it =
            m_ROBToMultiLayer.find(ROBId_list[i]);
        if (Rob_it == m_ROBToMultiLayer.end()) {
            log << MSG::WARNING << "Rod ID " << ROBId_list[i]
                << " not found, continuing with the rest of the ROBId"
                << endmsg;
            continue;
        }
        HashVec.insert(HashVec.end(), Rob_it->second.begin(),
                       Rob_it->second.end());
    }
    return HashVec;
}

const MuonMDT_CablingMap::ListOfROB& MuonMDT_CablingMap::getAllROBId() const {
    return m_listOfROB;
}

bool MuonMDT_CablingMap::finalize_init(MsgStream& log) {
    if (m_tdcs.empty()) {
        log << MSG::ERROR << "No tdc maps were loaded " << endmsg;
        return false;
    }
    for (const MezzCardPtr& card : m_mezzCards) {
        if (!card->checkConsistency(log))
            return false;
    }

    const unsigned int offToOnlChan =
        std::accumulate(m_toOnlineConv.begin(), m_toOnlineConv.end(), 0,
                        [](unsigned int N, const auto& map) {
                            return N + map.second.cards.size();
                        });
    const unsigned int onlToOffChan = std::accumulate(
        m_toOfflineConv.begin(), m_toOfflineConv.end(), 0,
        [](unsigned int N, const auto& map) {
            return N + std::accumulate(map.second.begin(), map.second.end(), 0,
                                       [](unsigned int M, const auto& tdc) {
                                           return M + tdc;
                                       });
        });

    if (offToOnlChan != onlToOffChan || onlToOffChan != m_tdcs.size()) {
        log << MSG::ERROR
            << "Offline <-> online conversion channels were lost. Expect "
            << m_tdcs.size()
            << " cabling channels. Counted Offline -> online channels "
            << offToOnlChan << ". Conted Online -> offline channels "
            << onlToOffChan << endmsg;
        return false;
    }
    for (auto& toOff : m_toOfflineConv) {
        MdtTdcModule& mod = toOff.second;
        TdcOnlSet::const_iterator itr = std::find_if(
            mod.begin(), mod.end(), [](const MdtTdcOnlSorter& sorter) {
                return sorter && sorter->tdcZero() == 0;
            });
        if (itr == mod.end()) {
            log << MSG::ERROR << "There is no tdc with channel 0 in "
                << toOff.first
                << ". That is important to decode the station names later"
                << endmsg;
            return false;
        }
        mod.zero_module = (*itr);
    }
    m_mezzCards.clear();
    log << MSG::INFO << "MdtCabling successfully loaded. Found in total "
        << m_tdcs.size() << " channels." << endmsg;
    return true;
}
bool MuonMDT_CablingMap::addMezanineLayout(
    std::unique_ptr<MdtMezzanineCard> card, MsgStream& log) {
    if (!card->checkConsistency(log))
        return false;
    MezzCardList::const_iterator itr =
        std::find_if(m_mezzCards.begin(), m_mezzCards.end(),
                     [&card](const MezzCardPtr& existing) {
                         return existing->id() == card->id();
                     });
    if (itr != m_mezzCards.end()) {
        log << MSG::ERROR << "Mezzanine card " << std::endl
            << (*card) << " has already been added " << std::endl
            << (**itr) << std::endl
            << " please check. " << endmsg;
        return false;
    }
    m_mezzCards.push_back(std::move(card));
    return true;
}
MezzCardPtr MuonMDT_CablingMap::legacyHedgehogCard(CablingData& cabling,
                                                   MsgStream& msg) const {
    using MezzMapping = MdtMezzanineCard::Mapping;
    MezzMapping chMap{make_array<uint8_t, 24>(MdtMezzanineCard::NOTSET)};

    const int mezzType = cabling.mezzanine_type;
    const int chanZero = cabling.channelId;
    const int layerZero = cabling.layer;
    const int tubeZero = cabling.tube;

    cabling.tube = MdtIdHelper::maxNTubesPerLayer;
    const bool debug = msg.level() <= MSG::VERBOSE;

    if (special_cards.count(mezzType)) {
        const MdtMezzanineCard dummy_card(chMap, 4, mezzType);
        if (debug)
            msg << MSG::VERBOSE
                << "legacyHedgehogCard() -- Special layout given " << mezzType
                << endmsg;
        for (size_t chan = 0; chan < chMap.size(); ++chan) {
            uint8_t tube_chan = ((chan - chan % 4) / 4);
            /// Octoberfest chambers need a mirror in the tube channel numbering
            /// https://its.cern.ch/jira/browse/ATLASRECTS-7411
            if (mezzType == 71) {
                tube_chan = 5 - tube_chan;
            }
            int layer{0}, tube{0};
            // special case of the BME of end 2013 and BMG 2017
            /*
             * cases 50 and 60 follow the same rules. hedgehog card 50 is the
             * mirror image of card 60, but the rules concerning how to decode
             * the tube mapping are the same. channel 0 of a mezzanine card for
             * case 50 is either top right or bottom left, while for case 60 it
             * is top left or bottom right. Another thing to keep in mind for
             * BMG is, that tube counting in some cases is along |z|, while in
             * some other cases it is opposite.
             *
             *  E.g: Numbering from top to bottom -> Tube counting along z
             *
             *       Layer |         Tube number
             *       4     |  0     4      8    12     16   20
             *       3     |  1     5      9    13     17   21
             *       2     |  2     6     10    14     18   22
             *       1     |  3     7     11    15     19   23
             */
            if (mezzType == 50 || mezzType == 60 || mezzType == 61) {
                /// Tube numbering is from bottom to top
                if (layerZero == 1) {
                    layer = chan % 4 + 1;
                }
                // Tube numbering is from top to bottom
                else {
                    layer = 4 - chan % 4;
                }
                if ((tubeZero - 1) % 6 == 0) {
                    tube = tubeZero + tube_chan;
                } else {
                    tube = tubeZero - tube_chan;
                }
            }
            /*  Few mezzanine cards in the BIS78 chambers are collateral vicitms
             * of the famous regional customs happening in the Bavarian autumun.
             * The tube staggering is kind of discontinous where the upper two
             * layers are swapped with the bottom two 1     5      9     13 17
             * 21 0     4      8     12     16      20 3     7     11     15 19
             * 23 2     6     10     14     18      22
             */
            else if (mezzType == 71) {
                layer = (chan % 4);
                layer += 2 * (layer > 1 ? -1 : +1) + 1;
                tube = tubeZero - tube_chan;
            }
            cabling.tube = std::min(cabling.tube, tube);
            chMap[chan] = dummy_card.tubeNumber(layer, tube);
            if (debug)
                msg << MSG::VERBOSE << "legacyHedgehogCard() -- Channel "
                    << chan << " mapped to " << layer << ", " << tube << endmsg;
        }
        return std::make_unique<MdtMezzanineCard>(chMap, 4, mezzType);
    }
    /// Legacy tube mapping
    MezzCardPtr card = getHedgeHogMapping(mezzType);
    if (!card)
        return nullptr;
    if (debug)
        msg << MSG::VERBOSE << "Assign local tube mapping from " << (*card)
            << std::endl
            << "to connect " << cabling << " tubeZero: " << tubeZero << endmsg;
    const int stationName = cabling.stationIndex;
    const int stationEta = std::abs(cabling.eta);

    for (size_t chan = 0; chan < chMap.size(); ++chan) {
        // calculate the layer
        int layer{0}, tube{0};
        if (layerZero == 1) {
            layer = chan / (card->numTubesPerLayer()) + 1;
        } else {
            layer = layerZero - chan / (card->numTubesPerLayer());
        }

        // calculate the tube in layer
        uint8_t localchan = chan % (card->numTubesPerLayer());

        // special case of the BIR with 3 tubes overlapping to another mezzanine
        if ((chanZero == 5 && stationName == 7 && stationEta == 3) &&
            ((card->id() > 40 && localchan % 2 == 0) ||
             (card->id() < 40 && localchan < 3)))
            continue;

        using OfflineCh = MdtMezzanineCard::OfflineCh;
        OfflineCh locTubeLay = card->offlineTube(
            localchan + (layer - 1) * card->numTubesPerLayer(), msg);
        OfflineCh zeroTubeLay = card->offlineTube(
            chanZero + (layer - 1) * card->numTubesPerLayer(), msg);
        if (!locTubeLay.isValid || !zeroTubeLay.isValid)
            continue;

        tube = (static_cast<int>(locTubeLay.tube) -
                static_cast<int>(zeroTubeLay.tube) + tubeZero);

        if (debug)
            msg << MSG::VERBOSE << "legacyHedgehogCard() -- Channel " << chan
                << " mapped to " << layer << ", " << tube
                << " locTube: " << static_cast<int>(locTubeLay.tube)
                << " zeroTubeLay: " << static_cast<int>(zeroTubeLay.tube)
                << endmsg;

        if (tube < 1 || tube >= MdtIdHelper::maxNTubesPerLayer)
            continue;
        cabling.tube = std::min(cabling.tube, tube);
        chMap[chan] = card->tubeNumber(layer, tube);
    }
    std::unique_ptr<MdtMezzanineCard> to_ret =
        std::make_unique<MdtMezzanineCard>(chMap, card->numTubeLayers(),
                                           mezzType);
    const uint8_t tubeZeroOff = (cabling.tube - 1) % to_ret->numTubesPerLayer();
    if (!tubeZeroOff)
        return to_ret;
    chMap = make_array<uint8_t, 24>(MdtMezzanineCard::NOTSET);
    for (size_t chan = 0; chan < chMap.size(); ++chan) {
        using OfflineCh = MdtMezzanineCard::OfflineCh;
        OfflineCh tube_lay = to_ret->offlineTube(chan, msg);
        if (!tube_lay.isValid)
            continue;
        uint8_t tubeNumber = tube_lay.tube + tubeZeroOff + 1;
        chMap[chan] = card->tubeNumber(tube_lay.layer, tubeNumber);
    }
    if (debug)
        msg << MSG::VERBOSE << "Final mapping " << cabling << endmsg;

    return std::make_unique<MdtMezzanineCard>(chMap, card->numTubeLayers(),
                                              mezzType);
}
