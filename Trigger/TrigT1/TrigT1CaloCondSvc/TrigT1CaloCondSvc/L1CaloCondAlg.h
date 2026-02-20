#ifndef L1CALOCONDALG_H
#define L1CALOCONDALG_H



#include "TString.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "StoreGate/CondHandleKeyArray.h"

#include "TrigT1CaloCalibConditions/L1CaloDisabledTowersContainer.h" 
#include "TrigT1CaloCalibConditions/L1CaloDerivedRunParsContainer.h"
#include "TrigT1CaloCalibConditions/L1CaloPprChanCalibContainer.h"
#include "TrigT1CaloCalibConditions/L1CaloPprChanDefaultsContainer.h"
#include "TrigT1CaloCalibConditions/L1CaloPpmFineTimeRefsContainer.h"
#include "TrigT1CaloCalibConditions/L1CaloRunParametersContainer.h"
#include "TrigT1CaloCalibConditions/L1CaloPprChanStrategyContainer.h"
#include "TrigT1CaloCalibConditions/L1CaloPpmDeadChannelsContainer.h"
#include "TrigT1CaloCalibConditions/L1CaloPpmDeadChannelsContainer.h"
#include "TrigT1CaloCalibConditions/L1CaloPprConditionsContainerRun2.h"
#include "TrigT1CaloCalibConditions/L1CaloPprDisabledChannelContainerRun2.h" 
#include "TrigT1CaloCalibConditions/L1CaloReadoutConfigContainer.h"
#include "TrigT1CaloCalibConditions/L1CaloReadoutConfigContainerJSON.h"


class L1CaloCondAlg: public ::AthCondAlgorithm { 
 public: 
    // this is a standard algorithm constructor

  L1CaloCondAlg( const std::string& name, ISvcLocator* pSvcLocator );


  // these are the functions inherited from Algorithm
  virtual StatusCode initialize () override;
  virtual StatusCode execute (const EventContext& ctx) const override;


 private: 


  Gaudi::Property<std::string> m_timingRegime{this,"timingRegime","","String-> Calib1, Calib2 or Physics"};  
  Gaudi::Property<std::string> m_strategy{this,"strategy","","String-> empty, HighMu or LowMu"};

  Gaudi::Property<bool> m_usePhysicsRegime
    { this, "UsePhysicsRegime", false, "Allow use of Physics timing regime" };
  Gaudi::Property<bool> m_useCalib1Regime
    { this, "UseCalib1Regime", false, "Allow use of Calib1 timing regime" };
  Gaudi::Property<bool> m_useCalib2Regime
    { this, "UseCalib2Regime", false, "Allow use of Calib2 timing regime" };

  enum PprKeys {
                // Should correspond with the name list in initialize().
                PPRCHANCALIB = 0,
                PPRCHANCOMMON = 1,
                PPRCHANHIGHMU = 2,
                PPRCHANLOWMU = 3
  };

  SG::ReadCondHandleKeyArray<CondAttrListCollection> m_physicsKeys
    { this, "PhysicsKeys", {}, "" };
  SG::ReadCondHandleKeyArray<CondAttrListCollection> m_calib1Keys
    { this, "Calib1Keys", {}, "" };
  SG::ReadCondHandleKeyArray<CondAttrListCollection> m_calib2Keys
    { this, "Calib2Keys", {}, "" };

  // Folders only located at V1
  SG::ReadCondHandleKey<CondAttrListCollection> m_disabledTowers{ this, "DisabledTowers", "", "DisabledTowers" };
  SG::ReadCondHandleKey<CondAttrListCollection> m_derivedRunPars{ this, "DerivedRunPars", "",  "DerivedRunPars"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_ppmDeadChannels{ this, "PpmDeadChannels", "","PpmDeadChannels" };
  SG::ReadCondHandleKey<CondAttrListCollection> m_ppmFineTimeRefs{ this, "PpmFineTimeRefs" ,"",  "PpmFineTimeRefs"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_runParameters{ this, "RunParameters" ,"",  "RunParameters"};
  // Folders with configuration information
  SG::ReadCondHandleKey<CondAttrListCollection> m_pprChanDefaults{ this, "PprChanDefaults" ,"",  "PprChanDefaults"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_pprChanStrategy{ this, "PprChanStrategy", "",  "PprChanStrategy"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_readoutConfig{ this, "ReadoutConfig",  "",  "ReadoutConfig"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_readoutConfigJSON{ this, "ReadoutConfigJSON",  "",  "ReadoutConfigJSON"};


  // Defining WriteCondHandleKey for saving condition containers 
  SG::WriteCondHandleKey<L1CaloDisabledTowersContainer> m_disabledTowersContainer{this,"OutputKeyDisTowers","L1CaloDisabledTowersContainer"}; 
  SG::WriteCondHandleKey<L1CaloDerivedRunParsContainer> m_derivedRunParsContainer{this, "OutputKeyDerRunsPars", "L1CaloDerivedRunParsContainer"};
  SG::WriteCondHandleKey<L1CaloPprChanCalibContainer>  m_pprChanCalibContainer{ this, "OutputKeyPPr", "L1CaloPprChanCalibContainer"}; 
  SG::WriteCondHandleKey<L1CaloPprChanDefaultsContainer>  m_pprChanDefaultsContainer{ this, "OutputKeyDef", "L1CaloPprChanDefaultsContainer"}; 
  SG::WriteCondHandleKey<L1CaloPpmFineTimeRefsContainer>  m_ppmFineTimeRefsContainer{ this, "OutputKeyTimeRefs", "L1CaloPpmFineTimeRefsContainer"}; 
  SG::WriteCondHandleKey<L1CaloRunParametersContainer>  m_runParametersContainer{ this, "OutputKeyRunParameters", "L1CaloRunParametersContainer"};
  SG::WriteCondHandleKey<L1CaloPprChanStrategyContainer>  m_pprChanStrategyContainer{ this, "OutputKeyPprChanStrategy", "L1CaloPprChanStrategyContainer"};
  SG::WriteCondHandleKey<L1CaloPpmDeadChannelsContainer> m_ppmDeadChannelsContainer{ this, "OutputKeyPpmDeadChannels", "L1CaloPpmDeadChannelsContainer"};
  SG::WriteCondHandleKey<L1CaloPprConditionsContainerRun2> m_pprConditionsContainer{ this, "OutputKeyPprConditionsRun2", "L1CaloPprConditionsContainerRun2"};
  SG::WriteCondHandleKey<L1CaloPprDisabledChannelContainerRun2> m_pprDisabledChannelContainer{ this, "OutputKeyDisabledChannelRun2", "L1CaloPprDisabledChannelContainerRun2"};
  SG::WriteCondHandleKey<L1CaloReadoutConfigContainer>  m_readoutConfigContainer{ this, "OutputKeyReadoutConfig", "L1CaloReadoutConfigContainer"};
  SG::WriteCondHandleKey<L1CaloReadoutConfigContainerJSON>  m_readoutConfigContainerJSON{ this, "OutputKeyReadoutConfigJSON", "L1CaloReadoutConfigContainerJSON"};


  template <typename T> StatusCode updateCond(const EventContext& ctx, const SG::WriteCondHandleKey<T>& wkey,
          const std::vector<std::reference_wrapper<const SG::ReadCondHandleKey<CondAttrListCollection>>>& rkeys,
          std::unique_ptr<T> obj = nullptr) const;


}; 

#endif //> !MYPACKAGE_MYPACKAGEALG_H
