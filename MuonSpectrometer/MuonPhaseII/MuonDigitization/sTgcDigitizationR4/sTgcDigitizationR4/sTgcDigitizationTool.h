/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/// Added a flag in MuonConfig to turn off the R4 sTGC Digitization until fully validated.
/// Using FastDigi tool at the moment.
#ifndef STGC_DIGITIZATIONR4_DIGITIZATIONTOOL_H
#define STGC_DIGITIZATIONR4_DIGITIZATIONTOOL_H

#include <memory>
#include <vector>
#include <string>

#include "MuonDigitizationR4/MuonDigitizationTool.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuonSimHit/MuonSimHit.h"

#include "MuonCondData/DigitEffiData.h"
#include "MuonCondData/NswCalibDbThresholdData.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "sTgcDigitizationR4/sTgcDigitMaker.h"

#include <NSWCalibTools/INSWCalibSmearingTool.h>
#include <NSWCalibTools/INSWCalibTool.h>

namespace MuonR4 {
  
  class sTgcDigitizationTool final: public MuonDigitizationTool {
    public:
      using MuonDigitizationTool::MuonDigitizationTool;  

      StatusCode initialize() override final;

      /** @brief ReadoutChannelType to distinguish the available readout channels
      *  Pad - pad readout channel
      *  Strip - eta strip readout channel
      *   Wire - phi wire group readout channel
      */
      using ReadoutChannelType = sTgcIdHelper::sTgcChannelTypes;
      using DigiConditions = sTgcDigitMaker::DigiConditions; // struct from digitmaker
      using sTgcDigitVec = sTgcDigitMaker::sTgcDigitVec; // vector of sTgcDigit
      using DigiCache = OutDigitCache_t<sTgcDigitCollection>; // vector of digit collection 

      class sTgcSimDigitHit {
        public:
            sTgcSimDigitHit() = default;
            sTgcSimDigitHit(const TimedHit& simHit, std::unique_ptr<sTgcDigit> digit):
                m_sTGCSimHit{simHit},
                m_sTGCDigit{std::move(digit)}{}
          /// Get the SimHit
          const TimedHit& getSimHit() const { return m_sTGCSimHit; }
          TimedHit& getSimHit() {return m_sTGCSimHit; }
          /// Get the sTGC digit
          const sTgcDigit& getDigit() const { return *m_sTGCDigit; }
          sTgcDigit& getDigit() { return *m_sTGCDigit; }
          std::unique_ptr<sTgcDigit> releaseDigit() { return std::move(m_sTGCDigit); }
          Identifier identify() const { return getDigit().identify(); }
          double time() const {return getDigit().time(); } 

        private:
            TimedHit m_sTGCSimHit;
            std::unique_ptr<sTgcDigit> m_sTGCDigit;
      };
      using sTgcSimDigitVec = std::vector<sTgcSimDigitHit>; //vector of custom class defined above
      StatusCode digitize(const EventContext& ctx,
                          const TimedHits& hitsToDigit,
                          xAOD::MuonSimHitContainer* sdoContainer) const override final;

    private:

      SG::WriteHandleKey<sTgcDigitContainer> m_writeKey{this, "OutputObjectName", "STGC_DIGITS"};

      SG::ReadCondHandleKey<Muon::DigitEffiData> m_effiDataKey{this, "EffiDataKey", "sTgcDigitEff", "Efficiency constants"};
      SG::ReadCondHandleKey<NswCalibDbThresholdData> m_condThrshldsKey{this, "CondThrshldsKey", "NswCalibDbThresholdData", "Calibration data"};

      ToolHandle<Muon::INSWCalibSmearingTool> m_smearingTool{this, "SmearingTool", "Muon::NSWCalibSmearingTool/STGCCalibSmearingTool"};
      ToolHandle<Muon::INSWCalibTool> m_calibrationTool{this, "CalibrationTool", "Muon::NSWCalibTool/STGCCalibTool"};

      Gaudi::Property<bool> m_digitizeMuonOnly{this, "ProcessTrueMuonsOnly", false};
      Gaudi::Property<bool> m_useTimeWindow{this, "UseTimeWindow", true};
      Gaudi::Property<bool> m_doSmearing{this, "doSmearing", false};
      Gaudi::Property<bool> m_doToFCorrection{this,"doToFCorrection", true};

      Gaudi::Property<int> m_digitMode{this, "digitMode", 3};  //
      Gaudi::Property<bool> m_doPadSharing{this,"padChargeSharing", false};
      Gaudi::Property<double> m_deadtimeStrip{this,"deadtimeStrip", 250};
      Gaudi::Property<double> m_deadtimePad{this,"deadtimePad"    , 250};
      Gaudi::Property<double> m_deadtimeWire{this,"deadtimeWire" , 250};
      Gaudi::Property<bool> m_doNeighborOn{this,"neighborOn", true};
      Gaudi::Property<double> m_runVoltage{this,"operatingHVinkV" , 2.8};
      Gaudi::Property<bool> m_useCondThresholds{this, "useCondThresholds", false,
                                                "Use conditions data to get VMM charge threshold values"};
      
      Gaudi::Property<double> m_energyDepositThreshold{this,"energyDepositThreshold",300.0*CLHEP::eV};
      Gaudi::Property<double> m_limitElectronKineticEnergy{this,"limitElectronKineticEnergy",5.0*CLHEP::MeV};

      Gaudi::Property<double> m_chargeThreshold{this,"chargeThreshold", 0.030};

      static constexpr double m_timeJitterElectronicsStrip{2.f};
      static constexpr double m_timeJitterElectronicsPad{2.f};
      static constexpr double m_hitTimeMergeThreshold{30.f};

      /** @brief Data type to deduplicate the SDOs && associate them with the digit
       *         produced by them */
      struct SimHitSorter{
          bool operator()(const TimedHit&a, const TimedHit& b) const {
              return a.get() < b.get();
          }
      };
      using SdoIdMap_t = std::map<const TimedHit, std::vector<Identifier>, SimHitSorter>; 

      StatusCode processDigitsWithVMM(const EventContext& ctx,
                                      const DigiConditions& digiCond,
                                      sTgcSimDigitVec&& digitsInChamber,
                                      const double vmmDeadTime,
                                      const bool isNeighbourOn,
                                      sTgcDigitCollection& outColl,
                                      SdoIdMap_t& sdoIdMap) const;
      
      sTgcSimDigitVec mergeDigitsVMM(const EventContext& ctx,
                                     const DigiConditions& digiCond, 
                                     const double vmmDeadTime, 
                                     const bool isNeighbourOn,
                                     sTgcSimDigitVec&& unmergedDigits) const;

                                          
      uint16_t bcTagging(const double digitTime) const;

      double getChannelThreshold(const EventContext& ctx,
                                                    const Identifier& channelID,
                                                    const NswCalibDbThresholdData& thresholdData) const;
                                                    
      std::unique_ptr<sTgcDigitMaker> m_digitizer{};
  };

}
#endif
