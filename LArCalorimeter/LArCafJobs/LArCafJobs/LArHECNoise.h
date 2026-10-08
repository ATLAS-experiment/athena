/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCAFJOBS_LARHECNOISE_H
#define LARCAFJOBS_LARHECNOISE_H

//LAr services:
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArElecCalib/ILArPedestal.h"

// Trigger
#include "TrigDecisionTool/TrigDecisionTool.h"

//Framework
#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ITHistSvc.h"

//STL:
#include <string>


class LArOnlineID;

class TTree;

class LArHECNoise : public AthAlgorithm  {

 public:

   LArHECNoise(const std::string& name, ISvcLocator* pSvcLocator);
   ~LArHECNoise();

   virtual StatusCode initialize() override;
   virtual StatusCode finalize() override;
   virtual StatusCode execute(const EventContext& ctx) override;

 private:

   ServiceHandle<ITHistSvc> m_thistSvc{this,"THistSvc","THistSvc"};
    
   TTree* m_tree{};

   SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
   SG::ReadCondHandleKey<ILArPedestal> m_pedKey{this,"PedestalKey","LArPedestal","SG Key of Pedestal obj"};
   /*Tools*/

  PublicToolHandle< Trig::TrigDecisionTool > m_trigDec{this, "TrigDecisionTool", "", "Handle to the TrigDecisionTool"};

   /*services*/
   const LArOnlineID* m_LArOnlineIDHelper;
   SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
       , "CaloDetDescrManager"
       , "CaloDetDescrManager"
       , "SG Key for CaloDetDescrManager in the Condition Store" };

   const CaloCell_ID*   m_calocell_id;

   /*declaration of branches*/
   int  m_nt_run ;
   int  m_nt_evtId;
   int  m_nt_evtCount;
   int  m_nt_evtTime;
   int  m_nt_evtTime_ns;
   int  m_nt_lb;
   int  m_nt_bcid;
   int  m_nt_gain;
   int  m_nt_side;
   int  m_nt_samp;
   int  m_nt_reg;
   int  m_nt_ieta;
   int  m_nt_iphi;
   int  m_nt_quality;
   short m_nt_digi[32];
   int m_nt_max;
   int m_nt_min;
   long m_nt_OID;
   float m_nt_avgMu;
   float m_nt_actMu;
   float m_nt_e;
   float m_nt_t;
   float m_nt_eta;
   float m_nt_phi;
   float m_nt_z;
   float m_nt_r;
   float m_nt_ped;
   float m_nt_pedRMS;
   std::vector<float> m_nt_prescale;
   std::vector<int> m_nt_trigger;

   // other members
   std::vector<std::string> m_TriggerLines;
   int m_MinDigitADC;
   int m_MaxDeltaT;


};

#endif // LArHECNoise_H

