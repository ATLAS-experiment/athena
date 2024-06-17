/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcMonitoring/ZdcLEDMonitorAlgorithm.h"
#include "ZdcAnalysis/ZDCPulseAnalyzer.h"
#include "ZdcAnalysis/RpdSubtractCentroidTool.h"

ZdcLEDMonitorAlgorithm::ZdcLEDMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
:AthMonitorAlgorithm(name,pSvcLocator){
    ATH_MSG_DEBUG("calling the constructor of ZdcLEDMonitorAlgorithm");
}


ZdcLEDMonitorAlgorithm::~ZdcLEDMonitorAlgorithm() {}


StatusCode ZdcLEDMonitorAlgorithm::initialize() {

    ATH_MSG_DEBUG("initializing for the monitoring algorithm");

    using namespace Monitored;
    ATH_CHECK( m_ZdcSumContainerKey.initialize() );
    ATH_CHECK( m_ZdcModuleContainerKey.initialize() );
    
    ATH_CHECK( m_eventTypeKey.initialize() );
    ATH_CHECK( m_DAQModeKey.initialize() );

    ATH_CHECK( m_robBCIDKey.initialize() );

    ATH_CHECK( m_LEDTypeKey.initialize() );
    ATH_CHECK( m_LEDPresampleADCKey.initialize() );
    ATH_CHECK( m_LEDADCSumKey.initialize() );
    ATH_CHECK( m_LEDMaxADCKey.initialize() );
    ATH_CHECK( m_LEDMaxSampleKey.initialize() );
    ATH_CHECK( m_LEDAvgTimeKey.initialize() );

    m_ZDCModuleLEDToolIndices = buildToolMap<std::vector<std::vector<int>>>(m_tools,"ZdcModLEDMonitor",m_LEDNames.size(),m_nSides,m_nModules);
    m_RPDChannelLEDToolIndices = buildToolMap<std::vector<std::vector<int>>>(m_tools,"RPDChanLEDMonitor",m_LEDNames.size(),m_nSides,m_nChannels);

    //---------------------------------------------------
    // initialize superclass

    return AthMonitorAlgorithm::initialize();
    //---------------------------------------------------
    
}


StatusCode ZdcLEDMonitorAlgorithm::fillLEDHistograms(unsigned int DAQMode, const EventContext& ctx ) const {

    ATH_MSG_DEBUG("calling the fillLEDHistograms function");
// ______________________________________________________________________________
    // declaring & obtaining event-level information of interest 
// ______________________________________________________________________________

    // lumi block 
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx); // already checked in fillHistograms that eventInfo is valid
 
    auto lumiBlock = Monitored::Scalar<uint32_t>("lumiBlock", eventInfo->lumiBlock());
    auto bcid = Monitored::Scalar<unsigned int>("bcid", eventInfo->bcid());


    if (DAQMode == ZdcEventInfo::Standalone) {  
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, std::vector<uint16_t> > robBCIDHandle(m_robBCIDKey, ctx);
        if (!robBCIDHandle.isValid()) return StatusCode::FAILURE;

        const xAOD::ZdcModule* moduleSumEventInfo_ptr = 0;

        SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_ZdcSumContainerKey, ctx); // already checked in fillHistograms that zdcSums is valid
        for (const auto& zdcSum : *zdcSums) { 
            if (zdcSum->zdcSide() == 0){
                moduleSumEventInfo_ptr = zdcSum;
            }
        }
        
        const std::vector<uint16_t>& robBCIDvec = robBCIDHandle(*moduleSumEventInfo_ptr);
        if (robBCIDHandle->size() == 0) return StatusCode::FAILURE;

        unsigned int checkBCID = robBCIDvec[0];
        for (unsigned int bcid : robBCIDvec) {
            if (bcid != checkBCID) {
                ATH_MSG_ERROR("Inconsistent BCIDs in rob header, cannot continue in standalone mode");
                return StatusCode::FAILURE;
            }
        }

        bcid = checkBCID;
    }


    // LED type (event-level info saved in the glocal sum entry of zdcSums)
    unsigned int iLEDType = 1000;
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> zdcLEDTypeHandle(m_LEDTypeKey, ctx);
    if (!zdcLEDTypeHandle.isAvailable()){
        ATH_MSG_WARNING("CANNOT find the variable " << m_LEDTypeKey << "!");
        return StatusCode::SUCCESS;
    } 

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_ZdcSumContainerKey, ctx); // already checked in fillHistograms that zdcSums is valid

    for (const auto& zdcSum : *zdcSums) { 
        if (zdcSum->zdcSide() == 0){
            iLEDType = zdcLEDTypeHandle(*zdcSum);
        }
    }

    if (iLEDType == 1000){
        ATH_MSG_WARNING("The LED type is unretrieved!");
        return StatusCode::SUCCESS;
    } 
    if (iLEDType >= m_LEDNames.size()){
        ATH_MSG_WARNING("The retrieved LED type is incorrect (larger than 2)!");
        return StatusCode::SUCCESS;
    } 


    // trigger passed


// ______________________________________________________________________________
    // declaring & obtaining LED variables of interest for the ZDC modules & RPD channels
    // filling arrays of monitoring tools (module/channel-level)
