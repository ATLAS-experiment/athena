/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//
//      2014-05-21 Author: Remi Lafaye (Annecy) 
//      2015-01-29 Author: Bertrand LAFORGE (LPNHE Paris)
//
/////////////////////////////////////////////////////////////

#ifndef electronMonTool_H
#define electronMonTool_H

#include "AthenaMonitoring/AthenaMonManager.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "StoreGate/StoreGateSvc.h"
#include "TH1F.h"
#include "TH2F.h"
#include "egammaMonToolBase.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronxAODHelpers.h"
#include "xAODEventInfo/EventInfo.h"
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct electronHist : egammaBaseHist
{
  bool m_fullHistoList;

  enum electronType
  {
    LhMedium=0,
    CbLoose,
    LhLoose,
    LhTight,
    CbTight,
    NumberOfTypesToMonitor
  };
  
  // electron ID per region histograms
  std::vector<TH1*> m_hvEoverP {};        
  
  // electrons per region histograms
  std::vector<TH1*> m_hvDeltaEta1 {};   // electron track histograms
  std::vector<TH1*> m_hvDeltaPhi2 {};     
  std::vector<TH1*> m_hvNOfBLayerHits {}; 
  std::vector<TH1*> m_hvNOfSiHits {};
  std::vector<TH1*> m_hvNOfTRTHits {};    
  std::vector<TH1*> m_hvNOfTRTHighThresholdHits {};
  std::vector<TH1*> m_hvd0 {};

  // Monitoring per lumiblock
  unsigned int m_nElectronsInCurrentLB {};
  unsigned int m_nElectrons {};
  std::vector<int> m_nElectronsPerLumiBlock {};
  std::vector<int> m_nElectronsPerRegion {};

  electronHist(const std::string& name, bool FullHistoList) 
    : egammaBaseHist(name)
  {
    m_fullHistoList = FullHistoList;
  }
};

class electronMonTool : public egammaMonToolBase
{
 public:
  electronMonTool(const std::string& type, const std::string& name, const IInterface* parent); 

  virtual ~electronMonTool();
  
  virtual StatusCode initialize() override;
  virtual StatusCode bookHistograms() override;
  virtual StatusCode bookHistogramsForOneElectronType(electronHist& myHist);

  virtual StatusCode fillHistograms(const EventContext& ctx) override;
  virtual StatusCode fillHistogramsForOneElectron(xAOD::ElectronContainer::const_iterator e_iter,
						  electronHist& myHist);

 protected:
  SG::ReadHandleKey<xAOD::ElectronContainer> m_ElectronContainer{this, "ElectronContainer", "Electrons", "Name of the electron collection"}; // Container name for electrons

  electronHist *m_LhLooseElectrons;   // LH Loose electrons histograms
  electronHist *m_LhMediumElectrons;  // LH Medium electrons histograms
  electronHist *m_CbLooseElectrons;   // Medium cut based electrons histograms
  electronHist *m_LhTightElectrons;   // LH Tight electrons histograms
  electronHist *m_CbTightElectrons;   // Cut based Tight electrons histograms

  MonGroup* m_electronGroup {}; 
  MonGroup* m_electronTrkGroup {};
  MonGroup* m_electronIdGroup {};
  MonGroup* m_electronLBGroup {};
};

#endif
