/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcMonitoring/ZdcMonitorAlgorithm.h"
#include "ZdcAnalysis/ZDCPulseAnalyzer.h"
#include "ZdcAnalysis/RpdSubtractCentroidTool.h"
#include "ZdcAnalysis/RPDDataAnalyzer.h"

ZdcMonitorAlgorithm::ZdcMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
:AthMonitorAlgorithm(name,pSvcLocator){
    ATH_MSG_DEBUG("calling the constructor of ZdcMonitorAlgorithm");
}


ZdcMonitorAlgorithm::~ZdcMonitorAlgorithm() {}


void ZdcMonitorAlgorithm::calculate_log_bin_edges(float min_value, float max_value, int num_bins, std::vector<float>& bin_edges) {
    // Clear the vector to ensure it's empty
    bin_edges.clear();

    // Calculate the logarithmic bin edges
    float log_min = std::log10(min_value);
    float log_max = std::log10(max_value);
    
    // Linear space between log_min and log_max with num_bins+1 points
    float step = (log_max - log_min) / num_bins;

    // Populate the vector with the bin edges
    for (int i = 0; i <= num_bins; ++i) {
        float edge = log_min + i * step;
        bin_edges.push_back(std::pow(10, edge));
    }
}


float ZdcMonitorAlgorithm::calculate_inverse_bin_width(float event_value, std::string variable_name, const std::vector<float>& bin_edges) const {
    // Check if the event_value is out of range
    if (event_value < bin_edges.front() || event_value > bin_edges.back()) {
        ATH_MSG_WARNING("Warning: in calculation of inverse-bin-width event weight for the variable " << variable_name << ", the current event value " << event_value << " is out of the bin range.");
        ATH_MSG_WARNING("Assign zero weight for the current event (event not filled)."); 
        return 0.0; // event weight is zero
    }
    
    // Find the bin in which event_value falls
    for (size_t i = 0; i < bin_edges.size() - 1; ++i) {
        if (event_value >= bin_edges[i] && event_value < bin_edges[i + 1]) {
            float bin_width = bin_edges[i + 1] - bin_edges[i];
            if (bin_width != 0) {
                return 1.0f / bin_width; // Return the inverse of bin width
            } else {
                ATH_MSG_WARNING("Warning: in calculation of inverse-bin-width event weight for the variable " << variable_name << ", bin width containing the event value " << event_value << " is zero.");
                ATH_MSG_WARNING("Assign zero weight for the current event (event not filled)."); 
                return 0.0; // event weight is zero
            }
        }
    }

    // Handle edge case where event_value == bin_edges.back()
    if (event_value == bin_edges.back()) {
        size_t last_bin_index = bin_edges.size() - 2;
        float bin_width = bin_edges[last_bin_index + 1] - bin_edges[last_bin_index];
        return 1.0 / bin_width;
    }

    // If no bin is found (should not reach here)
    ATH_MSG_WARNING("Warning: in calculation of inverse-bin-width event weight for the variable " << variable_name << ", no valid bin found for the event value " << event_value << ".");
    ATH_MSG_WARNING("Assign zero weight for the current event (event not filled)."); 
    return 0.0; // event weight is zero
}


StatusCode ZdcMonitorAlgorithm::initialize() {

    ATH_MSG_DEBUG("initializing for the monitoring algorithm");
    ATH_MSG_DEBUG("Is injected pulse? " << m_isInjectedPulse);

    using namespace Monitored;
    ATH_CHECK( m_ZdcSumContainerKey.initialize() );
    ATH_CHECK( m_ZdcModuleContainerKey.initialize() );
    ATH_CHECK( m_HIEventShapeContainerKey.initialize() );
    
    ATH_CHECK( m_eventTypeKey.initialize() );
    // ATH_CHECK( m_ZdcBCIDKey.initialize() );
    ATH_CHECK( m_DAQModeKey.initialize() );

    ATH_CHECK( m_ZdcSumCalibEnergyKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcSumUncalibSumKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcSumAverageTimeKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcSumModuleMaskKey.initialize(m_enableZDC) );

    ATH_CHECK( m_ZdcModuleStatusKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleAmplitudeKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleTimeKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleFitT0Key.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleChisqKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleCalibEnergyKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleCalibTimeKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleMaxADCKey.initialize(m_enableZDC) );

    ATH_CHECK( m_ZdcModuleAmpLGRefitKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleT0LGRefitKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleT0SubLGRefitKey.initialize(m_enableZDC) );
    ATH_CHECK( m_ZdcModuleChisqLGRefitKey.initialize(m_enableZDC) );

    ATH_CHECK( m_RPDChannelAmplitudeKey.initialize(m_enableRPDAmp) );
    ATH_CHECK( m_RPDChannelAmplitudeCalibKey.initialize(m_enableRPDAmp) );
    ATH_CHECK( m_RPDChannelMaxADCKey.initialize(m_enableRPDAmp) );
    ATH_CHECK( m_RPDChannelMaxSampleKey.initialize(m_enableRPDAmp) );
    ATH_CHECK( m_RPDChannelStatusKey.initialize(m_enableRPDAmp) );
    ATH_CHECK( m_RPDChannelPileupExpFitParamsKey.initialize(m_enableRPDAmp) );
    ATH_CHECK( m_RPDChannelPileupFracKey.initialize(m_enableRPDAmp) );

    ATH_CHECK( m_RPDChannelSubtrAmpKey.initialize(m_enableCentroid) );
    ATH_CHECK( m_RPDSubtrAmpSumKey.initialize(m_enableCentroid) );
    ATH_CHECK( m_RPDxCentroidKey.initialize(m_enableCentroid) );
    ATH_CHECK( m_RPDyCentroidKey.initialize(m_enableCentroid) );
    ATH_CHECK( m_RPDreactionPlaneAngleKey.initialize(m_enableCentroid) );
    ATH_CHECK( m_RPDcosDeltaReactionPlaneAngleKey.initialize(m_enableCentroid) );
    ATH_CHECK( m_RPDcentroidStatusKey.initialize(m_enableCentroid) );
    ATH_CHECK( m_RPDSideStatusKey.initialize(m_enableCentroid) );

    // calculate log binnings
    calculate_log_bin_edges(m_moduleChisqHistMinValue, m_moduleChisqHistMaxvalue, m_moduleChisqHistNumBins, m_ZdcModuleChisqBinEdges);
    calculate_log_bin_edges(m_moduleChisqOverAmpHistMinValue, m_moduleChisqOverAmpHistMaxvalue, m_moduleChisqOverAmpHistNumBins, m_ZdcModuleChisqOverAmpBinEdges);

    // read json file for LB-to-injector-pulse-amplitude mapping and fill the mapping vector 
    m_zdcInjPulserAmpMap = std::make_shared<ZdcInjPulserAmpMap>();

    // create monitoring tools and map the strings to the tools
    std::vector<std::string> sides = {"C","A"};
    std::vector<std::string> modules = {"0","1","2","3"};
    std::vector<std::string> channels = {"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"};
    m_ZDCModuleToolIndices = buildToolMap<std::map<std::string,int>>(m_tools,"ZdcModuleMonitor",sides,modules);
    if (m_enableZDCPhysics || m_enableRPDAmp || m_enableCentroid){ // none is true for injector pulse --> no Per-side monitoring tool
        m_ZDCSideToolIndices = buildToolMap<int>(m_tools,"ZdcSideMonitor",sides);
    }
    if (m_enableRPDAmp){
        m_RPDChannelToolIndices = buildToolMap<std::map<std::string,int>>(m_tools,"RpdChannelMonitor",sides,channels);
    }

    //---------------------------------------------------
    // initialize superclass

    return AthMonitorAlgorithm::initialize();
    //---------------------------------------------------
    
}