// ______________________________________________________________________________

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules(m_ZdcModuleContainerKey, ctx);

    auto zdcLEDADCSum = Monitored::Scalar<int>("zdcLEDADCSum",-1000);
    auto zdcLEDMaxADC = Monitored::Scalar<int>("zdcLEDMaxADC",-1000);
    auto zdcLEDMaxSample = Monitored::Scalar<unsigned int>("zdcLEDMaxSample",1000);
    auto zdcLEDAvgTime = Monitored::Scalar<float>("zdcLEDAvgTime",-1000);

    auto rpdLEDADCSum = Monitored::Scalar<int>("rpdLEDADCSum",-1000);
    auto rpdLEDMaxADC = Monitored::Scalar<int>("rpdLEDMaxADC",-1000);
    auto rpdLEDMaxSample = Monitored::Scalar<unsigned int>("rpdLEDMaxSample",1000);
    auto rpdLEDAvgTime = Monitored::Scalar<float>("rpdLEDAvgTime",-1000);

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, int> LEDADCSumHandle(m_LEDADCSumKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, int> LEDMaxADCHandle(m_LEDMaxADCKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> LEDMaxSampleHandle(m_LEDMaxSampleKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> LEDAvgTimeHandle(m_LEDAvgTimeKey, ctx);


    if (! zdcModules.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcModuleContainerKey);
       return StatusCode::SUCCESS;
    }

    

    for (const auto zdcMod : *zdcModules){
        int iside = (zdcMod->zdcSide() > 0)? 1 : 0;
    
        if (zdcMod->zdcType() == 0){ // zdc
            int imod = zdcMod->zdcModule();
            zdcLEDADCSum = LEDADCSumHandle(*zdcMod);
            zdcLEDMaxADC = LEDMaxADCHandle(*zdcMod);
            zdcLEDMaxSample = LEDMaxSampleHandle(*zdcMod);
            zdcLEDAvgTime = LEDAvgTimeHandle(*zdcMod);

            fill(m_tools[m_ZDCModuleLEDToolIndices[iLEDType][iside][imod]], lumiBlock, bcid, zdcLEDADCSum, zdcLEDMaxADC, zdcLEDMaxSample, zdcLEDAvgTime);
        } 
        else if (zdcMod->zdcType() == 1) { // rpd
            int ichannel = zdcMod->zdcChannel();
            if (ichannel >= m_nChannels){
                ATH_MSG_WARNING("The current channel number exceeds the zero-based limit (15): it is " << ichannel);
                continue;
            }
            rpdLEDADCSum = LEDADCSumHandle(*zdcMod);
            rpdLEDMaxADC = LEDMaxADCHandle(*zdcMod);
            rpdLEDMaxSample = LEDMaxSampleHandle(*zdcMod);
            rpdLEDAvgTime = LEDAvgTimeHandle(*zdcMod);

            fill(m_tools[m_RPDChannelLEDToolIndices[iLEDType][iside][ichannel]], lumiBlock, bcid, rpdLEDADCSum, rpdLEDMaxADC, rpdLEDMaxSample, rpdLEDAvgTime);
        }
    }
    
    return StatusCode::SUCCESS;
}


StatusCode ZdcLEDMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("calling the fillHistograms function");

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    if (! eventInfo.isValid() ) {
        ATH_MSG_WARNING("cannot retrieve event info from evtStore()!");
        return StatusCode::SUCCESS;
    }
    
    auto bcid = Monitored::Scalar<unsigned int>("bcid", eventInfo->bcid());
    auto l1TriggerType = Monitored::Scalar<unsigned int>("l1TriggerType", eventInfo->level1TriggerType());
    auto lumiBlock = Monitored::Scalar<uint32_t>("lumiBlock", eventInfo->lumiBlock());

    // fill in the lumi block, BCID and L1 trigger type information without any check
    // for diagnosis of bad events that, e.g, fail the reconstruction
    fill("ZdcLEDAllEventsDiagnosis", lumiBlock, bcid, l1TriggerType);
    
    unsigned int eventType = ZdcEventInfo::ZdcEventUnknown;
    unsigned int DAQMode = ZdcEventInfo::DAQModeUndef;

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> eventTypeHandle(m_eventTypeKey,ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> DAQModeHandle(m_DAQModeKey,ctx);

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_ZdcSumContainerKey, ctx);

    if (! zdcSums.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcSumContainerKey);
       return StatusCode::SUCCESS;
    }
    for (const auto& zdcSum : *zdcSums) { 
        if (zdcSum->zdcSide() == 0){
            if (!eventTypeHandle.isAvailable()){
                ATH_MSG_WARNING("The global sum entry in zdc sum container can be retrieved; but it does NOT have the variable eventType written as a decoration!");
                return StatusCode::SUCCESS;
            } 

            if (!DAQModeHandle.isAvailable()){
                ATH_MSG_WARNING("The global sum entry in zdc sum container can be retrieved; but it does NOT have the variable DAQMode written as a decoration!");
                return StatusCode::SUCCESS;
            }

            eventType = eventTypeHandle(*zdcSum);
            DAQMode = DAQModeHandle(*zdcSum);
        }
    }

    ATH_MSG_DEBUG("The event type is: " << eventType);

    if (eventType == ZdcEventInfo::ZdcEventUnknown || DAQMode == ZdcEventInfo::DAQModeUndef){
        ATH_MSG_WARNING("The zdc sum container can be retrieved from the evtStore() but");
        ATH_MSG_WARNING("Either the event type or the DAQ mode is the default unknown value");
        ATH_MSG_WARNING("Most likely, there is no global sum (side == 0) entry in the zdc sum container");
        return StatusCode::SUCCESS;
    }

    if (eventType == ZdcEventInfo::ZdcEventLED){
        return fillLEDHistograms(DAQMode, ctx);
    }
    
    ATH_MSG_WARNING("Event type should be LED but it is NOT");
    return StatusCode::SUCCESS;
}

