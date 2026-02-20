//Dear emacs, this is -*-c++-*-
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITIZATION_LARPILEUPTOOL_H
#define LARDIGITIZATION_LARPILEUPTOOL_H
// +======================================================================+
// +                                                                      +
// +  Author : G.Unal                                                     +
// +  move LAr digitization framework to use PileUpTool                   +
// +                                                                      +
// +======================================================================+
//
// ....... include
//

#include "PileUpTools/PileUpToolBase.h"

#include "AthenaKernel/IAthRNGSvc.h"

#include "CaloIdentifier/CaloGain.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "LArCabling/LArOnOffIdMapping.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "LArDigitization/LArHitEMap.h"
#include "PileUpTools/PileUpMergeSvc.h"

#include "LArRawEvent/LArDigitContainer.h"
#include "LArSimEvent/LArHitContainer.h"
#include "LArSimEvent/LArHitFloatContainer.h"
#include "LArRecConditions/LArXTalkWeightGlobal.h"
#include "CommissionEvent/CosTrigTime.h"

#include "GeneratorObjects/McEventCollection.h"

class StoreGateSvc;
class ITriggerTime;
class LArOnlineID;
class LArEM_ID;
class LArHEC_ID;
class LArFCAL_ID;
class CaloCell_ID;
class LArDigit;

namespace CLHEP {
  class HepRandomEngine;
}

class LArPileUpTool : public PileUpToolBase
{
//
// >>>>>>>> public method
//
 public:

  LArPileUpTool(const std::string& type,
                const std::string& name,
                const IInterface* parent);

  ~LArPileUpTool();

  virtual StatusCode initialize() override final;

  virtual StatusCode prepareEvent(const EventContext& ctx, unsigned int nInputEvents) override final;

  virtual StatusCode processBunchXing(int bunchXing,
                                      SubEventIterator bSubEvents,
                                      SubEventIterator eSubEvents) override final;


  //non-const version, (for backward compatiblity), calls teh const version
  virtual StatusCode processAllSubEvents(const EventContext& ctx) override final;


  //const method, reentrant 
  StatusCode processAllSubEvents(const EventContext& ctx) const;

private:
  struct perEventData_t {
    std::vector<float> m_energySum;
    std::vector<float> m_energySum_DigiHSTruth;
    LArHitEMap* m_hitmap{};
    LArHitEMap* m_hitmap_DigiHSTruth{};
    float m_trigtime=0;
    const LArXTalkWeightGlobal* m_weights=nullptr;
  };

  StatusCode fillMapFromHit(const EventContext& ctx,float tbunch,bool isSignal, perEventData_t& data) const;

  StatusCode fillMapFromHit(SubEventIterator iEvt, float bunchTime, bool isSignal, perEventData_t& data) const;


  StatusCode AddHit(const Identifier cellId, const float energy, const float time, const bool iSignal, perEventData_t& data) const;


  void   cross_talk(const IdentifierHash& idHash,
                    const Identifier& cellId,
                    const float& energy,
                    std::vector<IdentifierHash>& neighbourList,
                    std::vector<float>& energyList,
                    const LArXTalkWeightGlobal& weights) const;

  bool  fillMapfromSum(float bunchTime, perEventData_t& data) const;

//
// >>>>>>>> private data parts
//
  ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "PileUpMergeSvc", "PileUpMergeSvc", ""};


  SG::WriteHandleKey<LArHitEMap> m_hitMapKey{this,"LArHitEMapKey","LArHitEMap"};
  SG::WriteHandleKey<LArHitEMap> m_hitMapKey_DigiHSTruth{this,"LArHitEMap_DigiHSTruthKey","LArHitEMap_DigiHSTruth"};

  Gaudi::Property<bool> m_onlyUseContainerName{this, "OnlyUseContainerName", true, "Don't use the ReadHandleKey directly. Just extract the container name from it."};
  StringArrayProperty m_inputKeys{this, "InputHitContainers", {"LArHitEMB", "LArHitEMEC", "LArHitHEC", "LArHitFCAL"},
      "Name of input hit vectors (default=[LArHitEMB, LArHitEMEC, LArHitHEC, LArHitFCAL])" };
  SG::ReadHandleKeyArray<LArHitContainer> m_hitContainerKeys{this, "InputHitContainerKeys", {}, "Do not set manually!"};
  SG::ReadHandleKeyArray<LArHitFloatContainer> m_hitFloatContainerKeys{this, "InputHitFloatContainerKeys", {}, "Do not set manually!"};
  SG::ReadHandleKey<LArDigitContainer> m_inputDigitContainerKey{this, "InputDigitContainer", "",
      "Name of input digit container"}; // input digit container name
  SG::ReadHandleKey<CosTrigTime> m_timeKey{this,"CosTimeKey","CosTrigTime"};
  std::vector <std::string> m_hitContainerNames; // hit container name list

  SG::ReadHandleKey<McEventCollection> m_mcEventColl{this, "McEventCollectionKey", "TruthEvent", "McEventCollection"};

