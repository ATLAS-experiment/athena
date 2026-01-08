/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCondDbAlg.h"

#include "AthenaKernel/IOVInfiniteRange.h"
#include "CxxUtils/StringUtils.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"

using readOutPair = CondAttrListCollection::ChanAttrListPair;
using namespace MuonCond;
using namespace CxxUtils;

namespace Muon{
// Initialize
StatusCode MdtCondDbAlg::initialize() {
    ATH_MSG_DEBUG("initializing " << name());

    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_readKey_folder_da_pshv.initialize(!m_readKey_folder_da_pshv.empty() && m_isData && m_isRun1));
    ATH_CHECK(m_readKey_folder_da_psv0.initialize(!m_readKey_folder_da_psv0.empty() && m_isData && m_isRun1 && m_checkOnSetPoint));
    ATH_CHECK(m_readKey_folder_da_psv1.initialize(!m_readKey_folder_da_psv1.empty() && m_isData && m_isRun1 && m_checkOnSetPoint));
    ATH_CHECK(m_readKey_folder_da_pslv.initialize(!m_readKey_folder_da_pslv.empty() && m_isData && m_isRun1));
    ATH_CHECK(m_readKey_folder_da_hv.initialize(!m_readKey_folder_da_hv.empty() && m_isData && !m_isRun1));
    ATH_CHECK(m_readKey_folder_da_lv.initialize(!m_readKey_folder_da_lv.empty() && m_isData && !m_isRun1));
    ATH_CHECK(m_readKey_folder_da_droppedChambers.initialize(!m_readKey_folder_da_droppedChambers.empty() && m_isData && m_isRun1));
    ATH_CHECK(m_readKey_folder_mc_droppedChambers.initialize(!m_readKey_folder_mc_droppedChambers.empty() && !m_isData));
    ATH_CHECK(m_readKey_folder_mc_noisyChannels.initialize(!m_readKey_folder_mc_noisyChannels.empty() && !m_isData));
    // The calls to the functions that use these two are commented out,
    // so don't declare a dependencies on them.
    ATH_CHECK(m_readKey_folder_mc_deadElements.initialize(false /*!m_readKey_folder_mc_deadElements.empty() && !m_isData*/));
    ATH_CHECK(m_readKey_folder_mc_deadTubes.initialize(false /*!m_readKey_folder_mc_deadTubes.empty() && !m_isData*/));

    ServiceHandle<IGeoModelSvc> geoModel("GeoModelSvc", name());
    ATH_CHECK(geoModel.retrieve());

    std::string AtlasVersion = geoModel->atlasVersion();
    std::string MuonVersion = geoModel->muonVersionOverride();
    std::string detectorKey = MuonVersion.empty() ? AtlasVersion : MuonVersion;
    std::string detectorNode = MuonVersion.empty() ? "ATLAS" : "MuonSpectrometer";

    ServiceHandle<IRDBAccessSvc> accessSvc("RDBAccessSvc", name());
    ATH_CHECK(accessSvc.retrieve());

    IRDBRecordset_ptr switchSet = accessSvc->getRecordsetPtr("HwSwIdMapping", detectorKey, detectorNode);

    if ((*switchSet).size() == 0) {
        ATH_MSG_WARNING("Old Atlas Version : " << AtlasVersion << " Only Online Identifier. Falling back to HwSwIdMapping-00 tag");
        switchSet = accessSvc->getRecordsetPtr("HwSwIdMapping", "HwSwIdMapping-00");
    }

    for (unsigned int irow = 0; irow < (*switchSet).size(); ++irow) {
        const IRDBRecord* switches = (*switchSet)[irow];
        std::string hardwareName = switches->getString("HARDNAME");
        std::string stName = switches->getString("SOFTNAME");
        int stPhi = switches->getInt("SOFTOCTANT");
        int stEta = switches->getInt("SOFTIZ");
        bool isValid{false};
        Identifier ChamberId = m_idHelperSvc->mdtIdHelper().elementID(stName, stEta, stPhi, isValid);
        if (!isValid) continue;

        m_chamberNames[hardwareName] = ChamberId;
    }

