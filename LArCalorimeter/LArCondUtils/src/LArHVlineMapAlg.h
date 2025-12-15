/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCONDUTILS_LARHVLINEMAPALG_H
#define LARCONDUTILS_LARHVLINEMAPALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArHVNMap.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloInterface/ILArHVMapTool.h"
#include "LArIdentifier/LArHVLineID.h"
#include "LArRecConditions/LArHVIdMapping.h"

#include <map>

class HWIdentifier;
class CaloCell_ID;

class LArHVlineMapAlg:public AthCondAlgorithm {
 
 public: 
   LArHVlineMapAlg(const std::string& name, ISvcLocator* pSvcLocator);
   
   virtual ~LArHVlineMapAlg()=default;
   
   virtual StatusCode initialize() override final;
   virtual StatusCode execute(const EventContext& ctx) const override final;
   
 private:
   
   ToolHandle <ILArHVMapTool> m_hvmapTool;

   SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};
   SG::ReadCondHandleKey<LArHVIdMapping> m_hvCablingKey{this, "LArHVIdMapping", "LArHVIdMap", "SG key for HV ID mapping"};
   SG::WriteCondHandleKey<LArHVNMap> m_mapKey{this, "keyOutput", "LArHVNcells", "Output key for map of number of cells in HVline"}; 

   const CaloCell_ID*     m_caloHelper{nullptr};
   const LArHVLineID*     m_hvlineHelper{nullptr};

};

#endif
