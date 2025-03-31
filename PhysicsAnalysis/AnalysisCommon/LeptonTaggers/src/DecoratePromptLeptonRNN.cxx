/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local
#include "LeptonTaggers/DecoratePromptLeptonRNN.h"
#include "LeptonTaggers/PromptUtils.h"
#include "LeptonTaggers/VarHolder.h"

// xAOD
#include "xAODBase/IParticleHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "StoreGate/DecorKeyHelpers.h"
#include "GaudiKernel/ThreadLocalContext.h"

// ROOT
#include "TH1.h"

//=============================================================================
Prompt::DecoratePromptLeptonRNN::DecoratePromptLeptonRNN(const std::string& name, ISvcLocator* pSvcLocator):
  AthAlgorithm(name, pSvcLocator),
  m_histSvc   ("THistSvc/THistSvc", name),
  m_countEvent(0)
{}

//=============================================================================
StatusCode Prompt::DecoratePromptLeptonRNN::initialize()
{
  ATH_MSG_DEBUG("Initializing DecoratePromptLeptonRNN...");

  // Initialize read/write handles
  ATH_CHECK(m_inputContainerLeptonKey.initialize());
  ATH_CHECK(m_inputContainerTrackKey.initialize());
  ATH_CHECK(m_inputContainerTrackJetKey.initialize());
  ATH_CHECK(m_inputContainerPrimaryVerticesKey.initialize());

  ATH_CHECK(m_eventHandleKey.initialize());

  //
  // Initialize tools and services
  //
  ATH_CHECK( m_histSvc.retrieve() );

  m_decorHandleKeys.clear();
  for(const std::string &label: m_toolRNN->getOutputLabels()) {
    const std::string key = m_decorationPrefixRNN + label;

    ATH_MSG_DEBUG("Add output RNN label: \"" << key << "\"");

    m_decorNameMap.try_emplace (label, m_decorHandleKeys.size());
    m_decorHandleKeys.push_back (SG::makeContDecorKey (m_inputContainerLeptonKey, key));
  }
  ATH_CHECK(m_decorHandleKeys.initialize());

  ATH_MSG_DEBUG("inputContainerMuon=\"" << m_inputContainerLeptonKey << "\"");

  //
  // Instantiate Muon quality accessors
  //
  m_accessQuality = std::make_unique<SG::AuxElement::ConstAccessor<unsigned char> >("quality");

  m_timerEvent.Reset();

  ATH_MSG_DEBUG("DecoratePromptLeptonRNN initialized successfully.");

  return StatusCode::SUCCESS;

}

