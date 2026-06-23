/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// CaloCellPosition2Ntuple.h
//

#ifndef CALOCONDPHYSALGS_CALOCELLPOSITION2NTUPLE_H
#define CALOCONDPHYSALGS_CALOCELLPOSITION2NTUPLE_H

#include <string>

// Gaudi includes

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloConditions/CaloCellPositionShift.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "GaudiKernel/ITHistSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TTree.h"

class CaloCell_ID;

class CaloCellPosition2Ntuple : public AthAlgorithm {

 public:
  // Gaudi style constructor and execution methods
  /** Standard Athena-Algorithm Constructor */
  CaloCellPosition2Ntuple(const std::string& name, ISvcLocator* pSvcLocator);

  /** standard Athena-Algorithm method */
  virtual StatusCode initialize() override;
  /** standard Athena-Algorithm method */
  virtual StatusCode execute(const EventContext& ctx) override;
  /** standard Athena-Algorithm method */
  virtual StatusCode finalize() override;
  /** standard Athena-Algorithm method */
  virtual StatusCode stop() override;

 private:
  //---------------------------------------------------
  // Member variables
  //---------------------------------------------------

  ServiceHandle<ITHistSvc> m_thistSvc{this,"THistSvc","THistSvc"};

  const CaloCell_ID* m_calo_id = nullptr;

  SG::ReadCondHandleKey<CaloRec::CaloCellPositionShift> m_cellPosKey{this, "inputKey", "LArCellPositionShift", "Key for CaloCellPositionShift"};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this, "CaloDetDescrManager", "CaloDetDescrManager",
                                                          "SG Key for CaloDetDescrManager in the Condition Store"};

  int m_Hash;
  int m_OffId;
  float m_eta;
  float m_phi;
  int m_layer;
  float m_dx;
  float m_dy;
  float m_dz;
  float m_volume;
  TTree* m_tree;
};
#endif
