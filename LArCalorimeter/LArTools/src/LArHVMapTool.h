/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARHVMAPTOOL_H
#define LARHVMAPTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "CaloInterface/ILArHVMapTool.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArRecConditions/LArHVIdMapping.h"


class LArHVMapTool: public ILArHVMapTool, public AthAlgTool
{

     public:
          LArHVMapTool(const std::string& type, const std::string& name,
                                      const IInterface* parent);

          virtual ~LArHVMapTool(){};

          virtual StatusCode initialize();
          virtual StatusCode finalize(){return StatusCode::SUCCESS;}


          virtual void GetHVLines(const Identifier& id, const CaloDetDescrManager *cddm, 
                                  std::vector<int> &hvLineVec) const override final;

          virtual void GetHVLines(const Identifier& id, const CaloDetDescrManager *cddm, 
                                  std::vector<HWIdentifier> &hvLineId) const override final;
     private:

          void GetHVLinesCore(const Identifier& id, const CaloDetDescrManager *cddm, 
                              std::vector<int> *hvLineVec, std::vector<HWIdentifier> *hvLineId) const;

          const CaloIdManager *m_caloIdMgr;
          const LArEM_ID* m_larem_id;
          const LArHEC_ID* m_larhec_id;
          const LArFCAL_ID* m_larfcal_id;

          const LArHVIdMapping* m_hvmapping;

          SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};
          SG::ReadCondHandleKey<LArHVIdMapping> m_hvCablingKey
                 {this, "LArHVIdMapping", "LArHVIdMap", "SG key for HV ID mapping"};
};

#endif