//
// ........ Algorithm properties
//
  Gaudi::Property<bool> m_NoiseOnOff{this, "NoiseOnOff", true,
      "put electronic noise (default=true)"};            // noise (in all sub-detectors) is on if true
  Gaudi::Property<bool> m_PileUp{this, "PileUp", false,
      "Pileup mode (default=false)"};                // pile up or not

// Switches (true by default) on Noise for each sub-detector (can be combined)
  Gaudi::Property<bool> m_NoiseInEMB {this, "NoiseInEMB", true,
      "put noise in EMB (default=true)"};               // noise in Barrel is off if false
  Gaudi::Property<bool> m_NoiseInEMEC{this, "NoiseInEMEC", true,
      "put noise in EMEC (default=true)"};              // noise in EndCap is off if false
  Gaudi::Property<bool> m_NoiseInHEC{this, "NoiseInHEC", true,
      "put noise in HEC (default=true)"};                  // noise in HEC is off if false
  Gaudi::Property<bool> m_NoiseInFCAL{this, "NoiseInFCAL", true,
      "put noise in FCAL (default=true)"};              // noise in FCAL is off if false
  //put false if you want cancel the noise in one or several sub-detectors
  Gaudi::Property<bool> m_CrossTalk{this, "CrossTalk", true,
      "Simulate cross-talk (default=true)"};                // flag for Cross Talk
  Gaudi::Property<bool> m_CrossTalkStripMiddle{this, "CrossTalkStripMiddle", true,
      "Add strip/middle cross talk (if crosstalk is true) (default=true)"};     // flag for strip-middle cross talk (if m_CrooTalk is true)
  Gaudi::Property<bool> m_CrossTalk2Strip{this, "CrossTalk2Strip", true,
      "Add 2nd strip cross talk (if crosstalk is true) (default=true)"};          // flag for 2nd neighbor cross-talk
  Gaudi::Property<bool> m_CrossTalkMiddle{this, "CrossTalkMiddle", true,
      "Add middle to middle cross talk for barrel(if crosstalk is true) (default=true)"};          // flag for middle to middle cross-talk
  Gaudi::Property<float> m_scaleStripXtalk{this, "scaleStripXtalk", 1.,
      "Scale factor for strip xtalk"};          // scale factor for strip to strip cross-talk
  Gaudi::Property<float> m_scaleStripMiddle{this, "scaleStripMiddle", 1.,
      "Scale factor for strip-middle xtalk"};         // scale factor for strip-middle cross-talk
  Gaudi::Property<float> m_scaleMiddleXtalk{this, "scaleMiddleXtalk", 1.,
      "Scale factor for middle xtalk"};         // scale factor for middle-middle cross-talk
  // Windows mode
  Gaudi::Property<bool> m_Windows{this, "Windows", false,
      "Window mode (produce digits only around true e/photon) (default=false)"};
  Gaudi::Property<float> m_WindowsEtaSize{this, "WindowsEtaSize", 0.4,
      "Eta size of window (default=0.4)"};
  Gaudi::Property<float> m_WindowsPhiSize{this, "WindowsPhiSize", 0.5,
      "Phi size of window (default=0.5)"};
  Gaudi::Property<float> m_WindowsPtCut{this, "WindowsPtCut", 5000.,
      "Pt cut on e/photons for window mode (Default=5GeV)"};
  //
  Gaudi::Property<double> m_EnergyThresh{this, "EnergyThresh", -99.,
      "Hit energy threshold (default=-99)"};           // Zero suppression energy threshold

  Gaudi::Property<bool> m_rndmEvtRun{this, "UseRndmEvtRun", false,
      "Use Run and Event number to seed rndm number (default=false)"};               // use run,event number for random number seeding
  Gaudi::Property<bool> m_useTriggerTime{this, "UseTriggerTime", false,
      "Use Trigger tool (for commissioning) (default=false)"};
  Gaudi::Property<bool> m_RndmEvtOverlay{this, "RndmEvtOverlay", false,
      "Pileup and/or noise added by overlaying random events (default=false)"};         // Pileup and noise added by overlaying random events
  Gaudi::Property<bool> m_isMcOverlay{this, "isMcOverlay", false,
      "Is input Overlay from MC or data (default=false, from data)"};             // true if input RDO for overlay are from MC, false if from data

  Gaudi::Property<bool> m_useMBTime{this, "UseMBTime", false,
      "use detailed hit time from MB events in addition to bunch crossing time for pileup (default=false)"};
  Gaudi::Property<bool> m_recordMap{this, "RecordMap", true,
      "Record LArHitEMap in detector store for use by LArL1Sim (default=true)"};
  Gaudi::Property<bool> m_useLArHitFloat{this, "useLArFloat", true,
      "Use simplified transient LArHit (default=false)"};
  Gaudi::Property<bool> m_pedestalNoise{this, "PedestalNoise", false,
      "Use noise from Pedestal structure instead of LArNoise (default=false)"};
  Gaudi::Property<bool> m_addPhase{this, "AddPhase", false,
      "Add random phase (default = false)"};
  Gaudi::Property<float> m_phaseMin{this, "PhaseMin", 0.,
      "Minimum time to add (default=0)"};
  Gaudi::Property<float> m_phaseMax{this, "PhaseMax", 25.0,
      "Maximum time to add (default=25)"};
  Gaudi::Property<bool> m_ignoreTime{this, "IgnoreTime", false,
      "Set all hit time to 0, for debugging (default = false)"};
  Gaudi::Property<bool> m_roundingNoNoise{this, "RoundingNoNoise", true,
      "if true add random number [0:1[ in no noise case before rounding ADC to integer, if false add only 0.5 average"};  // flag used in NoNoise case: if true add random number [0;1[ in ADC count, if false add only average of 0.5
 
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};

  SG::ReadCondHandleKey<LArXTalkWeightGlobal>  m_xtalkKey{this,"LArXTalkWeightGlobal","LArXTalkWeightGlobal","SG Key of XTalk vector of object"};

 
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};
 

  const CaloCell_ID*     m_calocell_id{};
  const LArEM_ID*        m_larem_id{};
  const LArHEC_ID*       m_larhec_id{};
  const LArFCAL_ID*      m_larfcal_id{};
  const LArOnlineID*     m_laronline_id{};

  Gaudi::Property<bool> m_skipNoHit{this, "SkipNoHit", false,
      "Skip events with no LAr hits (default=false)"};

  ServiceHandle<IAthRNGSvc> m_rndmGenSvc{this, "RndmSvc", "AthRNGSvc", ""};

  Gaudi::Property<std::string> m_randomStreamName{this, "RandomStreamName", "LArDigitization", ""};

  Gaudi::Property<uint32_t> m_randomSeedOffset{this, "RandomSeedOffset", 2, ""}; //

  Gaudi::Property<bool> m_useLegacyRandomSeeds{this, "UseLegacyRandomSeeds", false,
      "Use MC16-style random number seeding"};

  Gaudi::Property<bool> m_doDigiTruth{this, "DoDigiTruthReconstruction", false,
      "Also create information about reconstructed digits for HS hits"};


  //Backward compatiblity: Keep per-event data as class-member
  perEventData_t m_data;

};

#endif
