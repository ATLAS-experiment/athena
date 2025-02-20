/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// CaloAddCellPedShift.h
//

#ifndef CALOCONDPHYSALGS_CALOADDCELLPEDSHIFT_H
#define CALOCONDPHYSALGS_CALOADDCELLPEDSHIFT_H

#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloCondBlobObjs/ICaloCoolIdTool.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArCabling/LArOnOffIdMapping.h"

#include "GaudiKernel/ITHistSvc.h"
#include "TTree.h"

#include "CxxUtils/checker_macros.h"

class CaloCondBlobFlt;
class CondAttrListCollection;


class ATLAS_NOT_THREAD_SAFE CaloAddCellPedShift : public AthAlgorithm {

 public:

  using AthAlgorithm::AthAlgorithm;

    
  /** standard Athena-Algorithm method */
  virtual StatusCode          initialize() override;
  /** standard Athena-Algorithm method */
  virtual StatusCode          execute() override;
  /** standard Athena-Algorithm method */
  virtual StatusCode          finalize() override;
  /** standard Athena-Algorithm method */
  virtual StatusCode          stop() override;
    
 private:

  //---------------------------------------------------
  // Member variables
  //---------------------------------------------------
    Gaudi::Property<std::string> m_fname{this,"inputFile",""};
  

    ServiceHandle<ITHistSvc> m_thistSvc{this,"THistSvc","THistSvc"};
    
    SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
    SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager","CaloDetDescrManager","SG Key for CaloDetDescrManager in the Condition Store" };
    SG::ReadCondHandleKey<CondAttrListCollection> m_pedKey{this,"FolderName","/CALO/Pedestal/CellPedestal"};
  
    const CaloCell_ID*       m_calo_id=nullptr;
    const LArOnlineID*      m_onlineID=nullptr;

    ToolHandle<ICaloCoolIdTool> m_caloCoolIdTool{this,"CaloCoolIdTool","CaloCoolIdTool"};

    int m_iCool=0;
    int m_SubHash=0;
    int m_Hash=0;
    int m_OffId=0;
    float m_eta=0;
    float m_phi=0;
    int m_layer=0;
    int m_Gain=0;
    int   m_bec=0;
    int   m_posneg=0;
    int   m_FT=0;
    int   m_slot=0;
    int   m_channel=0;
    float m_ped1=0;
    float m_ped1corr=0;
    float m_ped2=0;
    TTree* m_tree=nullptr;
};
#endif
