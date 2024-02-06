/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "EgammaMonitoring.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/ITHistSvc.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TProfile.h"

#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "TruthUtils/HepMCHelpers.h"

EgammaMonitoring::EgammaMonitoring(const std::string &name, ISvcLocator *pSvcLocator) :
  AthAlgorithm(name, pSvcLocator) {}

// ******

StatusCode EgammaMonitoring::initialize() {
  ATH_MSG_INFO("********************  Running over " << m_sampleType << " ******************");
  ATH_CHECK(service("THistSvc", rootHistSvc));

  showerShapesAll = std::make_unique<egammaMonitoring::ShowerShapesHistograms>(
    "showerShapesAll","Shower Shapes ", "/MONITORING/showerShapesAll/", rootHistSvc);

  showerShapes10GeV = std::make_unique<egammaMonitoring::ShowerShapesHistograms>(
    "showerShapes10GeV","Shower Shapes - 10 GeV", "/MONITORING/showerShapes10GeV/", rootHistSvc);

  isolationAll = std::make_unique<egammaMonitoring::IsolationHistograms>(
    "isolationAll","Isolation ", "/MONITORING/isolationAll/", rootHistSvc);

  ATH_CHECK(showerShapesAll->initializePlots());
  ATH_CHECK(showerShapes10GeV->initializePlots());
  ATH_CHECK(isolationAll->initializePlots(m_sampleType == "electron"));

  if ("dataZ" != m_sampleType) {
    clusterAll = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clustersAll","Clusters", "/MONITORING/clusterAll/", rootHistSvc);

    cluster10GeV= std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clusters10GeV","Clusters - 10 GeV", "/MONITORING/cluster10GeV/", rootHistSvc);

    clusterPromptAll = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clustersPromptAll","Clusters from Prompt", "/MONITORING/clusterPromptAll/", rootHistSvc);

    clusterPrompt10GeV = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clustersPrompt10GeV","Clusters from Prompt - 10 GeV", "/MONITORING/clusterPrompt10GeV/", rootHistSvc);

    ATH_CHECK(clusterAll->initializePlots());
    ATH_CHECK(cluster10GeV->initializePlots());
    ATH_CHECK(clusterPromptAll->initializePlots());
    ATH_CHECK(clusterPrompt10GeV->initializePlots());
  } else {
    m_clusterReco = std::make_unique<egammaMonitoring::RecoClusterHistograms>(
      "clustersReco","Clusters from Z->ee candidates in data", "/MONITORING/recoCluster/", rootHistSvc);
    recoElectronAll = std::make_unique<egammaMonitoring::RecoElectronHistograms>(
      "recoElectronAll","Reco electrons from Z->ee candidates in data",
      "/MONITORING/recoZElectron/", rootHistSvc);
    m_diElectron = std::make_unique<egammaMonitoring::DiObjectHistograms>(
      "diElectrons","Z->ee candidates in data",
      "/MONITORING/diElectrons/", rootHistSvc);
    ATH_CHECK(m_clusterReco->initializePlots());
    recoElectronAll->isData();
    ATH_CHECK(recoElectronAll->initializePlots());
    ATH_CHECK(m_diElectron->initializePlots());
  }

  if ("electron" == m_sampleType) {

    recoElectronAll = std::make_unique<egammaMonitoring::RecoElectronHistograms>(
      "recoElectronAll","Electrons Reco Electron",
      "/MONITORING/recoElectronAll/", rootHistSvc);

    truthElectronAll = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthElectronAll","All Truth Electrons", "/MONITORING/truthElectronAll/", rootHistSvc);

    truthPromptElectronAll = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthPromptElectronAll","All Truth Prompt Electrons", "/MONITORING/truthPromptElectronAll/", rootHistSvc);

    truthElectronRecoElectronAll = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthElectronRecoElectronAll","Truth Electrons Reco Electron", "/MONITORING/truthElectronRecoElectronAll/", rootHistSvc);

    truthPromptElectronWithTrack = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthPromptElectronWithTrack","Truth Prompt Electrons With Track", "/MONITORING/truthPromptElectronWithTrack/", rootHistSvc);

    truthPromptElectronWithGSFTrack = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthPromptElectronWithGSFTrack","Truth Prompt Electrons With GSFTrack", "/MONITORING/truthPromptElectronWithGSFTrack/", rootHistSvc);

    truthPromptElectronWithReco = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthPromptElectronWithReco","Truth Prompt Electrons With GSFTrack or just with a fwd cluster", "/MONITORING/truthPromptElectronWithReco/", rootHistSvc);

    truthPromptElectronWithRecoTrack = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthPromptElectronWithRecoTrack","Truth Prompt Electrons With GSFTrack", "/MONITORING/truthPromptElectronWithRecoTrack/", rootHistSvc);

    truthRecoElectronLooseLH = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthRecoElectronLooseLH","LLH Electrons Reco Electron", "/MONITORING/truthRecoElectronLooseLH/", rootHistSvc);

    truthRecoElectronMediumLH = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthRecoElectronMediumLH","MLH Electrons Reco Electron", "/MONITORING/truthRecoElectronMediumLH/", rootHistSvc);

    truthRecoElectronTightLH = std::make_unique<egammaMonitoring::TruthElectronHistograms>(
      "truthRecoElectronTightLH","TLH Electrons Reco Electron", "/MONITORING/truthRecoElectronTightLH/", rootHistSvc);

    if (!m_FwdElectronsKey.empty()) {
      recoElectronAll->hasFwd();
      truthElectronAll->hasFwd();
      truthPromptElectronAll->hasFwd();
      truthElectronRecoElectronAll->hasFwd();
      truthPromptElectronWithTrack->hasFwd();
      truthPromptElectronWithGSFTrack->hasFwd();
      truthPromptElectronWithReco->hasFwd();
      truthPromptElectronWithRecoTrack->hasFwd();
      truthRecoElectronLooseLH->hasFwd();
      truthRecoElectronMediumLH->hasFwd();
      truthRecoElectronTightLH->hasFwd();
    }

    ATH_CHECK(recoElectronAll->initializePlots());
    ATH_CHECK(truthRecoElectronLooseLH->initializePlots());
    ATH_CHECK(truthRecoElectronMediumLH->initializePlots());
    ATH_CHECK(truthRecoElectronTightLH->initializePlots());
    ATH_CHECK(truthElectronAll->initializePlots(true));
    ATH_CHECK(truthPromptElectronAll->initializePlots(true));
    ATH_CHECK(truthElectronRecoElectronAll->initializePlots());
    ATH_CHECK(truthPromptElectronWithTrack->initializePlots(true));
    ATH_CHECK(truthPromptElectronWithGSFTrack->initializePlots(true));
    ATH_CHECK(truthPromptElectronWithReco->initializePlots());
    ATH_CHECK(truthPromptElectronWithRecoTrack->initializePlots());
  } // electron Hists

  if ("gamma" == m_sampleType) {

    recoPhotonAll = std::make_unique<egammaMonitoring::RecoPhotonHistograms>(
      "recoPhotonAll","Reco Photon", "/MONITORING/recoPhotonAll/", rootHistSvc);

    clusterConvPhoton = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clusterConvPhoton","Clusters from Converted Photons", "/MONITORING/clusterConvPhoton/", rootHistSvc);

    clusterConvPhotonSi = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clusterConvPhotonSi","Clusters from Converted Photons - Si", "/MONITORING/clusterConvPhotonSi/", rootHistSvc);

    clusterConvPhotonSiSi = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clusterConvPhotonSiSi","Clusters from Converted Photons - SiSi", "/MONITORING/clusterConvPhotonSiSi/", rootHistSvc);
   
    clusterConvPhotonTRT = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clusterConvPhotonTRT","Clusters from Converted Photons - TRT", "/MONITORING/clusterConvPhotonTRT/", rootHistSvc);

    clusterConvPhotonTRTTRT = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clusterConvPhotonTRTTRT","Clusters from Converted Photons - TRTTRT", "/MONITORING/clusterConvPhotonTRTTRT/", rootHistSvc);

    clusterConvPhotonSiTRT = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clusterConvPhotonSiTRT","Clusters from Converted Photons - SiTRT", "/MONITORING/clusterConvPhotonSiTRT/", rootHistSvc);

    clusterUnconvPhoton = std::make_unique<egammaMonitoring::ClusterHistograms>(
      "clusterUnconvPhoton","Clusters from Converted Photons", "/MONITORING/clusterUnconvPhoton/", rootHistSvc);

    truthPhotonAll = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthPhotonAll","truthPhotonAll", "/MONITORING/truthPhotonAll/", rootHistSvc);

    truthPhotonAllUnconv = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthPhotonAllUnconv","truthPhotonAllUnconv", "/MONITORING/truthPhotonAllUnconv/", rootHistSvc);

    truthPhotonAllConv = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthPhotonAllConv","truthPhotonAllConv", "/MONITORING/truthPhotonAllConv/", rootHistSvc);

    truthPhotonRecoPhoton = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthPhotonRecoPhoton","truthPhotonRecoPhoton", "/MONITORING/truthPhotonRecoPhoton/", rootHistSvc);

    truthPhotonRecoPhotonOrElectron = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthPhotonRecoPhotonOrElectron","truthPhotonRecoPhotonOrElectron", "/MONITORING/truthPhotonRecoPhotonOrElectron/", rootHistSvc);

    truthPhotonConvPhoton = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthConvPhoton","truthConvPhoton", "/MONITORING/truthConvPhoton/", rootHistSvc);

    truthPhotonConvRecoConv = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthConvRecoConv","truthConvRecoConv", "/MONITORING/truthConvRecoConv/", rootHistSvc);

    truthPhotonConvRecoConv1Si = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthConvRecoConv1Si","truthConvRecoConv1Si", "/MONITORING/truthConvRecoConv1Si/", rootHistSvc);

    truthPhotonConvRecoConv1TRT = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthConvRecoConv1TRT","truthConvRecoConv1TRT", "/MONITORING/truthConvRecoConv1TRT/", rootHistSvc);

    truthPhotonConvRecoConv2Si = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthConvRecoConv2Si","truthConvRecoConv2Si", "/MONITORING/truthConvRecoConv2Si/", rootHistSvc);

    truthPhotonConvRecoConv2TRT = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthConvRecoConv2TRT","truthConvRecoConv2TRT", "/MONITORING/truthConvRecoConv2TRT/", rootHistSvc);

    truthPhotonConvRecoConv2SiTRT = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthConvRecoConv2SiTRT","truthConvRecoConv2SiTRT", "/MONITORING/truthConvRecoConv2SiTRT/", rootHistSvc);

    truthPhotonConvRecoUnconv= std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthConvRecoUnconv","truthConvRecoUnconv", "/MONITORING/truthConvRecoUnconv/", rootHistSvc);

    truthPhotonUnconvPhoton= std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthUnconvPhoton","truthUnconvPhoton", "/MONITORING/truthUnconvPhoton/", rootHistSvc);

    truthPhotonUnconvRecoConv= std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthUnconvRecoConv","truthUnconvRecoConv", "/MONITORING/truthUnconvRecoConv/", rootHistSvc);

    truthPhotonUnconvRecoConv1Si = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthUnconvRecoConv1Si","truthUnconvRecoConv1Si", "/MONITORING/truthUnconvRecoConv1Si/", rootHistSvc);

    truthPhotonUnconvRecoConv1TRT = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthUnconvRecoConv1TRT","truthUnconvRecoConv1TRT", "/MONITORING/truthUnconvRecoConv1TRT/", rootHistSvc);

    truthPhotonUnconvRecoConv2Si = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthUnconvRecoConv2Si","truthUnconvRecoConv2Si", "/MONITORING/truthUnconvRecoConv2Si/", rootHistSvc);

    truthPhotonUnconvRecoConv2TRT = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthUnconvRecoConv2TRT","truthUnconvRecoConv2TRT", "/MONITORING/truthUnconvRecoConv2TRT/", rootHistSvc);

    truthPhotonUnconvRecoConv2SiTRT = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthUnconvRecoConv2SiTRT","truthUnconvRecoConv2SiTRT", "/MONITORING/truthUnconvRecoConv2SiTRT/", rootHistSvc);

    truthPhotonUnconvRecoUnconv = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "truthUnconvRecoUnconv","truthUnconvRecoUnconv", "/MONITORING/truthUnconvRecoUnconv/", rootHistSvc);

    recoPhotonUnconvLooseLH = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonUnconvLooseLH","LLH Photons Reco Photon", "/MONITORING/recoPhotonUnconvLooseLH/", rootHistSvc);

    recoPhotonUnconvTightLH = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonUnconvTightLH","TLH Photons Reco Photon", "/MONITORING/recoPhotonUnconvTightLH/", rootHistSvc);

    recoPhotonConvLooseLH = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonConvLooseLH","LLH Photons Reco Photon", "/MONITORING/recoPhotonConvLooseLH/", rootHistSvc);

    recoPhotonConvTightLH = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonConvTightLH","LLH Photons Reco Photon", "/MONITORING/recoPhotonConvTightLH/", rootHistSvc);

    recoPhotonUnconvIsoFixedCutTight = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonUnconvIsoFixedCutTight","Isolation Fixed Cut Tight Photons Reco Photon", "/MONITORING/recoPhotonUnconvIsoFixedCutTight/", rootHistSvc);

    recoPhotonUnconvIsoFixedCutTightCaloOnly = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonUnconvIsoFixedCutTightCaloOnly","Isolation Fixed Cut Tight Calo Only Photons Reco Photon", "/MONITORING/recoPhotonUnconvIsoFixedCutTightCaloOnly/", rootHistSvc);

    recoPhotonUnconvIsoFixedCutLoose = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonUnconvIsoFixedCutLoose","Isolation Fixed Cut Loose Photons Reco Photon", "/MONITORING/recoPhotonUnconvIsoFixedCutLoose/", rootHistSvc);

    recoPhotonConvIsoFixedCutTight = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonConvIsoFixedCutTight","Isolation Fixed Cut Tight Photons Reco Photon", "/MONITORING/recoPhotonConvIsoFixedCutTight/", rootHistSvc);

    recoPhotonConvIsoFixedCutTightCaloOnly = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonConvIsoFixedCutTightCaloOnly","Isolation Fixed Cut Tight Calo Only Photons Reco Photon", "/MONITORING/recoPhotonConvIsoFixedCutTightCaloOnly/", rootHistSvc);

    recoPhotonConvIsoFixedCutLoose = std::make_unique<egammaMonitoring::TruthPhotonHistograms>(
      "recoPhotonConvIsoFixedCutLoose","Isolation Fixed Cut Loose Photons Reco Photon", "/MONITORING/recoPhotonConvIsoFixedCutLoose/", rootHistSvc);

    // for the track monitoring, consider
    // 4 track types (TRTSA or not, highpt or not) and
    // 4 truth matching types (all / e / not e / pion / no truth match)
    static const std::string typeMatch[5] = { "", "MatchElectron", "NotElectron", "MatchPion", "NotMatched" };
    static const std::string typeTrk[2]   = { "", "TRT" };
    static const std::string ptTrk[2]     = { "", "highpT" };
    static const std::string ctypeMatch[5] = { "", " match to electrons", " not matched to electrons", " match to pions", " not matched" };
    static const std::string ctypeTrk[2]   = { "", " TRTSA" };
    static const std::string cptTrk[2]     = { "", " pT > 3 GeV" };

    for (int im = 0; im < 5; im++) {
      std::string mN = typeMatch[im];
      for (int it = 0; it < 2; it++) {
	std::string tN = typeTrk[it];
	for (int ip = 0; ip < 2; ip++) {
	  std::string pN = ptTrk[ip];

	  std::string nN = "InDetTracks" + typeTrk[it] + typeMatch[im] + ptTrk[ip];
	  std::string fN = "/MONITORING/"+nN+"/";
	  std::string cN = "InDet Tracks" + ctypeTrk[it] + ctypeMatch[im] + cptTrk[ip];

	  ATH_MSG_INFO("Creating histograms for " << nN);
	  mapTrkHistograms[nN] = std::make_unique<egammaMonitoring::TrackHistograms>(
	    nN.c_str(), cN.c_str(),fN.c_str(),rootHistSvc);
	  ATH_CHECK(mapTrkHistograms[nN]->initializePlots());
	}
      }
    }

    ATH_CHECK(recoPhotonAll->initializePlots());
    ATH_CHECK(truthPhotonAll->initializePlots());
    ATH_CHECK(truthPhotonAllUnconv->initializePlots());
    ATH_CHECK(truthPhotonAllConv->initializePlots());
    ATH_CHECK(truthPhotonRecoPhoton->initializePlots());
    ATH_CHECK(truthPhotonRecoPhotonOrElectron->initializePlots());
    ATH_CHECK(truthPhotonConvPhoton->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv1Si->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv1TRT->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv2Si->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv2TRT->initializePlots());
    ATH_CHECK(truthPhotonConvRecoConv2SiTRT->initializePlots());
    ATH_CHECK(truthPhotonConvRecoUnconv->initializePlots());
    ATH_CHECK(truthPhotonUnconvPhoton->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv1Si->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv1TRT->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv2Si->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv2TRT->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoConv2SiTRT->initializePlots());
    ATH_CHECK(truthPhotonUnconvRecoUnconv->initializePlots());
    ATH_CHECK(recoPhotonUnconvLooseLH->initializePlots());
    ATH_CHECK(recoPhotonUnconvTightLH->initializePlots());
    ATH_CHECK(recoPhotonConvLooseLH->initializePlots());
    ATH_CHECK(recoPhotonConvTightLH->initializePlots());
    ATH_CHECK(recoPhotonUnconvIsoFixedCutTight->initializePlots());
    ATH_CHECK(recoPhotonUnconvIsoFixedCutTightCaloOnly->initializePlots());
    ATH_CHECK(recoPhotonUnconvIsoFixedCutLoose->initializePlots());
    ATH_CHECK(recoPhotonConvIsoFixedCutTight->initializePlots());
    ATH_CHECK(recoPhotonConvIsoFixedCutTightCaloOnly->initializePlots());
    ATH_CHECK(recoPhotonConvIsoFixedCutLoose->initializePlots());

    ATH_CHECK(clusterConvPhoton->initializePlots());
    ATH_CHECK(clusterConvPhotonSi->initializePlots());
    ATH_CHECK(clusterConvPhotonSiSi->initializePlots());
    ATH_CHECK(clusterConvPhotonTRT->initializePlots());
    ATH_CHECK(clusterConvPhotonTRTTRT->initializePlots());
    ATH_CHECK(clusterConvPhotonSiTRT->initializePlots());
    ATH_CHECK(clusterUnconvPhoton->initializePlots());

  } // gamma Hists

  if (m_sampleType != "gamma") {
    ATH_CHECK(m_ElectronsKey.initialize());
    ATH_CHECK(m_GSFTrackParticlesKey.initialize());
  }

  if ("electron" == m_sampleType) {
    //*****************ID selectors (3 levels)********************
    ATH_CHECK(m_LooseLH.retrieve());
    ATH_CHECK(m_MediumLH.retrieve());
    ATH_CHECK(m_TightLH.retrieve());

    ATH_CHECK(m_FwdElectronsKey.initialize(!m_FwdElectronsKey.empty()));

    m_dR1 = new TH1D("dR1",";#Delta R;Events",1000,0,0.1);
    ATH_CHECK(rootHistSvc->regHist("/MONITORING/Check/dR1",m_dR1));
    m_dR2 = new TH1D("dR2",";#Delta R;Events",1000,0,0.1);
    ATH_CHECK(rootHistSvc->regHist("/MONITORING/Check/dR2",m_dR2));
  }

  if ("gamma" == m_sampleType) {
    //*****************ID selectors (2 levels)********************
    ATH_CHECK(m_Loose_Photon.retrieve());
    ATH_CHECK(m_Tight_Photon.retrieve());

    //*****************Iso Requirements********************
    ATH_CHECK(m_IsoFixedCutTight.retrieve());
    ATH_CHECK(m_IsoFixedCutTightCaloOnly.retrieve());
    ATH_CHECK(m_IsoFixedCutLoose.retrieve());

    ATH_CHECK(m_PhotonsKey.initialize());
  }

  //***************** The handles used whatever the sample********************
  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_InDetTrackParticlesKey.initialize());

  if (m_sampleType != "dataZ") {
    ATH_CHECK(m_egTruthParticlesKey.initialize());
    ATH_CHECK(m_truthParticlesKey.initialize());

    //*****************MC Truth Classifier Requirement********************
    ATH_CHECK(m_mcTruthClassifier.retrieve());
  }

  return StatusCode::SUCCESS;
}

