/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>
#include "MuonMatchingTool.h"
#include "xAODTrigger/MuonRoIContainer.h"

namespace {
  
  const xAOD::TrackParticle* getMSTrack(const xAOD::Muon& mu){
    using Type = xAOD::Muon::TrackParticleType;
    constexpr std::array<Type, 3> tpTypes{Type::ExtrapolatedMuonSpectrometerTrackParticle,
                                          Type::MSOnlyExtrapolatedMuonSpectrometerTrackParticle,
                                          Type::MuonSpectrometerTrackParticle};
    for (Type type : tpTypes){
      const xAOD::TrackParticle* trk = mu.trackParticle(type);
      if (trk) {
        return trk;
      }
    }
    return nullptr;
  }
}

const static double ZERO_LIMIT = 1.e-5;

MuonMatchingTool::MuonMatchingTool(const std::string& type, const std::string& name, const IInterface*  parent)
  : AthAlgTool(type, name, parent)
{}


StatusCode MuonMatchingTool::initialize(){

  ATH_CHECK(m_trigDec.retrieve() );
  ATH_CHECK(m_thresholdTool.retrieve() );
  if(m_use_extrapolator){
    ATH_CHECK(m_extrapolator.retrieve() );
  }
  ATH_CHECK(m_MuonKey.initialize(!m_MuonKey.empty()));
  ATH_CHECK(m_MuonRoIKey.initialize());
  ATH_CHECK(m_L2SAMuonKey.initialize());
  ATH_CHECK(m_FastRecoSAMuonKey.initialize(!m_FastRecoSAMuonKey.empty()));
  ATH_CHECK(m_L2CBMuonKey.initialize());
  ATH_CHECK(m_EFSAMuonKey.initialize());
  ATH_CHECK(m_EFSAMlbktMuonKey.initialize(!m_EFSAMlbktMuonKey.empty()));
  ATH_CHECK(m_EFSANewFastMuonKey.initialize(!m_EFSANewFastMuonKey.empty()));
  ATH_CHECK(m_EFCBMuonKey.initialize());
  ATH_CHECK(m_EFSAFSMuonKey.initialize());
  ATH_CHECK(m_EFSAFSMlbktMuonKey.initialize(!m_EFSAFSMlbktMuonKey.empty()));
  ATH_CHECK(m_EFSAFSNewFastMuonKey.initialize(!m_EFSAFSNewFastMuonKey.empty()));
  ATH_CHECK(m_EFCBFSMuonKey.initialize());

  return StatusCode::SUCCESS;
}

Amg::Vector3D MuonMatchingTool::offlineMuonAtPivot(const EventContext& ctx, 
                                                   const xAOD::Muon& mu) const {
  const xAOD::TrackParticle* track{mu.trackParticle(xAOD::Muon::TrackParticleType::Primary)};
  std::unique_ptr<Trk::TrackParameters> extPars{extTrackToPivot(ctx, track)};
  return extPars ? extPars->position() : Amg::Vector3D::Zero();
}

template<>
std::tuple<bool, double,double> 
MuonMatchingTool::trigPosForMatch<xAOD::L2StandAloneMuon>(const xAOD::L2StandAloneMuon *trig){
  return std::forward_as_tuple(true, trig->roiEta(), trig->roiPhi());
}

std::tuple<bool, double,double> 
MuonMatchingTool::trigPosForMatchSATrack(const xAOD::Muon *mu){
  return mu->muonType() == xAOD::Muon::MuonType::MuonStandAlone 
    ? std::forward_as_tuple(true, mu->eta(), mu->phi()) 
    : std::forward_as_tuple(false, 0., 0.);
}

std::tuple<bool, double,double> 
MuonMatchingTool::trigPosForMatchCBTrack(const xAOD::Muon *mu){
  return mu->muonType() == xAOD::Muon::MuonType::Combined 
    ? std::forward_as_tuple(true, mu->eta(), mu->phi()) 
    : std::forward_as_tuple(false, 0., 0.);
}

std::tuple<bool, double,double> 
MuonMatchingTool::PosForMatchSATrack(const xAOD::Muon *mu,
                                     const bool isPhaseII){
  const xAOD::Muon::TrackParticleType type {isPhaseII 
    ? xAOD::Muon::TrackParticleType::Primary 
    : xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle};  
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(type);
  return MuonTrack 
    ? std::forward_as_tuple(true, MuonTrack->eta(), MuonTrack->phi()) 
    : std::forward_as_tuple(false, 0., 0.);
}

