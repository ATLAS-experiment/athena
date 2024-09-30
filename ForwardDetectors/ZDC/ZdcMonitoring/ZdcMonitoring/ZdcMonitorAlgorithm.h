/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCMONITORALGORITHM_H
#define ZDCMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "TRandom3.h"
#include "array"

//---------------------------------------------------
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
//---------------------------------------------------
#include "ZdcUtils/ZdcEventInfo.h"

class ZdcMonitorAlgorithm : public AthMonitorAlgorithm {
public:
    ZdcMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~ZdcMonitorAlgorithm();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
    StatusCode fillPhysicsDataHistograms( const EventContext& ctx ) const;

private:
    StatusCode calculate_log_bin_edges(float min_value, float max_value, int num_bins, std::vector<float>& bin_edges);
    float calculate_inverse_bin_width(float event_value, std::string variable_name, const std::vector<float>& bin_edges) const;
    
    // see the standalone version of the Gaudi::Property class (a wrapper in AsgTools) at
    // athena/Control/AthToolSupport/AsgTools/AsgTools/PropertyWrapper.h
    // input to constructor: owner, name, value, title = "" (by default)
    Gaudi::Property<bool> m_isOnline {this,"IsOnline",false};
    Gaudi::Property<bool> m_CalInfoOn {this,"CalInfoOn",false};
    Gaudi::Property<bool> m_enableTrigger {this,"EnableTrigger",true};
    Gaudi::Property<bool> m_isInjectedPulse {this,"IsInjectedPulse",false};

    Gaudi::Property<std::string> m_zdcModuleContainerName {this, "ZdcModuleContainerName", "ZdcModules", "Location of ZDC processed data"};
    Gaudi::Property<std::string> m_zdcSumContainerName {this, "ZdcSumContainerName", "ZdcSums", "Location of ZDC processed sums"};
    Gaudi::Property<std::string> m_auxSuffix{this, "AuxSuffix", "", "Append this tag onto end of AuxData"};
    Gaudi::Property<float> m_expected1N{this, "Expected1NADC", 1000., "Expected 1N position in ADC"}; // only needs to indicate the rough scale, only used in the >20N cut mask
    