StatusCode EgammaMonitoring::execute() {

  const EventContext& ctx = Gaudi::Hive::currentContext();
  
  // Retrieve things from the event store
  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx);
  const float mu = eventInfo->averageInteractionsPerCrossing();

  // Retrieve indet track particles
  SG::ReadHandle<xAOD::TrackParticleContainer > InDetTPs(m_InDetTrackParticlesKey, ctx);

  if ("dataZ" == m_sampleType) {
    return this->ZeeSelection(mu,ctx);
  }

  // Retrieve egamma truth particles
  SG::ReadHandle<xAOD::TruthParticleContainer> egTruthParticles (m_egTruthParticlesKey, ctx);

  // Retrieve truth particles
  SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_truthParticlesKey, ctx);

  if ("electron" == m_sampleType) {

    SG::ReadHandle<xAOD::ElectronContainer > RecoEl(m_ElectronsKey, ctx);
    SG::ReadHandle<xAOD::TrackParticleContainer > GSFTracks(m_GSFTrackParticlesKey, ctx);

    ATH_MSG_DEBUG( "------------ Truth Egamma Container ---------------" );
    for (const auto *egtruth : *egTruthParticles) {

      if (!egtruth) continue;

      const xAOD::Electron *electron = xAOD::EgammaHelpers::getRecoElectron(egtruth);

      if (!electron) continue;

      clusterPromptAll->fill(*electron,mu);
      if (egtruth->pt() > 10*Gaudi::Units::GeV) {
        clusterPrompt10GeV->fill(*electron,mu);
      }
    }

    ATH_MSG_DEBUG( "------------ Truth Particles Container ---------------" );
    unsigned int promptElectronTruthIndex = std::numeric_limits<unsigned int>::max();
    for (const auto *truth : *truthParticles) {

      if (!truth) continue;
      if (std::abs(truth->pdgId()) != 11) continue;

      auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
      MCTruthPartClassifier::ParticleOrigin TO = res.second;
      MCTruthPartClassifier::ParticleType TT = res.first;

      if (msgLvl(MSG::DEBUG)) {

	bool haspVtx = truth->hasProdVtx();
	double rProd = -1;
	if (haspVtx) {
	  const xAOD::TruthVertex* pvtx = truth->prodVtx();
	  rProd = pvtx->perp();
	}
	bool hasdVtx = truth->hasDecayVtx();
	double rDec = -1;
	if (hasdVtx) {
	  const xAOD::TruthVertex* dvtx = truth->decayVtx();
	  rDec = dvtx->perp();
	}

	ATH_MSG_DEBUG( " ******** Truth electron found: "
		       << truth 
		       << " TO  " << TO
		       << " TT   " << TT
		       << " Rprod = " << rProd
		       << " Rdec = " << rDec
		       << " eventNumber  " << eventInfo->eventNumber() );
      }

      // Check if it is the prompt electron
      if (TO == MCTruthPartClassifier::SingleElec &&
          TT == MCTruthPartClassifier::IsoElectron && MC::isSingleParticle(truth)) {
        truthPromptElectronAll->fill(truth);
        promptElectronTruthIndex = truth->index();
      }

      // Check that it is not from geant4 (but electron from photon conversion are kept)
      if (TT != MCTruthPartClassifier::NonPrimary) truthElectronAll->fill(truth);

    }

    ATH_MSG_DEBUG( "------------ InDetTracks ---------------" );

    for (const auto *tp : *InDetTPs) {

      if (!tp) continue;

      uint8_t nPi = 0;
      uint8_t nSCT = 0;

      tp->summaryValue(nPi, xAOD::numberOfPixelHits);
      tp->summaryValue(nSCT, xAOD::numberOfSCTHits);

      if ((nPi + nSCT) < 7) continue;

      const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(*tp);

      if (!truth || std::abs(truth->pdgId()) != 11) continue;

      auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
      MCTruthPartClassifier::ParticleOrigin TO = res.second;
      MCTruthPartClassifier::ParticleType TT = res.first;
      if (TO == MCTruthPartClassifier::SingleElec &&
          TT == MCTruthPartClassifier::IsoElectron &&
          truth->index() == promptElectronTruthIndex) {

        // we found the track from the prompt electron
        // let's count it
        truthPromptElectronWithTrack->fill(truth);
        break;

      } else {
        const xAOD::TruthParticle *firstElTruth = xAOD::EgammaHelpers::getBkgElectronMother(truth);
        if (!firstElTruth) continue;
        // this is not a prompt electron, we need to check the parents
        // but we need to make sure that we double count if already found the track
        // foundPromptElectron will check that

        if (firstElTruth->index() == promptElectronTruthIndex) {
	  truthPromptElectronWithTrack->fill(firstElTruth);
	  break;
        }

      }

    }

    ATH_MSG_DEBUG( "------------ GSFTracks ---------------" );
    for (const auto *gsf : *GSFTracks) {

      if (!gsf) continue;

      const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(*gsf);

      if (!truth) continue;

      uint8_t nPi = 0;
      uint8_t nSCT = 0;

      gsf->summaryValue(nPi, xAOD::numberOfPixelHits);
      gsf->summaryValue(nSCT, xAOD::numberOfSCTHits);

      if ((nPi + nSCT) < 7) continue;

      auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
      MCTruthPartClassifier::ParticleOrigin TO = res.second;
      MCTruthPartClassifier::ParticleType TT = res.first;
      if (msgLvl(MSG::DEBUG)) {
	auto res2 = m_mcTruthClassifier->checkOrigOfBkgElec(truth);
	MCTruthPartClassifier::ParticleOrigin bkgTO = res2.second;
	MCTruthPartClassifier::ParticleType bkgTT = res2.first;
	ATH_MSG_DEBUG(" ** Truth particle associated to track Found: "
		      << truth
		      << " index    " << truth->index()
		      << " bkg T0  " << bkgTO
		      << " bkg TT   " << bkgTT
		      << " T0  " << TO
		      << " TT   " << TT
		      << " eventNumber  " << eventInfo->eventNumber() );
      }
      if (TO == MCTruthPartClassifier::SingleElec &&
          TT == MCTruthPartClassifier::IsoElectron &&
          truth->index() == promptElectronTruthIndex) {

        // we found the track from the prompt electron
        // let's count it
        truthPromptElectronWithGSFTrack->fill(truth);
        break;

      } else {
        const xAOD::TruthParticle *firstElTruth = xAOD::EgammaHelpers::getBkgElectronMother(truth);
        if (!firstElTruth) continue;
        ATH_MSG_DEBUG( "********----- Getting Mother-----********" );
        ATH_MSG_DEBUG( " STATUS  " << firstElTruth << " index    " << firstElTruth->index() );
        // this is not a prompt electron, we need to check the parents
        // but we need to make sure that we double count if already found the track
        // foundPromptElectron will check that
        if (firstElTruth->index() == promptElectronTruthIndex) {
	  truthPromptElectronWithGSFTrack->fill(firstElTruth);
	  break;
        }

      }

    }

    ATH_MSG_DEBUG( "------------ Reco central electrons ---------------" );
    bool foundPromptElectron = false;

    for (const auto *elrec : *RecoEl) {

      if (!elrec) continue;

      bool toFill = false;

      clusterAll->fill(*elrec,mu);
      recoElectronAll->fill(*elrec);
      showerShapesAll->fill(*elrec);
      isolationAll->fill(*elrec);
      if (elrec->pt() > 10*Gaudi::Units::GeV) {
	cluster10GeV->fill(*elrec,mu);
        showerShapes10GeV->fill(*elrec);
      }

      const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(*elrec);
      if (!truth) continue;
      truthElectronRecoElectronAll->fill(truth, elrec);
      if (std::abs(truth->pdgId()) != 11 || foundPromptElectron)
	continue;
      const xAOD::TruthParticle *elTruth(nullptr);
      auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
      MCTruthPartClassifier::ParticleOrigin TO = res.second;
      MCTruthPartClassifier::ParticleType TT = res.first;
      if (TO == MCTruthPartClassifier::SingleElec &&
          TT == MCTruthPartClassifier::IsoElectron &&
          truth->index() == promptElectronTruthIndex) {
	toFill = true;
	elTruth = truth;
      } else {
        const xAOD::TruthParticle *firstElTruth =
	  xAOD::EgammaHelpers::getBkgElectronMother(truth);
        if (!firstElTruth) continue;
        // this is not a prompt electron, we need to check the parents
        // but we need to make sure that we double count if already found the track
        // foundPromptElectron will check that
        if (firstElTruth->index() == promptElectronTruthIndex) {
	  toFill = true;
	  elTruth = firstElTruth;
	}
      }
      if (toFill) {
	foundPromptElectron = true;
	truthPromptElectronWithReco->fill(elTruth,elrec);
	truthPromptElectronWithRecoTrack->fill(elTruth,elrec); // yes the same. This is different for fwd
	if (m_LooseLH->accept(elrec))
	  truthRecoElectronLooseLH->fill(elTruth,elrec);
	if (m_MediumLH->accept(elrec))
	  truthRecoElectronMediumLH->fill(elTruth,elrec);
	if (m_TightLH->accept(elrec))
	  truthRecoElectronTightLH->fill(elTruth,elrec);
      }

    } // RecoEl Loop

    if (!m_FwdElectronsKey.empty()) {
      SG::ReadHandle<xAOD::ElectronContainer > RecoFwdEl(m_FwdElectronsKey, ctx);
      for (const auto *el : *RecoFwdEl) {
	// This would be very weird ??
	if (!el)
	  continue;

	clusterAll->fill(*el,mu);
	recoElectronAll->fill(*el);
	showerShapesAll->fill(*el);
	if (el->pt() > 10*Gaudi::Units::GeV) {
	  cluster10GeV->fill(*el,mu);
	  showerShapes10GeV->fill(*el);
	}
	bool toFill = false;

	const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(*el);

	if (!truth) continue;
	//truthElectronRecoFwdElectronAll->fill(truth, elrec); // to be done

	// if the prompt electron is not too far from the reco candidate, which might be matched to a photon
	// use this prompt as the matched particle instead
	if (std::abs(truth->pdgId()) != 11 && promptElectronTruthIndex < truthParticles->size()) {
	  const xAOD::TruthParticle *vtruth = truthParticles->at(promptElectronTruthIndex);
	  double dR1 = vtruth->p4().DeltaR(el->caloCluster()->p4());
	  double dR2 = truth->p4().DeltaR(el->caloCluster()->p4());
	  m_dR1->Fill(dR1);
	  m_dR2->Fill(dR2);
	  if (dR1 < 0.05) // threshold to optimize
	    truth = vtruth;
	}

	if (std::abs(truth->pdgId()) != 11)
	  continue;
	const xAOD::TruthParticle *elTruth(nullptr);
	auto res = m_mcTruthClassifier->particleTruthClassifier(truth);
	if (res.second == MCTruthPartClassifier::SingleElec &&
	    res.first == MCTruthPartClassifier::IsoElectron &&
	    truth->index() == promptElectronTruthIndex) {
	  toFill = true;
	  elTruth = truth;
	} else {
	  const xAOD::TruthParticle *firstElTruth =
	    xAOD::EgammaHelpers::getBkgElectronMother(truth);
	  if (!firstElTruth) continue;
	  if (firstElTruth->index() == promptElectronTruthIndex) {
	    toFill = true;
	    elTruth = firstElTruth;
	  }
	}
	if (toFill) {
	  if (foundPromptElectron) {
	    ATH_MSG_DEBUG("A fwd electron also reconstructed as central "
			  "true eta = " << elTruth->eta() << " event = "
			  << eventInfo->eventNumber());
	    m_CenFwdOverlap[0]++;
	    if (el->nTrackParticles() > 0)
	      m_CenFwdOverlap[1]++;
	  } else {
	    foundPromptElectron = true;
	    truthPromptElectronWithReco->fill(elTruth,el);
	    if (el->nTrackParticles() > 0)
	      truthPromptElectronWithRecoTrack->fill(elTruth,el);
	  }
	}
      } // loop on fwdEl
    }

  } // if electron

  if ("gamma" == m_sampleType) {

    ATH_MSG_DEBUG( "------------ Photons ---------------" );
    SG::ReadHandle<xAOD::PhotonContainer > RecoPh(m_PhotonsKey, ctx);

    for (const auto *phrec : *RecoPh) {

      if (!phrec) continue;

      recoPhotonAll->fill(*phrec);
      isolationAll->fill(*phrec);
      showerShapesAll->fill(*phrec);
      clusterAll->fill(*phrec,mu);
      if (phrec->pt() > 10*Gaudi::Units::GeV) {
        cluster10GeV->fill(*phrec,mu);
      }
      if (phrec->pt() > 10*Gaudi::Units::GeV){ 
        showerShapes10GeV->fill(*phrec);
      }

    } // RecoPh Loop

    for (const auto *egtruth : *egTruthParticles) {

      if (!egtruth) continue;

      truthPhotonAll->fill(*egtruth, mu);

      bool isTrueConv = xAOD::EgammaHelpers::isTrueConvertedPhoton(egtruth);
      bool isTrueLateConv = xAOD::EgammaHelpers::isTrueConvertedPhoton(egtruth, 1200) and !isTrueConv;
      const xAOD::Photon *photon = xAOD::EgammaHelpers::getRecoPhoton(egtruth);
      const xAOD::Electron *electron = xAOD::EgammaHelpers::getRecoElectron(egtruth);

      if(isTrueConv) truthPhotonAllConv->fill(*egtruth, mu);
      if(!isTrueConv && !isTrueLateConv) truthPhotonAllUnconv->fill(*egtruth, mu);

      if(photon || electron)
        truthPhotonRecoPhotonOrElectron->fill(*egtruth, mu);

      if (!photon) continue;

      truthPhotonRecoPhoton->fill(*egtruth, mu);
      clusterPromptAll->fill(*photon,mu);
      if (egtruth->pt() > 10*Gaudi::Units::GeV) {
        clusterPrompt10GeV->fill(*photon,mu);
      }

      bool isRecoConv = xAOD::EgammaHelpers::isConvertedPhoton(photon);
      xAOD::EgammaParameters::ConversionType convType = xAOD::EgammaHelpers::conversionType(photon);

      if (isTrueConv) {

        truthPhotonConvPhoton->fill(*egtruth, mu);

        if (isRecoConv) {

          truthPhotonConvRecoConv->fill(*egtruth, mu);

          clusterConvPhoton->fill(*photon,mu);

          if (convType == xAOD::EgammaParameters::singleSi) {
            truthPhotonConvRecoConv1Si->fill(*egtruth, mu);
            clusterConvPhotonSi->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::singleTRT) {
            truthPhotonConvRecoConv1TRT->fill(*egtruth, mu);
            clusterConvPhotonTRT->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleSi) {
            truthPhotonConvRecoConv2Si->fill(*egtruth, mu);
            clusterConvPhotonSiSi->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleTRT) {
            truthPhotonConvRecoConv2TRT->fill(*egtruth, mu);
            clusterConvPhotonTRTTRT->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleSiTRT) {
            truthPhotonConvRecoConv2SiTRT->fill(*egtruth, mu);
            clusterConvPhotonSiTRT->fill(*photon,mu); 
          }

          if (m_IsoFixedCutTight->accept(*photon)) recoPhotonConvIsoFixedCutTight->fill(*egtruth, mu);
          if (m_IsoFixedCutTightCaloOnly->accept(*photon)) recoPhotonConvIsoFixedCutTightCaloOnly->fill(*egtruth, mu);
          if (m_IsoFixedCutLoose->accept(*photon)) recoPhotonConvIsoFixedCutLoose->fill(*egtruth, mu);
          if (m_Loose_Photon->accept(photon)) recoPhotonConvLooseLH->fill(*egtruth, mu);
          if (m_Tight_Photon->accept(photon)) recoPhotonConvTightLH->fill(*egtruth, mu);
        } // isRecoConv
        else {
          truthPhotonConvRecoUnconv->fill(*egtruth, mu);
          clusterUnconvPhoton->fill(*photon,mu); 
        } 

      } //isTrueConv
      else if (!isTrueLateConv) {

        truthPhotonUnconvPhoton->fill(*egtruth, mu);

        if (isRecoConv) {
          truthPhotonUnconvRecoConv->fill(*egtruth, mu);

          if (convType == xAOD::EgammaParameters::singleSi) {
            truthPhotonUnconvRecoConv1Si->fill(*egtruth, mu);
            clusterConvPhotonSi->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::singleTRT) {
            truthPhotonUnconvRecoConv1TRT->fill(*egtruth, mu);
            clusterConvPhotonTRT->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleSi) {
            truthPhotonUnconvRecoConv2Si->fill(*egtruth, mu);
            clusterConvPhotonSiSi->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleTRT) {
            truthPhotonUnconvRecoConv2TRT->fill(*egtruth, mu);
            clusterConvPhotonTRTTRT->fill(*photon,mu);
          } else if (convType == xAOD::EgammaParameters::doubleSiTRT) {
            truthPhotonUnconvRecoConv2SiTRT->fill(*egtruth, mu);
            clusterConvPhotonSiTRT->fill(*photon,mu); 
          }
        }
        else           truthPhotonUnconvRecoUnconv->fill(*egtruth, mu);

        if (m_IsoFixedCutTight->accept(*photon)) recoPhotonUnconvIsoFixedCutTight->fill(*egtruth, mu);
        if (m_IsoFixedCutTightCaloOnly->accept(*photon)) recoPhotonUnconvIsoFixedCutTightCaloOnly->fill(*egtruth, mu);
        if (m_IsoFixedCutLoose->accept(*photon)) recoPhotonUnconvIsoFixedCutLoose->fill(*egtruth, mu);
        if (m_Loose_Photon->accept(photon)) recoPhotonUnconvLooseLH->fill(*egtruth, mu);
        if (m_Tight_Photon->accept(photon)) recoPhotonUnconvTightLH->fill(*egtruth, mu);
      } // !isTrueLateConv
    } //egtruth Loop

    //loop over InDetTrackParticles
    for (const auto *tp : *InDetTPs) {

      mapTrkHistograms["InDetTracks"]->fill(*tp, mu);
      if (matchedToElectron(*tp)) mapTrkHistograms["InDetTracksMatchElectron"]->fill(*tp, mu);
      else {
	mapTrkHistograms["InDetTracksNotElectron"]->fill(*tp, mu);
	if (matchedToPion(*tp)) mapTrkHistograms["InDetTracksMatchPion"]->fill(*tp, mu);
	else if (notMatchedToTruth(*tp)) mapTrkHistograms["InDetTracksNotMatched"]->fill(*tp, mu);
      }
      if (tp->pt() > 3000.){
        mapTrkHistograms["InDetTrackshighpT"]->fill(*tp, mu);
        if (matchedToElectron(*tp)) {
          mapTrkHistograms["InDetTracksMatchElectronhighpT"]->fill(*tp, mu);
        } else {
          mapTrkHistograms["InDetTracksNotElectronhighpT"]->fill(*tp, mu);
	  if (matchedToPion(*tp)){
	    mapTrkHistograms["InDetTracksMatchPionhighpT"]->fill(*tp, mu);
	  }
	  else if (notMatchedToTruth(*tp)){
	    mapTrkHistograms["InDetTracksNotMatchedhighpT"]->fill(*tp, mu);
	  }
	}
      }

      if (xAOD::EgammaHelpers::numberOfSiHits(tp)==0) { //TRTSA tracks
	mapTrkHistograms["InDetTracksTRT"]->fill(*tp, mu);
	if (matchedToElectron(*tp)) mapTrkHistograms["InDetTracksTRTMatchElectron"]->fill(*tp, mu);
	else {
	  mapTrkHistograms["InDetTracksTRTNotElectron"]->fill(*tp, mu);
	  if (matchedToPion(*tp)) mapTrkHistograms["InDetTracksTRTMatchPion"]->fill(*tp, mu);
	  else if (notMatchedToTruth(*tp)) mapTrkHistograms["InDetTracksTRTNotMatched"]->fill(*tp, mu);
	}
	if (tp->pt() > 3000.){
	  mapTrkHistograms["InDetTracksTRThighpT"]->fill(*tp, mu);
	  if (matchedToElectron(*tp)) mapTrkHistograms["InDetTracksTRTMatchElectronhighpT"]->fill(*tp, mu);
	  else {
	    mapTrkHistograms["InDetTracksTRTNotElectronhighpT"]->fill(*tp, mu);
	    if (matchedToPion(*tp)) mapTrkHistograms["InDetTracksTRTMatchPionhighpT"]->fill(*tp, mu);
	    else if (notMatchedToTruth(*tp)) mapTrkHistograms["InDetTracksTRTNotMatchedhighpT"]->fill(*tp, mu);
	  }
	}
      }

    }//loop over InDetTPs

  } // if gamma

  return StatusCode::SUCCESS;
}

