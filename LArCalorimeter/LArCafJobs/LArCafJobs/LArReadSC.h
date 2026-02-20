/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARCAFJOBS_LArReadSC_H
#define LARCAFJOBS_LArReadSC_H 1

#include "AthenaBaseComps/AthAlgorithm.h"

#include "LArCabling/LArOnOffIdMapping.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "LArElecCalib/ILArPedestal.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "TTree.h"

class CaloCell_ID;


class  ATLAS_NOT_THREAD_SAFE LArReadSC: public ::AthAlgorithm { 
 public: 
  LArReadSC( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~LArReadSC(); 

  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();

 private: 

   Gaudi::Property<double> m_etcut{this,"etCut",7500.,"Et cut to dump cells"};
   Gaudi::Property< std::string > m_outStream{this, "output","SPLASH", "to which stream write the ntuple"};

   const CaloCell_SuperCell_ID*       m_calo_id = nullptr;
   const LArOnline_SuperCellID*       m_lar_online_id = nullptr;

   SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMapSC","SG Key of LArOnOffIdMapping object"};
   SG::ReadCondHandleKey<ILArPedestal> m_pedestalKey{this,"PedestalKey","LArPedestalSC","SG Key of Pedestal conditions object"};
   SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager", "CaloSuperCellDetDescrManager"};
    
   SG::ReadHandleKey<LArDigitContainer> m_contKey{this, "DigitsKey", "", "key for LArDigitContainer"};
   SG::ReadHandleKey<CaloCellContainer> m_SCKey{this, "SCContainerKey", "", "key for SC cell container from raw"};
   SG::ReadHandleKey<CaloCellContainer> m_SCRecoKey{this, "SCRecoContainerKey", "", "key for SC reco cell container"};

   TTree* m_tree = nullptr;
   int m_runNumber = 0;
   int m_lbNumber = 0;
   int m_eventNumber = 0;
   int m_bcid = 0; 
   int m_error = 0;
   int m_ncells = 0;
   std::vector<float> m_ECell    ;
   std::vector<float> m_EtaCell  ;
   std::vector<float> m_PhiCell  ;
   std::vector<int>   m_LayerCell;
   std::vector<int>   m_ProvCell ;
   std::vector<int>   m_ChidCell ;
   std::vector<int>   m_HwidCell ;
   std::vector<std::array<float ,32> > m_ADC    ;
   std::vector<float> m_TCell    ;
   std::vector<float> m_ErecoCell    ;



}; 

#endif 

