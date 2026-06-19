/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// CaloCellEnergyCorr2Ntuple.h
//

#ifndef CALOCONDPHYSALGS_CALOCELLENERGYCORR2NTUPLE_H
#define CALOCONDPHYSALGS_CALOCELLENERGYCORR2NTUPLE_H

#include <string>

// Gaudi includes

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

#include "GaudiKernel/ITHistSvc.h"
#include "TTree.h"


class CaloCellEnergyCorr2Ntuple : public AthAlgorithm {

  public:
    //Gaudi style constructor and execution methods
    /** Standard Athena-Algorithm Constructor */
    CaloCellEnergyCorr2Ntuple(const std::string& name, ISvcLocator* pSvcLocator);
    /** Default Destructor */
    ~CaloCellEnergyCorr2Ntuple();
    
    /** standard Athena-Algorithm method */
    virtual StatusCode          initialize() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          execute(const EventContext& ctx) override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          stop() override;
    
  private:

  //---------------------------------------------------
  // Member variables
  //---------------------------------------------------
  //=== blob storage
  ServiceHandle<ITHistSvc> m_thistSvc{this,"THistSvc","THistSvc"};

  const CaloCell_ID*       m_calo_id;

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };
  SG::ReadCondHandleKey<AthenaAttributeList> m_attrListCollKey
    { this, "AttrListCollKey", "/LAR/CellCorrOfl/EnergyCorr" };

  int m_Hash;
  int m_OffId;
  float m_eta;
  float m_phi;
  int m_layer;
  int m_detector;
  float m_corr;
  TTree* m_tree;

};
#endif
