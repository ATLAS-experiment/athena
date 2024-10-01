/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
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

    using namespace Monitored;
    ATH_CHECK( m_ZdcSumContainerKey.initialize() );
    ATH_CHECK( m_ZdcModuleContainerKey.initialize() );
    ATH_CHECK( m_HIEventShapeContainerKey.initialize() );
    
    ATH_CHECK( m_eventTypeKey.initialize() );
    // ATH_CHECK( m_ZdcBCIDKey.initialize() );
    ATH_CHECK( m_DAQModeKey.initialize() );

    ATH_CHECK( m_ZdcSumCalibEnergyKey.initialize() );
    ATH_CHECK( m_ZdcSumUncalibSumKey.initialize() );
    ATH_CHECK( m_ZdcSumAverageTimeKey.initialize() );
    ATH_CHECK( m_ZdcSumModuleMaskKey.initialize() );

    ATH_CHECK( m_ZdcModuleStatusKey.initialize() );
    ATH_CHECK( m_ZdcModuleAmplitudeKey.initialize() );
    ATH_CHECK( m_ZdcModuleTimeKey.initialize() );
    ATH_CHECK( m_ZdcModuleFitT0Key.initialize() );
    ATH_CHECK( m_ZdcModuleChisqKey.initialize() );
    ATH_CHECK( m_ZdcModuleCalibEnergyKey.initialize() );
    ATH_CHECK( m_ZdcModuleCalibTimeKey.initialize() );
    ATH_CHECK( m_ZdcModuleMaxADCKey.initialize() );

    ATH_CHECK( m_ZdcModuleAmpLGRefitKey.initialize() );
    ATH_CHECK( m_ZdcModuleT0LGRefitKey.initialize() );
    ATH_CHECK( m_ZdcModuleT0SubLGRefitKey.initialize() );
    ATH_CHECK( m_ZdcModuleChisqLGRefitKey.initialize() );

    ATH_CHECK( m_RPDrowKey.initialize() );
    ATH_CHECK( m_RPDcolKey.initialize() );
    ATH_CHECK( m_RPDChannelAmplitudeKey.initialize() );
    ATH_CHECK( m_RPDChannelAmplitudeCalibKey.initialize() );
    ATH_CHECK( m_RPDChannelMaxADCKey.initialize() );
    ATH_CHECK( m_RPDChannelStatusKey.initialize() );
    ATH_CHECK( m_RPDChannelPileupExpFitParamsKey.initialize() );
    ATH_CHECK( m_RPDChannelPileupFracKey.initialize() );

    ATH_CHECK( m_RPDChannelSubtrAmpKey.initialize() );
    ATH_CHECK( m_RPDSubtrAmpSumKey.initialize() );
    ATH_CHECK( m_RPDxCentroidKey.initialize() );
    ATH_CHECK( m_RPDyCentroidKey.initialize() );
    ATH_CHECK( m_RPDreactionPlaneAngleKey.initialize() );
    ATH_CHECK( m_RPDcosDeltaReactionPlaneAngleKey.initialize() );
    ATH_CHECK( m_RPDcentroidStatusKey.initialize() );
    ATH_CHECK( m_RPDSideStatusKey.initialize() );
        
    // calculate log binnings
    calculate_log_bin_edges(m_moduleChisqHistMinValue, m_moduleChisqHistMaxvalue, m_moduleChisqHistNumBins, m_ZdcModuleChisqBinEdges);
    calculate_log_bin_edges(m_moduleChisqOverAmpHistMinValue, m_moduleChisqOverAmpHistMaxvalue, m_moduleChisqOverAmpHistNumBins, m_ZdcModuleChisqOverAmpBinEdges);

    m_ZDCSideToolIndices = buildToolMap<int>(m_tools,"ZdcSideMonitor",m_nSides);
    m_ZDCModuleToolIndices = buildToolMap<std::vector<int>>(m_tools,"ZdcModuleMonitor",m_nSides,m_nModules);
    m_RPDChannelToolIndices = buildToolMap<std::vector<int>>(m_tools,"RpdChannelMonitor",m_nSides,m_nChannels);

    //---------------------------------------------------
    // initialize superclass

    return AthMonitorAlgorithm::initialize();
    //---------------------------------------------------
    
}