    return StatusCode::SUCCESS;
}
StatusCode MdtCondDbAlg::addDHDependency(const EventContext& ctx, const dataBaseKey_t& key,  writeHandle_t& wh) const {
    if (key.empty()) {
        ATH_MSG_VERBOSE("Key is empty");
        return StatusCode::SUCCESS;
    }
    SG::ReadCondHandle readHandle{key, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_FATAL("Failed to load conditions from "<<key.fullKey());
        return StatusCode::FAILURE;
    }
    wh.addDependency(readHandle);
    return StatusCode::SUCCESS;
}
 StatusCode MdtCondDbAlg::loadDependencies(const EventContext& ctx, writeHandle_t& wh) const {
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_da_pshv, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_da_psv0, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_da_psv1, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_da_pslv, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_da_hv, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_da_lv, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_da_droppedChambers, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_mc_droppedChambers, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_mc_noisyChannels, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_mc_deadElements, wh));
    ATH_CHECK(addDHDependency(ctx, m_readKey_folder_mc_deadTubes, wh));
    return StatusCode::SUCCESS;
 }

// execute
StatusCode MdtCondDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());

    if (m_isOnline) {
        ATH_MSG_DEBUG("IsOnline is set to True; nothing to do!");
        return StatusCode::SUCCESS;
    }

    // launching Write Cond Handle
    SG::WriteCondHandle writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << " In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    auto writeCdo{std::make_unique<MdtCondDbData>(m_idHelperSvc->mdtIdHelper())};
    ATH_CHECK(loadDependencies(ctx, writeHandle));
    // retrieving data
    if (m_isData && m_isRun1) {
        ATH_CHECK(loadDataPsHv(ctx, *writeCdo));
        ATH_CHECK(loadDataPsLv(ctx, *writeCdo));
        ATH_CHECK(loadDroppedChambers(ctx, *writeCdo, false));
    } else if (m_isData && !m_isRun1) {
        ATH_CHECK(loadDataHv(ctx, *writeCdo));
        ATH_CHECK(loadDataLv(ctx, *writeCdo));
    } else {
        ATH_CHECK(loadDroppedChambers(ctx, *writeCdo, true));
        ATH_CHECK(loadMcNoisyChannels(ctx, *writeCdo));
        // ATH_CHECK(loadMcDeadElements     (rangeW, writeCdo.get(),ctx));// keep for future development
        // ATH_CEHCK(loadMcDeadTubes        (rangeW, writeCdo.get(),ctx));// keep for future development
    }

    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    ATH_MSG_DEBUG("Recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");

    return StatusCode::SUCCESS;
}

