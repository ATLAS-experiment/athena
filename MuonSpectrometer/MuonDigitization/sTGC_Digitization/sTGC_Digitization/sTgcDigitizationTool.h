/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONDIGITIZATION_STGC_DIGITIZATIONTOOL_H
#define MUONDIGITIZATION_STGC_DIGITIZATIONTOOL_H

/** @class sTgcDigitizationTool

    @section sTGC_DigitizerDetails Class methods and properties


    In the initialize() method...
    In the execute() method...

*/

#include "PileUpTools/PileUpMergeSvc.h"
#include "PileUpTools/PileUpToolBase.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "HitManagement/TimedHitCollection.h"
#include "MuonSimEvent/sTGCSimHitCollection.h"
#include "MuonSimEvent/sTGCSimHit.h"
#include "xAODEventInfo/EventInfo.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "NSWCalibTools/INSWCalibSmearingTool.h"
#include "NSWCalibTools/INSWCalibTool.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "CLHEP/Units/PhysicalConstants.h"
#include "MuonCondData/DigitEffiData.h"
#include "sTGC_Digitization/sTgcDigitMaker.h"

/*******************************************************************************/
namespace MuonGM{
  class MuonDetectorManager;
}
namespace CLHEP {
  class HepRandomEngine;
}

class sTgcHitIdHelper;

/*******************************************************************************/
class sTgcDigitizationTool : public PileUpToolBase {

public:
  sTgcDigitizationTool(const std::string& type, const std::string& name, const IInterface* parent);

  /** Initialize */
  virtual StatusCode initialize();

  // /** When being run from PileUpToolsAlgs, this method is called at the start of
  //       the subevts loop. Not able to access SubEvents */
  StatusCode prepareEvent(const EventContext& ctx, const unsigned int /*nInputEvents*/);
  //
  //   /** When being run from PileUpToolsAlgs, this method is called for each active
  //       bunch-crossing to process current SubEvents bunchXing is in ns */
  StatusCode  processBunchXing(int bunchXing,
                               SubEventIterator bSubEvents,
                               SubEventIterator eSubEvents);

  //   /** When being run from PileUpToolsAlgs, this method is called at the end of
  //       the subevts loop. Not (necessarily) able to access SubEvents */
  StatusCode mergeEvent(const EventContext& ctx);
  /** alternative interface which uses the PileUpMergeSvc to obtain
      all the required SubEvents. */
  virtual StatusCode processAllSubEvents(const EventContext& ctx);

  /** Just calls processAllSubEvents - leaving for back-compatibility
      (IMuonDigitizationTool) */

  /**
     reads GEANT4 hits from StoreGate in each of detector
     components corresponding to sTGC modules which are triplets
     or doublets. A triplet has tree sensitive volumes and a
     double has two. This method calls
     sTgcDigitMaker::executeDigi, which digitizes every hit, for
     every readout element, i.e., a sensitive volume of a
     chamber. (IMuonDigitizationTool)
  */
  StatusCode digitize(const EventContext& ctx);

  class sTgcSimDigitData {
    public:
        sTgcSimDigitData() = default;
        sTgcSimDigitData(MuonSimData&& simData, sTgcDigit&& digit):
            m_sTGCSimData{std::move(simData)},
            m_sTGCDigit{std::move(digit)}{}
      /// Get the SimData
      const MuonSimData& getSimData() const { return m_sTGCSimData; }
      MuonSimData& getSimData() {return m_sTGCSimData; }
      /// Get the sTGC digit
      const sTgcDigit& getDigit() const { return m_sTGCDigit; }
      sTgcDigit& getDigit() { return m_sTGCDigit; }

      Identifier identify() const { return getDigit().identify(); }
      double time() const {return getDigit().time(); }   

    private:
        MuonSimData m_sTGCSimData;
        sTgcDigit m_sTGCDigit;

  };
  using sTgcSimDigitVec = std::vector<sTgcSimDigitData>;
  using sTgcSimDigitCont = std::vector<sTgcSimDigitVec>;

