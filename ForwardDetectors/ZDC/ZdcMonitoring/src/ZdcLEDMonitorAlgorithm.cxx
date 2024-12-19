/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
    ATH_CHECK( m_LEDPresampleADCKey.initialize(m_enableZDC || m_enableRPD) );
    ATH_CHECK( m_LEDADCSumKey.initialize(m_enableZDC || m_enableRPD) );
    ATH_CHECK( m_LEDMaxADCKey.initialize(m_enableZDC || m_enableRPD) );
    ATH_CHECK( m_LEDMaxSampleKey.initialize(m_enableZDC || m_enableRPD) );
    ATH_CHECK( m_LEDAvgTimeKey.initialize(m_enableZDC || m_enableRPD) );

    std::vector<std::string> sides = {"C","A"};
    std::vector<std::string> modules = {"0","1","2","3"};
    std::vector<std::string> channels = {"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"};

    if (m_enableZDC)    m_ZDCModuleLEDToolIndices = buildToolMap<std::map<std::string,std::map<std::string,int>>>(m_tools,"ZdcModLEDMonitor",m_LEDNames,sides,modules);
    if (m_enableRPD)    m_RPDChannelLEDToolIndices = buildToolMap<std::map<std::string,std::map<std::string,int>>>(m_tools,"RPDChanLEDMonitor",m_LEDNames,sides,channels);

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

// ______________________________________________________________________________
    // check for decoding errors
// ______________________________________________________________________________
    bool zdcDecodingError = eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::ZDCDECODINGERROR );
    bool rpdDecodingError = eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::RPDDECODINGERROR );
    std::array<float, m_nDecodingErrorBits> decodingErrorBitsArr = {0, 0, 0};
    if (!zdcDecodingError && !rpdDecodingError){
        decodingErrorBitsArr[0] += 1;
    } else if (zdcDecodingError){
        ATH_MSG_WARNING("ZDC Decoding error!");
        decodingErrorBitsArr[1] += 1;
    } else { // RPD decoding error
        ATH_MSG_WARNING("RPD Decoding error!");
        decodingErrorBitsArr[2] += 1;
    }
    
    auto decodingErrorBits = Monitored::Collection("decodingErrorBits", decodingErrorBitsArr);
    fill("ZdcLEDAllEventsDiagnosis", decodingErrorBits, lumiBlock);

    if (!m_enableZDC && !m_enableRPD){
        ATH_MSG_WARNING("Neither ZDC nor RPD are enabled! Quit LED histogram filling!");
    }

    if (zdcDecodingError && rpdDecodingError){
        ATH_MSG_WARNING("Both ZDC and RPD have decoding errors! Quit LED histogram filling!");
    }

// ______________________________________________________________________________
    // BCID
