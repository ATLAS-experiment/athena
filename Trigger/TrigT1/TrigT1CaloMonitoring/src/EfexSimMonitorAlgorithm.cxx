/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "EfexSimMonitorAlgorithm.h"
#include "eFEXTOBSimDataCompare.h"
#include <set>
#include "StoreGate/ReadDecorHandle.h"

EfexSimMonitorAlgorithm::EfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode EfexSimMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("EfexSimMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  ATH_MSG_DEBUG("m_eFexEmContainer"<< m_eFexEmContainerKey); 
  ATH_MSG_DEBUG("m_eFexEmSimContainer"<< m_eFexEmSimContainerKey); 
  ATH_MSG_DEBUG("m_eFexTauContainer"<< m_eFexTauContainerKey);
  ATH_MSG_DEBUG("m_eFexTauSimContainer"<< m_eFexTauSimContainerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_eFexEmContainerKey.initialize() );
  ATH_CHECK( m_eFexEmSimContainerKey.initialize() );
  ATH_CHECK( m_eFexTauContainerKey.initialize() );
  ATH_CHECK( m_eFexTauSimContainerKey.initialize() );
  //m_decorKey = "EventInfo.eTowerMakerFromEfexTowers_usedSecondary";
  //ATH_CHECK( m_decorKey.initialize() );
  ATH_CHECK( m_eFexTowerContainerKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_bcContKey.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}

StatusCode EfexSimMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    // check flag that indicates if simulation was done with fexReadout (primary) or not (secondary)
    // SG::ReadDecorHandle<xAOD::EventInfo,bool> usedSecondaryDecor(m_decorKey,ctx);
    // auto fexReadout = Monitored::Scalar<unsigned int>("fexReadout", !(usedSecondaryDecor.isAvailable() && usedSecondaryDecor(*GetEventInfo(ctx))));

    auto fexReadout = Monitored::Scalar<unsigned int>("fexReadout", 0);
    if(!m_eFexTowerContainerKey.empty()) {
        SG::ReadHandle<xAOD::eFexTowerContainer> towers{m_eFexTowerContainerKey, ctx};
        if(towers.isValid() && !towers->empty()) {
            fexReadout = 1;
        }
    }
    auto IsDataTowers = Monitored::Scalar<bool>("IsDataTowers",fexReadout==1);
    auto IsEmulatedTowers = Monitored::Scalar<bool>("IsEmulatedTowers",fexReadout==0);

    // mismatches can be caused by recent/imminent OTF maskings, so track timings
    auto timeSince = Monitored::Scalar<int>("timeSince", -1);
    auto timeUntil = Monitored::Scalar<int>("timeUntil", -1);
    SG::ReadCondHandle<LArBadChannelCont> larBadChan{ m_bcContKey, ctx };
    if(larBadChan.isValid()) {
        timeSince = ctx.eventID().time_stamp() - larBadChan.getRange().start().time_stamp();
        timeUntil = larBadChan.getRange().stop().time_stamp() - ctx.eventID().time_stamp();
    }

    std::string EventType = "DataTowers";
    if(fexReadout==0) {
        EventType = "EmulatedTowers";
        if((timeSince>=0&&timeSince<10)) EventType+="+JustAfter";
        else if((timeUntil>=0&&timeUntil<10)) EventType+="+JustBefore";
    }


    unsigned int nUnmatched_em = 0;
    nUnmatched_em += fillHistos(m_eFexEmSimContainerKey,m_eFexEmContainerKey,EventType,ctx,"eEM"); // match sim to data
    unsigned int nUnmatched_tau = 0;
    nUnmatched_tau += fillHistos(m_eFexTauSimContainerKey,m_eFexTauContainerKey,EventType,ctx,"eTAU"); // match sim to data


    if( (nUnmatched_em || nUnmatched_tau) ) {
        // record all tobs to the debug tree .. one entry in the tree = 1 tobType for 1 event
        auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",GetEventInfo(ctx)->eventNumber());
        auto lbn = Monitored::Scalar<ULong64_t>("LBN",GetEventInfo(ctx)->lumiBlock());
        auto lbnString = Monitored::Scalar<std::string>("LBNString","");
        auto& firstEvents = (fexReadout) ? m_firstEvents_DataTowers : m_firstEvents_EmulatedTowers;
        {
            std::scoped_lock lock(m_firstEventsMutex);
            auto itr = firstEvents.find(lbn);
            if(itr==firstEvents.end()) {
                firstEvents[lbn] = std::to_string(lbn)+":"+std::to_string(evtNumber);
                itr = firstEvents.find(lbn);
            }
            lbnString = itr->second;
        }

        std::vector<float> detas{};std::vector<float> setas{};
        std::vector<float> dphis{};std::vector<float> sphis{};
        std::vector<unsigned int> dword0s{};std::vector<unsigned int> sword0s{};
        auto dtobEtas = Monitored::Collection("dataEtas", detas);
        auto dtobPhis = Monitored::Collection("dataPhis", dphis);
        auto dtobWord0s = Monitored::Collection("dataWord0s", dword0s);
        auto stobEtas = Monitored::Collection("simEtas", setas);
        auto stobPhis = Monitored::Collection("simPhis", sphis);
        auto stobWord0s = Monitored::Collection("simWord0s", sword0s);

        auto tobType = Monitored::Scalar<unsigned int>("tobType",0);
        auto signature = Monitored::Scalar<std::string>("Signature","eEM");

        if(nUnmatched_em) {
            fillVectors(m_eFexEmContainerKey,ctx,detas,dphis,dword0s);
            fillVectors(m_eFexEmSimContainerKey,ctx,setas,sphis,sword0s);
            fill("mismatches",lbn,lbnString,evtNumber,tobType,dtobEtas,dtobPhis,dtobWord0s,stobEtas,stobPhis,stobWord0s,fexReadout,timeSince,timeUntil,IsDataTowers,IsEmulatedTowers,signature);
        }
        if(nUnmatched_tau) {
            tobType = 1; signature = "eTAU";
            fillVectors(m_eFexTauContainerKey,ctx,detas,dphis,dword0s);
            fillVectors(m_eFexTauSimContainerKey,ctx,setas,sphis,sword0s);
            fill("mismatches",lbn,lbnString,evtNumber,tobType,dtobEtas,dtobPhis,dtobWord0s,stobEtas,stobPhis,stobWord0s,fexReadout,timeSince,timeUntil,IsDataTowers,IsEmulatedTowers,signature);
        }


    }
  return StatusCode::SUCCESS;
}
