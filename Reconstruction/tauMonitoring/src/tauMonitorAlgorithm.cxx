/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "tauMonitorAlgorithm.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "xAODCore/ShallowCopy.h"
#include "AthContainers/ConstAccessor.h"
#include <algorithm>

using Gaudi::Units::GeV;
using namespace Monitored;

tauMonitorAlgorithm::tauMonitorAlgorithm(const std::string &name,
                                         ISvcLocator *pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator)
{}

tauMonitorAlgorithm::~tauMonitorAlgorithm() {}

StatusCode tauMonitorAlgorithm::initialize() {

  ATH_CHECK(m_TauContainerKey.initialize());

  m_offlineGNTauScoreDecorKey = m_TauContainerKey.key() + "." + m_offlineGNTauScoreDecorKey.key();
  ATH_CHECK(m_offlineGNTauScoreDecorKey.initialize());
 
  m_offlineGNTauTransScoreDecorKey = m_TauContainerKey.key() + "." + m_offlineGNTauTransScoreDecorKey.key();
  ATH_CHECK(m_offlineGNTauTransScoreDecorKey.initialize());

  m_offlineGNTauLooseWPDecorKey = m_TauContainerKey.key() + "." + m_offlineGNTauLooseWPDecorKey.key();
  ATH_CHECK(m_offlineGNTauLooseWPDecorKey.initialize());

  m_offlineGNTauMediumWPDecorKey = m_TauContainerKey.key() + "." + m_offlineGNTauMediumWPDecorKey.key();
  ATH_CHECK(m_offlineGNTauMediumWPDecorKey.initialize());  

  m_offlineGNTauTightWPDecorKey = m_TauContainerKey.key() + "." + m_offlineGNTauTightWPDecorKey.key();
  ATH_CHECK(m_offlineGNTauTightWPDecorKey.initialize());

  return AthMonitorAlgorithm::initialize();
}