// ______________________________________________________________________________

    if (DAQMode == ZdcEventInfo::Standalone) {  
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, std::vector<uint16_t> > robBCIDHandle(m_robBCIDKey, ctx);
        if (!robBCIDHandle.isValid()) return StatusCode::FAILURE;

        const xAOD::ZdcModule* moduleSumEventInfo_ptr = 0;

        SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_ZdcSumContainerKey, ctx); // already checked in fillHistograms that zdcSums is valid
        for (const auto zdcSum : *zdcSums) { 
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

// ______________________________________________________________________________
    // LED type
// ______________________________________________________________________________

    unsigned int iLEDType = 1000;
    std::string led_type_str;
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> zdcLEDTypeHandle(m_LEDTypeKey, ctx);
    if (!zdcLEDTypeHandle.isAvailable()){
        ATH_MSG_WARNING("CANNOT find the variable " << m_LEDTypeKey << "!");
        return StatusCode::SUCCESS;
    } 

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_ZdcSumContainerKey, ctx); // already checked in fillHistograms that zdcSums is valid

    for (const auto zdcSum : *zdcSums) { 
        if (zdcSum->zdcSide() == 0){
            iLEDType = zdcLEDTypeHandle(*zdcSum);
            led_type_str = m_LEDNames[iLEDType];
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


// ______________________________________________________________________________
    // declaring & obtaining LED variables of interest for the ZDC modules & RPD channels
    // filling arrays of monitoring tools (module/channel-level)
// ______________________________________________________________________________

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules(m_ZdcModuleContainerKey, ctx);

    auto zdcLEDADCSum = Monitored::Scalar<int>("zdcLEDADCSum",-1000);
    auto zdcLEDMaxADC = Monitored::Scalar<int>("zdcLEDMaxADC",-1000);
    auto zdcLEDMaxADCtoADCSumRatio = Monitored::Scalar<int>("zdcLEDMaxADCtoADCSumRatio",-1000);
    auto zdcLEDMaxSample = Monitored::Scalar<unsigned int>("zdcLEDMaxSample",1000);
    auto zdcLEDAvgTime = Monitored::Scalar<float>("zdcLEDAvgTime",-1000);

    auto rpdLEDADCSum = Monitored::Scalar<int>("rpdLEDADCSum",-1000);
    auto rpdLEDMaxADC = Monitored::Scalar<int>("rpdLEDMaxADC",-1000);
    auto rpdLEDMaxADCtoADCSumRatio = Monitored::Scalar<int>("rpdLEDMaxADCtoADCSumRatio",-1000);
    auto rpdLEDMaxSample = Monitored::Scalar<unsigned int>("rpdLEDMaxSample",1000);
    auto rpdLEDAvgTime = Monitored::Scalar<float>("rpdLEDAvgTime",-1000);

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, int> LEDADCSumHandle(m_LEDADCSumKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, int> LEDMaxADCHandle(m_LEDMaxADCKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> LEDMaxSampleHandle(m_LEDMaxSampleKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> LEDAvgTimeHandle(m_LEDAvgTimeKey, ctx);


    if (! zdcModules.isValid()) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcModuleContainerKey);
       return StatusCode::SUCCESS;
    }

    if (! LEDADCSumHandle.isAvailable()){
       ATH_MSG_WARNING("LED aux data is not available");
       return StatusCode::SUCCESS;
    }

    for (const auto zdcMod : *zdcModules){
        int iside = (zdcMod->zdcSide() > 0)? 1 : 0;
        std::string side_str = (iside == 0)? "C" : "A";
    
        if (zdcMod->zdcType() == 0){ // zdc
            int imod = zdcMod->zdcModule();
            std::string module_str = std::to_string(imod);

            zdcLEDADCSum = LEDADCSumHandle(*zdcMod);
            zdcLEDMaxADC = LEDMaxADCHandle(*zdcMod);
            zdcLEDMaxSample = LEDMaxSampleHandle(*zdcMod);
            zdcLEDAvgTime = LEDAvgTimeHandle(*zdcMod);

            zdcLEDMaxADCtoADCSumRatio = (zdcLEDADCSum == 0)? -1000. : zdcLEDMaxADC * 1. / zdcLEDADCSum;

            fill(m_tools[m_ZDCModuleLEDToolIndices.at(led_type_str).at(side_str).at(module_str)], lumiBlock, bcid, zdcLEDADCSum, zdcLEDMaxADC, zdcLEDMaxSample, zdcLEDAvgTime, zdcLEDMaxADCtoADCSumRatio);
        } 
        else if (zdcMod->zdcType() == 1) { // rpd
            int ichannel = zdcMod->zdcChannel();
            std::string channel_str = std::to_string(ichannel);

            if (ichannel >= m_nChannels){
                ATH_MSG_WARNING("The current channel number exceeds the zero-based limit (15): it is " << ichannel);
                continue;
            }
            rpdLEDADCSum = LEDADCSumHandle(*zdcMod);
            rpdLEDMaxADC = LEDMaxADCHandle(*zdcMod);
            rpdLEDMaxSample = LEDMaxSampleHandle(*zdcMod);
            rpdLEDAvgTime = LEDAvgTimeHandle(*zdcMod);

            rpdLEDMaxADCtoADCSumRatio = (rpdLEDADCSum == 0)? -1000. : rpdLEDMaxADC * 1. / rpdLEDADCSum;

            fill(m_tools[m_RPDChannelLEDToolIndices.at(led_type_str).at(side_str).at(channel_str)], lumiBlock, bcid, rpdLEDADCSum, rpdLEDMaxADC, rpdLEDMaxSample, rpdLEDAvgTime, rpdLEDMaxADCtoADCSumRatio);
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
    for (const auto zdcSum : *zdcSums) { 
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

