/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// TheCaloCellNoiseAlg.h
//

#ifndef CALOCONDPHYSALGS_CALOCELLNOISEALG_H
#define CALOCONDPHYSALGS_CALOCELLNOISEALG_H

#include <string>

// Gaudi includes

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "LArElecCalib/ILArPedestal.h"
#include "LArElecCalib/ILArNoise.h"
#include "LArRawConditions/LArADC2MeV.h"
#include "CaloConditions/CaloNoise.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#include "GaudiKernel/ITHistSvc.h"
#include "TTree.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "LArCabling/LArOnOffIdMapping.h"


  class CaloCellNoiseAlg : public AthAlgorithm {
  public:
    //Gaudi style constructor and execution methods
    /** Standard Athena-Algorithm Constructor */
    CaloCellNoiseAlg(const std::string& name, ISvcLocator* pSvcLocator);
    /** Default Destructor */
    virtual ~CaloCellNoiseAlg();
    
    /** standard Athena-Algorithm method */
    virtual StatusCode          initialize() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          execute(const EventContext& ctx) override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          stop() override;
    
  private:

    StatusCode         fillNtuple();
    StatusCode         fitNoise();
    static StatusCode         readNtuple();
    float              getLuminosity();

  //---------------------------------------------------
  // Member variables
  //---------------------------------------------------
    ServiceHandle<ITHistSvc> m_thistSvc{this,"THistSvc","THistSvc"};

    SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };

    const CaloCell_ID*       m_calo_id{nullptr};
    SG::ReadCondHandleKey<ILArNoise>    m_noiseKey{this,"NoiseKey","LArNoiseSym","SG Key of ILArNoise object"};
    SG::ReadCondHandleKey<ILArPedestal> m_pedestalKey{this,"PedestalKey","LArPedestal","SG Key of LArPedestal object"};
    SG::ReadCondHandleKey<LArADC2MeV> m_adc2mevKey
      { this, "ADC2MeVKey", "LArADC2MeV", "SG Key of the LArADC2MeV CDO" };
    SG::ReadCondHandleKey<CaloNoise> m_totalNoiseKey
      { this, "TotalNoiseKey", "totalNoise", "SG conditions key for total noise" };
    SG::ReadCondHandleKey<CaloNoise> m_elecNoiseKey
      { this, "ElecNoiseKey", "electronicNoise", "SG conditions key for electronic noise" };

    // list of cell energies
    struct CellInfo {
      int nevt;
      int nevt_good;
      double average;
      double rms;
      int identifier;
      int sampling;
      float eta;
      float phi;
      float reference;
    };
    std::vector<CellInfo> m_CellList;
    int m_ncell{0};

    unsigned int m_lumiblock{0};
    unsigned int m_lumiblockOld{0};
    bool m_first{false};

    // Split this out into a separate, dynamically-allocated block.
    // Otherwise, the CaloCellNoiseAlg is so large that it violates
    // the ubsan sanity checks.
    struct TreeData {
      float m_luminosity {0};
      int  m_ncell {0};
      int m_nevt[200000] {0};
      int m_nevt_good[200000] {0};
      int m_layer[200000] {0};
      int m_identifier[200000] {0};
      float m_eta[200000] {0};
      float m_phi[200000] {0};
      float m_average[200000] {0};
      float m_rms[200000] {0};
      float m_reference[200000] {0};
    };
    std::unique_ptr<TreeData> m_treeData;
    TTree* m_tree{};

    Gaudi::Property<bool> m_doMC{this, "doMC", false};
    Gaudi::Property<bool> m_readNtuple{this, "readNtuple", false};
    Gaudi::Property<bool> m_doFit{this, "doFit", true};
    Gaudi::Property<bool> m_doLumiFit{this, "doLumiFit", true};
    Gaudi::Property<int>  m_nmin{this, "nevtMin", 10};
    ToolHandle<Trig::TrigDecisionTool> m_trigDecTool{this, "TrigDecisionTool", ""}; //!< TDT handle
    Gaudi::Property<std::string>  m_triggerChainProp{this, "TriggerChain", ""};
    FloatArrayProperty m_cuts{this, "EnergyCuts", {}};
    SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
    Gaudi::Property<std::string> m_lumiFolderName{this, "LumiFolderName", "/TRIGGER/LUMI/LBLESTONL"};
    Gaudi::Property<int> m_addlumiblock{this, "NAddLumiBlock", 5, "Number of consecutive lumiblocks to add together"};
    Gaudi::Property<float> m_deltaLumi{this, "DeltaLumi", 0.05};
  };
#endif