// loadDataPsHv
StatusCode MdtCondDbAlg::loadDataPsHv(const EventContext& ctx, MdtCondDbData& writeCdo ) const {
    SG::ReadCondHandle readHandle{m_readKey_folder_da_pshv, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());

    std::vector<Identifier> cachedDeadMultiLayersId_standby;
    for (const readOutPair& itr : *readCdo) {
        const unsigned int chanNum = itr.first;
        const coral::AttributeList& atr = itr.second;
        const std::string& hv_payload = readCdo->chanName(chanNum);
        std::string hv_name;

        if (atr.size() == 1) {
            hv_name = *(static_cast<const std::string*>((atr["fsm_currentState"]).addressOfData()));
            
            auto tokens = tokenize(hv_name, " ");
            auto tokens2 = tokenize(hv_payload, "_");

            if (tokens[0] != "ON" && tokens[0] != "STANDBY" && tokens[0] != "UNKNOWN") {
                int multilayer = atoi(tokens2[3]);
                const auto  &chamber_name = tokens2[2];
                Identifier ChamberId = identifyChamber(chamber_name);
                if (ChamberId.is_valid()) {
                    Identifier MultiLayerId = m_idHelperSvc->mdtIdHelper().channelID(ChamberId, multilayer, 1, 1);
                    writeCdo.setDeadMultilayer(MultiLayerId);
                    writeCdo.setDeadChamber(ChamberId);
                    cachedDeadMultiLayersId_standby.push_back(MultiLayerId);
                }
            }
            if (tokens[0] == "STANDBY") {
                int multilayer = atoi(tokens2[3]);
                const auto &chamber_name = tokens2[2];
                Identifier ChamberId = identifyChamber(chamber_name);
                if (ChamberId.is_valid()) {
                    Identifier MultiLayerId = m_idHelperSvc->mdtIdHelper().channelID(ChamberId, multilayer, 1, 1);
                    writeCdo.setDeadMultilayer(MultiLayerId);
                    writeCdo.setDeadChamber(ChamberId);
                    cachedDeadMultiLayersId_standby.push_back(MultiLayerId);
                }
            }
        }
    }

    // moving on to SetPoints
    if (!m_checkOnSetPoint) return StatusCode::SUCCESS;

    std::map<Identifier, float> chamberML_V1;
    std::map<Identifier, float> chamberML_V0;

    // V0 handle
    SG::ReadCondHandle readHandle_v0{m_readKey_folder_da_psv0, ctx};
    const CondAttrListCollection* readCdo_v0{*readHandle_v0};
    if (!readCdo_v0) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }
 
    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle_v0.fullKey() << " readCdo->size()= " << readCdo_v0->size());
 
    // V1
    SG::ReadCondHandle readHandle_v1{m_readKey_folder_da_psv1, ctx};
    const CondAttrListCollection* readCdo_v1{*readHandle_v1};
    if (!readCdo_v1) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle_v1.fullKey() << " readCdo->size()= " << readCdo_v1->size());

    // V0 iteration
    for (const readOutPair& itr_v0 : *readCdo_v0) {
        const unsigned int chanNum = itr_v0.first;
        const coral::AttributeList& atr_v0 = itr_v0.second;
        const std::string& setPointsV0_payload = readCdo_v0->chanName(chanNum);
        float setPointsV0_name{0.};

        if (atr_v0.size() == 1) {
            setPointsV0_name = *(static_cast<const float*>((atr_v0["readBackSettings_v0"]).addressOfData()));
            
            auto tokens2 = tokenize(setPointsV0_payload, "_");

            int multilayer = atoi(tokens2[3]);
            const auto &chamber_name = tokens2[2];
            Identifier ChamberId = identifyChamber(chamber_name);
            Identifier MultiLayerId = m_idHelperSvc->mdtIdHelper().channelID(ChamberId, multilayer, 1, 1);
            chamberML_V0[MultiLayerId] = setPointsV0_name;
        }
    }

    // V1 iteration
    CondAttrListCollection::const_iterator itr_v1;
    for (const readOutPair& itr_v1 : *readCdo_v1) {
        const unsigned int chanNum = itr_v1.first;
        const coral::AttributeList& atr_v1 = itr_v1.second;
        const std::string& setPointsV1_payload = readCdo_v1->chanName(chanNum);
        float setPointsV1_name{0.};

        if (atr_v1.size() == 1) {
            setPointsV1_name = *(static_cast<const float*>((atr_v1["readBackSettings_v1"]).addressOfData()));

            
            auto tokens2= tokenize(setPointsV1_payload, "_");

            int multilayer = atoi(tokens2[3]);
            const auto &chamber_name = tokens2[2];
            Identifier ChamberId = identifyChamber(chamber_name);
            Identifier MultiLayerId = m_idHelperSvc->mdtIdHelper().channelID(ChamberId, multilayer, 1, 1);
            chamberML_V1[MultiLayerId] = setPointsV1_name;
        }
    }

    // check for chamber standby the correct value of Setpoint V0 vs SetpointV1
    // for chamber StandBy --> V0==V1 to be on
    for (const Identifier& MultilayerId_ch : cachedDeadMultiLayersId_standby) {
        if (chamberML_V1.find(MultilayerId_ch)->second == chamberML_V0.find(MultilayerId_ch)->second) {
            ATH_MSG_DEBUG("Chamber has  correct Voltage V1 = " << chamberML_V1.find(MultilayerId_ch)->second
                                                               << " V0=   " << chamberML_V0.find(MultilayerId_ch)->second);
        } else {
            ATH_MSG_DEBUG("Chamber has  wrong correct Voltage V1 = " << chamberML_V1.find(MultilayerId_ch)->second
                                                                     << " V0=   " << chamberML_V0.find(MultilayerId_ch)->second);
            ATH_MSG_DEBUG("Has to be masked!!!");
            writeCdo.setDeadMultilayer(MultilayerId_ch);
        }
    }

    return StatusCode::SUCCESS;
}

