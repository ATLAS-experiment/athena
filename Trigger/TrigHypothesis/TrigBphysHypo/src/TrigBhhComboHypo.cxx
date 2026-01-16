/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>
#include <numeric>
#include <iterator>

#include "TrigBhhComboHypo.h"

#include "xAODMuon/Muon.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTrigger/TrigComposite.h"
#include "xAODTrigBphys/TrigBphys.h"
#include "xAODTrigBphys/TrigBphysContainer.h"
#include "xAODTrigBphys/TrigBphysAuxContainer.h"

#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigCompositeUtils/HLTIdentifier.h"

#include "AthViews/View.h"
#include "AthViews/ViewHelper.h"
#include "AthContainers/AuxElement.h"

#include "Math/GenVector/VectorUtil.h"
#include "Math/Vector2D.h"


using TrigCompositeUtils::Decision;
using TrigCompositeUtils::DecisionContainer;
using TrigCompositeUtils::DecisionID;
using TrigCompositeUtils::DecisionIDContainer;
using ROOT::Math::XYVector;


TrigBhhComboHypo::TrigBhhComboHypo(const std::string& name, ISvcLocator* pSvcLocator)
    : ::ComboHypo(name, pSvcLocator) {}


StatusCode TrigBhhComboHypo::initialize() {
  ATH_MSG_DEBUG( "TrigBhhComboHypo::initialize()" );

  ATH_CHECK( ::ComboHypo::initialize() );

  ATH_CHECK( m_trackParticleContainerKey.initialize() );
  renounce(m_trackParticleContainerKey);
  ATH_CHECK( m_trigBphysContainerKey.initialize() );
  ATH_CHECK( m_beamSpotKey.initialize() );

  ATH_CHECK( m_vertexFitter.retrieve() );
  ATH_CHECK( m_vertexPointEstimator.retrieve() );

  // allowed IDs to filter out incoming decisions at L2 level
  for (const auto& item : triggerMultiplicityMap()) {
    const HLT::Identifier id = HLT::Identifier(item.first);
    m_allowedIDs.insert(id.numeric());
    if (item.second.size() > 1) {
      for (size_t i = 0; i < item.second.size(); i++) {
        m_allowedIDs.insert(TrigCompositeUtils::createLegName(id, i).numeric());
      }
    }
  }
  if (msgLvl(MSG::DEBUG)) {
    ATH_MSG_DEBUG( "Allowed decisions:" );
    for (const DecisionID& id : m_allowedIDs) {
      ATH_MSG_DEBUG( " +++ " << HLT::Identifier(id) );
    }
  }

  if (!m_monTool.empty()) {
    ATH_CHECK( m_monTool.retrieve() );
    ATH_MSG_DEBUG( "GenericMonitoringTool name:" << m_monTool );
  }
  else {
    ATH_MSG_DEBUG( "No GenericMonitoringTool configured: no monitoring histograms will be available" );
  }

  return StatusCode::SUCCESS;
}


StatusCode TrigBhhComboHypo::execute(const EventContext& context) const {

  ATH_MSG_DEBUG( "TrigBhhComboHypo::execute() starts" );

  ATH_MSG_DEBUG( "decision input key: " << decisionsInput().at(0).key() );
  auto previousDecisionsHandle = SG::makeHandle(decisionsInput().at(0), context);
  ATH_CHECK( previousDecisionsHandle.isValid() );
  ATH_MSG_DEBUG( "Running with " << previousDecisionsHandle->size() << " previous decisions" );

  SG::WriteHandle<DecisionContainer> outputDecisionsHandle = TrigCompositeUtils::createAndStore(decisionsOutput().at(0), context);

  auto trigBphysHandle = SG::makeHandle(m_trigBphysContainerKey, context);
  ATH_CHECK( trigBphysHandle.record(std::make_unique<xAOD::TrigBphysContainer>(), std::make_unique<xAOD::TrigBphysAuxContainer>()) );

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle {m_beamSpotKey, context};
  ATH_CHECK( beamSpotHandle.isValid() );

  auto state = std::make_unique<TrigBhhState>(context, *previousDecisionsHandle, *outputDecisionsHandle, trigBphysHandle.ptr(), *beamSpotHandle);

  ATH_CHECK( mergeMuonsFromDecisions(*state) );
  ATH_CHECK( mergeTracksFromViews(*state) );
  ATH_CHECK( findBhhCandidates(*state) );
  ATH_CHECK( createDecisionObjects(*state) );

  ATH_MSG_DEBUG( "TrigBhhComboHypo::execute() terminates with StatusCode::SUCCESS" );
  return StatusCode::SUCCESS;
}


