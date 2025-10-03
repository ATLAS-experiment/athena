/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGEGAMMAHYPO_TRIGPRECISIONELECTRONPRECISIONHYPOTOOLINC_H
#define TRIGEGAMMAHYPO_TRIGPRECISIONELECTRONPRECISIONHYPOTOOLINC_H 1

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "ITrigEgammaPrecisionElectronHypoTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "StoreGate/ReadDecorHandle.h"
#include "xAODEventInfo/EventInfo.h"

/**
 * @class Implementation of the precision selection for electrons
 * @brief 
 **/

class TrigEgammaPrecisionElectronHypoTool : public extends<AthAlgTool, ITrigEgammaPrecisionElectronHypoTool> { 
 public: 
  TrigEgammaPrecisionElectronHypoTool( const std::string& type, 
			 const std::string& name, 
			 const IInterface* parent );

  virtual StatusCode initialize() override;

  virtual StatusCode decide( std::vector<ITrigEgammaPrecisionElectronHypoTool::ElectronInfo>& input)  const override;
  virtual bool decide( const ITrigEgammaPrecisionElectronHypoTool::ElectronInfo& i) const override;

 private:
  HLT::Identifier m_decisionId;
  
  //Calorimeter electron ID  cuts
  Gaudi::Property< std::vector<float> > m_etabin { this, "EtaBins", {} , "Bins of eta" }; //!<  selection variable for PRECISION electron selection:eta bins
  Gaudi::Property< std::vector<float> > m_eTthr { this, "ETthr", {}, "ET Threshold" };
  Gaudi::Property< float >              m_detacluster { this, "dETACLUSTERthr", 0. , "" };
  Gaudi::Property< float >              m_dphicluster { this, "dPHICLUSTERthr", 0. , "" };  
  Gaudi::Property< float >              m_RelPtConeCut { this, "RelPtConeCut", -999., "Track isolation cut" };
  Gaudi::Property< float >              m_TopoEtConeCut { this, "TopoEtConeCut", -999., "Calorimeter isolation cut" };
  Gaudi::Property< float >              m_d0{ this,  "d0Cut", -1., "d0 cut" };
  Gaudi::Property<std::string>          m_pidName{this,"PidName", "", "Pid name"};
  Gaudi::Property< bool >               m_acceptAll { this, "AcceptAll", false , "accept all." };
  Gaudi::Property< bool >               m_doNoPid { this, "DoNoPid", false , "No Pid/Isolation applied" };
  ToolHandle< GenericMonitoringTool >   m_monTool { this, "MonTool", "", "Monitoring tool" };
  /*switch between relptvarcone30 and relptvarcone20 isolation*/
  Gaudi::Property<bool> m_useRelptvarcone30{ this, "UseRelptvarcone30", false};
  Gaudi::Property<bool> m_useTopoetCone20{ this, "UseTopoetcone20", false};


  int findCutIndex( float eta ) const;

}; 

#endif //> !TRIGEGAMMAHYPO_TRIGPRECISIONELECTRONPRECISIONHYPOTOOL_H