// loadDataPsLv
StatusCode MdtCondDbAlg::loadDataPsLv(const EventContext& ctx, MdtCondDbData& writeCdo ) const {
    SG::ReadCondHandle readHandle{m_readKey_folder_da_pslv, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    for (const auto& [chanNum, atr] : *readCdo) {
        const std::string& hv_payload = readCdo->chanName(chanNum);
        std::string hv_name;

        if (!atr.size()) { continue; }
        hv_name = *(static_cast<const std::string*>((atr["fsm_currentState"]).addressOfData()));
        
        auto tokens = tokenize(hv_name, " ");        
        auto tokens2 = tokenize(hv_payload, "_");
        if (tokens[0] != "ON") {
            const auto &chamber_name = tokens2[2];
            Identifier ChamberId = identifyChamber(chamber_name);
            if (ChamberId.is_valid()) { writeCdo.setDeadChamber(ChamberId); }
        }
    }

    return StatusCode::SUCCESS;
}

StatusCode MdtCondDbAlg::loadDataHv(const EventContext& ctx, MdtCondDbData& writeCdo ) const {
    SG::ReadCondHandle readCdo{m_readKey_folder_da_hv, ctx};
    
    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readCdo.fullKey() << " readCdo->size()= " << readCdo->size());
    for (const auto& [chanNum, atr] : **readCdo) {
        if (!atr.size()) { continue; }
   
        const std::string& hv_payload = readCdo->chanName(chanNum);       
        const std::string& hv_name_ml1{*(static_cast<const std::string*>((atr["fsmCurrentState_ML1"]).addressOfData()))};
        const std::string& hv_name_ml2{*(static_cast<const std::string*>((atr["fsmCurrentState_ML2"]).addressOfData()))};
        const float hv_v0_ml1{*(static_cast<const float*>((atr["v0set_ML1"]).addressOfData()))};
        const float hv_v1_ml1{*(static_cast<const float*>((atr["v1set_ML1"]).addressOfData()))};
        const float hv_v0_ml2{*(static_cast<const float*>((atr["v0set_ML2"]).addressOfData()))};
        const float hv_v1_ml2{*(static_cast<const float*>((atr["v1set_ML2"]).addressOfData()))};
        
        Identifier chamberId = identifyChamber(hv_payload);
        if (!chamberId.is_valid()) continue;
        auto addChamber = [&](const DcsFsmState& dcsState, 
                              const float standbyVolt, 
                              const float readyVolt,
                              const int multiLayer) {
            const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
            if (multiLayer > idHelper.numberOfMultilayers(chamberId)) return;
            const Identifier mlId = idHelper.multilayerID(chamberId, multiLayer);
            constexpr std::array<DcsFsmState, 3> goodStates{DcsFsmState::ON, DcsFsmState::STANDBY, DcsFsmState::UNKNOWN};
            if ( (std::find(goodStates.begin(), goodStates.end(), dcsState) == goodStates.end()) ||
                (dcsState != DcsFsmState::ON && readyVolt != standbyVolt)) {
                writeCdo.setDeadMultilayer(mlId);
            }
            writeCdo.setHvState(mlId, dcsState, standbyVolt, readyVolt);
        };
        addChamber(getFsmStateEnum(hv_name_ml1), hv_v0_ml1, hv_v1_ml1, 1);
        addChamber(getFsmStateEnum(hv_name_ml2), hv_v0_ml2, hv_v1_ml2, 2);
    }
    return StatusCode::SUCCESS;
}

