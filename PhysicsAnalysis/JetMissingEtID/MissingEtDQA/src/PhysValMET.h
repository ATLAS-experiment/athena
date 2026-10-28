
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// PhysValMET.h 
// Header file for class PhysValMET
// Contact: atlas-cp-jetetmiss-jsv-conveners@cern.ch
/////////////////////////////////////////////////////////////////// 
#ifndef MISSINGETDQA_PHYSVALMET_H
#define MISSINGETDQA_PHYSVALMET_H 1

// FrameWork includes
#include "GaudiKernel/ServiceHandle.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAuxContainer.h"

// Local includes
#include "AthenaMonitoring/ManagedMonitorToolBase.h"

#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "METInterface/IMETMaker.h"
#include "TauAnalysisTools/ITauSelectionTool.h"

#include "AsgTools/ToolHandle.h"
// STL includes
#include <string>
#include <map>
#include <vector>


// Forward declaration
class TH1D;
class TH2D;
class IMETMaker;
class IAsgElectronLikelihoodTool;
class IAsgPhotonIsEMSelector;
namespace TauAnalysisTools {
  class ITauSelectionTool;
}


namespace MissingEtDQA {

class PhysValMET
  : public ManagedMonitorToolBase
{ 
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  PhysValMET( const std::string& type,
	      const std::string& name, 
	      const IInterface* parent);

  /// Destructor: 
  virtual ~PhysValMET(); 

  // Athena algtool's Hooks
  virtual StatusCode initialize();
  virtual StatusCode bookHistograms();
  virtual StatusCode fillHistograms(const EventContext& ctx);
  virtual StatusCode procHistograms();


  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 


  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private:
  bool m_doTruth;
  bool m_inputIsDAOD;
  bool m_doMETRefPlots;

  /// Default constructor: 
  PhysValMET();

  // Containers
  std::string m_mapname;
  std::string m_corename;

  // Methods
  bool Accept(const xAOD::Electron* el);
  bool Accept(const xAOD::Photon* ph);
  bool Accept(const xAOD::TauJet* tau);
  bool Accept(const xAOD::Muon* muon);

  // vector of collections
  std::vector <std::string> m_jet_types = {"AntiKt4EMTopo", "AntiKt4EMPFlow"};

  // vector of terms
    std::vector <std::string> m_terms = {
    "RefEle", "RefGamma", "RefTau", "RefMuons", "RefJet",
    "SoftClus", "PVSoftTrk", "FinalTrk", "FinalClus"};

  // Map for names
  std::map <std::string,std::string> m_term_names = {
    {"RefEle", "Electron term"},
    {"RefGamma", "Photon term"},
    {"RefTau", "Tau term"},
    {"RefMuons", "Muon term"},
    {"RefJet", "Jet term"},
    {"SoftClus", "Cluster-based soft term"},
    {"PVSoftTrk", "Track-based soft term (PV-matched)"},
    {"FinalTrk", "Total MET with TST"},
    {"FinalClus", "Total MET with CST"},
    {"Track", "Track MET, loose selection"},
    {"PVTrack_Nominal", "Track MET for highest sum p_{T}^{2} PV"},
    {"PVTrack_Pileup", "Track MET for each pileup vertex"}
  };


  // Hists
  TH1D *m_MET_Track = nullptr, *m_MET_Track_x = nullptr, *m_MET_Track_y = nullptr, *m_MET_Track_phi = nullptr, *m_MET_Track_sum = nullptr;
  TH1D *m_MET_PVTrack_Nominal = nullptr, *m_MET_PVTrack_Nominal_x = nullptr, *m_MET_PVTrack_Nominal_y = nullptr, *m_MET_PVTrack_Nominal_phi = nullptr, *m_MET_PVTrack_Nominal_sum = nullptr;
  TH1D *m_MET_PVTrack_Pileup = nullptr, *m_MET_PVTrack_Pileup_x = nullptr, *m_MET_PVTrack_Pileup_y = nullptr, *m_MET_PVTrack_Pileup_phi = nullptr, *m_MET_PVTrack_Pileup_sum = nullptr;
  TH1D *m_MET_Calo = nullptr, *m_MET_Calo_x = nullptr, *m_MET_Calo_y = nullptr, *m_MET_Calo_phi = nullptr, *m_MET_Calo_sum = nullptr;

  //Maps
  std::map<std::string,std::vector<TH1D*> > m_MET;
  std::map<std::string,std::vector<TH1D*> > m_MET_x;
  std::map<std::string,std::vector<TH1D*> > m_MET_y;
  std::map<std::string,std::vector<TH1D*> > m_MET_phi;
  std::map<std::string,std::vector<TH1D*> > m_MET_sum;
  std::map<std::string,std::vector<TH1D*> > m_MET_Diff;
  std::map<std::string,std::vector<TH1D*> > m_MET_Diff_x;
  std::map<std::string,std::vector<TH1D*> > m_MET_Diff_y;
  std::map<std::string,std::vector<TH1D*> > m_MET_Diff_phi;
  std::map<std::string,std::vector<TH1D*> > m_MET_Diff_sum;
  std::map<std::string,std::vector<TH1D*> > m_MET_Cumu;
  std::map<std::string,std::vector<TH1D*> > m_MET_Resolution;
  std::map<std::string,std::vector<TH1D*> > m_MET_Significance;
  std::map<std::string,std::vector<TH1D*> > m_MET_dPhi;
  std::map<std::string,std::vector<TH2D*> > m_MET_CorrFinalTrk;
  std::map<std::string,std::vector<TH2D*> > m_MET_CorrFinalClus;
  std::map<std::string,std::vector<TH1D*> > m_MET_Kinematic_pt;
  std::map<std::string,std::vector<TH1D*> > m_MET_Kinematic_eta;
  std::map<std::string,std::vector<TH1D*> > m_MET_Kinematic_phi;
  std::map<std::string,std::vector<TH1D*> > m_MET_multi;

  std::vector<std::string> m_dir_met;

  ToolHandle<CP::IMuonSelectionTool> m_muonSelTool            {this, "MuonSelectionTool",        "", "Muon selection tool"};
  ToolHandle<IAsgElectronLikelihoodTool> m_elecSelLHTool      {this, "ElectronLHSelectionTool",  "", "Electron likelihood selection tool"};
  ToolHandle<IAsgPhotonIsEMSelector> m_photonSelIsEMTool      {this, "PhotonIsEMSelectionTool" , "", "Photon selection tool"};
  ToolHandle<IMETMaker> m_metmakerTopo                        {this, "METMakerTopo",             "", "METMaker for EMTopo jets"};
  ToolHandle<IMETMaker> m_metmakerPFlow                       {this, "METMakerPFlow",            "", "METMaker for EMPFlow jets"};
  ToolHandle<TauAnalysisTools::ITauSelectionTool> m_tauSelTool{this, "TauSelectionTool",         "", "Tau selection tool"};
  ToolHandle<IMETMaker>* m_metmaker;

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfoKey","EventInfo","EventInfo container"};
  SG::ReadHandleKey<xAOD::MuonContainer> m_muonColl{this,"MuonContainer","Muons","Muon container"};
  SG::ReadHandleKey<xAOD::ElectronContainer> m_eleColl{this,"ElectronContainer","Electrons","Electron container"};
  SG::ReadHandleKey<xAOD::PhotonContainer> m_gammaColl{this,"PhotonContainer","Photons","Photon container"};
  SG::ReadHandleKey<xAOD::TauJetContainer> m_tauColl{this,"TauContainer","Taus","Electron container"};
  SG::ReadHandleKeyArray<xAOD::JetContainer> m_jetKeys{this,"JetContainers", {"AntiKt4EMTopoJets", "AntiKt4EMPFlowJets"},"Jet container keys"};
  SG::ReadHandleKeyArray<xAOD::MissingETContainer> m_coreMetKeys{this,"CoreMET",{"MET_Core_AntiKt4EMTopo", "MET_Core_AntiKt4EMPFlow"},"Core MissingET containers"};
  SG::WriteHandleKey<xAOD::MissingETContainer> m_metCaloKey{this,"METCaloKey","MET_Calo","Output MET calo container"};
  SG::WriteHandleKey<xAOD::MissingETAuxContainer> m_metCaloAuxKey{this,"METCaloAuxKey","MET_CaloAux","Output MET calo aux container"};

}; 

}

#endif //> !MISSINGETDQA_PHYSVALMET_H

