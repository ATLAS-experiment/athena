/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// CaloNoise2Ntuple.h
//

#ifndef CALOCONDPHYSALGS_CALONOISE2NTUPLE_H
#define CALOCONDPHYSALGS_CALONOISE2NTUPLE_H

#include <string>

// Gaudi includes

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloCell_ID.h"

#include "GaudiKernel/ITHistSvc.h"
#include "TTree.h"

#include "StoreGate/ReadCondHandleKey.h"

class CaloNoise;

class CaloNoise2Ntuple : public AthAlgorithm {

  public:
    //Gaudi style constructor and execution methods
    /** Standard Athena-Algorithm Constructor */
    CaloNoise2Ntuple(const std::string& name, ISvcLocator* pSvcLocator);
    /** Default Destructor */
    virtual ~CaloNoise2Ntuple();
    
    /** standard Athena-Algorithm method */
    virtual StatusCode          initialize() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          execute() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          stop() override;
    
  private:

  //---------------------------------------------------
  // Member variables
  //---------------------------------------------------
  ServiceHandle<ITHistSvc> m_thistSvc{this,"THistSvc","THistSvc"};

  const CaloCell_ID*       m_calo_id{};

  SG::ReadCondHandleKey<CaloNoise> m_totalNoiseKey
    { this, "TotalNoiseKey", "totalNoise", "SG key for total noise" };
  SG::ReadCondHandleKey<CaloNoise> m_elecNoiseKey
    { this, "ElecNoiseKey", "electronicNoise", "SG key for electronic noise" };
  SG::ReadCondHandleKey<CaloNoise> m_pileupNoiseKey
    { this, "PileupNoiseKey", "pileupNoise", "SG key for pileup noise" };
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey
    {this,"CaloDetDescrManager","CaloDetDescrManager","SG Key for CaloDetDescrManager in the Condition Store" };

  Gaudi::Property<std::string> m_treeName{this, "TreeName", "mytree"};

  int m_iCool{0};
  int m_SubHash{0};
  int m_Hash{0};
  int m_OffId{0};
  float m_eta{0.};
  float m_phi{0.};
  int m_layer{0};
  int m_Gain{0};
  float m_noise{0.};
  float m_elecNoise{0.};
  float m_pileupNoise{0.}; 
  TTree* m_tree{};

  int m_runNumber{0};
  int m_lumiBlock{0};
};
#endif
