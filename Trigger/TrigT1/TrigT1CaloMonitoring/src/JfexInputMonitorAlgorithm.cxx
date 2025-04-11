/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JfexInputMonitorAlgorithm.h"

JfexInputMonitorAlgorithm::JfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode JfexInputMonitorAlgorithm::initialize() {

    ATH_MSG_DEBUG("Initializing JfexInputMonitorAlgorithm algorithm with name: "<< name());
    ATH_MSG_DEBUG("JfexInputMonitorAlgorith::initialize");
    ATH_MSG_DEBUG("Package Name "<< m_Grouphist);
    ATH_MSG_DEBUG("jFexDataTowerKey: "<< m_jFexDataTowerKey);
    ATH_MSG_DEBUG("jFexEmulatedTowerKey: "<< m_jFexEmulatedTowerKey);

    // we initialise all the containers that we need
    ATH_CHECK( m_jFexDataTowerKey.initialize() );
    ATH_CHECK( m_jFexEmulatedTowerKey.initialize() );

    ATH_CHECK( m_bcContKey.initialize() );


    return AthMonitorAlgorithm::initialize();
}

StatusCode JfexInputMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("JfexInputMonitorAlgorithm::fillHistograms");

    // Access jFex tower container
    SG::ReadHandle <xAOD::jFexTowerContainer> jFexTowerContainer{m_jFexDataTowerKey, ctx};
    if (!jFexTowerContainer.isValid()) {
        ATH_MSG_WARNING("No jFex Tower container valid in storegate with key: " << m_jFexDataTowerKey);
        return StatusCode::FAILURE;
    }

    // mismatches can be caused by recent/imminent OTF maskings, so track timings
    auto timeSince = Monitored::Scalar<int>("timeSince", -1);
    auto timeUntil = Monitored::Scalar<int>("timeUntil", -1);
    SG::ReadCondHandle <LArBadChannelCont> larBadChan{m_bcContKey, ctx};
    if (larBadChan.isValid()) {
        timeSince = ctx.eventID().time_stamp() - larBadChan.getRange().start().time_stamp();
        timeUntil = larBadChan.getRange().stop().time_stamp() - ctx.eventID().time_stamp();
    }
    auto EventType = Monitored::Scalar<std::string>("EventType", "Normal");
    if ((timeSince >= 0 && timeSince < 10)) EventType = "JustAfterMask";
    else if ((timeUntil >= 0 && timeUntil < 10)) EventType = "JustBeforeMask";

    auto Decision = Monitored::Scalar<std::string>("Error", "");
    auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber", GetEventInfo(ctx)->eventNumber());
    auto lbnString = Monitored::Scalar<std::string>("LBNString", std::to_string(GetEventInfo(ctx)->lumiBlock()));
    auto TowerId = Monitored::Scalar<uint32_t>("TowerId", 0);
    auto TowerSource = Monitored::Scalar<uint32_t>("TowerSource", 0);
    auto Towereta = Monitored::Scalar<float>("TowerEta", 0.0);
    auto Towerphi = Monitored::Scalar<float>("TowerPhi", 0.0);
    auto TowerCount = Monitored::Scalar<uint32_t>("TowerCount", 0);
    auto TowerRefCount = Monitored::Scalar<uint32_t>("RefTowerCount", 0);
    auto TowerSat = Monitored::Scalar<int>("TowerSat", 0);
    auto TowerRefSat = Monitored::Scalar<int>("RefTowerSat", 0);

    // these next flags are just used for cutmasks, keep them true
    auto TowerInvalid = Monitored::Scalar<bool>("TowerInvalid", true);
    auto TowerEmpty = Monitored::Scalar<bool>("TowerEmpty", true);



    //Run the monitoring only when the input data is filled (it is pre-scaled), otherwise skip
    if (jFexTowerContainer->empty()) {
        ATH_MSG_DEBUG("number of jfex towers = " << jFexTowerContainer->size());
        Decision += "MissingReadout;";
        fill(m_Grouphist, EventType, Decision);
        return StatusCode::SUCCESS;
    }

    SG::ReadHandle <xAOD::jFexTowerContainer> jFexEmulatedTowerContainer{m_jFexEmulatedTowerKey, ctx};
    if (!jFexEmulatedTowerContainer.isValid()) {
        ATH_MSG_ERROR("No jFex Tower container valid in storegate with key: " << m_jFexEmulatedTowerKey);
        return StatusCode::FAILURE;
    }

    // towers are uniquely identified by their ID+Source (bad ID design!)
    std::map<std::pair<uint32_t, int>, const xAOD::jFexTower *> dataTowers;
    for (const xAOD::jFexTower *tower: *jFexTowerContainer) {
        dataTowers[std::pair(tower->jFEXtowerID(), tower->Calosource())] = tower;
    }

    // check for more dataTowers than expected/emulated
    if (dataTowers.size() > jFexEmulatedTowerContainer->size()) {
        Decision = "MissingEmulatedTowers";
        fill("errors", Decision, timeSince, timeUntil, evtNumber, lbnString, TowerId, TowerSource, Towereta, Towerphi,
             TowerCount, TowerRefCount, TowerSat, TowerRefSat);
        ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb " << std::string(lbnString));
    }

    for (const xAOD::jFexTower *tower: *jFexEmulatedTowerContainer) {
        // check et_count codes match, and saturation bits match
        TowerId = tower->jFEXtowerID();
        TowerSource = tower->Calosource();
        Towereta = tower->eta() + 1e-5;
        Towerphi = tower->phi();
        auto dataTowerItr = dataTowers.find(std::pair(uint32_t(TowerId), int(TowerSource)));
        if (dataTowerItr == dataTowers.end()) {
            Decision = "MissingDataTower";
            fill("errors", Decision, timeSince, timeUntil, evtNumber, lbnString, TowerId, TowerSource, Towereta,
                 Towerphi, TowerCount, TowerRefCount, TowerSat, TowerRefSat);
            ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb " << std::string(lbnString)
                                                  << " TowerId=" << TowerId);
            continue;
        }
        TowerRefCount = tower->et_count().at(0);
        TowerCount = dataTowerItr->second->et_count().at(0);
        TowerRefSat = int(tower->isjTowerSat().at(0));
        TowerSat = int(dataTowerItr->second->isjTowerSat().at(0));
        if (TowerRefCount != TowerCount) {
            Decision = "CountMismatch";
            fill("errors", Decision, timeSince, timeUntil, evtNumber, lbnString, TowerId, TowerSource, Towereta,
                 Towerphi, TowerCount, TowerRefCount, TowerSat, TowerRefSat);
            ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb " << std::string(lbnString)
                                                  << " TowerId=" << TowerId << " Count=" << TowerCount << " RefCount="
                                                  << TowerRefCount << " Sat=" << TowerSat << " RefSat=" << TowerRefSat);
        }
        if (TowerRefSat != TowerSat) {
            Decision = "SatMismatch";
            fill("errors", Decision, timeSince, timeUntil, evtNumber, lbnString, TowerId, TowerSource, Towereta,
                 Towerphi, TowerCount, TowerRefCount, TowerSat, TowerRefSat);
            ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb " << std::string(lbnString)
                                                  << " TowerId=" << TowerId << " Count=" << TowerCount << " RefCount="
                                                  << TowerRefCount << " Sat=" << TowerSat << " RefSat=" << TowerRefSat);
        }
        // also log the locations of the saturated towers, invalidCodes, and empty codes
        // only relevant for LAr
        if (TowerSource != 1) { // (0=barrel, 1=tile, 2=emec, 3=hec, 4=fcal1, 5=fcal2, 6=fcal3)
            if (TowerSat) {
                fill(m_Grouphist, Towereta, Towerphi, TowerSat);
            }
            if (TowerCount == m_InvalidCode) {
                fill(m_Grouphist, Towereta, Towerphi, TowerInvalid);
            } else if (TowerCount == m_EmptyCode) {
                fill(m_Grouphist, Towereta, Towerphi, TowerEmpty);
            }
        }
    }

    return StatusCode::SUCCESS;

}