StatusCode tauMonitorAlgorithm::fillHistograms(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::TauJetContainer> taus(m_TauContainerKey, ctx);

  if (!taus.isValid()) {
    ATH_MSG_ERROR("evtStore() does not contain tau Collection with name "
                  << m_TauContainerKey);
    return StatusCode::FAILURE;
  }

  auto shallowCopy = xAOD::shallowCopyContainer(*taus);
  std::unique_ptr<xAOD::TauJetContainer> shallowTaus(shallowCopy.first);
  std::unique_ptr<xAOD::ShallowAuxContainer> shallowTausAux(shallowCopy.second);

  const int lowerEtThreshold = 15;
  const int higherEtThreshold = 75;
  auto tool = getGroup(m_kinGroupName);

  auto tauEta = Monitored::Scalar<float>("tauEta", 0.0);

  auto tauPhi = Monitored::Scalar<float>("tauPhi", 0.0);
  auto tauEt = Monitored::Scalar<float>("tauEt", 0.0);

  std::string tauid = "";
  if(m_TauContainerKey.key().find("TMTauJets") != std::string::npos)
  {
    tauid = "GNTau";
  } else {
    tauid = "RNN";
  }

  auto tauEtEt15Loose = Monitored::Scalar<float>("tauEtEt15"+tauid+"Loose", 0.0);
  auto panModeEt15Loose =
      Monitored::Scalar<float>("panModeEt15"+tauid+"Loose", 0.0);
  auto panModeSubstructure =
      Monitored::Scalar<float>("panModeSubstructure", 0.0);

  auto tauPhiEt15 = Monitored::Scalar<float>("tauPhiEt15", 0.0);
  auto tauEtaEt15 = Monitored::Scalar<float>("tauEtaEt15", 0.0);

  auto tauPhiEt15Loose = Monitored::Scalar<float>("tauPhiEt15"+tauid+"Loose", 0.0);
  auto tauEtaEt15Loose = Monitored::Scalar<float>("tauEtaEt15"+tauid+"Loose", 0.0);

  auto tauCharge = Monitored::Scalar<int>("tauCharge", 0.0);
  auto JetScore = Monitored::Scalar<float>(tauid+"JetScore", 0.0);
  auto JetScoreSigTrans =
      Monitored::Scalar<float>(tauid+"JetScoreSigTrans", 0.0);

  auto RNNEleScore = Monitored::Scalar<float>("RNNEleScore", 0.0);
  auto RNNEleScoreSigTrans =
      Monitored::Scalar<float>("RNNEleScoreSigTrans", 0.0);

  auto NumTracks = Monitored::Scalar<int>("NumTracks", 0.0);
  auto NumTracksEt15Loose =
      Monitored::Scalar<int>("NumTracksEt15"+tauid+"Loose", 0.0);

  auto nTauCandidates = Monitored::Scalar<int>("nTauCandidates", 0.0);
  auto nHighPtTauCandidates =
      Monitored::Scalar<int>("nHighPtTauCandidates", 0.0);
  auto nClusters = Monitored::Scalar<int>("nClusters", 0.0);
  auto nClustersEt15Loose =
      Monitored::Scalar<int>("nClustersEt15"+tauid+"Loose", 0.0);

  auto tauEtLoose = Monitored::Scalar<float>("tauEt"+tauid+"Loose", 0.0);
  auto tauEtaLoose = Monitored::Scalar<float>("tauEta"+tauid+"Loose", 0.0);
  auto tauPhiLoose = Monitored::Scalar<float>("tauPhi"+tauid+"Loose", 0.0);
  auto NumTracksLoose = Monitored::Scalar<float>("NumTracks"+tauid+"Loose", 0.0);

  auto tauEtMedium = Monitored::Scalar<float>("tauEt"+tauid+"Medium", 0.0);
  auto tauEtaMedium = Monitored::Scalar<float>("tauEta"+tauid+"Medium", 0.0);
  auto tauPhiMedium = Monitored::Scalar<float>("tauPhi"+tauid+"Medium", 0.0);
  auto NumTracksMedium = Monitored::Scalar<float>("NumTracks"+tauid+"Medium", 0.0);

  auto tauEtTight = Monitored::Scalar<float>("tauEt"+tauid+"Tight", 0.0);
  auto tauEtaTight = Monitored::Scalar<float>("tauEta"+tauid+"Tight", 0.0);
  auto tauPhiTight = Monitored::Scalar<float>("tauPhi"+tauid+"Tight", 0.0);
  auto NumTracksTight = Monitored::Scalar<float>("NumTracks"+tauid+"Tight", 0.0);

  auto LB = Monitored::Scalar<int>("LB", 0.0);

  auto EMRadius = Monitored::Scalar<float>("EMRadius", 0.0);
  auto hadRadius = Monitored::Scalar<float>("hadRadius", 0.0);
  auto isolFrac = Monitored::Scalar<float>("isolFrac", 0.0);
  auto stripWidth2 = Monitored::Scalar<float>("stripWidth2", 0.0);
  auto nStrip = Monitored::Scalar<float>("nStrip", 0.0);
  auto etEMAtEMScale = Monitored::Scalar<float>("etEMAtEMScale", 0.0);

  auto etHadAtEMScale = Monitored::Scalar<float>("etHadAtEMScale", 0.0);
  auto centFrac = Monitored::Scalar<float>("centFrac", 0.0);
  auto jetSeedEta = Monitored::Scalar<float>("jetSeedEta", 0.0);
  auto jetSeedPhi = Monitored::Scalar<float>("jetSeedPhi", 0.0);
  auto jetSeedPt = Monitored::Scalar<float>("jetSeedPt", 0.0);

  auto muonVeto = Monitored::Scalar<float>("muonVeto", 0.0);

  auto tauLoose = Monitored::Scalar<float>("tau"+tauid+"Loose", 0.0);
  auto tauMedium = Monitored::Scalar<float>("tau"+tauid+"Medium", 0.0);
  auto tauTight = Monitored::Scalar<float>("tau"+tauid+"Tight", 0.0);

  auto PSSFrac = Monitored::Scalar<float>("PSSFrac", 0.0);
  auto EMFrac = Monitored::Scalar<float>("EMFrac", 0.0);

  auto EMFracTrk = Monitored::Scalar<float>("EMFracTrk", 0.0);
  auto EfracL2EffCluster = Monitored::Scalar<float>("EfracL2EffCluster", 0.0);
  auto EisoEffCluster = Monitored::Scalar<float>("EisoEffCluster", 0.0);
  auto InvMassEffClusters = Monitored::Scalar<float>("InvMassEffClusters", 0.0);
  auto nNeutPFO = Monitored::Scalar<float>("nNeutPFO", 0.0);
  auto nShot = Monitored::Scalar<float>("nShot", 0.0);
  auto pt3 = Monitored::Scalar<float>("pt3", -9.0);

  auto BDTScoreAsP0 = Monitored::Scalar<float>("BDTScoreAsP0", 0.0);
  auto dRmax = Monitored::Scalar<float>("dRmax", 0.0);

  auto ipSigLeadTrk = Monitored::Scalar<float>("ipSigLeadTrk", 0.0);
  auto massTrkSys = Monitored::Scalar<float>("massTrkSys", 0.0);
  auto etOverPtLeadTrack = Monitored::Scalar<float>("etOverPtLeadTrack", 0.0);
  auto ptRatioEflowApprox = Monitored::Scalar<float>("ptRatioEflowApprox", 0.0);
  auto trFlightPathSig = Monitored::Scalar<float>("trFlightPathSig", 0.0);
  auto trkAvgDist = Monitored::Scalar<float>("trkAvgDist", 0.0);

  auto panEta = Monitored::Scalar<float>("panEta", 0.0);
  auto panPhi = Monitored::Scalar<float>("panPhi", 0.0);
  auto panPt = Monitored::Scalar<float>("panPt", 0.0);
  auto d0 = Monitored::Scalar<float>("d0", 0.0);
  auto dRJetSeedAxis = Monitored::Scalar<float>("dRJetSeedAxis", 0.0);
  auto z0 = Monitored::Scalar<float>("z0", 0.0);

  auto etaTrack = Monitored::Scalar<float>("etaTrack", 0.0);
  auto ptTrack = Monitored::Scalar<float>("ptTrack", 0.0);
  auto phiTrack = Monitored::Scalar<float>("phiTrack", 0.0);
  auto leadTrkPt = Monitored::Scalar<float>("leadTrkPt", 0.0);
  auto nHighPtTaus = Monitored::Scalar<float>("nHighPtTaus", 0.0);
  auto numberOfTRTHighThresholdHits =
      Monitored::Scalar<float>("numberOfTRTHighThresholdHits", 0.0);
  auto numberOfTRTHighThresholdOutliers =
      Monitored::Scalar<float>("numberOfTRTHighThresholdOutliers", 0.0);
  auto numberOfTRTHits = Monitored::Scalar<float>("numberOfTRTHits", 0.0);
  auto numberOfTRTOutliers =
      Monitored::Scalar<float>("numberOfTRTOutliers", 0.0);
  auto trkWidth2 = Monitored::Scalar<float>("trkWidth2", 0.0);
  auto ipZ0SinThetaSigLeadTrk =
      Monitored::Scalar<float>("ipZ0SinThetaSigLeadTrk", 0.0);
  auto numberOfPixelHits = Monitored::Scalar<float>("numberOfPixelHits", 0.0);
  auto numberOfPixelSharedHits =
      Monitored::Scalar<float>("numberOfPixelSharedHits", 0.0);
  auto numberOfSCTHits = Monitored::Scalar<float>("numberOfSCTHits", 0.0);
  auto numberOfSCTSharedHits =
      Monitored::Scalar<float>("numberOfSCTSharedHits", 0.0);
  auto rConv = Monitored::Scalar<float>("rConv", 0.0);
  auto rConvII = Monitored::Scalar<float>("rConvII", 0.0);

  // trackLogSeedJetPt
  auto trackLogSeedJetPt = Monitored::Scalar<float>("trackLogSeedJetPt", 0.0);
  auto trackLogPt = Monitored::Scalar<float>("trackLogPt", 0.0);
  auto trackEta = Monitored::Scalar<float>("trackEta", 0.0);
  auto trackd0TJVA = Monitored::Scalar<float>("trackd0TJVA", 0.0);
  auto trackZ0SinthetaTJVA =
      Monitored::Scalar<float>("trackZ0SinthetaTJVA", 0.0);
  auto trackD0SigTJVA = Monitored::Scalar<float>("trackD0SigTJVA", 0.0);
  auto trackZ0sinthetaSigTJVA =
      Monitored::Scalar<float>("trackZ0sinthetaSigTJVA", 0.0);
  auto trackCharge = Monitored::Scalar<float>("trackCharge", 0.0);
  auto trackqOverP = Monitored::Scalar<float>("trackqOverP", 0.0);
  auto trackLogRConv = Monitored::Scalar<float>("trackLogRConv", 0.0);
  auto trackTanhRConvII = Monitored::Scalar<float>("trackTanhRConvII", 0.0);
  auto trackPtRatioSeedJet =
      Monitored::Scalar<float>("trackPtRatioSeedJet", 0.0);
  auto trackdRJetSeedAxis = Monitored::Scalar<float>("trackdRJetSeedAxis", 0.0);
  auto trackNInnermostPixHits =
      Monitored::Scalar<float>("trackNInnermostPixHits", 0.0);
  auto trackNPixHits = Monitored::Scalar<float>("trackNPixHits", 0.0);
  auto trackNSiHits = Monitored::Scalar<float>("trackNSiHits", 0.0);
  auto trackeProbabilityHT =
      Monitored::Scalar<float>("trackeProbabilityHT", 0.0);
  auto trackeProbabilityNN =
      Monitored::Scalar<float>("trackeProbabilityNN", 0.0);
  auto trackeProbabilityHTorNN =
      Monitored::Scalar<float>("trackeProbabilityHTorNN", 0.0);
  auto trackIdScoreCharged = Monitored::Scalar<float>("track", 0.0);
  auto trackIdScoreIso = Monitored::Scalar<float>("track", 0.0);
  auto trackIdScoreConv = Monitored::Scalar<float>("track", 0.0);
  auto trackIdScoreFake = Monitored::Scalar<float>("track", 0.0);

  auto clusterLogEt = Monitored::Scalar<float>("clusterLogEt", 0.0);
  auto clusterEta = Monitored::Scalar<float>("clusterEta", 0.0);
  auto clusterPhi = Monitored::Scalar<float>("clusterPhi", 0.0);
  auto clusterSecondR = Monitored::Scalar<float>("clusterSecondR", 0.0);
  auto clusterSecondLambda =
      Monitored::Scalar<float>("clusterSecondLambda", 0.0);
  auto clusterCenterLambda =
      Monitored::Scalar<float>("clusterCenterLambda", 0.0);
  auto clusterFirstEngDens =
      Monitored::Scalar<float>("clusterFirstEngDens", 0.0);
  auto clusterEMproba = Monitored::Scalar<float>("clusterEMproba", 0.0);
  auto clustersMeanCenterLambda =
      Monitored::Scalar<float>("clustersMeanCenterLambda", 0.0);
  auto clustersMeanFirstEngDens =
      Monitored::Scalar<float>("clustersMeanFirstEngDens", 0.0);
  auto clustersMeanEMProbability =
      Monitored::Scalar<float>("clustersMeanEMProbability", 0.0);
  auto clustersMeanSecondLambda =
      Monitored::Scalar<float>("clustersMeanSecondLambda", 0.0);
  auto clustersMeanPresamplerFrac =
      Monitored::Scalar<float>("clustersMeanPresamplerFrac", 0.0);
  auto clustersPFOEngRelDiff =
      Monitored::Scalar<float>("clustersPFOEngRelDiff", 0.0);

  nTauCandidates = 0;

  static const SG::ConstAccessor<char> passThinningAcc("passThinning");

  for (const auto tau : *shallowTaus) {

    if (!passThinningAcc.withDefault(*tau, true)) continue;

    tauEta = tau->eta();
    tauPhi = tau->phi();
    tauEt = tau->pt() / GeV;
    tauCharge = tau->charge();
    NumTracks = tau->nTracks();
    nClusters = tau->detail<int>(xAOD::TauJetParameters::numTopoClusters);
    LB = GetEventInfo(ctx)->lumiBlock();

    // calo
    EMRadius = tau->detail<float>(xAOD::TauJetParameters::EMRadius);
    hadRadius = tau->detail<float>(xAOD::TauJetParameters::hadRadius);
    isolFrac = tau->detail<float>(xAOD::TauJetParameters::isolFrac);
    stripWidth2 = tau->detail<float>(xAOD::TauJetParameters::stripWidth2);
    nStrip = tau->detail<int>(xAOD::TauJetParameters::nStrip);
    etEMAtEMScale = tau->detail<float>(xAOD::TauJetParameters::etEMAtEMScale);
    etHadAtEMScale = tau->detail<float>(xAOD::TauJetParameters::etHadAtEMScale);

    centFrac = tau->detail<float>(xAOD::TauJetParameters::centFrac);
    jetSeedEta = tau->etaJetSeed();
    jetSeedPhi = tau->phiJetSeed();
    jetSeedPt = tau->ptJetSeed() / GeV;

    RNNEleScore = tau->discriminant(xAOD::TauJetParameters::TauID::RNNEleScore);
    RNNEleScoreSigTrans =
        tau->discriminant(xAOD::TauJetParameters::TauID::RNNEleScoreSigTrans);

    muonVeto = tau->isTau(xAOD::TauJetParameters::MuonVeto);

    // check to understand which TauID we can use
    SG::ReadDecorHandle<xAOD::TauJetContainer, float> trans_score{m_offlineGNTauTransScoreDecorKey, ctx};
    if( trans_score.isValid()) { 
      
      // we are using a special container decorated with GNTau
      SG::ReadDecorHandle<xAOD::TauJetContainer, float> score{m_offlineGNTauScoreDecorKey, ctx};
      JetScore = score.isValid() ? score(*tau) : -1234; 

      JetScoreSigTrans = trans_score(*tau);

      SG::ReadDecorHandle<xAOD::TauJetContainer, char> tauid_loose{m_offlineGNTauLooseWPDecorKey, ctx};
      tauLoose = tauid_loose.isValid() ? tauid_loose(*tau) : -1234;

      SG::ReadDecorHandle<xAOD::TauJetContainer, char> tauid_medium{m_offlineGNTauMediumWPDecorKey, ctx};
      tauMedium = tauid_medium.isValid() ? tauid_medium(*tau) : -1234;

      SG::ReadDecorHandle<xAOD::TauJetContainer, char> tauid_tight{m_offlineGNTauTightWPDecorKey, ctx};
      tauTight = tauid_tight.isValid() ? tauid_tight(*tau) : -1234;

    } else{
      // GNTau is not present -> use RNN based TauID
      JetScore = tau->discriminant(xAOD::TauJetParameters::TauID::RNNJetScore);
      JetScoreSigTrans =
        tau->discriminant(xAOD::TauJetParameters::TauID::RNNJetScoreSigTrans);

      tauLoose = tau->isTau(xAOD::TauJetParameters::JetRNNSigLoose);
      tauMedium = tau->isTau(xAOD::TauJetParameters::JetRNNSigMedium);
      tauTight = tau->isTau(xAOD::TauJetParameters::JetRNNSigTight);
    }

    dRmax = tau->detail<float>(xAOD::TauJetParameters::dRmax);
    ptRatioEflowApprox =
        tau->detail<float>(xAOD::TauJetParameters::ptRatioEflowApprox);
    trkAvgDist = tau->detail<float>(xAOD::TauJetParameters::trkAvgDist);

    panEta = tau->etaPanTauCellBased();
    panPhi = tau->phiPanTauCellBased();
    panPt = tau->ptPanTauCellBased() / GeV; // GeV ;

    // TauB/Identification/EleVetoBDTinputs
    PSSFrac = tau->detail<float>(xAOD::TauJetParameters::PSSFraction);
    static const SG::ConstAccessor<float> EMFracFixedAcc("EMFracFixed");
    EMFrac = EMFracFixedAcc(*tau);

    // TauB/SubStructure
    EMFracTrk = tau->detail<float>(xAOD::TauJetParameters::ChPiEMEOverCaloEME);
    EfracL2EffCluster = tau->detail<float>(
        xAOD::TauJetParameters::lead2ClusterEOverAllClusterE);
    EisoEffCluster =
        tau->detail<float>(xAOD::TauJetParameters::caloIsoCorrected) / GeV;
    InvMassEffClusters =
        tau->detail<float>(xAOD::TauJetParameters::effTopoInvMass) /
        GeV; // puts it in GeV
    nNeutPFO = tau->nProtoNeutralPFOs();
    nShot = tau->nShotPFOs();

    int panModeDummy = -1;
    int panModeSubstructureDummy = -1;

    if (m_etaMin < std::abs(tauEta) && std::abs(tauEta) < m_etaMax) {

      if( tauEt > lowerEtThreshold) {	    
        nTauCandidates += 1;
      }

      if (tauEt > higherEtThreshold) {
        nHighPtTauCandidates += 1;
        nHighPtTaus += 1;
      }

      if (m_kinGroupName != "tauMonKinGroupGlobal" &&
          tauEt > lowerEtThreshold && tauLoose) {

        tauPhiEt15Loose = tau->phi();
        tauEtaEt15Loose = tau->eta();
        tauEtEt15Loose = tau->pt() / GeV;
        nClustersEt15Loose =
            tau->detail<int>(xAOD::TauJetParameters::numTopoClusters);
        NumTracksEt15Loose = tau->nTracks();

        tau->panTauDetail(xAOD::TauJetParameters::PanTau_DecayMode,
                          panModeDummy);
        panModeEt15Loose = panModeDummy;
        fill(tool, tauPhiEt15Loose, tauEtaEt15Loose,
             nClustersEt15Loose, NumTracksEt15Loose, tauEtEt15Loose,
             panModeEt15Loose);
      }

      if (m_kinGroupName != "tauMonKinGroupGlobal" && tauLoose) {
        tauPhiLoose = tau->phi();
        tauEtaLoose = tau->eta();
        tauEtLoose = tau->pt() / GeV;
        NumTracksLoose = tau->nTracks();

        fill(tool, tauPhiLoose, tauEtaLoose, NumTracksLoose,
             tauEtLoose);
      }

      if (m_kinGroupName != "tauMonKinGroupGlobal" && tauMedium) {
        tauPhiMedium = tau->phi();
        tauEtaMedium = tau->eta();
        tauEtMedium = tau->pt() / GeV;
        NumTracksMedium = tau->nTracks();

        fill(tool, tauPhiMedium, tauEtaMedium, NumTracksMedium,
             tauEtMedium);
      }

      if (m_kinGroupName != "tauMonKinGroupGlobal" && tauTight) {
        tauPhiTight = tau->phi();
        tauEtaTight = tau->eta();
        tauEtTight = tau->pt() / GeV;
        NumTracksTight = tau->nTracks();

        fill(tool, tauPhiTight, tauEtaTight, NumTracksTight,
             tauEtTight);
      }


      // tracks - old
      if (tau->nTracks() != 0) {

        massTrkSys =
            tau->detail<float>(xAOD::TauJetParameters::massTrkSys) / GeV; // GeV
        trkWidth2 = tau->detail<float>(xAOD::TauJetParameters::trkWidth2);
        trFlightPathSig =
            tau->detail<float>(xAOD::TauJetParameters::trFlightPathSig);

        static const SG::ConstAccessor<float> d0SigTJVAAcc("d0SigTJVA");
	if (d0SigTJVAAcc.isAvailable(*tau->track(0))) {
	  ipSigLeadTrk = tau->track(0)->d0SigTJVA();
	}
	else {
	  ipSigLeadTrk = tau->detail<float>(xAOD::TauJetParameters::ipSigLeadTrk);
	}

        static const SG::ConstAccessor<float> z0sinthetaSigTJVAAcc("z0sinthetaSigTJVA");
	if (z0sinthetaSigTJVAAcc.isAvailable(*tau->track(0))) {
	  ipZ0SinThetaSigLeadTrk = tau->track(0)->z0sinthetaSigTJVA();
	}
	else {
	  ipZ0SinThetaSigLeadTrk = tau->detail<float>(xAOD::TauJetParameters::ipZ0SinThetaSigLeadTrk);
	}

        etOverPtLeadTrack =
            tau->detail<float>(xAOD::TauJetParameters::etOverPtLeadTrk);
        leadTrkPt = tau->detail<float>(xAOD::TauJetParameters::leadTrkPt) / GeV;

        fill(tool, massTrkSys, etOverPtLeadTrack, trkWidth2, trFlightPathSig,
             ipSigLeadTrk, ipZ0SinThetaSigLeadTrk, leadTrkPt);

        if (environment() != Environment_t::AOD) {
          const xAOD::TrackParticle *track = tau->track(0)->track();
          const Trk::Perigee perigee = track->perigeeParameters();

          uint8_t dummy(0);

          if (track->summaryValue(dummy, xAOD::numberOfSCTSharedHits)) {
            numberOfSCTSharedHits = dummy;
            fill(tool, numberOfSCTSharedHits);
          }

          if (track->summaryValue(dummy, xAOD::numberOfSCTHits)) {
            numberOfSCTHits = dummy;
            fill(tool, numberOfSCTHits);
          }

          if (track->summaryValue(dummy, xAOD::numberOfPixelSharedHits)) {
            numberOfPixelSharedHits = dummy;
            fill(tool, numberOfPixelSharedHits);
          }

          if (track->summaryValue(dummy, xAOD::numberOfPixelHits)) {
            numberOfPixelHits = dummy;
            fill(tool, numberOfPixelHits);
          }
          if (track->summaryValue(dummy, xAOD::numberOfTRTHighThresholdHits)) {
            numberOfTRTHighThresholdHits = dummy;
            fill(tool, numberOfTRTHighThresholdHits);
          }
          if (track->summaryValue(dummy,
                                  xAOD::numberOfTRTHighThresholdOutliers)) {
            numberOfTRTHighThresholdOutliers = dummy;
            fill(tool, numberOfTRTHighThresholdOutliers);
          }
          if (track->summaryValue(dummy, xAOD::numberOfTRTHits)) {
            numberOfTRTHits = dummy;
            fill(tool, numberOfTRTHits);
          }
          if (track->summaryValue(dummy, xAOD::numberOfTRTOutliers)) {
            numberOfTRTOutliers = dummy;
            fill(tool, numberOfTRTOutliers);
          }

          d0 = perigee.parameters()[Trk::d0];
          z0 = perigee.parameters()[Trk::z0];

          phiTrack = perigee.parameters()[Trk::phi];
          etaTrack = perigee.eta();
          ptTrack = perigee.pT() / GeV;

          fill(tool, d0, z0, phiTrack, etaTrack, ptTrack);
        }
      }
      // this else can be removed, but it sets any track variable to 0 if
      // there are no tracks this solution makes entry numbers match calo which
      // is desired but there are too many zeros.
      else {
        leadTrkPt = 0;
        fill(tool, leadTrkPt);
      }

      // Code for All Tracks and Mean track variables.

      // pre loop variables:
      double tauSeedPt = tau->ptJetSeed();
      double logTauSeedPt = std::log(tauSeedPt);
      trackLogSeedJetPt = logTauSeedPt;

      fill(tool, trackLogSeedJetPt);

      static const SG::ConstAccessor<float> acc_trackScoreCharged("rnn_chargedScore");
      static const SG::ConstAccessor<float> acc_trackScoreIso("rnn_isolationScore");
      static const SG::ConstAccessor<float> acc_trackScoreConv("rnn_conversionScore");
      // rnn_fakeScore may not be available (it is not provided by the TauJets smart slimming list), it can be obtained from unitarity

      for (const xAOD::TauTrack *track : tau->allTracks()) {

        static const SG::Accessor<
            xAOD::TauTrack::TrackParticleLinks_t>
            trackAcc("trackLinks");
        if (!trackAcc(*track)[0]) {
          continue;
        }

        const xAOD::TrackParticle *trackParticle = track->track();

        float d0TJVA = track->track()->d0();
        trackZ0SinthetaTJVA = track->z0sinThetaTJVA(*tau);
        trackD0SigTJVA = 999.;
        trackZ0sinthetaSigTJVA = 999.;
        float rConv = 999.;
        float rConvII = 999.;
        static const SG::ConstAccessor<float> z0sinthetaTJVAAcc("z0sinthetaTJVA");
        if (z0sinthetaTJVAAcc.isAvailable(*track)) {
          d0TJVA = track->d0TJVA();
          trackZ0SinthetaTJVA = track->z0sinthetaTJVA();
          trackD0SigTJVA = track->d0SigTJVA();
          trackZ0sinthetaSigTJVA = track->z0sinthetaSigTJVA();
          rConv = track->rConv();
          rConvII = track->rConvII();
        }
        trackdRJetSeedAxis =
            track->p4().DeltaR(tau->p4(xAOD::TauJetParameters::JetSeed));
        double qOverP = trackParticle->qOverP();
        double trackPt = trackParticle->pt();
        uint8_t nInnermostPixelLayerHits = 0;
        trackParticle->summaryValue(nInnermostPixelLayerHits,
                                    xAOD::numberOfInnermostPixelLayerHits);
        uint8_t nPixelHits = 0;
        trackParticle->summaryValue(nPixelHits, xAOD::numberOfPixelHits);
        uint8_t nPixelDeadSensors = 0;
        trackParticle->summaryValue(nPixelDeadSensors,
                                    xAOD::numberOfPixelDeadSensors);
        uint8_t nSCTHits = 0;
        trackParticle->summaryValue(nSCTHits, xAOD::numberOfSCTHits);
        uint8_t nSCTDeadSensors = 0;
        trackParticle->summaryValue(nSCTDeadSensors,
                                    xAOD::numberOfSCTDeadSensors);
        uint8_t nTRTHighThresholdHits = 0;
        trackParticle->summaryValue(nTRTHighThresholdHits,
                                    xAOD::numberOfTRTHighThresholdHits);
        uint8_t numberOfPixelHoles = 0;
        trackParticle->summaryValue(numberOfPixelHoles,
                                    xAOD::numberOfPixelHoles);
        uint8_t numberOfSCTHoles = 0;
        trackParticle->summaryValue(numberOfSCTHoles, xAOD::numberOfSCTHoles);
        float eProbabilityHT = 0.;
        trackParticle->summaryValue(eProbabilityHT, xAOD::eProbabilityHT);
        static const SG::ConstAccessor<float> eProbabilityNNAcc("eProbabilityNN");
        float eProbabilityNN = eProbabilityNNAcc.withDefault(*trackParticle, -1);
        // hybrid variable (eProbabilityNN is not computed for tracks with pt
        // < 2 GeV)
        trackeProbabilityHTorNN =
            (trackPt > 2000.) ? eProbabilityNN : eProbabilityHT;

        trackeProbabilityNN = eProbabilityNN;
        trackeProbabilityHT = eProbabilityHT;
        trackLogPt = std::log(trackPt);
        trackEta = track->eta();
        trackd0TJVA = std::tanh(d0TJVA / 10.);
        trackCharge = trackParticle->charge();
        trackqOverP = qOverP * 1000.;
        trackLogRConv = std::log(rConv);
        trackTanhRConvII = std::tanh(rConvII / 500.0);
        trackPtRatioSeedJet = trackPt / tauSeedPt;
        trackNInnermostPixHits = nInnermostPixelLayerHits;
        trackNPixHits = nPixelHits + nPixelDeadSensors;
        trackNSiHits =
            nPixelHits + nPixelDeadSensors + nSCTHits + nSCTDeadSensors;

        fill(tool, trackLogPt, trackEta, trackd0TJVA, trackZ0SinthetaTJVA,
             trackD0SigTJVA, trackZ0sinthetaSigTJVA, trackCharge, trackqOverP,
             trackLogRConv, trackTanhRConvII, trackPtRatioSeedJet,
             trackdRJetSeedAxis, trackNInnermostPixHits,
             trackNPixHits,
             trackNSiHits,
             trackeProbabilityHT, trackeProbabilityNN, trackeProbabilityHTorNN);

	if (acc_trackScoreCharged.isAvailable(*track)) {
	  float chargedScore = acc_trackScoreCharged(*track);
	  float isolationScore = acc_trackScoreIso(*track);
	  float conversionScore = acc_trackScoreConv(*track);
	  float fakeScore = 1. - chargedScore - isolationScore - conversionScore;
	  // ensure the probability is within [0.,1.]
	  fakeScore = std::max(0.f, fakeScore);
	  fakeScore = std::min(1.f, fakeScore);

	  trackIdScoreCharged = chargedScore;
	  trackIdScoreIso = isolationScore;
	  trackIdScoreConv = conversionScore;
	  trackIdScoreFake = fakeScore;

          fill(tool, trackIdScoreCharged, trackIdScoreIso, trackIdScoreConv,
               trackIdScoreFake);
        }
      }

      // clusters
      std::vector<const xAOD::IParticle *> particleList = tau->clusters();
      std::vector<xAOD::CaloVertexedTopoCluster> clusters;
      const xAOD::Vertex *vertex = nullptr;
      if (tau->vertexLink().isValid())
        vertex = tau->vertex();

      for (const xAOD::IParticle *particle : particleList) {
        const xAOD::CaloCluster *cluster =
            static_cast<const xAOD::CaloCluster *>(particle);
        if (vertex) {
          clusters.emplace_back(*cluster, xAOD::CaloCluster::State::CALIBRATED,
                                vertex->position());
        } else {
          clusters.emplace_back(*cluster, xAOD::CaloCluster::State::CALIBRATED);
        }
      }

      // sort by decreasing Et
      auto et_cmp = [](const xAOD::CaloVertexedTopoCluster &lhs,
                       const xAOD::CaloVertexedTopoCluster &rhs) {
        return lhs.p4().Et() > rhs.p4().Et();
      };
      std::sort(clusters.begin(), clusters.end(), et_cmp);

      // keep first 6 leading clusters as in RNN ID
      if (clusters.size() > 6) {
        clusters.resize(6, clusters[0]);
      }

      double moment;
      for (const auto& vertexedCluster : clusters) {
        const xAOD::CaloCluster &cluster = vertexedCluster.clust();

        clusterLogEt = std::log10(vertexedCluster.p4().Et());
        clusterEta = vertexedCluster.eta();
        clusterPhi = vertexedCluster.phi();

        cluster.retrieveMoment(xAOD::CaloCluster::MomentType::SECOND_R, moment);
        clusterSecondR = std::log10(moment + 0.1);

        cluster.retrieveMoment(xAOD::CaloCluster::MomentType::SECOND_LAMBDA,
                               moment);
        clusterSecondLambda = std::log10(moment + 0.1);

        cluster.retrieveMoment(xAOD::CaloCluster::MomentType::CENTER_LAMBDA,
                               moment);
        clusterCenterLambda = std::log10(moment + 1e-6);

        cluster.retrieveMoment(xAOD::CaloCluster::MomentType::FIRST_ENG_DENS,
                               moment);
        if (moment != 0.)
          moment = std::log10(std::abs(moment));
        clusterFirstEngDens = moment;

        cluster.retrieveMoment(xAOD::CaloCluster::MomentType::EM_PROBABILITY,
                               moment);
        clusterEMproba = moment;

        fill(tool, clusterLogEt, clusterEta, clusterPhi, clusterSecondR,
             clusterSecondLambda, clusterCenterLambda, clusterFirstEngDens,
             clusterEMproba);
      }

      // Et-weighted average of cluster moments (MVA TES)
      float avariable = 0.;
      bool test = tau->detail(xAOD::TauJetParameters::ClustersMeanCenterLambda,
                              avariable);
      if (test)
        clustersMeanCenterLambda = avariable;

      test = tau->detail(xAOD::TauJetParameters::ClustersMeanFirstEngDens,
                         avariable);
      if (test)
        clustersMeanFirstEngDens = avariable;

      test = tau->detail(xAOD::TauJetParameters::ClustersMeanEMProbability,
                         avariable);
      if (test)
        clustersMeanEMProbability = avariable;

      test = tau->detail(xAOD::TauJetParameters::ClustersMeanSecondLambda,
                         avariable);
      if (test)
        clustersMeanSecondLambda = avariable;

      test = tau->detail(xAOD::TauJetParameters::ClustersMeanPresamplerFrac,
                         avariable);
      if (test)
        clustersMeanPresamplerFrac = avariable;

      test = tau->detail(xAOD::TauJetParameters::PFOEngRelDiff, avariable);
      if (test)
        clustersPFOEngRelDiff = avariable;

      fill(tool, clustersMeanCenterLambda, clustersMeanFirstEngDens,
           clustersMeanEMProbability, clustersMeanSecondLambda,
           clustersMeanPresamplerFrac, clustersPFOEngRelDiff);

      for (int s = 0; s < nShot; s++) {
        const xAOD::PFO *shot = tau->shotPFO(s);
        if (shot != nullptr) {
          float pt3Temp = -9.0;
          shot->attribute(xAOD::PFODetails::PFOAttributes::tauShots_pt3,
                          pt3Temp);
          pt3 = pt3Temp / GeV; // GeV
          fill(tool, pt3);
        }
      }

      for (unsigned int np = 0; np < nNeutPFO; np++) {
        const xAOD::PFO *npfo = tau->protoNeutralPFO(np);
        BDTScoreAsP0 = npfo->bdtPi0Score();
        fill(tool, BDTScoreAsP0);
      }

      if (tauEt > lowerEtThreshold) {
        tauPhiEt15 = tau->phi();
        tauEtaEt15 = tau->eta();
        fill(tool, tauPhiEt15, tauEtaEt15);
      }

      fill(tool, tauPhi, tauEta, LB, tauEt, centFrac, isolFrac, EMRadius,
           hadRadius, stripWidth2, nStrip, etEMAtEMScale, etHadAtEMScale,
           tauCharge, JetScore, JetScoreSigTrans, RNNEleScore, RNNEleScoreSigTrans,
	   muonVeto, tauLoose, tauMedium, tauTight, PSSFrac, EMFrac, EMFracTrk,
           EfracL2EffCluster, EisoEffCluster, InvMassEffClusters, nNeutPFO,
           nShot, NumTracks, nClusters, jetSeedEta, jetSeedPhi, jetSeedPt,
           dRmax, ptRatioEflowApprox, trkAvgDist);

      tau->panTauDetail(xAOD::TauJetParameters::PanTau_DecayMode,
                        panModeSubstructureDummy);
      panModeSubstructure = panModeSubstructureDummy;

      fill(tool, panModeSubstructure);
      if (panPhi > -100) {
        fill(tool, panEta, panPhi, panPt);
      }
    }
  }

  fill(tool, nHighPtTauCandidates, nTauCandidates, nHighPtTaus);

  return StatusCode::SUCCESS;
}
