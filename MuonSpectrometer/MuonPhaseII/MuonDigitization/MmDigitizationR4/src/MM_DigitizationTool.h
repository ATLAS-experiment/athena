/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MM_DIGITIZATIONR4_DIGITIZATIONTOOL_H
#define MM_DIGITIZATIONR4_DIGITIZATIONTOOL_H

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "MuonDigitizationR4/MuonDigitizationTool.h"
#include "MuonDigitContainer/MmDigitContainer.h"
#include "MagFieldElements/AtlasFieldCache.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

#include "MuonCondData/DigitEffiData.h"
#include "MuonCondData/NswErrorCalibData.h"
#include "MuonCondData/NswCalibDbThresholdData.h"
#include "MuonCondData/NswCalibDbTimeChargeData.h"

#include "NSWCalibTools/INSWCalibSmearingTool.h"
#include "NSWCalibTools/INSWCalibTool.h"

#include "MM_Digitization/MM_ElectronicsResponseSimulation.h"
#include "MM_Digitization/MM_StripsResponseSimulation.h"

// ----------------------------------------------------------------------------------------------
namespace MuonR4{
  /***
   *  @brief: Smearing of hits stemming from muons using the best knowledge uncertainties
   */
  class MM_DigitizationTool final: public MuonDigitizationTool {
  public:

    using MuonDigitizationTool::MuonDigitizationTool;

    StatusCode initialize() override final;
    StatusCode finalize() override final;
  protected:
    StatusCode digitize(const EventContext& ctx,
			const TimedHits& hitsToDigit,
			xAOD::MuonSimHitContainer* sdoContainer) const override final; 
        
 
  private:

    MM_ElectronicsToolInput combinedStripResponseAllHits(const EventContext&ctx,
                                                         const std::vector<MM_ElectronicsToolInput>& v_stripDigitOutput) const;
    SG::WriteHandleKey<MmDigitContainer> m_writeKey{this, "OutputObjectName", "MM_DIGITS"};

    SG::ReadCondHandleKey<Muon::DigitEffiData> m_effiDataKey{this, "EffiDataKey", "MmDigitEff", "Efficiency constants of the individual MM gasGaps"};
    SG::ReadCondHandleKey<NswErrorCalibData> m_uncertCalibKey{this, "ErrorCalibKey", "NswUncertData", "Key of the parametrized NSW uncertainties"};
    
    SG::ReadCondHandleKey<NswCalibDbThresholdData> m_condThrshldsKey{ this, "CondThrshldsKey", "NswCalibDbThresholdData",
								      "Key of NswCalibDbThresholdData object containing calibration data (VMM thresholds)"};
    // The magnetic field in local coordinates is needed for the strip response simulation 
    SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey{this, "AtlasFieldCacheCondObj", "fieldCondObj"};
    ToolHandle<Muon::INSWCalibSmearingTool> m_smearingTool{this, "SmearingTool", "Muon::NSWCalibSmearingTool/MMCalibSmearingTool"};
    ToolHandle<Muon::INSWCalibTool> m_calibrationTool{this, "CalibrationTool", "Muon::NSWCalibTool/NSWCalibTool"};
    
    Gaudi::Property<bool> m_digitizeMuonOnly{this, "ProcessTrueMuonsOnly", true, "If set to true hit with pdgId != 13 are skipped"};
    Gaudi::Property<std::string> m_vmmReadoutMode{this, "vmmReadoutMode", "peak", "For readout (DAQ) path. Can be peak or threshold"};
    Gaudi::Property<std::string> m_vmmARTMode{this, "vmmARTMode", "threshold", "For ART (trigger) path. Can be peak or threshold"};

    Gaudi::Property<double> m_energyThreshold{this, "EnergyThreshold", 50, "Minimal energy to process an electron sim hit"};
    Gaudi::Property<double> m_timeWindowLowerOffset{this, "WindowLowerOffset", -12.5,
                                                    "lower boundary of the time window in which digits are accepted"};
    Gaudi::Property<double> m_timeWindowUpperOffset{this, "WindowUpperOffset", 187.5,
                                                    "upper boundary of the time window in which digits are accepted"};
    Gaudi::Property<double> m_DiffMagSecondMuonHit{this, "DiffMagSecondMuonHit", 0.1};

    Gaudi::Property<int> m_maskMultiplet{this, "MaskMultiplet", 0, "0: all, 1: first, 2: second, 3: both"};