std::tuple<bool, double,double> 
MuonMatchingTool::PosForMatchCBTrack(const xAOD::Muon *mu){
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle);
  return MuonTrack 
    ? std::forward_as_tuple(true, MuonTrack->eta(), MuonTrack->phi()) 
    : std::forward_as_tuple(false, 0., 0.);
}

const xAOD::Muon* 
MuonMatchingTool::matchEFSA(const xAOD::Muon *mu, 
                            std::string trig, 
                            bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSA()");
  const xAOD::TrackParticle* MuonTrack = getMSTrack(*mu);

  const std::string& containerKey {trig.contains("newFast") 
    ? m_EFSANewFastMuonKey.key() 
    : (trig.contains("mlbkt") ? m_EFSAMlbktMuonKey.key() : m_EFSAMuonKey.key())};

  return MuonTrack
    ? match<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, containerKey + ".*", &MuonMatchingTool::trigPosForMatchSATrack) 
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchEFSA(const xAOD::TruthParticle *mu, 
                            std::string trig, 
                            bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSA() for truth particle");

  const std::string& containerKey {trig.contains("newFast") 
    ? m_EFSANewFastMuonKey.key() 
    : (trig.contains("mlbkt") ? m_EFSAMlbktMuonKey.key() : m_EFSAMuonKey.key())};

  return mu
    ? match<xAOD::Muon>(mu, std::move(trig), m_EFreqdR, pass, containerKey + ".*", &MuonMatchingTool::trigPosForMatchSATrack) 
    : nullptr;
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> 
MuonMatchingTool::matchEFSALinkInfo(const xAOD::Muon *mu, 
                                    std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSALinkInfo()");
  bool pass = false;
  const xAOD::TrackParticle* MuonTrack = getMSTrack(*mu);
  
  const std::string& containerKey {trig.contains("newFast") 
    ? m_EFSANewFastMuonKey.key() 
    : (trig.contains("mlbkt") ? m_EFSAMlbktMuonKey.key() : m_EFSAMuonKey.key())};

  return MuonTrack
    ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, containerKey + ".*", &MuonMatchingTool::trigPosForMatchSATrack) 
    : TrigCompositeUtils::LinkInfo<xAOD::MuonContainer>{};
}

