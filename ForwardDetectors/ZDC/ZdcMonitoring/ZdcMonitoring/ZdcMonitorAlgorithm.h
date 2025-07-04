/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCMONITORALGORITHM_H
#define ZDCMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TRandom3.h"
#include "array"

#include "CoolKernel/IObject.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

//---------------------------------------------------
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include <xAODTrigger/TrigDecision.h>
//---------------------------------------------------
#include "ZdcUtils/ZdcEventInfo.h"
#include "ZdcConditions/ZdcInjPulserAmpMap.h"

class ZdcMonitorAlgorithm : public AthMonitorAlgorithm {
public:
    enum DecodingErrors{
        NoDecodingErrorBit  = 0,
        ZDCDecodingErrorBit = 1,
        RPDDecodingErrorBit = 2
    };
    enum UCCTriggers{
        UCCTrigEnabledBit   = 0,
        TrigHELT50Bit       = 1,
        TrigHELT35Bit       = 2,
        TrigHELT25Bit       = 3,
        TrigHELT20Bit       = 4,
        TrigHELT15Bit       = 5,
        UCCTrigDisabledBit  = 6
    };

    ZdcMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~ZdcMonitorAlgorithm();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
    StatusCode fillPhysicsDataHistograms( const EventContext& ctx ) const;

private:
    bool check_equal_within_rounding(float a, float b, float epsilon = 1e-6f) const;
    void calculate_log_bin_edges(float min_value, float max_value, int num_bins, std::vector<float>& bin_edges);
    float calculate_inverse_bin_width(float event_value, const std::string& variable_name, const std::vector<float>& bin_edges) const;

    Gaudi::Property<unsigned int> m_runNumber {this, "RunNumber", 0, "Run number for current job"};
    ZdcInjPulserAmpMap::Token m_injMapRunToken{};
    
    Gaudi::Property<std::string> m_zdcModuleContainerName {this, "ZdcModuleContainerName", "ZdcModules", "Location of ZDC processed data"};
    Gaudi::Property<std::string> m_zdcSumContainerName {this, "ZdcSumContainerName", "ZdcSums", "Location of ZDC processed sums"};
    Gaudi::Property<std::string> m_auxSuffix{this, "AuxSuffix", "", "Append this tag onto end of AuxData"};
    
    Gaudi::Property<float> m_expected1N{this, "Expected1NADC", 1000., "Expected 1N position in ADC"}; // only needs to indicate the rough scale, only used in the >20N cut mask
    
    Gaudi::Property<float> m_energyCutForModuleFractMonitor{this, "EnergyCutForModuleFractMonitor", 13400., "Minimum energy sum required on each side for event to enter module fraction monitoring (default at 5 * 2680 GeV)"};
    Gaudi::Property<float> m_ZDCEnergyCutForCentroidValidBitMonitor{this, "ZDCEnergyCutForCentroidValidBitMonitor", 13400., "Minimum energy required in ZDC for event to enter centroid valid bit monitoring (default at 5 * 2680 GeV)"};
    