    Gaudi::Property<bool> m_writeOutputFile{this, "SaveInternalHistos", false};
    Gaudi::Property<bool> m_needsMcEventCollHelper{this, "UseMcEventCollectionHelper", false};
    Gaudi::Property<bool> m_checkMMSimHits{this, "CheckSimHits", true, "Control on the hit validity"};
    Gaudi::Property<bool> m_useTimeWindow{this, "UseTimeWindow", true};
    Gaudi::Property<bool> m_vmmNeighborLogic{this, "VMMNeighborLogic", false};
    Gaudi::Property<bool> m_doSmearing{this, "doSmearing", true,
                                       "set the usage or not of the smearing tool for realistic detector performance"};
    // Constants vars for the MM_StripsResponseSimulation class
    // qThreshold=2e, we accept a good strip if the charge is >=2e

    // Three gas mixture mode,	Ar/CO2=93/7, Ar/CO2=80/20, Ar/CO2/Iso=93/5/2
    // each mode have different
    // transverseDiffusionSigma/longitudinalDiffusionSigma/driftVelocity/avalancheGain/interactionDensityMean/interactionDensitySigma/lorentzAngle
    Gaudi::Property<float> m_qThreshold{this, "qThreshold", 0.001, "Charge Threshold"};
    Gaudi::Property<float> m_driftGapWidth{this, "DriftGapWidth", 5.04, "Drift Gap Width of 5.04 mm"};
    Gaudi::Property<float> m_crossTalk1{this, "crossTalk1", 0.3, "Strip Cross Talk with Nearest Neighbor"};
    Gaudi::Property<float> m_crossTalk2{this, "crossTalk2", 0.09, "Strip Cross Talk with 2nd Nearest Neighbor"};

    Gaudi::Property<float> m_avalancheGain{this, "AvalancheGain", 6.0e3, "avalanche Gain for rach gas mixture"};

    // Constants vars for the MM_ElectronicsResponseSimulation
    Gaudi::Property<float> m_electronicsThreshold{this, "electronicsThreshold", 15000,
                                                  "threshold Voltage for histoBNL, 2*(Intrinsic noise ~3k e)"};
    Gaudi::Property<float> m_stripdeadtime{this, "StripDeadTime", 200, "dead-time for strip, default value 200 ns = 8 BCs"};
    Gaudi::Property<float> m_ARTdeadtime{this, "ARTDeadTime", 200, "dead-time for ART, default value 200 ns = 8 BCs"};


    Gaudi::Property<bool> m_useCondThresholds{this, "useCondThresholds", false,
                                              "Use conditions data to get thresholds, overrules useThresholdScaling"};
    Gaudi::Property<bool> m_useThresholdScaling{this, "useThresholdScaling", true,
                                                "Use a strip length dependent threshold in MM digitiation"};
    Gaudi::Property<float> m_thresholdScaleFactor{this, "thresholdScaleFactor", 7.0,
                                                  "Use x times the strip length dependent noise as MM threshold"};
    Gaudi::Property<float> m_vmmDeadtime{this, "vmmDeadtime", 200, "Specifies how much before the lower time limit the VMM simulation should start evaluating the signal"};
    // The following job option is a hack until the underlying bug in the VMM sim is found which creates a peak at the upper boundary of the
    // time window.
    Gaudi::Property<float> m_vmmUpperGrazeWindow{this, "vmmUpperGrazeWindow", 150, "Specifies how much above the upper time window boundary the VMM sim evaluates the signal."};

    // Dead time between two hits in the same channel
    Gaudi::Property<double> m_deadTime{this, "deadTime", 300. * Gaudi::Units::ns};

    std::unique_ptr<MM_StripsResponseSimulation> m_StripsResponseSimulation{};
    std::unique_ptr<MM_ElectronicsResponseSimulation> m_ElectronicsResponseSimulation{};

    using NoiseCalibConstants = NswCalibDbTimeChargeData::CalibConstants;
    /// Define a map to cache the noise parameters individually
    /// Key: stationName * std::abs(stationEta)
    std::map<int, NoiseCalibConstants> m_noiseParams{};

    mutable std::array<std::atomic<unsigned>, 8> m_allHits ATLAS_THREAD_SAFE{};
    mutable std::array<std::atomic<unsigned>, 8> m_acceptedHits ATLAS_THREAD_SAFE{};

    using DigiCache = OutDigitCache_t<MmDigitCollection>;

  };
}
#endif