    Gaudi::Property<float> m_moduleChisqHistMinValue{this, "ZDCModuleChisqHistMinValue", 1000., "Min value for logarithmic binning for ZDC module chisq distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqHistMaxvalue{this, "ZDCModuleChisqHistMaxvalue", 1000., "Max value for logarithmic binning for ZDC module chisq distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqHistNumBins{this, "ZDCModuleChisqHistNumBins", 1000., "Number of bins for logarithmic binning for ZDC module chisq distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqOverAmpHistMinValue{this, "ZDCModuleChisqOverAmpHistMinValue", 1000., "Min value for logarithmic binning for ZDC module chisq over amplitude distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqOverAmpHistMaxvalue{this, "ZDCModuleChisqOverAmpHistMaxvalue", 1000., "Max value for logarithmic binning for ZDC module chisq over amplitude distribution"}; // to manually calculate inverse-bin-width weight
    Gaudi::Property<float> m_moduleChisqOverAmpHistNumBins{this, "ZDCModuleChisqOverAmpHistNumBins", 1000., "Number of bins for logarithmic binning for ZDC module chisq over amplitude distribution"}; // to manually calculate inverse-bin-width weight

    // single side triggers - less error-prone if defined as separate properties then in a vector (where order would be crucial)
    Gaudi::Property<std::string> m_triggerSideA{this, "triggerSideA", "L1_ZDC_A", "Trigger on side A, needed for 1N-peak monitoring on side C"};
    Gaudi::Property<std::string> m_triggerSideC{this, "triggerSideC", "L1_ZDC_C", "Trigger on side C, needed for 1N-peak monitoring on side A"};


    static const int m_nSides = 2;
    static const int m_nModules = 4;
    static const int m_nChannels = 16;
    static const int m_nZdcStatusBits = 18; // ignoring the last one
    static const int m_nRpdStatusBits = 15; // ignoring the last one
    static const int m_nRpdCentroidStatusBits = 21; // ignoring the last one

    // the i-th element (or (i,j)-th element for 2D vector) here gives the index of the generic monitoring tool (GMT)
    // in the array of all GMT's --> allows faster tool retrieving and hence faster histogram filling
    std::vector<int> m_ZDCSideToolIndices;
    std::vector<std::vector<int>> m_ZDCModuleToolIndices;
    std::vector<std::vector<int>> m_RPDChannelToolIndices;

    std::vector<float> m_ZdcModuleChisqBinEdges;
    std::vector<float> m_ZdcModuleChisqOverAmpBinEdges;
    //---------------------------------------------------
    
    // owner, name (allows us to modify the key in python configuration), key
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumContainerKey {this, "ZdcSumContainerKey", "ZdcSums"};
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleContainerKey {this, "ZdcModuleContainerKey", "ZdcModules"};
    SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_HIEventShapeContainerKey {this, "HIEventShapeContainerKey", "HIEventShape"};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_eventTypeKey {this, "ZdcEventTypeKey", m_zdcSumContainerName + ".EventType" + m_auxSuffix};
    // SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcBCIDKey {this, "ZdcBCIDKey", m_zdcSumContainerName + ".BCID" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_DAQModeKey {this, "ZdcDAQModeKey", m_zdcSumContainerName + ".DAQMode" + m_auxSuffix};
    
    // Per-am (module sum) observables
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumCalibEnergyKey {this, "ZdcSumCalibEnergyKey", m_zdcSumContainerName + ".CalibEnergy" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumAverageTimeKey {this, "ZdcSumAverageTimeKey", m_zdcSumContainerName + ".AverageTime" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumUncalibSumKey {this, "ZdcSumUncalibSumKey", m_zdcSumContainerName + ".UncalibSum" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumModuleMaskKey {this, "ZdcSumModuleMaskKey", m_zdcSumContainerName + ".ModuleMask" + m_auxSuffix};
    
    // Module observables
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleStatusKey {this, "ZdcModuleStatusKey", m_zdcModuleContainerName + ".Status" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleAmplitudeKey {this, "ZdcModuleAmplitudeKey", m_zdcModuleContainerName + ".Amplitude" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleTimeKey {this, "ZdcModuleTimeKey", m_zdcModuleContainerName + ".Time" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleFitT0Key {this, "ZdcModuleFitT0Key", m_zdcModuleContainerName + ".FitT0" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleChisqKey {this, "ZdcModuleChisqKey", m_zdcModuleContainerName + ".Chisq" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibEnergyKey {this, "ZdcModuleCalibEnergyKey", m_zdcModuleContainerName + ".CalibEnergy" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibTimeKey {this, "ZdcModuleCalibTimeKey", m_zdcModuleContainerName + ".CalibTime" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleMaxADCKey {this, "ZdcModuleMaxADCKey", m_zdcModuleContainerName + ".MaxADC" + m_auxSuffix};
    
    // LG refit data
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleAmpLGRefitKey{this, "ZdcModuleAmpLGRefitKey", m_zdcModuleContainerName + ".AmpLGRefit" + m_auxSuffix, "ZDC module fit amp LG refit"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleT0LGRefitKey{this, "ZdcModuleT0LGRefitKey", m_zdcModuleContainerName + ".T0LGRefit" + m_auxSuffix, "ZDC module fit t0 LG refit"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleT0SubLGRefitKey{this, "ZdcModuleT0SubLGRefitKey", m_zdcModuleContainerName + ".T0SubLGRefit" + m_auxSuffix, "ZDC module subtracted t0 LG refit"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleChisqLGRefitKey{this, "ZdcModuleChisqLGRefitKey", m_zdcModuleContainerName + ".ChisqLGRefit" + m_auxSuffix, "ZDC module LG refit chi square"};

    // RPD channel observables
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeKey {this, "RPDChannelAmplitudeKey", m_zdcModuleContainerName + ".RPDChannelAmplitude" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeCalibKey {this, "RPDChannelAmplitudeCalibKey", m_zdcModuleContainerName + ".RPDChannelAmplitudeCalib" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelMaxADCKey {this, "RPDChannelMaxADCKey", m_zdcModuleContainerName + ".RPDChannelMaxADC" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelStatusKey {this, "RPDChannelStatusKey", m_zdcModuleContainerName + ".RPDChannelStatus" + m_auxSuffix};
    
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDrowKey { // needed since the subtracted amplitudes for each side are written in a nrow * ncol matrix and saved in zdcSums
        this, "rowKey", m_zdcModuleContainerName + ".row" + m_auxSuffix, // need to convert channel to row & column to plot the subtracted amplitudes using rpdChannelMonToolArr (reading in the row and column for each RPD channel, instead of using an analytical expression, allows flexibility for future different RPD geometry / run conditions)
        "Row index of RPD channel"
    };
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDcolKey { // needed since the subtracted amplitudes for each side are written in a nrow * ncol matrix and saved in zdcSums
        this, "colKey", m_zdcModuleContainerName + ".col" + m_auxSuffix, // need to convert channel to row & column to plot the subtracted amplitudes using rpdChannelMonToolArr (reading in the row and column for each RPD channel, instead of using an analytical expression, allows flexibility for future different RPD geometry / run conditions)
        "Column index of RPD channel"
    };

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