StatusCode ZdcMonitorAlgorithm::fillPhysicsDataHistograms( const EventContext& ctx ) const {
    ATH_MSG_DEBUG("calling the fillPhysicsDataHistograms function");    

    auto zdcTool = getGroup("genZdcMonTool"); // get the tool for easier group filling
// ______________________________________________________________________________
    // declaring & obtaining event-level information of interest 
// ______________________________________________________________________________
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    // already checked in fillHistograms that eventInfo is valid
    auto lumiBlock = Monitored::Scalar<uint32_t>("lumiBlock", eventInfo->lumiBlock());
    auto bcid = Monitored::Scalar<unsigned int>("bcid", eventInfo->bcid());

    auto passTrigSideA = Monitored::Scalar<bool>("passTrigSideA",false); // if trigger isn't enabled (e.g, MC) the with-trigger histograms are never filled (cut mask never satisfied)
    auto passTrigSideC = Monitored::Scalar<bool>("passTrigSideC",false);

    if(m_enableTrigger){
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
   
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> RPDsubAmpHandle(m_RPDChannelSubtrAmpKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDsubAmpSumHandle(m_RPDSubtrAmpSumKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDxCentroidHandle(m_RPDxCentroidKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDyCentroidHandle(m_RPDyCentroidKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDreactionPlaneAngleHandle(m_RPDreactionPlaneAngleKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDcosDeltaReactionPlaneAngleHandle(m_RPDcosDeltaReactionPlaneAngleKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> RPDcentroidStatusHandle(m_RPDcentroidStatusKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> RPDsideStatusHandle(m_RPDSideStatusKey, ctx);
    
    auto zdcEnergySumA = Monitored::Scalar<float>("zdcEnergySumA",-1000.0);
    auto zdcEnergySumC = Monitored::Scalar<float>("zdcEnergySumC",-1000.0);
    auto zdcUncalibSumA = Monitored::Scalar<float>("zdcUncalibSumA",-1000.0);
    auto zdcUncalibSumC = Monitored::Scalar<float>("zdcUncalibSumC",-1000.0);
    auto rpdCosDeltaReactionPlaneAngle = Monitored::Scalar<float>("rpdCosDeltaReactionPlaneAngle",-1000.0);
    auto bothReactionPlaneAngleValid = Monitored::Scalar<bool>("bothReactionPlaneAngleValid",true);
    auto bothHasCentroid = Monitored::Scalar<bool>("bothHasCentroid",true); // the looser requirement that both centroids were calculated (ignore valid)
    
    std::array<bool, 2> centroidSideValid;
    std::array<bool, 2> rpdSideValid = {false, false};
    std::array<std::vector<float>,2> rpdSubAmpVecs;
    auto zdcAvgTimeCurSide = Monitored::Scalar<float>("zdcAvgTime",-1000.0);
    auto zdcModuleMaskCurSide = Monitored::Scalar<bool>("zdcModuleMask",false);
    auto rpdSubAmpSumCurSide = Monitored::Scalar<float>("rpdSubAmpSum",-1000.0);
    auto rpdXCentroidCurSide = Monitored::Scalar<float>("xCentroid",-1000.0);
    auto rpdYCentroidCurSide = Monitored::Scalar<float>("yCentroid",-1000.0);
    auto rpdReactionPlaneAngleCurSide = Monitored::Scalar<float>("ReactionPlaneAngle",-1000.0);
    auto centroidValid = Monitored::Scalar<bool>("centroidValid",false);

    // need to recognize same-side correlation among the following observables
    // since they are filled differently, it is helpful to store each of their values in the 2-dimension array first
    // and fill the side monitoring tool in the same "monitoring group"
    std::array<float, 2> zdcEMModuleEnergy = {-100,-100};
    std::array<float, 2> zdcEnergySum = {0,0};
    std::array<float, 2> rpdAmplitudeCalibSum = {0,0};
    std::array<float, 2> rpdMaxADCSum = {0,0};

    std::array<float, m_nRpdCentroidStatusBits> centroidStatusBitsCountCurSide;

    if (! zdcSums.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcSumContainerKey);
       return StatusCode::SUCCESS;
    }
    else{
        for (const auto& zdcSum : *zdcSums) { // side -1: C; side 1: A
            
            if (zdcSum->zdcSide() == 0){ // contains the RPD Cos Delta reaction plane
                rpdCosDeltaReactionPlaneAngle = RPDcosDeltaReactionPlaneAngleHandle(*zdcSum);
            }else{
                int iside = (zdcSum->zdcSide() > 0)? 1 : 0; // already exclude the possibility of global sum
                
                zdcAvgTimeCurSide = ZdcSumAverageTimeHandle(*zdcSum);
                zdcModuleMaskCurSide = (ZdcSumModuleMaskHandle(*zdcSum) > 0);
                zdcEnergySum[iside] = ZdcSumCalibEnergyHandle(*zdcSum);

                rpdSubAmpVecs[iside] = RPDsubAmpHandle(*zdcSum);
                rpdSubAmpSumCurSide = RPDsubAmpSumHandle(*zdcSum);
                rpdXCentroidCurSide = RPDxCentroidHandle(*zdcSum);
                rpdYCentroidCurSide = RPDyCentroidHandle(*zdcSum);
                rpdReactionPlaneAngleCurSide = RPDreactionPlaneAngleHandle(*zdcSum);
                
                unsigned int rpdCentroidStatusCurSide = RPDcentroidStatusHandle(*zdcSum);
                unsigned int rpdStatusCurSide = RPDsideStatusHandle(*zdcSum);

                centroidValid = (rpdCentroidStatusCurSide & 1 << ZDC::RpdSubtractCentroidTool::ValidBit);
                centroidSideValid.at(iside) = rpdCentroidStatusCurSide & 1 << ZDC::RpdSubtractCentroidTool::ValidBit;
                rpdSideValid.at(iside) = rpdStatusCurSide & 1 << RPDDataAnalyzer::ValidBit;
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
                    fill(m_tools[m_ZDCSideToolIndices[iside]], zdcAvgTimeCurSide, zdcModuleMaskCurSide, rpdSubAmpSumCurSide, centroidValid, rpdXCentroidCurSide, rpdYCentroidCurSide, rpdReactionPlaneAngleCurSide, centroidStatusBits, lumiBlock, bcid);
                }else{
                    fill(m_tools[m_ZDCSideToolIndices[iside]], zdcAvgTimeCurSide, zdcModuleMaskCurSide, rpdSubAmpSumCurSide, centroidStatusBits, lumiBlock, bcid);
                }
                
                
                ATH_MSG_DEBUG("The current side is " << iside << ", the module mask is " << zdcModuleMaskCurSide);

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

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, uint16_t> RPDrowHandle(m_RPDrowKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, uint16_t> RPDcolHandle(m_RPDcolKey, ctx);

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> RPDChannelStatusHandle(m_RPDChannelStatusKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDChannelAmplitudeHandle(m_RPDChannelAmplitudeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDChannelMaxADCHandle(m_RPDChannelMaxADCKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDChannelAmplitudeCalibHandle(m_RPDChannelAmplitudeCalibKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer,std::vector<float>> RPDChannelPileupExpFitParamsHandle(m_RPDChannelPileupExpFitParamsKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer,float> RPDChannelPileupFracHandle(m_RPDChannelPileupFracKey, ctx);


    auto zdcModuleAmp = Monitored::Scalar<float>("zdcModuleAmp", -1000.0);
    auto zdcModuleMaxADC = Monitored::Scalar<float>("zdcModuleMaxADC", -1000.0);
    auto zdcModuleFract = Monitored::Scalar<float>("zdcModuleFract", -1000.0);
    auto zdcUncalibSumCurrentSide = Monitored::Scalar<float>("zdcUncalibSumCurrentSide", -1000.0);
    auto zdcAbove20NCurrentSide = Monitored::Scalar<bool>("zdcAbove20NCurrentSide", false);
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
    auto rpdChannelAmplitudeCalib = Monitored::Scalar<float>("RPDChannelAmplitudeCalib", -1000.0);
    auto rpdChannelStatus = Monitored::Scalar<unsigned int>("RPDChannelStatus", 1000);
    auto rpdChannelPileupFitSlope = Monitored::Scalar<float>("RPDChannelPileupFitSlope", -1000);
    auto absRpdChannelAmplitude = Monitored::Scalar<float>("absRPDChannelAmplitude", -1000.); // EM module energy on the same side (assuming filled already)
    auto rpdChannelValid = Monitored::Scalar<bool>("RPDChannelValid", false);
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


    for (const auto zdcMod : *zdcModules){ // separate ZDC and RPD variable retrieval into two for loops to make sure the EM module energy array is properly filled before being filled into RPD channel monitoring
        int iside = (zdcMod->zdcSide() > 0)? 1 : 0;
    
        if (zdcMod->zdcType() == 0){
            int imod = zdcMod->zdcModule();
            int status = zdcModuleStatusHandle(*zdcMod);
            
            for (int bit = 0; bit < m_nZdcStatusBits; bit++) zdcStatusBitsCount[bit] = 0; // reset
            for (int bit = 0; bit < m_nZdcStatusBits; bit++){
                if (status & 1 << bit){
                    zdcStatusBitsCount[bit] += 1;
                }
            }

            auto zdcStatusBits = Monitored::Collection("zdcStatusBits", zdcStatusBitsCount);
            fill(m_tools[m_ZDCModuleToolIndices[iside][imod]], zdcStatusBits, lumiBlock, bcid);

            if ((status & 1 << ZDCPulseAnalyzer::PulseBit) != 0){ // has pulse
                zdcModuleAmp = zdcModuleAmplitudeHandle(*zdcMod);
                zdcModuleMaxADC = zdcModuleMaxADCHandle(*zdcMod);
                zdcModuleTime = zdcModuleTimeHandle(*zdcMod);
                zdcModuleFitT0 = zdcModuleFitT0Handle(*zdcMod);
                zdcModuleChisq = zdcModuleChisqHandle(*zdcMod);
                zdcModuleCalibAmp = zdcModuleCalibEnergyHandle(*zdcMod);
                zdcModuleCalibTime = zdcModuleCalibTimeHandle(*zdcMod);
                zdcUncalibSumCurrentSide = (zdcMod->zdcSide() > 0)? 1. * zdcUncalibSumA : 1. * zdcUncalibSumC;
                zdcAbove20NCurrentSide = (zdcUncalibSumCurrentSide > 20 * m_expected1N);
                zdcModuleFract = (zdcUncalibSumCurrentSide == 0)? -1000. : zdcModuleAmp / zdcUncalibSumCurrentSide;
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

                if (imod == 0) zdcEMModuleEnergy[iside] = zdcModuleCalibAmp;

                fill(m_tools[m_ZDCModuleToolIndices[iside][imod]], zdcModuleAmp, zdcModuleMaxADC, zdcModuleFract, zdcUncalibSumCurrentSide, zdcAbove20NCurrentSide, zdcModuleTime, zdcModuleFitT0, zdcModuleChisq, zdcModuleChisqOverAmp, zdcModuleChisqEventWeight, zdcModuleChisqOverAmpEventWeight, zdcModuleCalibAmp, zdcModuleCalibTime, zdcModuleLG, zdcModuleHG, zdcModuleHGValid, zdcModuleAmpLGRefit, zdcModuleAmpLGRefitTimes10, zdcModuleT0LGRefit, zdcModuleT0SubLGRefit, zdcModuleChisqLGRefit, zdcModuleHGtoLGAmpRatio, zdcModuleHGtoLGT0Diff, lumiBlock, bcid);
            } 
        } 
    }

    ATH_MSG_DEBUG("After one loop over all zdc modules, the current values in zdcEMModuleEnergy is : " << zdcEMModuleEnergy[0] << ", " << zdcEMModuleEnergy[1]);

    for (const auto zdcMod : *zdcModules){
        int iside = (zdcMod->zdcSide() > 0)? 1 : 0;
        if (zdcMod->zdcType() == 1) {
            // this is the RPD

            int ichannel = zdcMod->zdcChannel(); // zero-based
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
            rpdChannelAmplitudeCalib = RPDChannelAmplitudeCalibHandle(*zdcMod);
            std::vector<float> rpdChannelPileupFitParams = RPDChannelPileupExpFitParamsHandle(*zdcMod);
            rpdChannelPileupFitSlope = rpdChannelPileupFitParams[1];
            rpdChannelPileupFrac = RPDChannelPileupFracHandle(*zdcMod);

            absRpdChannelAmplitude = abs(rpdChannelAmplitude);
            zdcEMModuleEnergySameSide = zdcEMModuleEnergy[iside];
            zdcEnergySumSameSide = zdcEnergySum[iside];
            bool curRpdChannelValid = status & 1 << RPDDataAnalyzer::ValidBit;
            rpdChannelValid = curRpdChannelValid;
            rpdChannelCentroidValid = centroidSideValid.at(iside);

            rpdAmplitudeCalibSum[iside] += rpdChannelAmplitudeCalib;
            rpdMaxADCSum[iside] += rpdChannelMaxADC;

            fill(m_tools[m_RPDChannelToolIndices[iside][ichannel]], rpdChannelSubAmp, rpdChannelAmplitude, rpdChannelAmplitudeCalib, rpdChannelMaxADC, rpdStatusBits, rpdChannelPileupFitSlope, absRpdChannelAmplitude, rpdChannelPileupFrac, zdcEMModuleEnergySameSide, zdcEnergySumSameSide, rpdChannelValid, rpdChannelCentroidValid, lumiBlock, bcid);
        }
    }
    
// ______________________________________________________________________________
    // obtaining fCalEt on A,C side
// ______________________________________________________________________________

    SG::ReadHandle<xAOD::HIEventShapeContainer> eventShapes(m_HIEventShapeContainerKey, ctx);
    auto fcalEtA = Monitored::Scalar<float>("fcalEtA", -1000.0);
    auto fcalEtC = Monitored::Scalar<float>("fcalEtC", -1000.0);
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
                if (eta > 0) fcalEtA += et;
                if (eta < 0) fcalEtC += et;
            }
        }
    }

// ______________________________________________________________________________
    // filling generic ZDC tool (within the same Monitored::Group, so that any possible correlation is recognized)
// ______________________________________________________________________________

    if (m_CalInfoOn){ // calorimeter information is turned on
        fill(zdcTool, lumiBlock, bcid, passTrigSideA, passTrigSideC, zdcEnergySumA, zdcEnergySumC, zdcUncalibSumA, zdcUncalibSumC, rpdCosDeltaReactionPlaneAngle, bothReactionPlaneAngleValid, bothHasCentroid, fcalEtA, fcalEtC);
    } else{
        fill(zdcTool, lumiBlock, bcid, passTrigSideA, passTrigSideC, zdcEnergySumA, zdcEnergySumC, zdcUncalibSumA, zdcUncalibSumC, rpdCosDeltaReactionPlaneAngle, bothReactionPlaneAngleValid, bothHasCentroid);
    }


    for (int iside = 0; iside < m_nSides; iside++){
        auto zdcEnergySumCurSide = Monitored::Scalar<float>("zdcEnergySum",zdcEnergySum[iside]); // this is duplicate information as A,C but convenient for filling per-side histograms
        auto zdcEMModuleEnergyCurSide = Monitored::Scalar<float>("zdcEMModuleEnergy",zdcEMModuleEnergy[iside]);
        auto rpdAmplitudeCalibSumCurSide = Monitored::Scalar<float>("rpdAmplitudeCalibSum",rpdAmplitudeCalibSum[iside]);
        auto rpdMaxADCSumCurSide = Monitored::Scalar<float>("rpdMaxADCSum",rpdMaxADCSum[iside]);
        auto rpdCurSideValid = Monitored::Scalar<bool>("RPDSideValid",rpdSideValid[iside]);
        fill(m_tools[m_ZDCSideToolIndices[iside]], zdcEnergySumCurSide, zdcEMModuleEnergyCurSide, rpdAmplitudeCalibSumCurSide, rpdMaxADCSumCurSide, rpdCurSideValid, lumiBlock, bcid);
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