StatusCode TrigBhhComboHypo::mergeMuonsFromDecisions(TrigBhhState& state) const {

  auto& muons = state.muons;
  muons.clear();

  // all muons from views are already connected with previous decisions by TrigMuonEFHypoAlg
  for (const Decision* decision : state.previousDecisions()) {
    if (!TrigCompositeUtils::isAnyIDPassing(decision, m_allowedIDs)) continue;

    ATH_CHECK( decision->hasObjectLink(TrigCompositeUtils::featureString(), ClassID_traits<xAOD::MuonContainer>::ID()) );
    auto muonEL = decision->objectLink<xAOD::MuonContainer>(TrigCompositeUtils::featureString());
    const xAOD::Muon* muon = *muonEL;
    if (!muon->trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle)) continue;

    auto decisionEL = TrigCompositeUtils::decisionToElementLink(decision, state.context());
    auto itr = std::find_if(muons.begin(), muons.end(), [this, muon](const auto& x){ return isIdenticalTracks(muon, *x.link); });
    if (itr == muons.end()) {
      muons.push_back({muonEL, std::vector<ElementLink<DecisionContainer>>(1, decisionEL), DecisionIDContainer()});
    }
    else {
      (*itr).decisionLinks.push_back(decisionEL);
    }
  }

  // muon->pt() is equal to muon->trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle)->pt()
  // and the later is used by TrigMuonEFHypoTool for classification of muEFCB candidates
  std::sort(muons.begin(), muons.end(), [](const auto& lhs, const auto& rhs){ return ((*lhs.link)->pt() > (*rhs.link)->pt()); });

  // for each muon we extract DecisionIDs stored in the associated Decision objects and copy them at muon.decisionIDs
  for (auto& item : muons) {
    for (const ElementLink<xAOD::TrigCompositeContainer>& decisionEL : item.decisionLinks) {
      TrigCompositeUtils::decisionIDs(*decisionEL, item.decisionIDs);
    }
  }

  if (msgLvl(MSG::DEBUG)) {
    ATH_MSG_DEBUG( "Dump found muons before vertex fit: " << muons.size() << " candidates" );
    for (const auto& item : muons) {
      const xAOD::Muon* muon = *item.link;
      const xAOD::TrackParticle* track = *muon->inDetTrackParticleLink();
      ATH_MSG_DEBUG( " -- muon InDetTrackParticle pt/eta/phi/q: " << track->pt() << " / " << track->eta() << " / " << track->phi() << " / " << track->charge() );
      ATH_MSG_DEBUG( "    muon CombinedTrackParticle pt: " << muon->pt() );
      ATH_MSG_DEBUG( "    allowed decisions:" );
      for (const DecisionID& id : item.decisionIDs) {
        ATH_MSG_DEBUG( "    +++ " << HLT::Identifier(id) );
      }
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode TrigBhhComboHypo::mergeTracksFromViews(TrigBhhState& state) const {

  const auto& muons = state.muons;
  auto& tracks = state.tracks;
  tracks.clear();

  size_t viewCounter = 0;
  for (const Decision* decision : state.previousDecisions()) {
    if (!TrigCompositeUtils::isAnyIDPassing(decision, m_allowedIDs)) continue;

    auto viewLinkInfo = TrigCompositeUtils::findLink<ViewContainer>(decision, TrigCompositeUtils::viewString(), true);
    ATH_CHECK( viewLinkInfo.isValid() );
    auto view = *viewLinkInfo.link;

    auto roiLinkInfo = TrigCompositeUtils::findLink<TrigRoiDescriptorCollection>(decision, TrigCompositeUtils::roiString(), true);
    ATH_CHECK( roiLinkInfo.isValid() );
    const auto roi = *roiLinkInfo.link;

    auto tracksHandle = ViewHelper::makeHandle(view, m_trackParticleContainerKey, state.context());
    ATH_CHECK( tracksHandle.isValid() );
    ATH_MSG_DEBUG( "tracks handle " << m_trackParticleContainerKey << " size: " << tracksHandle->size() );

    std::vector<ElementLink<xAOD::TrackParticleContainer>> tracksFromView;
    tracksFromView.reserve(tracksHandle->size());
    for (size_t idx = 0; idx < tracksHandle->size(); ++idx) {
      tracksFromView.emplace_back(ViewHelper::makeLink<xAOD::TrackParticleContainer>(view, tracksHandle, idx));
    }

    for (const auto& trackEL : tracksFromView) {
      const xAOD::TrackParticle* track = *trackEL;
      if (track->definingParametersCovMatrixVec().empty()) continue;

      if (viewCounter == 0 ||
          std::find_if(tracks.begin(), tracks.end(),
                       [this, track](const auto& x){ return isIdenticalTracks(track, *x); }) == tracks.end()) {
        tracks.emplace_back(trackEL);
      }
    }
    viewCounter++;
    if (roi->composite()) {
      break;
    }
  }

  if (msgLvl(MSG::DEBUG)) {
    std::sort(tracks.begin(), tracks.end(), [](const auto& lhs, const auto& rhs){ return ((*lhs)->pt() > (*rhs)->pt()); });
    ATH_MSG_DEBUG( "Found " << tracks.size() << " tracks" );
    for (const auto& trackEL : tracks) {
      const xAOD::TrackParticle* track = *trackEL;
      ATH_MSG_DEBUG( " -- track pt/eta/phi/q: " << track->pt() << " / " << track->eta() << " / " << track->phi() << " / " << track->charge() );
    }
  }

  if (tracks.size() < 2) {
    tracks.clear();
    return StatusCode::SUCCESS;
  }

  // remove muon duplicates
  if (m_applyMuonRemoval) {
    for (size_t i = 0; i < muons.size(); ++i) {
      const xAOD::Muon* muon = *muons[i].link;
      const xAOD::TrackParticle* track = *muon->inDetTrackParticleLink();
      std::sort(tracks.begin(), tracks.end(), [p_mu=track->genvecP4()](const auto& lhs, const auto& rhs){ return ROOT::Math::VectorUtil::DeltaR(p_mu, (*lhs)->genvecP4()) > ROOT::Math::VectorUtil::DeltaR(p_mu, (*rhs)->genvecP4()); });
      if (isIdenticalTracks(track, *tracks.back())) tracks.pop_back();
    }
    std::sort(tracks.begin(), tracks.end(), [](const auto& lhs, const auto& rhs){ return ((*lhs)->pt() > (*rhs)->pt()); });

    if (msgLvl(MSG::DEBUG)) {
      ATH_MSG_DEBUG( "Found " << tracks.size() << " tracks after muon removal" );
      for (const auto& trackEL : tracks) {
        const xAOD::TrackParticle* track = *trackEL;
        ATH_MSG_DEBUG( " -- track pt/eta/phi/q: " << track->pt() << " / " << track->eta() << " / " << track->phi() << " / " << track->charge() );
      }
    }
  }

  // apply pT threshold cut
  std::sort(tracks.begin(), tracks.end(), [](const auto& lhs, const auto& rhs){ return ((*lhs)->pt() > (*rhs)->pt()); });
  tracks.erase(std::upper_bound(tracks.begin(), tracks.end(), m_trkPt, [](double trkPt, const auto& trackEL){ return trkPt > (*trackEL)->pt(); }), tracks.end());

  if (msgLvl(MSG::DEBUG)) {
    ATH_MSG_DEBUG( "Found " << tracks.size() << " tracks after pT threshold cut" );
    for (const auto& trackEL : tracks) {
      const xAOD::TrackParticle* track = *trackEL;
      ATH_MSG_DEBUG( " -- track pt/eta/phi/q: " << track->pt() << " / " << track->eta() << " / " << track->phi() << " / " << track->charge() );
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode TrigBhhComboHypo::findBhhCandidates(TrigBhhState& state) const {

  // monitored variables
  auto mon_nMuon = Monitored::Scalar<int>("nMuon", 0);
  auto mon_nTrk = Monitored::Scalar<int>("nTrk", 0);
  auto mon_nBPhysObject = Monitored::Scalar<int>("nBPhysObject", 0);

  auto group = Monitored::Group(m_monTool,
    mon_nMuon, mon_nTrk, mon_nBPhysObject);

  const auto& tracks = state.tracks;

  size_t iterations = 0;
  bool isOverWarningThreshold = false;

  std::vector<ElementLink<xAOD::TrackParticleContainer>> trackParticleLinks(2);
  for (size_t itrk1 = 0; itrk1 < tracks.size(); ++itrk1) {
    trackParticleLinks[0] = tracks[itrk1];
    const xAOD::TrackParticle* trk1 = *trackParticleLinks[0];
    auto p_trk1 = trk1->genvecP4();
    auto charge1 = trk1->charge();

    for (size_t itrk2 = itrk1 + 1; itrk2 < tracks.size(); ++itrk2) {
      trackParticleLinks[1] = tracks[itrk2];
      const xAOD::TrackParticle* trk2 = *trackParticleLinks[1];
      auto p_trk2 = trk2->genvecP4();
      auto charge2 = trk2->charge();

      auto mKK = (p_trk1.SetM(PDG::mKaon) + p_trk2.SetM(PDG::mKaon)).M();
      auto mPiPi = (p_trk1.SetM(PDG::mPion) + p_trk2.SetM(PDG::mPion)).M();

      if (charge1 * charge2 > 0. || mKK < m_massRange.value().first || mPiPi > m_massRange.value().second) continue;

      // fit tracks to the common vertex
      auto vertex = fit(state.context(), trackParticleLinks);
      iterations++;
      if (!vertex) continue;

      state.trigBphysCollection().push_back(new xAOD::TrigBphys());
      xAOD::TrigBphys* triggerObject = state.trigBphysCollection().back();

      ATH_CHECK( fillTriggerObject(*triggerObject, *vertex, state.beamSpotPosition()) );
      triggerObject->setMass(mKK);

      ATH_MSG_DEBUG( "Found B -> h+ h- candidate: M(K+,K-) = " << mKK << " M(pi+,pi-) = " << mPiPi );

      if (iterations > m_fitAttemptsWarningThreshold && !isOverWarningThreshold) {
        ATH_MSG_WARNING( "B -> hh: " << state.trigBphysCollection().size() << " vertices created after " << iterations << " vertex fitter calls" );
        isOverWarningThreshold = true;
      }
      if (iterations > m_fitAttemptsBreakThreshold) {
        ATH_MSG_WARNING( "b -> hh: the number of fit attempts has exceeded the limit, breaking the loop at this point" );
        itrk1 = tracks.size();
        break;
      }

    }
  }

  mon_nMuon = state.muons.size();
  mon_nTrk = tracks.size();
  mon_nBPhysObject = state.trigBphysCollection().size();

  return StatusCode::SUCCESS;
}


StatusCode TrigBhhComboHypo::createDecisionObjects(TrigBhhState& state) const {

  // state.muons() are common seeds for all B -> hh candidates
  std::vector<ElementLink<TrigCompositeUtils::DecisionContainer>> previousDecisionLinks;
  DecisionIDContainer decisionIDs;
  std::vector<const DecisionIDContainer*> previousDecisionIDs = { &decisionIDs };

  for (const Decision* previousDecision : state.previousDecisions()) {
    if (!TrigCompositeUtils::isAnyIDPassing(previousDecision, m_allowedIDs)) continue;

    previousDecisionLinks.emplace_back(TrigCompositeUtils::decisionToElementLink(previousDecision, state.context()));
    TrigCompositeUtils::decisionIDs(previousDecision, decisionIDs);
  }

  for (const xAOD::TrigBphys* triggerObject : state.trigBphysCollection()) {
    ATH_MSG_DEBUG( "Found xAOD::TrigBphys object: fitMass = " << triggerObject->fitmass() << ", chi2 = " << triggerObject->fitchi2() );

    auto triggerObjectEL = ElementLink<xAOD::TrigBphysContainer>(state.trigBphysCollection(), triggerObject->index());
    ATH_CHECK( triggerObjectEL.isValid() );

    // create a new output Decision object, backed by the 'decisions' container.
    Decision* decision = TrigCompositeUtils::newDecisionIn(&state.decisions(), TrigCompositeUtils::comboHypoAlgNodeName());

    // attach all previous decisions, no need to use TrigCompositeUtils::linkToPrevious()
    decision->addObjectCollectionLinks(TrigCompositeUtils::seedString(), previousDecisionLinks);

    // set mandatory link to the trigger object
    decision->setObjectLink<xAOD::TrigBphysContainer>(TrigCompositeUtils::featureString(), triggerObjectEL);

    for (const auto& tool : hypoTools()) {
      ATH_MSG_DEBUG( "Go to " << tool );
      ATH_CHECK( tool->decideOnSingleObject(decision, previousDecisionIDs) );
    }
  }

  return StatusCode::SUCCESS;
}


std::unique_ptr<xAOD::Vertex> TrigBhhComboHypo::fit(
    const EventContext& context,
    const std::vector<ElementLink<xAOD::TrackParticleContainer>>& trackParticleLinks,
    const std::vector<double>& trkMass) const {

  ATH_MSG_DEBUG( "Perform vertex fit" );

  if (trackParticleLinks.size() != 2) {
    ATH_MSG_WARNING( "Exactly two tracks should be given to the vertex fitter" );
    return std::unique_ptr<xAOD::Vertex>(nullptr);
  }

  std::vector<const xAOD::TrackParticle*> tracklist(trackParticleLinks.size(), nullptr);
  std::transform(trackParticleLinks.begin(), trackParticleLinks.end(), tracklist.begin(),
                 [](const ElementLink<xAOD::TrackParticleContainer>& link){ return *link; });

  Amg::Vector3D startingPoint = Amg::Vector3D::Zero(3);
  int flag = 0;
  int errorcode = 0;
  const Trk::Perigee& perigee1 = tracklist[0]->perigeeParameters();
  const Trk::Perigee& perigee2 = tracklist[1]->perigeeParameters();
  startingPoint = m_vertexPointEstimator->getCirclesIntersectionPoint(&perigee1, &perigee2, flag, errorcode);
  if (errorcode != 0) startingPoint = Amg::Vector3D::Zero(3);
  ATH_MSG_DEBUG( "Starting point: (" << startingPoint(0) << ", " << startingPoint(1) << ", " << startingPoint(2) << ")" );

  auto fitterState = m_vertexFitter->makeState(context);
  m_vertexFitter->setMassInputParticles(trkMass, *fitterState);

  std::unique_ptr<xAOD::Vertex> vertex(m_vertexFitter->fit(tracklist, startingPoint, *fitterState));
  if (!vertex) {
    ATH_MSG_DEBUG( "Vertex fit fails" );
    return vertex;
  }
  if (vertex->chiSquared() > m_chi2) {
    ATH_MSG_DEBUG( "Fit is successful, but vertex chi2 is too high, we are not going to save it (chi2 = " << vertex->chiSquared() << ")" );
    vertex.reset();
    return vertex;
  }
  ATH_MSG_DEBUG( "Fit is successful" );

  // update trackParticleLinks()
  vertex->clearTracks();
  vertex->setTrackParticleLinks(trackParticleLinks);

  return vertex;
}


StatusCode TrigBhhComboHypo::fillTriggerObject(
    xAOD::TrigBphys& triggerObject,
    const xAOD::Vertex& vertex,
    const Amg::Vector3D& productionVertex,
    const std::vector<double>& trkMass) const {

  // refitted track momentum as a 4-vector for mass hypothesis defined by the given decay value
  xAOD::TrackParticle::GenVecFourMom_t momentum;
  std::vector<xAOD::TrackParticle::GenVecFourMom_t> momenta;
  ATH_CHECK( vertex.vxTrackAtVertexAvailable() );
  ATH_CHECK( vertex.vxTrackAtVertex().size() == trkMass.size() );
  for (size_t i = 0; i < vertex.vxTrackAtVertex().size(); ++i) {
    const Trk::TrackParameters* perigee = vertex.vxTrackAtVertex()[i].perigeeAtVertex();
    ATH_CHECK( perigee != nullptr );
    const Amg::Vector3D& p = perigee->momentum();
    momenta.emplace_back(p.x(), p.y(), p.z(), trkMass[i]);
    momentum += momenta.back();
  }

  triggerObject.initialise(0, momentum.Eta(), momentum.Phi(), momentum.Pt(), xAOD::TrigBphys::BHH, momentum.M(), xAOD::TrigBphys::EF);

  triggerObject.setFitmass(momentum.M());
  triggerObject.setFitx(vertex.x());
  triggerObject.setFity(vertex.y());
  triggerObject.setFitz(vertex.z());
  triggerObject.setFitchi2(vertex.chiSquared());
  triggerObject.setFitndof(vertex.numberDoF());
  triggerObject.setLxy(Lxy(productionVertex, vertex.position(), momenta));

  // set all the particles associated with the decay
  triggerObject.setTrackParticleLinks(vertex.trackParticleLinks());

  ATH_MSG_DEBUG(
    "TrigBphys object:\n\t  " <<
    "roiId:         " << triggerObject.roiId()  << "\n\t  " <<
    "particleType:  " << triggerObject.particleType() << "\n\t  " <<
    "level:         " << triggerObject.level() << "\n\t  " <<
    "eta:           " << triggerObject.eta() << "\n\t  " <<
    "phi:           " << triggerObject.phi() << "\n\t  " <<
    "mass:          " << triggerObject.mass() << "\n\t  " <<
    "fitmass:       " << triggerObject.fitmass() << "\n\t  " <<
    "chi2/NDF:      " << triggerObject.fitchi2() << " / " << triggerObject.fitndof() << "\n\t  " <<
    "vertex:        (" << triggerObject.fitx() << ", " << triggerObject.fity() << ", " << triggerObject.fitz() << ")" );

  return StatusCode::SUCCESS;
}


bool TrigBhhComboHypo::isIdenticalTracks(const xAOD::TrackParticle* lhs, const xAOD::TrackParticle* rhs) const {

  if (lhs->charge() * rhs->charge() < 0.) return false;
  return (ROOT::Math::VectorUtil::DeltaR(lhs->genvecP4(), rhs->genvecP4()) < m_deltaR);
}


bool TrigBhhComboHypo::isIdenticalTracks(const xAOD::Muon* lhs, const xAOD::Muon* rhs) const {

  return isIdenticalTracks(*lhs->inDetTrackParticleLink(), *rhs->inDetTrackParticleLink());
}


double TrigBhhComboHypo::Lxy(const Amg::Vector3D& productionVertex, const Amg::Vector3D& decayVertex, const std::vector<xAOD::TrackParticle::GenVecFourMom_t>& momenta) const {

  XYVector R(decayVertex.x() - productionVertex.x(), decayVertex.y() - productionVertex.y());
  XYVector pT;
  for (const auto& p : momenta) {
    pT += XYVector(p.x(), p.y());
  }
  return R.Dot(pT.unit());
}