//=============================================================================
StatusCode Prompt::DecoratePromptLeptonRNN::execute()
{
  //
  // Process current event
  //
  TimerScopeHelper timer(m_timerEvent);

  const EventContext& ctx = Gaudi::Hive::currentContext();

  ATH_MSG_DEBUG("execute() - begin...");

  m_countEvent++;

  //
  // Retrieve object containers and event info
  //
  SG::ReadHandle<xAOD::IParticleContainer> h_leptons (m_inputContainerLeptonKey, ctx);
  SG::ReadHandle<xAOD::TrackParticleContainer> h_tracks (m_inputContainerTrackKey, ctx);
  SG::ReadHandle<xAOD::JetContainer> h_trackJets (m_inputContainerTrackJetKey, ctx);
  SG::ReadHandle<xAOD::VertexContainer> h_vertices  (m_inputContainerPrimaryVerticesKey, ctx);

  SG::ReadHandle<xAOD::EventInfo> event_handle (m_eventHandleKey, ctx);

  ATH_MSG_DEBUG("Size of LeptonContainer: " << h_leptons->size());

  std::vector<decoratorFloatH_t> decors;
  decors.reserve (m_decorHandleKeys.size());
  for (const SG::WriteDecorHandleKey<xAOD::IParticleContainer>& k : m_decorHandleKeys) {
    decors.emplace_back (k, ctx);
  }

  //
  // Find default Primary Vertex
  //
  const xAOD::Vertex *primaryVertex = nullptr;

  for(const xAOD::Vertex *vertex: *h_vertices) {
    if(vertex->vertexType() == xAOD::VxType::PriVtx) {
      primaryVertex = vertex;
      break;
    }
  }

  //
  // Collect tracks
  //
  for(const xAOD::IParticle *lepton: *h_leptons) {
    //
    // Select lepton track
    //
    const xAOD::TrackParticle *trackLep = nullptr;
    const xAOD::Electron      *elec     = dynamic_cast<const xAOD::Electron*>(lepton);
    const xAOD::Muon          *muon     = dynamic_cast<const xAOD::Muon    *>(lepton);

    if(elec) {
      const xAOD::TrackParticle *bestmatchedGSFElTrack = elec->trackParticle(0);
      if(bestmatchedGSFElTrack) {
        trackLep = xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(bestmatchedGSFElTrack);
      }
    }
    else if (muon) {
      trackLep = findMuonTrack(muon);
    }
    else {
      ATH_MSG_WARNING("execute - failed to find electron or muon: should never happen!");
    }

    //
    // Find closest track jet
    //
    const xAOD::Jet *trackJet = findClosestTrackJet(trackLep, *h_trackJets);

    if(!trackLep || !trackJet) {
      compDummy(*lepton, decors);
      continue;
    }

    //
    // Select tracks within cone around lepton track.
    //
    std::vector<Prompt::VarHolder > select_tracks;

    Prompt::VarHolder lepton_obj;

    if(!prepTrackObject(lepton_obj, *trackLep, *trackLep, *trackJet, *primaryVertex, *event_handle)) {
      continue;
    }

    //
    // Add lepton track as one of the cone tracks.
    //
    select_tracks.push_back(lepton_obj);

    for(const xAOD::TrackParticle *track: *h_tracks) {

      if(!track) {
        ATH_MSG_WARNING("Prompt::DecoratePromptLeptonRNN::execute - skip null track pointer - should never happen");
        continue;
      }

      Prompt::VarHolder track_obj;

      if(!prepTrackObject(track_obj, *track, *trackLep, *trackJet, *primaryVertex, *event_handle)) {
  continue;
      }

      if(passTrack(track_obj)) {
  select_tracks.push_back(track_obj);

  ATH_MSG_DEBUG("Prompt::DecoratePromptLeptonRNN::execute - passed track pT= " << track->pt());
      }
    }

    //
    // Sort tracks by DR distance to lepton
    //
    std::sort(select_tracks.begin(), select_tracks.end(), Prompt::SortObjectByVar(Def::LepTrackDR, msg()));

    //
    // Compute RNN
    //
    compScore(*lepton, select_tracks, decors);

    ATH_MSG_DEBUG("DecoratePromptLeptonRNN::CompScore - " << std::endl
                    << "lepton pT= " << lepton->pt()
        << ", number of tracks: " << select_tracks.size());
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode Prompt::DecoratePromptLeptonRNN::finalize()
{
  //
  // Finalize output
  //
  if(m_printTime) {
    //
    // Print full time stopwatch
    //
    ATH_MSG_INFO("DecoratePromptLeptonRNN - total time:   " << PrintResetStopWatch(m_timerEvent));
    ATH_MSG_INFO("DecoratePromptLeptonRNN - processed "     << m_countEvent << " events.");
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
const xAOD::TrackParticle* Prompt::DecoratePromptLeptonRNN::findMuonTrack(const xAOD::Muon *muon)
{
  //
  // Process muon - return true if all information present for RNN
  //
  if(muon->muonType() != xAOD::Muon::Combined || !muon->inDetTrackParticleLink().isValid()) {
    return 0;
  }

  const unsigned char quality = (*m_accessQuality)(*muon);

  ATH_MSG_DEBUG("muon pT=" << muon->pt() << " quality=" << int(quality) << " medium=" << int(xAOD::Muon::Medium));

  const xAOD::TrackParticle *trackLep = *(muon->inDetTrackParticleLink());

  return trackLep;
}

//=============================================================================
const xAOD::Jet* Prompt::DecoratePromptLeptonRNN::findClosestTrackJet(const xAOD::TrackParticle *particle,
                                  const xAOD::JetContainer &trackJets)
{
  //
  // Find track jet closest to IParticle and within fixed cone
  //
  if(!particle) {
    return 0;
  }

  const xAOD::Jet *trackJet = 0;
  double currTrackJetDR = -1.0;

  //
  // Select track jet within cone around muon
  //
  for(const xAOD::Jet *jet: trackJets) {
    const double dr = particle->p4().DeltaR(jet->p4());

    if(currTrackJetDR < 0.0 || dr < currTrackJetDR) {
      trackJet       = jet;
      currTrackJetDR = dr;
    }
  }

  if(trackJet && currTrackJetDR < m_maxLepTrackJetDR) {
    return trackJet;
  }

  return 0;
}

//=============================================================================
bool Prompt::DecoratePromptLeptonRNN::prepTrackObject(
  Prompt::VarHolder         &p,
  const xAOD::TrackParticle &track,
  const xAOD::TrackParticle &lepton,
  const xAOD::Jet           &trackJet,
  const xAOD::Vertex        &priVtx,
  const xAOD::EventInfo     event
)
{
  //
  // Add xAOD::TrackParticle variables to the track object VarHolder
  //
  uint8_t numberOfPixelHits       = 0;
  uint8_t numberOfSCTHits         = 0;
  uint8_t numberOfPixelHoles      = 0;
  uint8_t numberOfSCTHoles        = 0;
  uint8_t numberOfPixelSharedHits = 0;
  uint8_t numberOfSCTSharedHits   = 0;

  if(!(track.summaryValue(numberOfPixelHits,       xAOD::numberOfPixelHits)))       return false;
  if(!(track.summaryValue(numberOfSCTHits,         xAOD::numberOfSCTHits)))         return false;
  if(!(track.summaryValue(numberOfPixelHoles,      xAOD::numberOfPixelHoles)))      return false;
  if(!(track.summaryValue(numberOfSCTHoles,        xAOD::numberOfSCTHoles)))        return false;
  if(!(track.summaryValue(numberOfPixelSharedHits, xAOD::numberOfPixelSharedHits))) return false;
  if(!(track.summaryValue(numberOfSCTSharedHits,   xAOD::numberOfSCTSharedHits)))   return false;

  const uint8_t NSiHits   = numberOfPixelHits  + numberOfSCTHits;
  const uint8_t NSiHoles  = numberOfPixelHoles + numberOfSCTHoles;
  const float   NSiShHits = float(numberOfPixelSharedHits) + float(numberOfSCTSharedHits)/2.0;

  p.addVar(Def::Pt,                    track.pt());
  p.addVar(Def::AbsEta,                std::fabs(track.eta()));
  p.addVar(Def::NumberOfPIXHits,       numberOfPixelHits);
  p.addVar(Def::NumberOfSCTHits,       numberOfSCTHits);
  p.addVar(Def::NumberOfSiHits,        NSiHits);
  p.addVar(Def::NumberOfSharedSiHits,  NSiShHits);
  p.addVar(Def::NumberOfSiHoles,       NSiHoles);
  p.addVar(Def::NumberOfPixelHoles,    numberOfPixelHoles);

  //
  // Add lepton - jet variables to VarHolder
  //
  double PtFrac = -99.;

  if(track.pt() > 0.0 && trackJet.pt() > 0.0) {
    PtFrac = track.pt() / trackJet.pt();
  }

  p.addVar(Def::TrackPtOverTrackJetPt, PtFrac);
  p.addVar(Def::TrackJetDR,            track.p4().DeltaR(trackJet.p4()));
  p.addVar(Def::LepTrackDR,            track.p4().DeltaR(lepton.p4()));

  //
  // Add Impact Parameters
  //
  double d0_significance = -99.;
  double Z0Sin           = 0.0;

  if(track.definingParametersCovMatrixVec().size() > 0 && track.definingParametersCovMatrixVec().at(0) > 0.0) {
    d0_significance = xAOD::TrackingHelpers::d0significance(&track,
                  event.beamPosSigmaX(),
                  event.beamPosSigmaY(),
                  event.beamPosSigmaXY());
  }

  const double deltaZ0  = track.z0() + track.vz() - priVtx.z();
  Z0Sin                 = deltaZ0*std::sin(track.theta());

  p.addVar(Def::Z0Sin,  Z0Sin);
  p.addVar(Def::D0Sig,  d0_significance);

  return true;
}

//=============================================================================
bool Prompt::DecoratePromptLeptonRNN::passTrack(Prompt::VarHolder &p)
{
  //
  // Select cone tracks
  //
  if(p.getVar(Def::LepTrackDR)           < m_minTrackLeptonDR)     return false;
  if(p.getVar(Def::LepTrackDR)           > m_maxTrackLeptonDR)     return false;

  //
  // Kinematic track selection
  //
  if(p.getVar(Def::Pt)                   < m_minTrackpT)           return false;
  if(p.getVar(Def::AbsEta)               > m_maxTrackEta)          return false;
  if(std::fabs(p.getVar(Def::Z0Sin))     > m_maxTrackZ0Sin)        return false;

  //
  // Hit quality track selection
  //
  if(p.getVar(Def::NumberOfSiHits)       < m_minTrackSiHits)       return false;
  if(p.getVar(Def::NumberOfSharedSiHits) > m_maxTrackSharedSiHits) return false;
  if(p.getVar(Def::NumberOfSiHoles)      > m_maxTrackSiHoles)      return false;
  if(p.getVar(Def::NumberOfPixelHoles)   > m_maxTrackPixHoles)      return false;

  return true;
}

//=============================================================================
bool Prompt::DecoratePromptLeptonRNN::compScore(const xAOD::IParticle &particle,
                              const std::vector<Prompt::VarHolder> &tracks,
                              std::vector<decoratorFloatH_t>& decors)
{
  //
  // Call the RNN tool to get the RNN prediction for the leptons and decorate the lepton with those RNN scores.
  //
  ATH_MSG_DEBUG("compScore - number of tracks: " << tracks.size());

  for(const Prompt::VarHolder &o: tracks) {
    ATH_MSG_DEBUG("compScore - track: LepTrackDR = " << o.getVar(Def::LepTrackDR)
      << ", TrackJetDR = " << o.getVar(Def::TrackJetDR)
      << ", D0Sig = " << o.getVar(Def::D0Sig)
      << ", Z0Sin = " << o.getVar(Def::Z0Sin)
      << ", NumberOfPIXHits = " << o.getVar(Def::NumberOfPIXHits)
      << ", NumberOfSCTHits = " << o.getVar(Def::NumberOfSCTHits)
      << ", PtFrac = " << o.getVar(Def::TrackPtOverTrackJetPt) );
  }

  const std::map<std::string, double> results = m_toolRNN->computeRNNOutput(tracks);

  for(const std::pair<const std::string, double>& v: results) {
    //
    // Decorate muon
    //

    ATH_MSG_DEBUG("DecoratePromptLeptonRNN compScore - " << v.first << " = " << v.second );

    auto dit = m_decorNameMap.find (v.first);

    if(dit != m_decorNameMap.end()) {
      decors.at(dit->second)(particle) = v.second;
    }
    else {
      ATH_MSG_WARNING("CompScore - unknown output label=\"" << v.first << "\"");
    }

    if(m_debug) {
      std::map<std::string, TH1*>::iterator hit = m_hists.find(v.first);

      if(hit == m_hists.end()) {
        TH1* h = 0;

        StatusCode hist_status = makeHist(h, v.first, 100, 0.0, 1.0);
        if (hist_status != StatusCode::SUCCESS){
          ATH_MSG_WARNING("DecoratePromptLeptonRNN compScore - failed to make hist");
        }

        hit = m_hists.insert(std::map<std::string, TH1*>::value_type(v.first, h)).first;
      }

      if(hit->second) {
        hit->second->Fill(v.second);
      }
    }
  }

  return true;
}

//=============================================================================
bool Prompt::DecoratePromptLeptonRNN::compDummy(const xAOD::IParticle &particle,
                                                std::vector<decoratorFloatH_t>& decors) const
{
  //
  // Fill dummy values for RNN outputs
  //
  for (auto& d : decors) {
    d(particle) = -1.0;
  }

  return true;
}

//=============================================================================
StatusCode Prompt::DecoratePromptLeptonRNN::makeHist(TH1 *&h, const std::string &key, int nbin, double xmin, double xmax)
{
  //
  // Initiliase histogram pointer. If configured to run in validation mode, then create and register histogram
  //
  h = 0;

  if(m_outputStream.empty() || key.empty()) {
    return StatusCode::SUCCESS;
  }

  const std::string hname    = name() + "_" + key;
  const std::string hist_key = "/"+m_outputStream+"/"+hname;

  h = new TH1D(hname.c_str(), hname.c_str(), nbin, xmin, xmax);
  h->SetDirectory(0);

  return m_histSvc->regHist(hist_key, h);
}