// ******

StatusCode EgammaMonitoring::finalize() {


  if ("electron" == m_sampleType) {

    ATH_MSG_INFO("Number of events with electron reconstructed "
		 "as both Central and Forward "
		 << m_CenFwdOverlap[0]
		 << " and with a track for the Forward "
		 << m_CenFwdOverlap[1]);

    egammaMonitoring::EfficiencyPlot trackEfficiency("trackingEfficiency", "/MONITORING/trackingEfficiency/", rootHistSvc );
    ATH_CHECK(trackEfficiency.divide(truthPromptElectronWithTrack.get(), truthPromptElectronAll.get()));
    egammaMonitoring::EfficiencyPlot GSFEfficiency("GSFEfficiency", "/MONITORING/GSFEfficiency/", rootHistSvc );
    ATH_CHECK(GSFEfficiency.divide(truthPromptElectronWithGSFTrack.get(),truthPromptElectronWithTrack.get()));
    egammaMonitoring::EfficiencyPlot matchingEfficiency("matchingEfficiency", "/MONITORING/matchingEfficiency/", rootHistSvc );
    ATH_CHECK(matchingEfficiency.divide(truthPromptElectronWithRecoTrack.get(), truthPromptElectronWithGSFTrack.get()));
    egammaMonitoring::EfficiencyPlot reconstructionEfficiency("reconstructionEfficiency", "/MONITORING/reconstructionEfficiency/", rootHistSvc );
    ATH_CHECK(reconstructionEfficiency.divide(truthPromptElectronWithReco.get(), truthPromptElectronAll.get()));
    egammaMonitoring::EfficiencyPlot recoElectronLooseLHEfficiency("recoElectronLooseLHEfficiency", "/MONITORING/recoElectronLooseLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoElectronLooseLHEfficiency.divide(truthRecoElectronLooseLH.get(), truthPromptElectronAll.get()));
    egammaMonitoring::EfficiencyPlot recoElectronMediumLHEfficiency("recoElectronMediumLHEfficiency", "/MONITORING/recoElectronMediumLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoElectronMediumLHEfficiency.divide(truthRecoElectronMediumLH.get(), truthPromptElectronAll.get()));
    egammaMonitoring::EfficiencyPlot recoElectronTightLHEfficiency("recoElectronTightLHEfficiency", "/MONITORING/recoElectronTightLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoElectronTightLHEfficiency.divide(truthRecoElectronTightLH.get(), truthPromptElectronAll.get()));

  }

  else if ("gamma" == m_sampleType) {

    egammaMonitoring::EfficiencyPlot truthPhotonRecoPhotonEfficiency("truthPhotonRecoPhotonEfficiency", "/MONITORING/truthPhotonRecoPhotonEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonRecoPhotonEfficiency.divide(truthPhotonRecoPhoton.get(),truthPhotonAll.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonRecoPhotonOrElectronEfficiency("truthPhotonRecoPhotonOrElectronEfficiency", "/MONITORING/truthPhotonRecoPhotonOrElectronEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonRecoPhotonOrElectronEfficiency.divide(truthPhotonRecoPhotonOrElectron.get(),truthPhotonAll.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoEfficiency("truthPhotonConvRecoEfficiency", "/MONITORING/truthPhotonConvRecoEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoEfficiency.divide(truthPhotonConvPhoton.get(),truthPhotonAllConv.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoEfficiency("truthPhotonUnconvRecoEfficiency", "/MONITORING/truthPhotonUnconvRecoEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoEfficiency.divide(truthPhotonUnconvPhoton.get(),truthPhotonAllUnconv.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConvEfficiency("truthPhotonConvRecoConvEfficiency", "/MONITORING/truthPhotonConvRecoConvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConvEfficiency.divide(truthPhotonConvRecoConv.get(),truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv1SiEfficiency("truthPhotonConvRecoConv1SiEfficiency", "/MONITORING/truthPhotonConvRecoConv1SiEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv1SiEfficiency.divide( truthPhotonConvRecoConv1Si.get()   , truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv1TRTEfficiency("truthPhotonConvRecoConv1TRTEfficiency", "/MONITORING/truthPhotonConvRecoConv1TRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv1TRTEfficiency.divide(truthPhotonConvRecoConv1TRT.get()  , truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv2SiEfficiency("truthPhotonConvRecoConv2SiEfficiency", "/MONITORING/truthPhotonConvRecoConv2SiEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv2SiEfficiency.divide(truthPhotonConvRecoConv2Si.get()   , truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv2TRTEfficiency("truthPhotonConvRecoConv2TRTEfficiency", "/MONITORING/truthPhotonConvRecoConv2TRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv2TRTEfficiency.divide(truthPhotonConvRecoConv2TRT.get()  , truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoConv2SiTRTEfficiency("truthPhotonConvRecoConv2SiTRTEfficiency", "/MONITORING/truthPhotonConvRecoConv2SiTRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoConv2SiTRTEfficiency.divide(truthPhotonConvRecoConv2SiTRT.get(), truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonConvRecoUnconvEfficiency("truthPhotonConvRecoUnconvEfficiency", "/MONITORING/truthPhotonConvRecoUnconvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonConvRecoUnconvEfficiency.divide(truthPhotonConvRecoUnconv.get(), truthPhotonConvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConvEfficiency("truthPhotonUnconvRecoConvEfficiency", "/MONITORING/truthPhotonUnconvRecoConvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConvEfficiency.divide(truthPhotonUnconvRecoConv.get(),truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv1SiEfficiency("truthPhotonUnconvRecoConv1SiEfficiency", "/MONITORING/truthPhotonUnconvRecoConv1SiEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv1SiEfficiency.divide( truthPhotonUnconvRecoConv1Si.get()   , truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv1TRTEfficiency("truthPhotonUnconvRecoConv1TRTEfficiency", "/MONITORING/truthPhotonUnconvRecoConv1TRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv1TRTEfficiency.divide(truthPhotonUnconvRecoConv1TRT.get()  , truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv2SiEfficiency("truthPhotonUnconvRecoConv2SiEfficiency", "/MONITORING/truthPhotonUnconvRecoConv2SiEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv2SiEfficiency.divide(truthPhotonUnconvRecoConv2Si.get()   , truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv2TRTEfficiency("truthPhotonUnconvRecoConv2TRTEfficiency", "/MONITORING/truthPhotonUnconvRecoConv2TRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv2TRTEfficiency.divide(truthPhotonUnconvRecoConv2TRT.get()  , truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoConv2SiTRTEfficiency("truthPhotonUnconvRecoConv2SiTRTEfficiency", "/MONITORING/truthPhotonUnconvRecoConv2SiTRTEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoConv2SiTRTEfficiency.divide(truthPhotonUnconvRecoConv2SiTRT.get(), truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonUnconvRecoUnconvEfficiency("truthPhotonUnconvRecoUnconvEfficiency", "/MONITORING/truthPhotonUnconvRecoUnconvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonUnconvRecoUnconvEfficiency.divide(truthPhotonUnconvRecoUnconv.get(), truthPhotonUnconvPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonRecoConvEfficiency("truthPhotonRecoConvEfficiency", "/MONITORING/truthPhotonRecoConvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonRecoConvEfficiency.divide(truthPhotonConvRecoConv.get(), truthPhotonRecoPhoton.get()));
    egammaMonitoring::EfficiencyPlot truthPhotonRecoUnconvEfficiency("truthPhotonRecoUnconvEfficiency", "/MONITORING/truthPhotonRecoUnconvEfficiency/", rootHistSvc );
    ATH_CHECK(truthPhotonRecoUnconvEfficiency.divide(truthPhotonUnconvRecoUnconv.get(), truthPhotonRecoPhoton.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvIsoFixedCutTightEfficiency("recoPhotonUnconvIsoFixedCutTightEfficiency", "/MONITORING/recoPhotonUnconvIsoFixedCutTightEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvIsoFixedCutTightEfficiency.divide( recoPhotonUnconvIsoFixedCutTight.get(), truthPhotonUnconvRecoUnconv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvIsoFixedCutTightCaloOnlyEfficiency("recoPhotonUnconvIsoFixedCutTightCaloOnlyEfficiency", "/MONITORING/recoPhotonUnconvIsoFixedCutTightCaloOnlyEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvIsoFixedCutTightCaloOnlyEfficiency.divide( recoPhotonUnconvIsoFixedCutTightCaloOnly.get(), truthPhotonUnconvRecoUnconv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvIsoFixedCutLooseEfficiency("recoPhotonUnconvIsoFixedCutLooseEfficiency", "/MONITORING/recoPhotonUnconvIsoFixedCutLooseEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvIsoFixedCutLooseEfficiency.divide( recoPhotonUnconvIsoFixedCutLoose.get(), truthPhotonUnconvRecoUnconv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonConvIsoFixedCutTightEfficiency("recoPhotonConvIsoFixedCutTightEfficiency", "/MONITORING/recoPhotonConvIsoFixedCutTightEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvIsoFixedCutTightEfficiency.divide( recoPhotonConvIsoFixedCutTight.get(), truthPhotonConvRecoConv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonConvIsoFixedCutTightCaloOnlyEfficiency("recoPhotonConvIsoFixedCutTightCaloOnlyEfficiency", "/MONITORING/recoPhotonConvIsoFixedCutTightCaloOnlyEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvIsoFixedCutTightCaloOnlyEfficiency.divide( recoPhotonConvIsoFixedCutTightCaloOnly.get(), truthPhotonConvRecoConv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonConvIsoFixedCutLooseEfficiency("recoPhotonConvIsoFixedCutLooseEfficiency", "/MONITORING/recoPhotonConvIsoFixedCutLooseEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvIsoFixedCutLooseEfficiency.divide( recoPhotonConvIsoFixedCutLoose.get(), truthPhotonConvRecoConv.get()));
  
    egammaMonitoring::EfficiencyPlot recoPhotonConvLooseLHEfficiency("recoPhotonConvLooseLHEfficiency", "/MONITORING/recoPhotonConvLooseLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvLooseLHEfficiency.divide( recoPhotonConvLooseLH.get(), truthPhotonConvRecoConv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonConvTightLHEfficiency("recoPhotonConvTightLHEfficiency", "/MONITORING/recoPhotonConvTightLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonConvTightLHEfficiency.divide( recoPhotonConvTightLH.get(), truthPhotonConvRecoConv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvLooseLHEfficiency("recoPhotonUnconvLooseLHEfficiency", "/MONITORING/recoPhotonUnconvLooseLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvLooseLHEfficiency.divide( recoPhotonUnconvLooseLH.get(), truthPhotonUnconvRecoUnconv.get()));
    egammaMonitoring::EfficiencyPlot recoPhotonUnconvTightLHEfficiency("recoPhotonUnconvTightLHEfficiency", "/MONITORING/recoPhotonUnconvTightLHEfficiency/", rootHistSvc );
    ATH_CHECK(recoPhotonUnconvTightLHEfficiency.divide( recoPhotonUnconvTightLH.get(), truthPhotonUnconvRecoUnconv.get()));

    egammaMonitoring::WidthPlot truthPhotonRecoPhotonWidth("truthPhotonRecoPhotonWidth", "/MONITORING/truthPhotonRecoPhotonWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonRecoPhotonWidth.fill(truthPhotonRecoPhoton.get()));
    egammaMonitoring::WidthPlot truthPhotonConvPhotonWidth("truthPhotonConvPhotonWidth", "/MONITORING/truthPhotonConvPhotonWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvPhotonWidth.fill(truthPhotonConvPhoton.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConvWidth("truthPhotonConvRecoConvWidth", "/MONITORING/truthPhotonConvRecoConvWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConvWidth.fill(truthPhotonConvRecoConv.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv1SiWidth("truthPhotonConvRecoConv1SiWidth", "/MONITORING/truthPhotonConvRecoConv1SiWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv1SiWidth.fill(truthPhotonConvRecoConv1Si.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv1TRTWidth("truthPhotonConvRecoConv1TRTWidth", "/MONITORING/truthPhotonConvRecoConv1TRTWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv1TRTWidth.fill(truthPhotonConvRecoConv1TRT.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv2SiWidth("truthPhotonConvRecoConv2SiWidth", "/MONITORING/truthPhotonConvRecoConv2SiWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv2SiWidth.fill(truthPhotonConvRecoConv2Si.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv2TRTWidth("truthPhotonConvRecoConv2TRTWidth", "/MONITORING/truthPhotonConvRecoConv2TRTWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv2TRTWidth.fill(truthPhotonConvRecoConv2TRT.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoConv2SiTRTWidth("truthPhotonConvRecoConv2SiTRTWidth", "/MONITORING/truthPhotonConvRecoConv2SiTRTWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoConv2SiTRTWidth.fill(truthPhotonConvRecoConv2SiTRT.get()));
    egammaMonitoring::WidthPlot truthPhotonConvRecoUnconvWidth("truthPhotonConvRecoUnconvWidth", "/MONITORING/truthPhotonConvRecoUnconvWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonConvRecoUnconvWidth.fill(truthPhotonConvRecoUnconv.get()));
    egammaMonitoring::WidthPlot truthPhotonUnconvPhotonWidth("truthPhotonUnconvPhotonWidth", "/MONITORING/truthPhotonUnconvPhotonWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonUnconvPhotonWidth.fill(truthPhotonUnconvPhoton.get()));
    egammaMonitoring::WidthPlot truthPhotonUnconvRecoConvWidth("truthPhotonUnconvRecoConvWidth", "/MONITORING/truthPhotonUnconvRecoConvWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonUnconvRecoConvWidth.fill(truthPhotonUnconvRecoConv.get()));
    egammaMonitoring::WidthPlot truthPhotonUnconvRecoUnconvWidth("truthPhotonUnconvRecoUnconvWidth", "/MONITORING/truthPhotonUnconvRecoUnconvWidth/", rootHistSvc);
    ATH_CHECK(truthPhotonUnconvRecoUnconvWidth.fill(truthPhotonUnconvRecoUnconv.get()));
  } else if ("dataZ" == m_sampleType) {
    ATH_CHECK(ZeePostProc());
  }

  return StatusCode::SUCCESS;
}


bool EgammaMonitoring::matchedToElectron(const xAOD::TrackParticle& tp) {
  const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(tp);
  return truth && abs(truth->pdgId())==11;
}


bool EgammaMonitoring::matchedToPion(const xAOD::TrackParticle& tp) {
  const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(tp);
  return truth && abs(truth->pdgId())==211;
}


bool EgammaMonitoring::notMatchedToTruth(const xAOD::TrackParticle& tp) {
  const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(tp);
  return !truth;
}

StatusCode EgammaMonitoring::ZeeSelection(float mu, const EventContext& ctx) {
  SG::ReadHandle<xAOD::ElectronContainer > RecoEl(m_ElectronsKey, ctx);
  SG::ReadHandle<xAOD::TrackParticleContainer > GSFTracks(m_GSFTrackParticlesKey, ctx);

  const xAOD::Electron *e1 = nullptr, *e2 = nullptr;
  double pTL = -9e9, pTSL = -9e9;
  for (const auto *el : *RecoEl) {
    double pt = el->pt();
    if (pt < 15e3)
      continue;
    if (pt > pTL) {
      e2   = e1;
      pTSL = pTL;
      e1   = el;
      pTL  = pt;
    } else if (pt > pTSL) {
      e2   = el;
      pTSL = pt;
    }
  }

  if (!e2)
    return StatusCode::SUCCESS;

  m_diElectron->fill(*e1,*e2,mu);

  m_clusterReco->fill(*e1);
  m_clusterReco->fill(*e2);
  recoElectronAll->fill(*e1);
  recoElectronAll->fill(*e2);

  showerShapesAll->fill(*e1);
  showerShapesAll->fill(*e2);
  if (e1->pt() > 10*Gaudi::Units::GeV)
    showerShapes10GeV->fill(*e1);
  if (e2->pt() > 10*Gaudi::Units::GeV)
    showerShapes10GeV->fill(*e2);
  isolationAll->fill(*e1);
  isolationAll->fill(*e2);

  return StatusCode::SUCCESS;
}

StatusCode EgammaMonitoring::ZeePostProc() {

  TH1D *hRef = (TH1D*)m_diElectron->histoMap["mass"];
  double xmi = 66, xma = 116, eps = 0.01;
  int ib1 = hRef->FindBin(xmi+eps);
  int ib2 = hRef->FindBin(xma-eps);
  TH1D *hmee = new TH1D("mee",";m_{ee} [GeV];Events / 0.5 GeV",ib2-ib1+1,xmi,xma);
  for (int ib = ib1; ib <= ib2; ib++) {
    double c  = hRef->GetBinContent(ib);
    double ec = hRef->GetBinError(ib);
    hmee->SetBinContent(ib-ib1+1,c);
    hmee->SetBinError(ib-ib1+1,ec);
  }
  ATH_CHECK(rootHistSvc->regHist("/MONITORING/Data/mee",hmee));

  TString vsX[3] = { "Et", "Etetale0p8", "Eta" };
  TString coo[3] = { "x", "x", "y" };
  for (int iL = 0; iL < 5; iL++) {
    TH3D *nRef;
    if (iL < 4)
      nRef = m_clusterReco->m_histo3DMap[Form("hNcellsvseteta_Lr%i",iL)];
    else
      nRef = recoElectronAll->histo3DMap["eteta_eop"];
    for (int iX = 0; iX < 3; iX++) {
      if (iL == 4 && iX == 2)
	continue;
      // nCells vs eta
      if (iX == 1) {
	int ieta08 = nRef->GetYaxis()->FindBin(0.79);
	nRef->GetYaxis()->SetRange(1,ieta08);
      } else if (iX == 2) {
	nRef->GetYaxis()->SetRange(1,25);
      }
      TH2D *nRefvsX = (TH2D*)nRef->Project3D(Form("z%s",coo[iX].Data()));
      TString pN;
      if (iL < 4)
	pN = Form("pn%ivs%s",iL,vsX[iX].Data());
      else
	pN = Form("pEopvs%s",vsX[iX].Data());
      TProfile *pRefvsX = nRefvsX->ProfileX(pN.Data());
      pRefvsX->SetTitle(pN.Data());
      pRefvsX->GetXaxis()->SetTitle(iX < 2 ? "E_{T} [GeV]" : "|#eta|");
      pRefvsX->GetYaxis()->SetTitle(iL < 4 ? Form("nCells_Lr%i",iL) : "E/p");
      ATH_CHECK(rootHistSvc->regHist(
	Form("/MONITORING/Data/%s",pRefvsX->GetName()),pRefvsX));
      delete nRefvsX;
    }
  }
  return StatusCode::SUCCESS;

}