  using DigiConditions = sTgcDigitMaker::DigiConditions;
  using sTgcDigitVec = sTgcDigitMaker::sTgcDigitVec;
  using sTgcDigtCont = std::vector<sTgcDigitVec>;
private:

  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName, const EventContext& ctx) const;
  template <class CondType> StatusCode retrieveCondData(const EventContext& ctx,
                                                        SG::ReadCondHandleKey<CondType>& key,
                                                        const CondType* & condPtr) const;

  /** Get next event and extract collection of hit collections */
  StatusCode getNextEvent(const EventContext& ctx);
  /** Core part of digitization use by mergeEvent (IPileUpTool) and digitize (IMuonDigitizationTool) */
  StatusCode doDigitization(const EventContext& ctx);

  ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "MergeSvc", "PileUpMergeSvc", "Merge service used in digitization"};
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", "Random Number Service used in Muon digitization"};
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                            "Key of input MuonDetectorManager condition data"};
  
  SG::ReadCondHandleKey<DigitEffiData> m_effiKey{this, "EffiDigiKey", "sTgcDigitEff",
                                                      "Key of the efficiency data in the CondStore"};

  
  std::unique_ptr<sTgcDigitMaker> m_digitizer{};
  std::unique_ptr<TimedHitCollection<sTGCSimHit>> m_thpcsTGC{};
  std::vector<std::unique_ptr<sTGCSimHitCollection>> m_STGCHitCollList{};

  ToolHandle<Muon::INSWCalibSmearingTool> m_smearingTool{this,"SmearingTool","Muon::NSWCalibSmearingTool/STgcCalibSmearingTool"};
  ToolHandle<Muon::INSWCalibTool> m_calibTool{this,"CalibrationTool","Muon::NSWCalibTool/NSWCalibTool"};

  SG::WriteHandleKey<sTgcDigitContainer> m_outputDigitCollectionKey{this,"OutputObjectName","sTGC_DIGITS","WriteHandleKey for Output sTgcDigitContainer"}; // name of the output digits
  SG::WriteHandleKey<MuonSimDataCollection> m_outputSDO_CollectionKey{this,"OutputSDOName","sTGC_SDO","WriteHandleKey for Output MuonSimDataCollection"}; // name of the output SDOs

  Gaudi::Property<bool> m_doSmearing{this,"doSmearing",false};
  Gaudi::Property<bool> m_doToFCorrection{this,"doToFCorrection",false};
  Gaudi::Property<bool> m_doEfficiencyCorrection{this,"doEfficiencyCorrection",false};

  // voltage applied to gas gaps: Nominal condition in RUN3 as of Nov 2022 is 2.8 kV
  // this value serves to modify the gas gain caused by the electric field in the sTGC
  Gaudi::Property<double> m_runVoltage{this,"operatingHVinkV",2.8};

  Gaudi::Property<std::string> m_rndmEngineName{this,"RndmEngine","MuonDigitization","Random engine name"};

  Gaudi::Property<bool> m_onlyUseContainerName{this, "OnlyUseContainerName", true, "Don't use the ReadHandleKey directly. Just extract the container name from it."};
  SG::ReadHandleKey<sTGCSimHitCollection> m_hitsContainerKey{this, "InputObjectName", "sTGC_Hits", "name of the input object"};
  std::string m_inputObjectName{""};

  Gaudi::Property<bool> m_useCondThresholds{this, "useCondThresholds", false, "Use conditions data to get VMM charge threshold values"};
  SG::ReadCondHandleKey<NswCalibDbThresholdData> m_condThrshldsKey {this, "CondThrshldsKey", "NswCalibDbThresholdData", "Key of NswCalibDbThresholdData object containing calibration data (VMM thresholds)"};

  Gaudi::Property<int> m_doChannelTypes{this,"doChannelTypes",3};
  Gaudi::Property<bool> m_doPadSharing{this,"padChargeSharing", false};

  // sTgc VMM configurables accessible by python steering
  Gaudi::Property<double> m_deadtimeStrip{this,"deadtimeStrip", 250};
  Gaudi::Property<double> m_deadtimePad{this,"deadtimePad"    , 250};
  Gaudi::Property<double> m_deadtimeWire{this,"deadtimeWire" , 250};
  Gaudi::Property<bool> m_doNeighborOn{this,"neighborOn", true};

  Gaudi::Property<double> m_energyDepositThreshold{this,"energyDepositThreshold",300.0*CLHEP::eV,"Minimum energy deposit for hit to be digitized"};
  Gaudi::Property<double> m_limitElectronKineticEnergy{this,"limitElectronKineticEnergy",5.0*CLHEP::MeV,"Minimum kinetic energy for electron hit to be digitized"};

  Gaudi::Property<double> m_chargeThreshold{this,"chargeThreshold", 0.002, "vmm charge threshold in pC, need to set useCondThresholds to false if one wants to use this threshold value otherwise the one from the conditions database is used"};

  Gaudi::Property<double> m_stripChargeScale{this, "stripChargeScale",0.4, "strip charge scale"};

  Gaudi::Property<bool> m_applyAsBuiltBLines{this, "applyAsBuiltBLines",false, "correct the hit position by the as built and B-lines parameters"};

  const double m_timeJitterElectronicsStrip{2.f}; //ns
  const double m_timeJitterElectronicsPad{2.f}; //ns
  const double m_hitTimeMergeThreshold{30.f}; //30ns = resolution of peak finding descriminator

  static uint16_t bcTagging(const double digittime) ;

  double getChannelThreshold(const EventContext& ctx, 
                             const Identifier& channelID, 
                             const NswCalibDbThresholdData& thresholdData) const;

  
  StatusCode processDigitsWithVMM(const EventContext& ctx,
                                  const DigiConditions& digiCond,
                                  sTgcSimDigitCont& unmergedContainer,
                                  const double vmmDeadTime,
                                  const bool isNeighbourOn,
                                  sTgcDigtCont& outDigitContainer,
                                  MuonSimDataCollection& outSdoContainer) const;

  sTgcSimDigitVec processDigitsWithVMM(const EventContext& ctx,
                                      const DigiConditions& digiCond, 
                                      const double vmmDeadTime, 
                                      sTgcSimDigitVec& unmergedDigits, 
                                      const bool isNeighborOn) const;

};

#endif // MUONDIGITIZATION_STGC_DIGITIZATIONTOOL_H