const xAOD::Muon* 
MuonMatchingTool::matchEFSAReadHandle(const EventContext& ctx, 
                                      const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAReadHandle()");
  const xAOD::TrackParticle* MuonTrack = getMSTrack(*mu);

  return MuonTrack
    ? matchReadHandle<xAOD::Muon>(MuonTrack, m_EFreqdR, m_EFSAMuonKey, ctx, &MuonMatchingTool::trigPosForMatchSATrack)
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchEFCB(const xAOD::TruthParticle *mu, 
                            std::string trig, 
                            bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCB() for TruthParticle");
  return mu
    ? match<xAOD::Muon>(mu, std::move(trig), m_EFreqdR, pass, m_EFCBMuonKey.key() + "*.*", &MuonMatchingTool::trigPosForMatchCBTrack) 
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchEFCB(const xAOD::Muon *mu, 
                            std::string trig, 
                            bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCB()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  return MuonTrack 
    ? match<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, m_EFCBMuonKey.key() + "*.*", &MuonMatchingTool::trigPosForMatchCBTrack) 
    : nullptr;
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> 
MuonMatchingTool::matchEFCBLinkInfo(const xAOD::Muon *mu, 
                                    std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBLinkInfo()");
  bool pass = false;
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  return MuonTrack 
    ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, m_EFCBMuonKey.key() + "*.*", &MuonMatchingTool::trigPosForMatchCBTrack) 
    : TrigCompositeUtils::LinkInfo<xAOD::MuonContainer>{};
}

const xAOD::Muon* 
MuonMatchingTool::matchEFCBReadHandle(const EventContext& ctx, 
                                      const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBReadHandle()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  return MuonTrack 
    ? matchReadHandle<xAOD::Muon>( MuonTrack, m_EFreqdR, m_EFCBMuonKey, ctx, &MuonMatchingTool::trigPosForMatchCBTrack) 
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchEFSAFS(const xAOD::Muon *mu, 
                              std::string trig, 
                              bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAFS()");
  const xAOD::TrackParticle* MuonTrack = getMSTrack(*mu);

  const std::string& containerKey {trig.contains("newFast") 
    ? m_EFSAFSNewFastMuonKey.key() 
    : (trig.contains("mlbkt") ? m_EFSAFSMlbktMuonKey.key() : m_EFSAFSMuonKey.key())};

  return MuonTrack
    ? match<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, containerKey + ".*", &MuonMatchingTool::trigPosForMatchSATrack) 
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchEFSAFS(const xAOD::TruthParticle *mu, 
                              std::string trig, 
                              bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAFS() for truth particle");

  const std::string& containerKey {trig.contains("newFast") 
    ? m_EFSAFSNewFastMuonKey.key() 
    : (trig.contains("mlbkt") ? m_EFSAFSMlbktMuonKey.key() : m_EFSAFSMuonKey.key())};

  return mu
    ? match<xAOD::Muon>(mu, std::move(trig), m_EFreqdR, pass, containerKey + ".*", &MuonMatchingTool::trigPosForMatchSATrack) 
    : nullptr;
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> 
MuonMatchingTool::matchEFSAFSLinkInfo(const xAOD::Muon *mu, 
                                      std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAFSLinkInfo()");
  bool pass = false;
  const xAOD::TrackParticle* MuonTrack = getMSTrack(*mu);

  const std::string& containerKey {trig.contains("newFast") 
    ? m_EFSAFSNewFastMuonKey.key() 
    : (trig.contains("mlbkt") ? m_EFSAFSMlbktMuonKey.key() : m_EFSAFSMuonKey.key())};

  return MuonTrack
    ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, containerKey + ".*", &MuonMatchingTool::trigPosForMatchSATrack) 
    : TrigCompositeUtils::LinkInfo<xAOD::MuonContainer>{};
}

const xAOD::Muon* 
MuonMatchingTool::matchEFSAFSReadHandle(const EventContext& ctx, 
                                        const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAFSReadHandle()");
  const xAOD::TrackParticle* MuonTrack = getMSTrack(*mu);
  return MuonTrack 
    ? matchReadHandle<xAOD::Muon>(MuonTrack, m_EFreqdR, m_EFSAFSMuonKey, ctx, &MuonMatchingTool::trigPosForMatchSATrack) 
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchEFCBFS(const xAOD::TruthParticle *mu, 
                              std::string trig, 
                              bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBFS() for TruthParticle");
  return mu 
    ? match<xAOD::Muon>( mu, std::move(trig), m_EFreqdR, pass, m_EFCBFSMuonKey.key() + ".*", &MuonMatchingTool::trigPosForMatchCBTrack) 
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchEFCBFS(const xAOD::Muon *mu, 
                              std::string trig, 
                              bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBFS()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  return MuonTrack ? 
    match<xAOD::Muon>( MuonTrack, std::move(trig), m_EFreqdR, pass, m_EFCBFSMuonKey.key() + ".*", &MuonMatchingTool::trigPosForMatchCBTrack) 
    : nullptr;
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> 
MuonMatchingTool::matchEFCBFSLinkInfo(const xAOD::Muon *mu, 
                                      std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBFSLinkInfo()");
  bool pass = false;
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  return MuonTrack 
    ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, m_EFCBFSMuonKey.key() + ".*", &MuonMatchingTool::trigPosForMatchCBTrack) 
    : TrigCompositeUtils::LinkInfo<xAOD::MuonContainer>{};
}

const xAOD::Muon* 
MuonMatchingTool::matchEFCBFSReadHandle(const EventContext& ctx, 
                                        const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBFSReadHandle()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  return MuonTrack 
    ? matchReadHandle<xAOD::Muon>(MuonTrack, m_EFreqdR, m_EFCBFSMuonKey, ctx, &MuonMatchingTool::trigPosForMatchCBTrack) 
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchEFIso(const xAOD::Muon *mu, 
                             std::string trig, 
                             bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFIso()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  return MuonTrack 
    ? match<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_MuonsIso", &MuonMatchingTool::trigPosForMatchCBTrack) 
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchEFIso(const xAOD::TruthParticle *mu, 
                             std::string trig, 
                             bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFIso() for truth particle");
  return mu 
    ? match<xAOD::Muon>( mu, std::move(trig), m_EFreqdR, pass, "HLT_MuonsIso", &MuonMatchingTool::trigPosForMatchCBTrack) 
    : nullptr;
}

const xAOD::TrackParticle* 
MuonMatchingTool::SearchEFTrack(const EventContext &ctx, 
                                const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer>& muLinkInfo, 
                                const SG::ReadHandleKey<xAOD::TrackParticleContainer>& ReadHandleKey) const {
  const xAOD::TrackParticle* MatchedTrack = nullptr;
  const ElementLink<xAOD::MuonContainer> muEL = muLinkInfo.link;
  float EFEta = (*muEL)->eta();
  float EFPhi = (*muEL)->phi();
  float mindR = 999.;

  SG::ReadHandle<xAOD::TrackParticleContainer> trackHandle(ReadHandleKey, ctx);
  if ( !trackHandle.isValid() ) return MatchedTrack;

  const auto track = m_trigDec->associateToEventView<xAOD::TrackParticleContainer>(trackHandle, muLinkInfo);
  const xAOD::TrackParticleContainer::const_iterator begin = track.first;
  const xAOD::TrackParticleContainer::const_iterator end  = track.second;

  for (xAOD::TrackParticleContainer::const_iterator it = begin; it != end; ++it) {

    float deta = EFEta - (*it)->eta();
    float dphi = xAOD::P4Helpers::deltaPhi(EFPhi, (*it)->phi() );
    float dR = std::sqrt(deta*deta + dphi*dphi);

    if( dR< mindR ){
      mindR = dR;
      MatchedTrack = (*it);
    }
  }
  return MatchedTrack;
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> 
MuonMatchingTool::matchEFIsoLinkInfo(const xAOD::Muon *mu, 
                                     std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBLinkInfo()");
  bool pass = false;
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  return MuonTrack 
    ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_MuonsIso", &MuonMatchingTool::trigPosForMatchCBTrack) 
    : TrigCompositeUtils::LinkInfo<xAOD::MuonContainer>{};
}

const xAOD::L2StandAloneMuon* 
MuonMatchingTool::matchL2SA(const EventContext& ctx, 
                            const xAOD::Muon *mu, 
                            const std::string& trig, 
                            bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2SA()");
  float reqdR = m_L2SAreqdR;
  if(m_use_extrapolator){
    reqdR = reqdRL1byPt(mu->pt());
    const Amg::Vector3D extPos = offlineMuonAtPivot(ctx, *mu);
    if(extPos.norm()>ZERO_LIMIT){
      return match<xAOD::L2StandAloneMuon>( &extPos, trig, reqdR, pass);
    }
  }
  return match<xAOD::L2StandAloneMuon>(mu, trig, reqdR, pass, m_L2SAMuonKey.key());
}

const xAOD::Muon* 
MuonMatchingTool::matchFastRecoSA(const EventContext& ctx, 
                                  const xAOD::Muon *mu, 
                                  const std::string& trig, 
                                  bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchFastRecoSA()");
  float reqdR = m_L2SAreqdR;
  if(m_use_extrapolator){
    reqdR = reqdRL1byPt(mu->pt());
    const Amg::Vector3D extPos = offlineMuonAtPivot(ctx, *mu);
    if(extPos.norm()>ZERO_LIMIT){
      return match<xAOD::Muon>( &extPos, trig, reqdR, pass);
    }
  }
  return match<xAOD::Muon>( mu, trig, reqdR, pass, m_FastRecoSAMuonKey.key());
}

const xAOD::L2StandAloneMuon* 
MuonMatchingTool::matchL2SA(const xAOD::TruthParticle *mu, 
                            const std::string& trig, 
                            bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2SA() from TruthParticle");
  return match<xAOD::L2StandAloneMuon>( mu, trig, m_L2SAreqdR, pass, m_L2SAMuonKey.key());
}

const xAOD::Muon* 
MuonMatchingTool::matchFastRecoSA(const xAOD::TruthParticle *mu, 
                                  const std::string& trig, 
                                  bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchFastRecoSA() from TruthParticle");
  return match<xAOD::Muon>( mu, trig, m_L2SAreqdR, pass, m_FastRecoSAMuonKey.key());
}

const TrigCompositeUtils::LinkInfo<xAOD::L2StandAloneMuonContainer> 
MuonMatchingTool::searchL2SALinkInfo(const xAOD::Muon *mu, 
                                     std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::searchL2SALinkInfo()");
  bool pass = false;
  return matchLinkInfo<xAOD::L2StandAloneMuon>(mu, std::move(trig), 1000., pass, m_L2SAMuonKey.key());
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> 
MuonMatchingTool::searchFastRecoSALinkInfo(const xAOD::Muon *mu, 
                                           std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::searchFastRecoLinkInfo()");
  bool pass = false;
  return matchLinkInfo<xAOD::Muon>(mu, std::move(trig), 1000., pass, m_FastRecoSAMuonKey.key());
}

const xAOD::L2StandAloneMuon* 
MuonMatchingTool::matchL2SAReadHandle(const EventContext& ctx, 
                                      const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2SAReadHandle()");
  float reqdR = m_L2SAreqdR;
  if(m_use_extrapolator){
    reqdR = reqdRL1byPt(mu->pt());
    const Amg::Vector3D extPos = offlineMuonAtPivot(ctx, *mu);
    if(extPos.norm()>ZERO_LIMIT){
      return matchReadHandle<xAOD::L2StandAloneMuon>(&extPos, reqdR, m_L2SAMuonKey, ctx);
    }
  }
  const xAOD::TrackParticle* MuonTrack = getMSTrack(*mu);
  return MuonTrack 
    ? matchReadHandle<xAOD::L2StandAloneMuon>(MuonTrack, reqdR, m_L2SAMuonKey, ctx) 
    : nullptr;
}

const xAOD::Muon* 
MuonMatchingTool::matchFastRecoSAReadHandle(const EventContext& ctx, 
                                            const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchFastRecoSAReadHandle()");
  float reqdR = m_L2SAreqdR;
  if(m_use_extrapolator){
    reqdR = reqdRL1byPt(mu->pt());
    const Amg::Vector3D extPos = offlineMuonAtPivot(ctx, *mu);
    if(extPos.norm()>ZERO_LIMIT){
      return matchReadHandle<xAOD::Muon>(&extPos, reqdR, m_FastRecoSAMuonKey, ctx);
    }
  }
  const xAOD::TrackParticle* MuonTrack = getMSTrack(*mu);
  return MuonTrack 
    ? matchReadHandle<xAOD::Muon>(MuonTrack, reqdR, m_FastRecoSAMuonKey, ctx) 
    : nullptr;
}

const xAOD::L2CombinedMuon* 
MuonMatchingTool::matchL2CB(const xAOD::Muon *mu, 
                            std::string trig, 
                            bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2CB()");
  return match<xAOD::L2CombinedMuon>(mu, std::move(trig), m_L2CBreqdR, pass, m_L2CBMuonKey.key());
}

const xAOD::L2CombinedMuon* 
MuonMatchingTool::matchL2CB(const xAOD::TruthParticle *mu, 
                            std::string trig, 
                            bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2CB() from TruthParticle");
  return match<xAOD::L2CombinedMuon>(mu, std::move(trig), m_L2CBreqdR, pass, m_L2CBMuonKey.key());
}

const TrigCompositeUtils::LinkInfo<xAOD::L2CombinedMuonContainer> 
MuonMatchingTool::searchL2CBLinkInfo(const xAOD::Muon *mu, 
                                     std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::searchL2CBLinkInfo()");
  bool pass = false;
  return matchLinkInfo<xAOD::L2CombinedMuon>(mu, std::move(trig),  1000., pass, m_L2CBMuonKey.key());
}

const xAOD::L2CombinedMuon* 
MuonMatchingTool::matchL2CBReadHandle(const EventContext& ctx, 
                                      const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2CBReadHandle()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  return MuonTrack 
    ? matchReadHandle<xAOD::L2CombinedMuon>(MuonTrack, m_L2CBreqdR, m_L2CBMuonKey, ctx) 
    : nullptr;
}

const xAOD::MuonRoI* 
MuonMatchingTool::matchL1(const EventContext& ctx, 
                          double refEta, 
                          double refPhi, 
                          double reqdR, 
                          const std::string& trig, 
                          bool &pass) const {

    /// Retrieve the chain configuration and the lower name corresponding to the L1 threshold
    const TrigConf::HLTChain* chainCfg = m_trigDec->ExperimentalAndExpertMethods().getChainConfigurationDetails(trig);
    const std::string L1toMatch = chainCfg->lower_chain_name().substr(3);
    
    SG::ReadHandle<xAOD::MuonRoIContainer> L1rois(m_MuonRoIKey, ctx);
    const xAOD::MuonRoI* closest = nullptr;

    for (const xAOD::MuonRoI* l1muon : *L1rois){

        // get all L1 thresholds from the L1 menu along with whether the L1roi passed each of those or not
        const std::vector<std::pair<std::shared_ptr<TrigConf::L1Threshold>, bool> > L1thr_list = m_thresholdTool-> getThresholdDecisions(
                    l1muon->roiWord(), ctx);
        
        // check the L1 threshold we are looking for
        bool L1thr_isMatch = false;
        for(const std::pair<std::shared_ptr<TrigConf::L1Threshold>, bool>&  L1thr : L1thr_list){
            if (L1toMatch == L1thr.first->name()){
                L1thr_isMatch = L1thr.second;
                break;
            }
        }
        if (!L1thr_isMatch) continue;

        double l1muonEta = l1muon->eta();
        double l1muonPhi = l1muon->phi();
        
        double deta = refEta - l1muonEta;
        double dphi = xAOD::P4Helpers::deltaPhi(refPhi, l1muonPhi);
        double dR = std::sqrt(deta*deta + dphi*dphi);
        ATH_MSG_DEBUG("L1 muon candidate eta=" << l1muonEta << " phi=" << l1muonPhi << " dR=" << dR);
        if( dR<reqdR ){
            reqdR = dR;
            pass = true;
            closest = l1muon;
            ATH_MSG_DEBUG("*** L1 muon eta=" << l1muonEta << " phi=" << l1muonPhi << " dR=" << dR <<  " isPassed=true" ); 
        }
        else{
            ATH_MSG_DEBUG("*** L1 muon eta=" << l1muonEta << " phi=" << l1muonPhi << " dR=" << dR <<  " isPassed=false" );
        }
    }
    return closest;
}

const xAOD::MuonRoI* 
MuonMatchingTool::matchL1(const EventContext& ctx, 
                          const xAOD::Muon *mu, 
                          const std::string& trig, 
                          bool &pass) const {
  double refEta = mu->eta();
  double refPhi = mu->phi();
  double reqdR = 0.25;

  if(m_use_extrapolator){
    reqdR = reqdRL1byPt(mu->pt());
    const Amg::Vector3D extPos = offlineMuonAtPivot(ctx, *mu);
    if(extPos.norm()>ZERO_LIMIT){
      refEta = extPos.eta();
      refPhi = extPos.phi();
    }
  }
  return matchL1(ctx, refEta, refPhi, reqdR, trig, pass);
}

const xAOD::MuonRoI* 
MuonMatchingTool::matchL1(const EventContext& ctx, 
                          const xAOD::TruthParticle *mu, 
                          const std::string& trig, 
                          bool &pass) const {
  double refEta = mu->eta();
  double refPhi = mu->phi();
  double reqdR = 0.25;
  return matchL1(ctx, refEta, refPhi, reqdR, trig, pass);
}

const xAOD::Muon* 
MuonMatchingTool::matchL2SAtoOff(const EventContext& ctx, 
                                 const xAOD::L2StandAloneMuon* samu) const {
  return matchOff(ctx, samu, m_L2SAreqdR, 
    [this](const xAOD::Muon* mu) {return MuonMatchingTool::PosForMatchSATrack(mu, m_isPhII);});
}

const xAOD::Muon* 
MuonMatchingTool::matchFastRecoSAtoOff(const EventContext& ctx, 
                                       const xAOD::Muon* samu) const {
  return matchOff(ctx, samu, m_L2SAreqdR, 
    [this](const xAOD::Muon* mu) {return MuonMatchingTool::PosForMatchSATrack(mu, m_isPhII);});
}

const xAOD::Muon* 
MuonMatchingTool::matchL2CBtoOff(const EventContext& ctx, 
                                 const xAOD::L2CombinedMuon* cbmu) const {
  return matchOff(ctx, cbmu, m_L2CBreqdR, &MuonMatchingTool::PosForMatchCBTrack);
}


bool 
MuonMatchingTool::isMatchedL2SA(const xAOD::L2StandAloneMuon* samu, 
                                const xAOD::Muon* mu) const {
  return xAOD::P4Helpers::deltaR(mu, samu, false) < m_L2SAreqdR;
}

bool 
MuonMatchingTool::isMatchedFastRecoSA(const xAOD::Muon* samu, 
                                      const xAOD::Muon* mu) const {
  return xAOD::P4Helpers::deltaR(mu, samu, false) < m_L2SAreqdR;
}


bool 
MuonMatchingTool::isMatchedL2CB(const xAOD::L2CombinedMuon* cbmu, 
                                const xAOD::Muon* mu) const {
  return xAOD::P4Helpers::deltaR(cbmu, mu, false) < m_L2CBreqdR;
}

bool 
MuonMatchingTool::isMatchedL2InsideOut(const xAOD::L2CombinedMuon* cbiomu, 
                                       const xAOD::Muon* mu) const {
  return xAOD::P4Helpers::deltaR(cbiomu, mu, false) < m_L2InsideOutreqdR;
}
  
std::unique_ptr<Trk::TrackParameters> 
MuonMatchingTool::extTrackToPivot(const EventContext& ctx, 
                                  const xAOD::TrackParticle* track) const {
  if (!track) return nullptr;

  double trkEta = track->eta();
  using Return_t = std::unique_ptr<Trk::TrackParameters>;
  
  // BARREL REGION
  if (std::abs(trkEta) < 1.05) {
    Return_t extRPC = extTrackToRPC(ctx, *track);
    if (!extRPC) {
      return extTrackToTGC(ctx, *track);
    }
    //If the extrapolated eta is in the endcap region, return the TGC extrapolation
    if (std::abs(extRPC->position().eta()) >= 1.05){
      if (Return_t extTGC = extTrackToTGC(ctx, *track)) {
        return extTGC;
      }
    }
    //If the extrapolated eta is still in the barrel region, or the TGC extrapolation fails, return the RPC extrapolation
    return extRPC;
  }

  // ENDCAP REGION (same logic as above, but reversed)
  Return_t extTGC = extTrackToTGC(ctx, *track);
  if (!extTGC) {
    return extTrackToRPC(ctx, *track);
  }
  if (std::abs(extTGC->position().eta()) < 1.05) {
    if (Return_t extRPC = extTrackToRPC(ctx, *track)) {
      return extRPC;
    }
  }
  return extTGC;
}

std::unique_ptr<Trk::TrackParameters> 
MuonMatchingTool::extTrackToTGC(const EventContext& ctx, 
                                const xAOD::TrackParticle& trk) const {
  double TGC_Z = ( trk.eta()>0 )? 15153.0:-15153.0;
  Amg::Transform3D matrix = Amg::Transform3D(Amg::Vector3D( 0.,0.,TGC_Z));
  Trk::DiscSurface disc{matrix, 0., 15000.};
  const bool boundaryCheck = true;

  return m_extrapolator->extrapolate(ctx,
                                     trk.perigeeParameters(),
                                     disc,
                                     Trk::anyDirection,
                                     boundaryCheck,
                                     Trk::muon);
}

std::unique_ptr<Trk::TrackParameters> 
MuonMatchingTool::extTrackToRPC(const EventContext& ctx, 
                                const xAOD::TrackParticle& trk) const {
  Trk::CylinderSurface barrel {7478., 15000.};
  const bool boundaryCheck = true;

  return m_extrapolator->extrapolate(ctx,
                                     trk.perigeeParameters(),
                                     barrel,
                                     Trk::anyDirection,
                                     boundaryCheck,
                                     Trk::muon);
}

double 
MuonMatchingTool::reqdRL1byPt(double mupt){
  double dR = 0.08;
  if( mupt < 10000. ) {
    dR = -0.00001*mupt + 0.18;
  } 
  return dR;
}