    Gaudi::Property<float> m_moduleChisqHistMinValue{this, "ZDCModuleChisqHistMinValue", 1000., "Min value for logarithmic binning for ZDC module chisq distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqHistMaxvalue{this, "ZDCModuleChisqHistMaxvalue", 1000., "Max value for logarithmic binning for ZDC module chisq distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqHistNumBins{this, "ZDCModuleChisqHistNumBins", 1000., "Number of bins for logarithmic binning for ZDC module chisq distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqOverAmpHistMinValue{this, "ZDCModuleChisqOverAmpHistMinValue", 1000., "Min value for logarithmic binning for ZDC module chisq over amplitude distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqOverAmpHistMaxvalue{this, "ZDCModuleChisqOverAmpHistMaxvalue", 1000., "Max value for logarithmic binning for ZDC module chisq over amplitude distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqOverAmpHistNumBins{this, "ZDCModuleChisqOverAmpHistNumBins", 1000., "Number of bins for logarithmic binning for ZDC module chisq over amplitude distribution"}; // to manually calculate inverse-bin-width weight

    // single side triggers - less error-prone if defined as separate properties then in a vector (where order would be crucial)
    Gaudi::Property<std::string> m_triggerSideA{this, "triggerSideA", "L1_ZDC_A", "Trigger on side A, needed for 1N-peak monitoring on side C"};
    Gaudi::Property<std::string> m_triggerSideC{this, "triggerSideC", "L1_ZDC_C", "Trigger on side C, needed for 1N-peak monitoring on side A"};
    
    // ZDC UCC trigger - ultra-central event selection requiring energy sum in ZDC hadronic modules be below different thresholds
    Gaudi::Property<std::string> m_UCCtriggerHELT15{this, "triggerUCCHELT15", "L1_ZDC_HELT15_jTE4000", "UCC trigger requiring ZDC hadronic energy be less than 15 TeV"};
    Gaudi::Property<std::string> m_UCCtriggerHELT20{this, "triggerUCCHELT20", "L1_ZDC_HELT20_jTE4000", "UCC trigger requiring ZDC hadronic energy be less than 20 TeV"};
    Gaudi::Property<std::string> m_UCCtriggerHELT25{this, "triggerUCCHELT25", "L1_ZDC_HELT25_jTE4000", "UCC trigger requiring ZDC hadronic energy be less than 25 TeV"};
    Gaudi::Property<std::string> m_UCCtriggerHELT35{this, "triggerUCCHELT35", "L1_ZDC_HELT35_jTE4000", "UCC trigger requiring ZDC hadronic energy be less than 35 TeV"};
    Gaudi::Property<std::string> m_UCCtriggerHELT50{this, "triggerUCCHELT50", "L1_ZDC_HELT50_jTE4000", "UCC trigger requiring ZDC hadronic energy be less than 50 TeV"};
    
    float m_timingCutsInjectorPulse [2][4][2] = {{{30, 38}, {30, 38}, {28, 38}, {30, 38}}, {{30, 38}, {30, 38}, {30, 38}, {30, 38}}}; // Timing cuts (array of dimension 2 * 4 * 2) for event to enter reco-amp-vs-input-voltage histograms in the injector pulse stream
    Gaudi::Property<unsigned int> m_nSecondsRejectStartofLBInjectorPulse {this, "NSecondsRejectStartofLBInjectorPulse", 3, "The number of seconds to reject at beginning of each LB in reco-amp-vs-input-voltage histograms in the injector pulse stream"};
    Gaudi::Property<float> m_minAmpRequiredHGInjectorPulse {this, "MinAmpRequiredHGInjectorPulse", 20, "HG Minimum amplitude required for event to enter reco-amp-vs-input-voltage histograms in the injector pulse stream"};
    Gaudi::Property<float> m_minAmpRequiredLGInjectorPulse {this, "MinAmpRequiredLGInjectorPulse", 20, "LG Minimum amplitude required for event to enter reco-amp-vs-input-voltage histograms in the injector pulse stream"};
    Gaudi::Property<float> m_minVInjToImposeAmpRequirementHGInjectorPulse {this, "MinVInjToImposeAmpRequirementHGInjectorPulse", 0.002, "Minimum input voltage to impose HG minimum amplitude requirement in the injector pulse stream; set to negative value to cancel HG minimum-amplitude requirement"};
    Gaudi::Property<float> m_minVInjToImposeAmpRequirementLGInjectorPulse {this, "MinVInjToImposeAmpRequirementLGInjectorPulse", 0.002, "Minimum input voltage to impose LG minimum amplitude requirement in the injector pulse stream; set to negative value to cancel LG minimum-amplitude requirement"};

    Gaudi::Property<std::vector<std::string>> m_OOpOtriggerChains {this, "OOpOTriggers", {}, "List of trigger chains to monitor"};
    Gaudi::Property<std::map<int,std::string>> m_OOpOL1TriggerFromCTPIDMap {this, "OOpOL1TriggerFromCTPIDMap", {}, "Map of CTP ID to trigger name for ZdcCalib PEB stream pO/OO monitoring"};

    Gaudi::Property<std::string > m_lbTimeCoolFolderName{ this, "LumiBlockTimeCoolFolderName", "/TRIGGER/LUMI/LBLB", "COOL folder in COOLONL_TRIGGER holding info about start and stop times for luminosity blocks" };


    static const int m_nSides = 2;
    static const int m_nModules = 4;
    static const int m_nChannels = 16;
    static const int m_nDecodingErrorBits = 3;
    static const int m_nUCCTrigBits = 7;
    static const int m_nZdcStatusBits = 18;
    static const int m_nRpdStatusBits = 15;
    static const int m_nRpdCentroidStatusBits = 21;

    // the 2D mapping maps a string "pair" to an integer: the index of the corresponding generic monitoring tool (GMT) in the array of all GMT's
    std::map<std::string,int> m_ZDCSideToolIndices;
    std::map<std::string,std::map<std::string,int>> m_ZDCModuleToolIndices;
    std::map<std::string,std::map<std::string,int>> m_RPDChannelToolIndices;
    std::map<std::string,std::map<std::string,std::map<std::string,int>>> m_LucrodResponseSingleVoltageToolIndices;

    std::vector<float> m_ZdcModuleChisqBinEdges;
    std::vector<float> m_ZdcModuleChisqOverAmpBinEdges;

    std::shared_ptr<ZdcInjPulserAmpMap> m_zdcInjPulserAmpMap;

    //---------------------------------------------------
    
    // see the standalone version of the Gaudi::Property class (a wrapper in AsgTools) at
    // athena/Control/AthToolSupport/AsgTools/AsgTools/PropertyWrapper.h
    // input to constructor: owner, name, value, title = "" (by default)
    Gaudi::Property<bool> m_isOnline {this,"IsOnline",false};
    Gaudi::Property<bool> m_isSim {this,"IsSim",false}; // is simulation
    Gaudi::Property<bool> m_CalInfoOn {this,"CalInfoOn",false};
    Gaudi::Property<bool> m_TrkInfoOn {this,"TrkInfoOn",false};
    Gaudi::Property<bool> m_EnableZDCSingleSideTriggers {this,"EnableZDCSingleSideTriggers",true};
    Gaudi::Property<bool> m_EnableUCCTriggers {this,"EnableUCCTriggers",false};
    Gaudi::Property<bool> m_EnableOOpOTriggers {this,"EnableOOpOTriggers",false};
    Gaudi::Property<bool> m_IsPEBStream {this,"IsPEBStream",true};
    Gaudi::Property<bool> m_isPPMode {this,"IsPPMode",true};
    Gaudi::Property<bool> m_ispOMode {this,"IspOMode",true};
    Gaudi::Property<bool> m_isOOMode {this,"IsOOMode",true};
    Gaudi::Property<bool> m_isInjectedPulse {this,"IsInjectedPulse",false};
    Gaudi::Property<bool> m_isStandalone {this,"IsStandalone",false}; // determine if standalone via metadata
    Gaudi::Property<bool> m_enableZDC {this,"EnableZDC",true};
    Gaudi::Property<bool> m_enableZDCPhysics {this,"EnableZDCPhysics",true};
    Gaudi::Property<bool> m_enableRPD {this,"EnableRPD",true};
    Gaudi::Property<bool> m_enableRPDAmp {this,"EnableRPDAmp",true};
    Gaudi::Property<bool> m_enableCentroid {this,"EnableCentroid",true};
    
    Gaudi::Property<std::vector<float>> m_injPulseVoltageSteps {this, "InjPulseVoltageSteps", {0.}};
    Gaudi::Property<std::vector<std::string>> m_injPulseVoltageStepsStr {this, "InjPulseVoltageStepsStr", {""}};

    // owner, name (allows us to modify the key in python configuration), key
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumContainerKey {this, "ZdcSumContainerKey", "ZdcSums"};
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleContainerKey {this, "ZdcModuleContainerKey", "ZdcModules"};
    SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_HIEventShapeContainerKey {this, "HIEventShapeContainerKey", "HIEventShape"};
    
    SG::ReadCondHandleKey<AthenaAttributeList> m_LBLBFolderInputKey{ this, "LBLBFolderInputKey", "/TRIGGER/LUMI/LBLB" };

    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_eventTypeKey {this, "ZdcEventTypeKey", m_zdcSumContainerName + ".EventType" + m_auxSuffix};
    // SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcBCIDKey {this, "ZdcBCIDKey", m_zdcSumContainerName + ".BCID" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_DAQModeKey {this, "ZdcDAQModeKey", m_zdcSumContainerName + ".DAQMode" + m_auxSuffix};
    
    Gaudi::Property<std::string> m_vertexContainerKey{this, "VertexContainerKey", "PrimaryVertices", "Vertex container name"};
    Gaudi::Property<std::string> m_trackContainerKey{this, "TrackContainerKey", "InDetTrackParticles", "Track container name"};    

    // Per-am (module sum) observables
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumCalibEnergyKey {this, "ZdcSumCalibEnergyKey", m_zdcSumContainerName + ".CalibEnergy" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumAverageTimeKey {this, "ZdcSumAverageTimeKey", m_zdcSumContainerName + ".AverageTime" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumUncalibSumKey {this, "ZdcSumUncalibSumKey", m_zdcSumContainerName + ".UncalibSum" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumModuleMaskKey {this, "ZdcSumModuleMaskKey", m_zdcSumContainerName + ".ModuleMask" + m_auxSuffix};
    
    // Module observables
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleStatusKey {this, "ZdcModuleStatusKey", m_zdcModuleContainerName + ".Status" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleAmplitudeKey {this, "ZdcModuleAmplitudeKey", m_zdcModuleContainerName + ".Amplitude" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleTimeKey {this, "ZdcModuleTimeKey", m_zdcModuleContainerName + ".Time" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleAmpNoNonLinKey {this, "ZdcModuleAmpNoNonLinKey", m_zdcModuleContainerName + ".AmpNoNonLin" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleFitAmpKey {this, "ZdcModuleFitAmpKey", m_zdcModuleContainerName + ".FitAmp" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleFitT0Key {this, "ZdcModuleFitT0Key", m_zdcModuleContainerName + ".FitT0" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleChisqKey {this, "ZdcModuleChisqKey", m_zdcModuleContainerName + ".Chisq" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibEnergyKey {this, "ZdcModuleCalibEnergyKey", m_zdcModuleContainerName + ".CalibEnergy" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibTimeKey {this, "ZdcModuleCalibTimeKey", m_zdcModuleContainerName + ".CalibTime" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleMaxADCKey {this, "ZdcModuleMaxADCKey", m_zdcModuleContainerName + ".MaxADC" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleMaxADCHGKey {this, "ZdcModuleMaxADCHGKey", m_zdcModuleContainerName + ".MaxADCHG" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleMaxADCLGKey {this, "ZdcModuleMaxADCLGKey", m_zdcModuleContainerName + ".MaxADCLG" + m_auxSuffix};
    
    // LG refit data
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleFitAmpLGRefitKey{this, "ZdcModuleFitAmpLGRefitKey", m_zdcModuleContainerName + ".FitAmpLGRefit" + m_auxSuffix, "ZDC module fit amp LG refit"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleAmpLGRefitKey{this, "ZdcModuleAmpLGRefitKey", m_zdcModuleContainerName + ".AmpLGRefit" + m_auxSuffix, "ZDC module fit amp LG refit"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleT0LGRefitKey{this, "ZdcModuleT0LGRefitKey", m_zdcModuleContainerName + ".T0LGRefit" + m_auxSuffix, "ZDC module fit t0 LG refit"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleT0SubLGRefitKey{this, "ZdcModuleT0SubLGRefitKey", m_zdcModuleContainerName + ".T0SubLGRefit" + m_auxSuffix, "ZDC module subtracted t0 LG refit"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleChisqLGRefitKey{this, "ZdcModuleChisqLGRefitKey", m_zdcModuleContainerName + ".ChisqLGRefit" + m_auxSuffix, "ZDC module LG refit chi square"};

    // RPD channel observables
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeKey {this, "RPDChannelAmplitudeKey", m_zdcModuleContainerName + ".RPDChannelAmplitude" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeCalibKey {this, "RPDChannelAmplitudeCalibKey", m_zdcModuleContainerName + ".RPDChannelAmplitudeCalib" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelMaxADCKey {this, "RPDChannelMaxADCKey", m_zdcModuleContainerName + ".RPDChannelMaxADC" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelMaxSampleKey {this, "RPDChannelMaxSampleKey", m_zdcModuleContainerName + ".RPDChannelMaxSample" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelStatusKey {this, "RPDChannelStatusKey", m_zdcModuleContainerName + ".RPDChannelStatus" + m_auxSuffix};
    
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelPileupExpFitParamsKey{
        this, "RpdChannelPileupExpFitParamsKey", m_zdcModuleContainerName+".RPDChannelPileupExpFitParams"+m_auxSuffix, 
        "RPD channel pileup exponential fit parameters: exp( [0] + [1]*sample )"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelPileupFracKey{
        this, "RPDChannelPileupFracKey", m_zdcModuleContainerName+".RPDChannelPileupFrac"+m_auxSuffix, 
        "RPD channel pileup as fraction of total (nominal baseline-subtracted) sum ADC"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelSubtrAmpKey {
        this, "RPDChannelSubtrAmpKey", m_zdcSumContainerName + ".RPDChannelSubtrAmp" + m_auxSuffix,
        "RPD channel subtracted amplitudes (tile mass) used in centroid calculation"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDSubtrAmpSumKey {
        this, "RPDSubtrAmpSumKey", m_zdcSumContainerName + ".RPDSubtrAmpSum" + m_auxSuffix,
        "Sum of RPD channel subtracted amplitudes (total mass) used in centroid calculation"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDxCentroidKey {
        this, "xCentroidKey", m_zdcSumContainerName + ".xCentroid" + m_auxSuffix, 
        "X centroid after geometry corrections and after average centroid subtraction"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDyCentroidKey {
        this, "yCentroidKey", m_zdcSumContainerName + ".yCentroid" + m_auxSuffix, 
        "Y centroid after geometry corrections and after average centroid subtraction"};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDreactionPlaneAngleKey {
        this, "reactionPlaneAngleKey", m_zdcSumContainerName + ".reactionPlaneAngle" + m_auxSuffix, 
        "Reaction plane angle in [-pi, pi) from the positive x axis (angle of centorid on side C, angle of centroid + pi on side A)"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDcosDeltaReactionPlaneAngleKey {
        this, "cosDeltaReactionPlaneAngleKey", m_zdcSumContainerName + ".cosDeltaReactionPlaneAngle" + m_auxSuffix, 
        "Cosine of the difference between the reaction plane angles of the two sides"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDcentroidStatusKey {
        this, "centroidStatusKey", m_zdcSumContainerName + ".centroidStatus" + m_auxSuffix, 
        "Centroid status word"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDSideStatusKey {
        this, "RPDSideStatusKey", m_zdcSumContainerName + ".RPDStatus" + m_auxSuffix, 
        "Centroid status word"};
    //---------------------------------------------------

};
#endif
