/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//
//      2014-05-21 Author: Remi Lafaye (Annecy) 
//      2015-02-15 Author: Bertrand Laforge (LPNHE Paris)
//
/////////////////////////////////////////////////////////////

#ifndef photonMonTool_H
#define photonMonTool_H

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

struct photonHist : egammaBaseHist
{
  enum photonType
  {
    CbLoose=0,
    CbMedium,
    CbTight,
    NumberOfTypesToMonitor
  };

  // Global panel histograms
  TH1 * m_hNUnconv {};  // Histogram for number of Unconverted photons
  TH1 * m_hNConv {};    // Histogram for number of Converted photons
  TH1 * m_hEtUnconv {}; // Histogram for Unconverted photon transverse energies
  TH1 * m_hEtConv {};   // Histogram for Converted photon transverse energies
  TH1 * m_hEtaUnconv {};// Histogram for Unconverted photon eta
  TH1 * m_hEtaConv {};  // Histogram for Converted photon eta
  TH1 * m_hPhiUnconv {};// Histogram for Unconverted photon phi
  TH1 * m_hPhiConv {};  // Histogram for Converted photon phi
  TH2 * m_hEtaPhiUnconv {};// Histogram for Unconverted photon eta,phi
  TH2 * m_hEtaPhiConv {};  // Histogram for Converted photon eta,phi
  TH1 * m_hTopoEtCone40Unconv {};// Histogram for calo based Unconverted photon isolation energy
  TH1 * m_hTopoEtCone40Conv {};  // Histogram for calo based Converted photon isolation energy
  TH1 * m_hPtCone20Unconv {};// Histogram for track based Unconverted photon isolation energy
  TH1 * m_hPtCone20Conv {};  // Histogram for track based Converted photon isolation energy
  TH1 * m_hRConv {};      // Histogram for photon convertion radius distribution
  
  // photons per region histograms
  std::vector<TH1*> m_hvTopoEtCone40Unconv {};  // Histograms for unconv. photon calo-based isolation transverse energies
  std::vector<TH1*> m_hvPtCone20Unconv {};  // Histograms for unconv. photon track-based isolation transverse energies
  std::vector<TH1*> m_hvTopoEtCone40Conv {};  // Histograms conv. for photon calo-based isolation transverse energies
  std::vector<TH1*> m_hvPtCone20Conv {};  // Histograms for conv. photon track-based isolation transverse energies

  // Converted Photon Trk per region histograms
  std::vector<TH1*> m_hvConvType {};
  std::vector<TH1*> m_hvConvTrkMatch1 {};
  std::vector<TH1*> m_hvConvTrkMatch2 {};
  std::vector<TH1*> m_hvDeltaEta1 {};   
  std::vector<TH1*> m_hvDeltaPhi2 {};     
  std::vector<TH1*> m_hvNOfBLayerHits {}; 
  std::vector<TH1*> m_hvNOfSiHits {};
  std::vector<TH1*> m_hvNOfTRTHits {};    
  std::vector<TH1*> m_hvNOfTRTHighThresholdHits {};
  std::vector<TH1*> m_hvd0 {};

  // Monitoring per lumiblock
  unsigned int m_nPhotonsInCurrentLB {};
  unsigned int m_nPhotonsInCurrentLBUnconv {};
  unsigned int m_nPhotonsInCurrentLBConv {};
  unsigned int m_nPhotons {};
  unsigned int m_nPhotonsUnconv {};
  unsigned int m_nPhotonsConv {};
  std::vector<int> m_nPhotonsPerLumiBlock {};
  std::vector<int> m_nPhotonsPerLumiBlockUnconv {};
  std::vector<int> m_nPhotonsPerLumiBlockConv {};
  std::vector<int> m_nPhotonsPerRegion {};
  std::vector<int> m_nPhotonsPerRegionUnconv {};
  std::vector<int> m_nPhotonsPerRegionConv {};

  TH1 *m_hLB_NUnconv {}; // Histogram for number of photons vs LB
  TH1 *m_hLB_NConv {}; // Histogram for number of photons vs LB
  TH1 *m_hLB_fConv {}; // Histogram of Conv. photon fraction vs LB

  photonHist(const std::string& name) : egammaBaseHist(name) {}
};


class photonMonTool : public egammaMonToolBase
{
 public:
  photonMonTool(const std::string& type, const std::string& name, const IInterface* parent); 
  virtual ~photonMonTool();
  
  virtual StatusCode initialize() override;
  virtual StatusCode bookHistograms() override;
  virtual StatusCode bookHistogramsForOnePhotonType(photonHist& myHist);

  virtual StatusCode fillHistograms(const EventContext& ctx) override;
  virtual StatusCode fillHistogramsForOnePhoton(xAOD::PhotonContainer::const_iterator g_iter, photonHist& myHist);

 protected:
  SG::ReadHandleKey<xAOD::PhotonContainer> m_PhotonContainer{this, "PhotonContainer", "PhotonCollection", "Name of the photon collection"}; // Container name for photons

  photonHist *m_CbLoosePhotons {}; // Loose cut based photons histograms
  photonHist *m_CbTightPhotons {}; // Tight cut based photons histograms

  MonGroup* m_photonGroup {}; 
  MonGroup* m_photonConvGroup {};
  MonGroup* m_photonUnconvGroup {};
  MonGroup* m_photonIdGroup {};
  MonGroup* m_photonRegionGroup {};
  MonGroup* m_photonLBGroup {};
};

#endif