// loadDataLv
StatusCode MdtCondDbAlg::loadDataLv(const EventContext& ctx, MdtCondDbData& writeCdo ) const {
    SG::ReadCondHandle readHandle{m_readKey_folder_da_lv, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());

    for (const auto& [chanNum, atr] : *readCdo) {
        if (!atr.size()) { continue; }
        const std::string& lv_payload = readCdo->chanName(chanNum);        
        const std::string& lv_name{*static_cast<const std::string*>((atr["fsmCurrentState_LV"]).addressOfData())};
        if (lv_payload.empty() || lv_name.empty()){
            ATH_MSG_WARNING("The read data with chanNum "<<chanNum<<", lv_payload: "<<lv_payload<<", hv_name: "<<lv_name
                            <<". Does not have any fsmCurrentState_LV attribute. "
                            <<"May be this is related to ATLASRECTS-6920 / ATLASRECTS-6879. Skip it");
            continue;
        }
        ATH_MSG_VERBOSE("Channel "<<lv_name<<" "<<lv_payload);
        auto tokens = tokenize(lv_name, " "); 

        if (tokens[0] != "ON") {
           Identifier ChamberId = identifyChamber(lv_payload);
           writeCdo.setDeadChamber(ChamberId);
        }
    }

    return StatusCode::SUCCESS;
}

// loadDataDroppedChambers
StatusCode MdtCondDbAlg::loadDroppedChambers(const EventContext& ctx, MdtCondDbData& writeCdo , bool isMC) const {
    SG::ReadCondHandle readHandle{(isMC ? m_readKey_folder_mc_droppedChambers 
                                                                : m_readKey_folder_da_droppedChambers), ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());

    for (const readOutPair& itr : *readCdo) {
        const coral::AttributeList& atr = itr.second;
        const std::string& chamber_dropped{*(static_cast<const std::string*>((atr["Chambers_disabled"]).addressOfData()))};
        
        auto tokens = tokenize(chamber_dropped, " ");
        for (auto & token : tokens) {
            if (token != "0") {
                const auto &chamber_name = token;
                Identifier ChamberId = identifyChamber(chamber_name);
                if (ChamberId.is_valid()) { writeCdo.setDeadChamber(ChamberId); }
            }
        }
    }
    return StatusCode::SUCCESS;
}