StatusCode ZdcMonitorAlgorithm::fillPhysicsDataHistograms( const EventContext& ctx ) const {
    ATH_MSG_DEBUG("calling the fillPhysicsDataHistograms function");    

// ______________________________________________________________________________
    // declaring & obtaining event-level information of interest 
// ______________________________________________________________________________
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    // already checked in fillHistograms that eventInfo is valid
    auto lumiBlock = Monitored::Scalar<uint32_t>("lumiBlock", eventInfo->lumiBlock());
    auto bcid = Monitored::Scalar<unsigned int>("bcid", eventInfo->bcid());

    auto passTrigSideA = Monitored::Scalar<bool>("passTrigSideA",false); // if trigger isn't enabled (e.g, MC) the with-trigger histograms are never filled (cut mask never satisfied)
    auto passTrigSideC = Monitored::Scalar<bool>("passTrigSideC",false);

    if(m_enableTrigger){ // if not enable trigger, the pass-trigger booleans will still be defined but with value always set to false
        const auto &trigDecTool = getTrigDecisionTool();
        passTrigSideA = trigDecTool->isPassed(m_triggerSideA, TrigDefs::Physics);
        passTrigSideC = trigDecTool->isPassed(m_triggerSideC, TrigDefs::Physics);
        if (passTrigSideA) ATH_MSG_DEBUG("passing trig on side A!");    
        if (passTrigSideC) ATH_MSG_DEBUG("passing trig on side C!");    
    }


// ______________________________________________________________________________
    // declaring & obtaining variables of interest for the ZDC sums
    // including the RPD x,y positions, reaction plane and status
// ______________________________________________________________________________
    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_ZdcSumContainerKey, ctx);

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> ZdcSumCalibEnergyHandle(m_ZdcSumCalibEnergyKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> ZdcSumUncalibSumHandle(m_ZdcSumUncalibSumKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> ZdcSumAverageTimeHandle(m_ZdcSumAverageTimeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> ZdcSumModuleMaskHandle(m_ZdcSumModuleMaskKey, ctx);
       
    auto zdcEnergySumA = Monitored::Scalar<float>("zdcEnergySumA",-1000.0);
    auto zdcEnergySumC = Monitored::Scalar<float>("zdcEnergySumC",-1000.0);
    auto zdcUncalibSumA = Monitored::Scalar<float>("zdcUncalibSumA",-1000.0);
    auto zdcUncalibSumC = Monitored::Scalar<float>("zdcUncalibSumC",-1000.0);
    auto rpdCosDeltaReactionPlaneAngle = Monitored::Scalar<float>("rpdCosDeltaReactionPlaneAngle",-1000.0);
    auto bothReactionPlaneAngleValid = Monitored::Scalar<bool>("bothReactionPlaneAngleValid",true);
    auto bothHasCentroid = Monitored::Scalar<bool>("bothHasCentroid",true); // the looser requirement that both centroids were calculated (ignore valid)
    
    std::array<bool, 2> centroidSideValidArr;
    std::array<bool, 2> rpdSideValidArr = {false, false};
    std::array<std::vector<float>,2> rpdSubAmpVecs;
    auto rpdSubAmpSumCurSide = Monitored::Scalar<float>("rpdSubAmpSum",-1000.0);
    auto rpdXCentroidCurSide = Monitored::Scalar<float>("xCentroid",-1000.0);
    auto rpdYCentroidCurSide = Monitored::Scalar<float>("yCentroid",-1000.0);
    auto rpdReactionPlaneAngleCurSide = Monitored::Scalar<float>("ReactionPlaneAngle",-1000.0);
    auto centroidValid = Monitored::Scalar<bool>("centroidValid",false);
    auto centroidValidBitFloat = Monitored::Scalar<float>("centroidValidBitFloat", -1000.0); // 0.5 if valid, 1.5 if invalid --> needed for DQ
    auto passMinZDCEnergyCutForCentroidValidEvaluation = Monitored::Scalar<bool>("passMinZDCEnergyCutForCentroidValidEvaluation",false);

    // need to recognize same-side correlation among the following observables
    // since they are filled differently, it is helpful to store each of their values in the 2-dimension array first
    // and fill the side monitoring tool in the same "monitoring group"
    std::array<float, 2>    zdcEMModuleEnergyArr = {-100,-100};
    std::array<float, 2>    zdcEnergySumArr = {0,0};
    std::array<float, 2>    zdcUncalibSumArr = {0,0};
    std::array<float, 2>    zdcAvgTimeArr = {0,0};
    std::array<bool, 2>     zdcModuleMaskArr;
    std::array<bool, 2>     passTrigOppSideArr;
    std::array<float, 2>    rpdAmplitudeCalibSum = {0,0};
    std::array<float, 2>    rpdMaxADCSum = {0,0};

    std::array<float, m_nRpdCentroidStatusBits> centroidStatusBitsCountCurSide;

    if (! zdcSums.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcSumContainerKey);
       return StatusCode::SUCCESS;
    }

    // write ZDC per-arm information to arrays
    if (m_enableZDCPhysics){ // write down energy sum, uncalib sum, average time, and module mask if we enable ZDC physics
        for (const auto& zdcSum : *zdcSums) { // side -1: C; side 1: A
            if (zdcSum->zdcSide() != 0){
                int iside = (zdcSum->zdcSide() > 0)? 1 : 0;

                zdcEnergySumArr[iside] = ZdcSumCalibEnergyHandle(*zdcSum);
                zdcUncalibSumArr[iside] = ZdcSumUncalibSumHandle(*zdcSum);
                zdcAvgTimeArr[iside] = ZdcSumAverageTimeHandle(*zdcSum);
                zdcModuleMaskArr[iside] = ZdcSumModuleMaskHandle(*zdcSum);

                passTrigOppSideArr[iside] = (iside == 0)? passTrigSideA : passTrigSideC;
                
                if (zdcSum->zdcSide() == 1){
                    zdcEnergySumA = ZdcSumCalibEnergyHandle(*zdcSum);
                    zdcUncalibSumA = ZdcSumUncalibSumHandle(*zdcSum);
                } 
                else {
                    zdcEnergySumC = ZdcSumCalibEnergyHandle(*zdcSum);
                    zdcUncalibSumC = ZdcSumUncalibSumHandle(*zdcSum);
                }
            }
        } // having filled both sides
    } else if (m_enableZDC){ // enable ZDC but not physics - for now, the only case is injector pulse --> no energy, only record uncalib sum
        for (const auto& zdcSum : *zdcSums) { // side -1: C; side 1: A
            if (zdcSum->zdcSide() != 0){
                int iside = (zdcSum->zdcSide() > 0)? 1 : 0;
                zdcUncalibSumArr[iside] = ZdcSumUncalibSumHandle(*zdcSum);
            }
        }
    }

    // write RPD per-arm status to arrays
    if (m_enableRPDAmp){
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> RPDsideStatusHandle(m_RPDSideStatusKey, ctx);
        for (const auto& zdcSum : *zdcSums) { // side -1: C; side 1: A
            if (zdcSum->zdcSide() != 0){ // contains the RPD Cos Delta reaction plane
                int iside = (zdcSum->zdcSide() > 0)? 1 : 0;
                unsigned int rpdStatusCurSide = RPDsideStatusHandle(*zdcSum);
                rpdSideValidArr.at(iside) = rpdStatusCurSide & 1 << ZDC::RPDDataAnalyzer::ValidBit;
            }
        }
    }

    // fill RPD centroid information to monitoring tools
    if (m_enableCentroid){
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> RPDsubAmpHandle(m_RPDChannelSubtrAmpKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDsubAmpSumHandle(m_RPDSubtrAmpSumKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDxCentroidHandle(m_RPDxCentroidKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDyCentroidHandle(m_RPDyCentroidKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDreactionPlaneAngleHandle(m_RPDreactionPlaneAngleKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDcosDeltaReactionPlaneAngleHandle(m_RPDcosDeltaReactionPlaneAngleKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> RPDcentroidStatusHandle(m_RPDcentroidStatusKey, ctx);

        for (const auto& zdcSum : *zdcSums) { // side -1: C; side 1: A
            
            if (zdcSum->zdcSide() == 0){ // contains the RPD Cos Delta reaction plane
                rpdCosDeltaReactionPlaneAngle = RPDcosDeltaReactionPlaneAngleHandle(*zdcSum);
            }else{
                int iside = (zdcSum->zdcSide() > 0)? 1 : 0; // already exclude the possibility of global sum
                std::string side_str = (iside == 0)? "C" : "A";
                
                rpdSubAmpVecs[iside] = RPDsubAmpHandle(*zdcSum);
                rpdSubAmpSumCurSide = RPDsubAmpSumHandle(*zdcSum);
                rpdXCentroidCurSide = RPDxCentroidHandle(*zdcSum);
                rpdYCentroidCurSide = RPDyCentroidHandle(*zdcSum);
                rpdReactionPlaneAngleCurSide = RPDreactionPlaneAngleHandle(*zdcSum);
                
                unsigned int rpdCentroidStatusCurSide = RPDcentroidStatusHandle(*zdcSum);

                // Remarks - Oct 2024
                // HasCentroidBit is false if RPD on the current side is invalid
                // The centroid ValidBit, compared with Has HasCentroidBit, also checks that ZDC is valid
                // and has the infrastruture to require (1) ZDC total energy to be in given range
                // (2) EM-module energy to be in given range
                // (3) pile up fraction is below a threshold
                // but these are currently NOT implemented 
                // for online, we only monitor the ones requiring valid bit 
                // for offline, we plot both sets, with the expectation that they are the same for now
                centroidValid = (rpdCentroidStatusCurSide & 1 << ZDC::RpdSubtractCentroidTool::ValidBit);
                
                centroidValidBitFloat = (centroidValid)? 0.5 : 1.5;
            
                centroidSideValidArr.at(iside) = rpdCentroidStatusCurSide & 1 << ZDC::RpdSubtractCentroidTool::ValidBit;
                bool curSideHasCentroid = (rpdCentroidStatusCurSide & 1 << ZDC::RpdSubtractCentroidTool::HasCentroidBit);

                bothReactionPlaneAngleValid &= centroidValid;
                bothHasCentroid &= curSideHasCentroid;
                
                for (int bit = 0; bit < m_nRpdCentroidStatusBits; bit++) centroidStatusBitsCountCurSide[bit] = 0; // reset
                for (int bit = 0; bit < m_nRpdCentroidStatusBits; bit++){
                    if (rpdCentroidStatusCurSide & 1 << bit){
                        centroidStatusBitsCountCurSide[bit] += 1;
                    }
                }
                auto centroidStatusBits = Monitored::Collection("centroidStatusBits", centroidStatusBitsCountCurSide);

                if (curSideHasCentroid){ // only impose the looser requirement that this side has centroid; have a set of histograms for the more stringent centroid-valid requirement
                    if (m_enableZDCPhysics){ // if not enable ZDC physics, no ZDC energy --> the boolean requiring minimum ZDC energy will always be set to false
                        passMinZDCEnergyCutForCentroidValidEvaluation = (zdcEnergySumArr[iside] > m_ZDCEnergyCutForCentroidValidBitMonitor);
                    }
                    fill(m_tools[m_ZDCSideToolIndices.at(side_str)], rpdSubAmpSumCurSide, centroidValid, passMinZDCEnergyCutForCentroidValidEvaluation, centroidValidBitFloat, rpdXCentroidCurSide, rpdYCentroidCurSide, rpdReactionPlaneAngleCurSide, centroidStatusBits, lumiBlock, bcid);
                }else{
                    fill(m_tools[m_ZDCSideToolIndices.at(side_str)], rpdSubAmpSumCurSide, centroidStatusBits, lumiBlock, bcid);
                }
                
                if (zdcSum->zdcSide() == 1){
                    zdcEnergySumA = ZdcSumCalibEnergyHandle(*zdcSum);
                    zdcUncalibSumA = ZdcSumUncalibSumHandle(*zdcSum);
                }
                else {
                    zdcEnergySumC = ZdcSumCalibEnergyHandle(*zdcSum);
                    zdcUncalibSumC = ZdcSumUncalibSumHandle(*zdcSum);
                }
            }
        } // having filled both sides
    }

// ______________________________________________________________________________
    // declaring & obtaining variables of interest for the ZDC modules & RPD channels
    // filling arrays of monitoring tools (module/channel-level)
    // updating status bits
// ______________________________________________________________________________

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules(m_ZdcModuleContainerKey, ctx);

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> zdcModuleStatusHandle(m_ZdcModuleStatusKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleAmplitudeHandle(m_ZdcModuleAmplitudeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleTimeHandle(m_ZdcModuleTimeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleFitT0Handle(m_ZdcModuleFitT0Key, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleChisqHandle(m_ZdcModuleChisqKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleCalibEnergyHandle(m_ZdcModuleCalibEnergyKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleCalibTimeHandle(m_ZdcModuleCalibTimeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleMaxADCHandle(m_ZdcModuleMaxADCKey, ctx);
    
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleAmpLGRefitHandle(m_ZdcModuleAmpLGRefitKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleT0LGRefitHandle(m_ZdcModuleT0LGRefitKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleT0SubLGRefitHandle(m_ZdcModuleT0SubLGRefitKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleChisqLGRefitHandle(m_ZdcModuleChisqLGRefitKey, ctx);

    auto zdcModuleAmp = Monitored::Scalar<float>("zdcModuleAmp", -1000.0);
    auto zdcModuleMaxADC = Monitored::Scalar<float>("zdcModuleMaxADC", -1000.0);
    auto zdcModuleAmpToMaxADCRatio = Monitored::Scalar<float>("zdcModuleAmpToMaxADCRatio", -1000.0);
    auto zdcModuleFract = Monitored::Scalar<float>("zdcModuleFract", -1000.0);
    auto zdcUncalibSumCurrentSide = Monitored::Scalar<float>("zdcUncalibSumCurrentSide", -1000.0);
    auto zdcEnergySumCurrentSide = Monitored::Scalar<float>("zdcEnergySumCurrentSide", -1000.0);
    auto zdcAbove20NCurrentSide = Monitored::Scalar<bool>("zdcAbove20NCurrentSide", false);
    auto zdcEnergyAboveModuleFractCut = Monitored::Scalar<bool>("zdcEnergyAboveModuleFractCut", false);
    auto zdcModuleTime = Monitored::Scalar<float>("zdcModuleTime", -1000.0);
    auto zdcModuleFitT0 = Monitored::Scalar<float>("zdcModuleFitT0", -1000.0);
    auto zdcModuleChisq = Monitored::Scalar<float>("zdcModuleChisq", -1000.0);
    auto zdcModuleChisqEventWeight = Monitored::Scalar<float>("zdcModuleChisqEventWeight", -1000.0);
    auto zdcModuleChisqOverAmp = Monitored::Scalar<float>("zdcModuleChisqOverAmp", -1000.0);
    auto zdcModuleChisqOverAmpEventWeight = Monitored::Scalar<float>("zdcModuleChisqOverAmpEventWeight", -1000.0);
    auto zdcModuleCalibAmp = Monitored::Scalar<float>("zdcModuleCalibAmp", -1000.0);
    auto zdcModuleCalibTime = Monitored::Scalar<float>("zdcModuleCalibTime", -1000.0);
    auto zdcModuleLG = Monitored::Scalar<bool>("zdcModuleLG", false);
    auto zdcModuleHG = Monitored::Scalar<bool>("zdcModuleHG", false);
    auto zdcModuleHGValid = Monitored::Scalar<bool>("zdcModuleHGValid", false);
    auto injectedPulseInputVoltage = Monitored::Scalar<float>("injectedPulseInputVoltage", -1000.0);
    
    auto zdcModuleAmpLGRefit = Monitored::Scalar<float>("zdcModuleAmpLGRefit", -1000.0);
    auto zdcModuleAmpLGRefitTimes10 = Monitored::Scalar<float>("zdcModuleAmpLGRefitTimes10", -1000.0);
    auto zdcModuleT0LGRefit = Monitored::Scalar<float>("zdcModuleT0LGRefit", -1000.0);
    auto zdcModuleT0SubLGRefit = Monitored::Scalar<float>("zdcModuleT0SubLGRefit", -1000.0);
    auto zdcModuleChisqLGRefit = Monitored::Scalar<float>("zdcModuleChisqLGRefit", -1000.0);

    auto zdcModuleHGtoLGAmpRatio = Monitored::Scalar<float>("zdcModuleHGtoLGAmpRatio", -1000.0);
    auto zdcModuleHGtoLGT0Diff = Monitored::Scalar<float>("zdcModuleHGtoLGT0Diff", -1000.0);

    auto rpdChannelSubAmp = Monitored::Scalar<float>("RPDChannelSubAmp", -1000.0);
    auto rpdChannelAmplitude = Monitored::Scalar<float>("RPDChannelAmplitude", -1000.0);
    auto rpdChannelMaxADC = Monitored::Scalar<float>("RPDChannelMaxADC", -1000.0);
    auto rpdChannelMaxSample = Monitored::Scalar<unsigned int>("RPDChannelMaxSample", 1000);
    auto rpdChannelAmplitudeCalib = Monitored::Scalar<float>("RPDChannelAmplitudeCalib", -1000.0);
    auto rpdChannelStatus = Monitored::Scalar<unsigned int>("RPDChannelStatus", 1000);
    auto rpdChannelPileupFitSlope = Monitored::Scalar<float>("RPDChannelPileupFitSlope", -1000);
    auto absRpdChannelAmplitude = Monitored::Scalar<float>("absRPDChannelAmplitude", -1000.); // EM module energy on the same side (assuming filled already)
    auto rpdChannelValid = Monitored::Scalar<bool>("RPDChannelValid", false);
    auto rpdChannelValidBitFloat = Monitored::Scalar<float>("RPDChannelValidBitFloat", -1000.0); // 0.5 if valid, 1.5 if invalid --> needed for DQ
    auto rpdChannelCentroidValid = Monitored::Scalar<bool>("RPDChannelCentroidValid", false);
    auto rpdChannelPileupFrac = Monitored::Scalar<float>("RPDChannelPileupFrac", -1000.);
    auto zdcEMModuleEnergySameSide = Monitored::Scalar<float>("zdcEMModuleEnergySameSide", -1000.); // EM module energy on the same side (assuming filled already)
    auto zdcEnergySumSameSide = Monitored::Scalar<float>("zdcEnergySumSameSide", -1000.); // EM module energy on the same side (assuming filled already)

    std::array<float, m_nZdcStatusBits> zdcStatusBitsCount;
    std::array<float, m_nRpdStatusBits> rpdStatusBitsCount;
    
    if (! zdcModules.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcModuleContainerKey);
       return StatusCode::SUCCESS;
    }

    if (m_isInjectedPulse){
        if (m_isStandalone){
            injectedPulseInputVoltage = zdcModuleAmp * 1. / 25000.; // for filling dummy histograms - delete this line when LB-dependent function is in place
        }else{
            injectedPulseInputVoltage = m_zdcInjPulserAmpMap->getPulserAmplitude(lumiBlock);
            if (injectedPulseInputVoltage > 0){ // LB > startLB
                ATH_MSG_INFO("Lumi block: " << lumiBlock << "; pulser amplitude: " << injectedPulseInputVoltage);        
            }
        }
    }

    // first loop over zdcModules - read ZDC-module information & fill in ZDC histograms
    // separate ZDC and RPD variable retrieval into two for loops to make sure 
    // essential ZDC information (e.g, the EM module energy and total energy sum on both sides) is properly filled 
    // before they are required in RPD channel monitoring
    if (m_enableZDC){
        for (const auto zdcMod : *zdcModules){ 
            int iside = (zdcMod->zdcSide() > 0)? 1 : 0;
            std::string side_str = (iside == 0)? "C" : "A";

            if (zdcMod->zdcType() == 0){
                int imod = zdcMod->zdcModule();
                std::string module_str = std::to_string(imod);

                int status = zdcModuleStatusHandle(*zdcMod);
                
                for (int bit = 0; bit < m_nZdcStatusBits; bit++) zdcStatusBitsCount[bit] = 0; // reset
                for (int bit = 0; bit < m_nZdcStatusBits; bit++){
                    if (status & 1 << bit){
                        zdcStatusBitsCount[bit] += 1;
                    }
                }

                auto zdcStatusBits = Monitored::Collection("zdcStatusBits", zdcStatusBitsCount);
                fill(m_tools[m_ZDCModuleToolIndices.at(side_str).at(module_str)], zdcStatusBits, lumiBlock, bcid);

                if ((status & 1 << ZDCPulseAnalyzer::PulseBit) != 0){ // has pulse
                    zdcModuleAmp = zdcModuleAmplitudeHandle(*zdcMod);
                    zdcModuleMaxADC = zdcModuleMaxADCHandle(*zdcMod);
                    zdcModuleAmpToMaxADCRatio = (zdcModuleMaxADC == 0)? -1000. : zdcModuleAmp / zdcModuleMaxADC;
                    zdcModuleTime = zdcModuleTimeHandle(*zdcMod);
                    zdcModuleFitT0 = zdcModuleFitT0Handle(*zdcMod);
                    zdcModuleChisq = zdcModuleChisqHandle(*zdcMod);
                    zdcModuleCalibAmp = zdcModuleCalibEnergyHandle(*zdcMod);
                    zdcModuleCalibTime = zdcModuleCalibTimeHandle(*zdcMod);
                    zdcUncalibSumCurrentSide = zdcUncalibSumArr[iside];
                    zdcEnergySumCurrentSide = zdcEnergySumArr[iside];
                    zdcAbove20NCurrentSide = (zdcUncalibSumCurrentSide > 20 * m_expected1N);
                    zdcEnergyAboveModuleFractCut = (zdcEnergySumCurrentSide > m_energyCutForModuleFractMonitor);

                    if (m_enableZDCPhysics){
                        zdcModuleFract = (zdcEnergySumCurrentSide == 0)? -1000. : zdcModuleCalibAmp / zdcEnergySumCurrentSide; // use calibrated amplitudes + energy sum
                    }else{
                        zdcModuleFract = (zdcUncalibSumCurrentSide == 0)? -1000. : zdcModuleAmp / zdcUncalibSumCurrentSide; // use uncalibrated amplitudes + amplitude sum
                    }
                    zdcModuleChisqOverAmp = (zdcModuleAmp == 0)? -1000. : zdcModuleChisq / zdcModuleAmp;
                    zdcModuleLG = (status & 1 << ZDCPulseAnalyzer::LowGainBit);
                    zdcModuleHG = !(zdcModuleLG);
                    zdcModuleHGValid = !(status & 1 << ZDCPulseAnalyzer::HGOverflowBit) && !(status & 1 << ZDCPulseAnalyzer::HGUnderflowBit); // HG neither overflow nor underflow

                    zdcModuleAmpLGRefit = zdcModuleAmpLGRefitHandle(*zdcMod);
                    zdcModuleAmpLGRefitTimes10 = zdcModuleAmpLGRefit * 10.;
                    zdcModuleT0LGRefit = zdcModuleT0LGRefitHandle(*zdcMod);
                    zdcModuleT0SubLGRefit = zdcModuleT0SubLGRefitHandle(*zdcMod);
                    zdcModuleChisqLGRefit = zdcModuleChisqLGRefitHandle(*zdcMod);

                    zdcModuleHGtoLGAmpRatio = (!zdcModuleHGValid || zdcModuleAmpLGRefit == 0)? -1000. : zdcModuleAmp * 1. / zdcModuleAmpLGRefitTimes10; // HG/LG ratio if HG is valid and LG-refit amplitude is nonzero (shouldn't be)
                    zdcModuleHGtoLGT0Diff = (!zdcModuleHGValid)? -1000. : zdcModuleFitT0 - zdcModuleT0LGRefit;

                    zdcModuleChisqEventWeight = calculate_inverse_bin_width(zdcModuleChisq, "module chisq", m_ZdcModuleChisqBinEdges);
                    zdcModuleChisqOverAmpEventWeight = calculate_inverse_bin_width(zdcModuleChisqOverAmp, "module chisq over amplitude", m_ZdcModuleChisqOverAmpBinEdges);

                    if (imod == 0) zdcEMModuleEnergyArr[iside] = zdcModuleCalibAmp;

                    if (m_isInjectedPulse){
                        fill(m_tools[m_ZDCModuleToolIndices.at(side_str).at(module_str)], zdcModuleAmp, zdcModuleMaxADC, zdcModuleAmpToMaxADCRatio, zdcModuleFract, zdcUncalibSumCurrentSide, zdcEnergySumCurrentSide, zdcModuleTime, zdcModuleFitT0, zdcModuleChisq, zdcModuleChisqOverAmp, zdcModuleChisqEventWeight, zdcModuleChisqOverAmpEventWeight, zdcModuleCalibAmp, zdcModuleCalibTime, zdcModuleLG, zdcModuleHG, zdcModuleHGValid, zdcModuleAmpLGRefit, zdcModuleAmpLGRefitTimes10, zdcModuleT0LGRefit, zdcModuleT0SubLGRefit, zdcModuleChisqLGRefit, zdcModuleHGtoLGAmpRatio, zdcModuleHGtoLGT0Diff, injectedPulseInputVoltage, lumiBlock, bcid);
                    }else{
                        fill(m_tools[m_ZDCModuleToolIndices.at(side_str).at(module_str)], zdcModuleAmp, zdcModuleMaxADC, zdcModuleAmpToMaxADCRatio, zdcModuleFract, zdcUncalibSumCurrentSide, zdcEnergySumCurrentSide, zdcAbove20NCurrentSide, zdcEnergyAboveModuleFractCut, zdcModuleTime, zdcModuleFitT0, zdcModuleChisq, zdcModuleChisqOverAmp, zdcModuleChisqEventWeight, zdcModuleChisqOverAmpEventWeight, zdcModuleCalibAmp, zdcModuleCalibTime, zdcModuleLG, zdcModuleHG, zdcModuleHGValid, zdcModuleAmpLGRefit, zdcModuleAmpLGRefitTimes10, zdcModuleT0LGRefit, zdcModuleT0SubLGRefit, zdcModuleChisqLGRefit, zdcModuleHGtoLGAmpRatio, zdcModuleHGtoLGT0Diff, lumiBlock, bcid);
                    }
                }
            } 
        }
    }


    // second loop over zdcModules - read RPD-channel information & fill in RPD histograms
    // only run if NOT injector pulse
    if (m_enableRPDAmp){
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> RPDChannelStatusHandle(m_RPDChannelStatusKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDChannelAmplitudeHandle(m_RPDChannelAmplitudeKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDChannelMaxADCHandle(m_RPDChannelMaxADCKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> RPDChannelMaxSampleHandle(m_RPDChannelMaxSampleKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDChannelAmplitudeCalibHandle(m_RPDChannelAmplitudeCalibKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer,std::vector<float>> RPDChannelPileupExpFitParamsHandle(m_RPDChannelPileupExpFitParamsKey, ctx);
        SG::ReadDecorHandle<xAOD::ZdcModuleContainer,float> RPDChannelPileupFracHandle(m_RPDChannelPileupFracKey, ctx);

        for (const auto zdcMod : *zdcModules){
            int iside = (zdcMod->zdcSide() > 0)? 1 : 0;
            std::string side_str = (iside == 0)? "C" : "A";

            if (zdcMod->zdcType() == 1) {
                // this is the RPD

                int ichannel = zdcMod->zdcChannel(); // zero-based
                std::string channel_str = std::to_string(ichannel);

                int status = RPDChannelStatusHandle(*zdcMod);

                for (int bit = 0; bit < m_nRpdStatusBits; bit++) rpdStatusBitsCount[bit] = 0; // reset
                for (int bit = 0; bit < m_nRpdStatusBits; bit++){
                    if (status & 1 << bit){
                        rpdStatusBitsCount[bit] += 1;
                    }
                }

                auto rpdStatusBits = Monitored::Collection("RPDStatusBits", rpdStatusBitsCount);
                
                rpdChannelSubAmp = rpdSubAmpVecs[iside][ichannel];
                rpdChannelAmplitude = RPDChannelAmplitudeHandle(*zdcMod);
                rpdChannelMaxADC = RPDChannelMaxADCHandle(*zdcMod);
                rpdChannelMaxSample = RPDChannelMaxSampleHandle(*zdcMod);
                rpdChannelAmplitudeCalib = RPDChannelAmplitudeCalibHandle(*zdcMod);
                std::vector<float> rpdChannelPileupFitParams = RPDChannelPileupExpFitParamsHandle(*zdcMod);
                rpdChannelPileupFitSlope = rpdChannelPileupFitParams[1];
                rpdChannelPileupFrac = RPDChannelPileupFracHandle(*zdcMod);

                absRpdChannelAmplitude = abs(rpdChannelAmplitude);
                zdcEMModuleEnergySameSide = zdcEMModuleEnergyArr[iside];
                zdcEnergySumSameSide = zdcEnergySumArr[iside];
                bool curRpdChannelValid = status & 1 << ZDC::RPDDataAnalyzer::ValidBit;
                rpdChannelValid = curRpdChannelValid;
                rpdChannelValidBitFloat = (curRpdChannelValid)? 0.5 : 1.5;
                rpdChannelCentroidValid = centroidSideValidArr.at(iside);

                rpdAmplitudeCalibSum[iside] += rpdChannelAmplitudeCalib;
                rpdMaxADCSum[iside] += rpdChannelMaxADC;

                fill(m_tools[m_RPDChannelToolIndices.at(side_str).at(channel_str)], rpdChannelSubAmp, rpdChannelAmplitude, rpdChannelAmplitudeCalib, rpdChannelMaxADC, rpdChannelMaxSample, rpdStatusBits, rpdChannelPileupFitSlope, absRpdChannelAmplitude, rpdChannelPileupFrac, zdcEMModuleEnergySameSide, zdcEnergySumSameSide, rpdChannelValid, rpdChannelValidBitFloat, rpdChannelCentroidValid, lumiBlock, bcid);
            }
        }        
    }

    
// ______________________________________________________________________________
    // obtaining fCalEt on A,C side
// ______________________________________________________________________________

    auto fcalEtA = Monitored::Scalar<float>("fcalEtA", -1000.0);
    auto fcalEtC = Monitored::Scalar<float>("fcalEtC", -1000.0);
    std::array<float,2> fcalEtArr = {0.,0.};
    if (m_enableZDCPhysics){
        SG::ReadHandle<xAOD::HIEventShapeContainer> eventShapes(m_HIEventShapeContainerKey, ctx);
        if (! eventShapes.isValid()) {
            // we often don't expect calorimeter info to be on (e.g, when using the ZDC calibration stream)
            // only print out warning if we expect the calorimeter info to be on
            // not returning since the ZDC side-sum & module info is more important and yet need to be filled
            if (m_CalInfoOn) ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_HIEventShapeContainerKey);
        }
        else{
            ATH_MSG_DEBUG("Able to retrieve container "<< m_HIEventShapeContainerKey);
            ATH_MSG_DEBUG("Used to obtain fCalEtA, fCalEtC");

            for (const auto eventShape : *eventShapes){
                int layer = eventShape->layer();
                float eta = eventShape->etaMin();
                float et = eventShape->et();
                if (layer == 21 || layer == 22 || layer == 23){
                    if (eta > 0){
                        fcalEtA += et;
                        fcalEtArr[1] += et;
                    }
                    if (eta < 0){
                        fcalEtC += et;
                        fcalEtArr[0] += et;
                    }
                }
            }
        }
    }

// ______________________________________________________________________________
    // filling generic ZDC tool (within the same Monitored::Group, so that any possible correlation is recognized)
// ______________________________________________________________________________

    if (m_enableZDCPhysics){
        auto zdcTool = getGroup("genZdcMonTool"); // get the tool for easier group filling

        if (m_CalInfoOn){ // calorimeter information is turned on
            fill(zdcTool, lumiBlock, bcid, passTrigSideA, passTrigSideC, zdcEnergySumA, zdcEnergySumC, zdcUncalibSumA, zdcUncalibSumC, rpdCosDeltaReactionPlaneAngle, bothReactionPlaneAngleValid, bothHasCentroid, fcalEtA, fcalEtC);
        } else{
            fill(zdcTool, lumiBlock, bcid, passTrigSideA, passTrigSideC, zdcEnergySumA, zdcEnergySumC, zdcUncalibSumA, zdcUncalibSumC, rpdCosDeltaReactionPlaneAngle, bothReactionPlaneAngleValid, bothHasCentroid);
        }
    }

    if (m_enableZDCPhysics && m_enableRPDAmp){
        for (int iside = 0; iside < m_nSides; iside++){
            std::string side_str = (iside == 0)? "C" : "A";
            
            auto passTrigOppSide                = Monitored::Scalar<bool>("passTrigOppSide",passTrigOppSideArr[iside]); // this is duplicate information as A,C but convenient for filling per-side histograms
            auto zdcEnergySumCurSide            = Monitored::Scalar<float>("zdcEnergySum",zdcEnergySumArr[iside]); // this is duplicate information as A,C but convenient for filling per-side histograms
            auto zdcUncalibSumCurSide           = Monitored::Scalar<float>("zdcUncalibSum",zdcUncalibSumArr[iside]); // this is duplicate information as A,C but convenient for filling per-side histograms
            auto zdcEMModuleEnergyCurSide       = Monitored::Scalar<float>("zdcEMModuleEnergy",zdcEMModuleEnergyArr[iside]);
            auto zdcAvgTimeCurSide              = Monitored::Scalar<float>("zdcAvgTime",zdcAvgTimeArr[iside]);
            auto zdcModuleMaskCurSide           = Monitored::Scalar<bool>("zdcModuleMask",zdcModuleMaskArr[iside]);

            auto fcalEtCurSide                  = Monitored::Scalar<float>("fCalEt",fcalEtArr[iside]);

            auto rpdAmplitudeCalibSumCurSide    = Monitored::Scalar<float>("rpdAmplitudeCalibSum",rpdAmplitudeCalibSum[iside]);
            auto rpdMaxADCSumCurSide            = Monitored::Scalar<float>("rpdMaxADCSum",rpdMaxADCSum[iside]);
            auto rpdCurSideValid                = Monitored::Scalar<bool>("RPDSideValid",rpdSideValidArr[iside]);
            
            fill(m_tools[m_ZDCSideToolIndices.at(side_str)], passTrigOppSide, zdcEnergySumCurSide, zdcUncalibSumCurSide, zdcEMModuleEnergyCurSide, zdcAvgTimeCurSide, zdcModuleMaskCurSide, rpdAmplitudeCalibSumCurSide, rpdMaxADCSumCurSide, rpdCurSideValid, fcalEtCurSide, lumiBlock, bcid);
        }
    }else if (m_enableZDCPhysics){
        for (int iside = 0; iside < m_nSides; iside++){
            std::string side_str = (iside == 0)? "C" : "A";
            
            auto passTrigOppSide                = Monitored::Scalar<bool>("passTrigOppSide",passTrigOppSideArr[iside]); // this is duplicate information as A,C but convenient for filling per-side histograms
            auto zdcEnergySumCurSide            = Monitored::Scalar<float>("zdcEnergySum",zdcEnergySumArr[iside]); // this is duplicate information as A,C but convenient for filling per-side histograms
            auto zdcUncalibSumCurSide           = Monitored::Scalar<float>("zdcUncalibSum",zdcUncalibSumArr[iside]); // this is duplicate information as A,C but convenient for filling per-side histograms
            auto zdcEMModuleEnergyCurSide       = Monitored::Scalar<float>("zdcEMModuleEnergy",zdcEMModuleEnergyArr[iside]);
            auto zdcAvgTimeCurSide              = Monitored::Scalar<float>("zdcAvgTime",zdcAvgTimeArr[iside]);
            auto zdcModuleMaskCurSide           = Monitored::Scalar<bool>("zdcModuleMask",zdcModuleMaskArr[iside]);
            auto fcalEtCurSide                  = Monitored::Scalar<float>("fCalEt",fcalEtArr[iside]);

            fill(m_tools[m_ZDCSideToolIndices.at(side_str)], passTrigOppSide, zdcEnergySumCurSide, zdcUncalibSumCurSide, zdcEMModuleEnergyCurSide, zdcAvgTimeCurSide, zdcModuleMaskCurSide, fcalEtCurSide, lumiBlock, bcid);
        }
    }else if (m_enableRPDAmp){
        for (int iside = 0; iside < m_nSides; iside++){
            std::string side_str = (iside == 0)? "C" : "A";
            
            auto rpdAmplitudeCalibSumCurSide    = Monitored::Scalar<float>("rpdAmplitudeCalibSum",rpdAmplitudeCalibSum[iside]);
            auto rpdMaxADCSumCurSide            = Monitored::Scalar<float>("rpdMaxADCSum",rpdMaxADCSum[iside]);
            auto rpdCurSideValid                = Monitored::Scalar<bool>("RPDSideValid",rpdSideValidArr[iside]);
            
            fill(m_tools[m_ZDCSideToolIndices.at(side_str)], rpdAmplitudeCalibSumCurSide, rpdMaxADCSumCurSide, rpdCurSideValid, lumiBlock, bcid);
        }
    }

    return StatusCode::SUCCESS;
}


StatusCode ZdcMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("calling the fillHistograms function");

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    if (! eventInfo.isValid() ) {
        ATH_MSG_WARNING("cannot retrieve event info from evtStore()!");
        return StatusCode::SUCCESS;
    }
    
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

    if (eventType == ZdcEventInfo::ZdcEventPhysics || eventType == ZdcEventInfo::ZdcSimulation){
        return fillPhysicsDataHistograms(ctx);
    }
    
    ATH_MSG_WARNING("Event type should be PhysicsData/Simulation but it is NOT");
    return StatusCode::SUCCESS;
}