// loadMcDeadElements
StatusCode MdtCondDbAlg::loadMcDeadElements(const EventContext& ctx, MdtCondDbData& writeCdo ) const {
    SG::ReadCondHandle readHandle{m_readKey_folder_mc_deadElements, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());

    for (const readOutPair& itr : *readCdo) {
        const coral::AttributeList& atr = itr.second;
        const std::string& chamber_name{*(static_cast<const std::string*>((atr["Chambers_Name"]).addressOfData()))};
        const std::string& list_mlayer{*(static_cast<const std::string*>((atr["Dead_multilayer"]).addressOfData()))};
        const std::string& list_layer{*(static_cast<const std::string*>((atr["Dead_layer"]).addressOfData()))};
        const std::string& list_tube{*(static_cast<const std::string*>((atr["Dead_tube"]).addressOfData()))};

        Identifier ChamberId = identifyChamber(chamber_name);
        auto tokens = tokenize(list_tube, " ");
        auto tokens_mlayer = tokenize(list_mlayer, " ");
        auto tokens_layer = tokenize(list_layer, " ");

        for (auto & token : tokens) {
            
            if (token != "0") {
                int ml = atoi(token.substr(0, 1));
                int layer = atoi(token.substr(1, 2));
                int tube = atoi(token.substr(2));
                Identifier ChannelId = m_idHelperSvc->mdtIdHelper().channelID(ChamberId, ml, layer, tube);
                writeCdo.setDeadTube(ChannelId);
                writeCdo.setDeadChamber(ChamberId);
            }
        }

        for (unsigned int i = 0; i < tokens_mlayer.size(); i++) {
            if (tokens_mlayer[i] != "0") {
                int ml = atoi(tokens_mlayer[i].substr(0));
                Identifier ChannelId = m_idHelperSvc->mdtIdHelper().channelID(ChamberId, ml, 1, 1);
                writeCdo.setDeadMultilayer(ChannelId);
                writeCdo.setDeadChamber(ChamberId);
            }
        }

        for (unsigned int i = 0; i < tokens_layer.size(); i++) {
            if (tokens_layer[i] != "0") {
                int ml = atoi(tokens_layer[i].substr(0, 1));
                int layer = atoi(tokens_layer[i].substr(1));
                Identifier ChannelId = m_idHelperSvc->mdtIdHelper().channelID(ChamberId, ml, layer, 1);
                writeCdo.setDeadLayer(ChannelId);
                writeCdo.setDeadChamber(ChamberId);
            }
        }
    }

    return StatusCode::SUCCESS;
}

// loadMcDeadTubes
StatusCode MdtCondDbAlg::loadMcDeadTubes(const EventContext& ctx, MdtCondDbData& writeCdo ) const {
    SG::ReadCondHandle readHandle{m_readKey_folder_mc_deadTubes, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());

    for (const readOutPair& itr : *readCdo) {
        const coral::AttributeList& atr = itr.second;

        std::string dead_tube = *(static_cast<const std::string*>((atr["DeadTube_List"]).addressOfData()));
        std::string chamber_name = *(static_cast<const std::string*>((atr["Chamber_Name"]).addressOfData()));

        auto tokens = tokenize(dead_tube, " ");
        Identifier ChamberId = identifyChamber(std::move(chamber_name));

        for (auto & token : tokens) {
            int ml = atoi(token.substr(0, 1));
            int layer = atoi(token.substr(1, 2));
            int tube = atoi(token.substr(2));
            Identifier ChannelId = m_idHelperSvc->mdtIdHelper().channelID(ChamberId, ml, layer, tube);
            writeCdo.setDeadTube(ChannelId);
        }
        writeCdo.setDeadChamber(ChamberId);
    }

    return StatusCode::SUCCESS;
}

// loadMcNoisyChannels
StatusCode MdtCondDbAlg::loadMcNoisyChannels(const EventContext& ctx, MdtCondDbData& writeCdo ) const {
    SG::ReadCondHandle readHandle{m_readKey_folder_mc_noisyChannels, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    for (const auto &[chanNum, atr] : *readCdo) {
        if (!atr.size()) {
            continue;
        }
        const std::string& hv_payload = readCdo->chanName(chanNum);
        const std::string& hv_name{*(static_cast<const std::string*>((atr["fsm_currentState"]).addressOfData()))};
        
        auto tokens = tokenize(hv_name, " ");
        
        auto tokens2 = tokenize(hv_payload, "_");

        if (tokens[0] != "ON") {
            Identifier ChamberId = identifyChamber(tokens2[2]);
            writeCdo.setDeadChamber(ChamberId);
        }
        
    }
    return StatusCode::SUCCESS;
}
Identifier MdtCondDbAlg::identifyChamber(std::string chamber) const {
    if (chamber[2] == 'Y' || chamber[2] == 'X') chamber[2] = 'S';    
    auto itr = m_chamberNames.find(chamber.substr(0, chamber.find('_')));
    if (itr != m_chamberNames.end()) return itr->second;
    ATH_MSG_DEBUG("The chamber "<<chamber<<" is unknown.");
    return Identifier{};
}
}