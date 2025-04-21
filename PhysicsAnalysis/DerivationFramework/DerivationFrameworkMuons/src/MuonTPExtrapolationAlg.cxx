/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTPExtrapolationAlg.h"
#include "DerivationFrameworkMuons/Utils.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "xAODEventInfo/EventInfo.h"
namespace {
    constexpr float min_warn_pt = 3500;
    constexpr float dummy_result = 5.;

    enum ExtStatus {
        Success = 1,
        Failed = 2,
        NotPresent = 0,
    };
}  // namespace

namespace DerivationFramework{
MuonTPExtrapolationAlg::MuonTPExtrapolationAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonTPExtrapolationAlg::initialize() {
    ATH_CHECK(m_extrapolator.retrieve());
    ATH_CHECK(m_partKey.initialize());
 
    for (const std::string& selDecor : m_trkSelDecors) {
        m_trkSelKeys.emplace_back(m_partKey, selDecor);
    }
    ATH_CHECK(m_trkSelKeys.initialize());
    ATH_CHECK(m_extEtaKey.initialize());
    ATH_CHECK(m_extPhiKey.initialize());
    ATH_CHECK(m_extStatKey.initialize()); 
    return StatusCode::SUCCESS;
}

StatusCode MuonTPExtrapolationAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::IParticleContainer> muons{m_partKey, ctx};
    if (!muons.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve " << m_partKey.fullKey());
        return StatusCode::FAILURE;
    }
    
    auto dec_Eta = makeHandle<float>(ctx, m_extEtaKey, dummy_result);
    auto dec_Phi = makeHandle<float>(ctx, m_extPhiKey, dummy_result);
    auto dec_Decorated = makeHandle<char>(ctx, m_extStatKey, ExtStatus::NotPresent);
    
    using SelDecorator = SG::ReadDecorHandle<xAOD::IParticleContainer, bool>;
    
    std::vector<SelDecorator> selDecors;
    for (const SG::ReadDecorHandleKey<xAOD::IParticleContainer>& key : m_trkSelKeys) {
        selDecors.emplace_back(key, ctx);
    }

    for (const xAOD::IParticle* muon : *muons) {
       /// Track particle conversion
       const xAOD::TrackParticle* track{nullptr};
       if (muon->type() == xAOD::Type::ObjectType::TruthParticle) {
            ATH_MSG_FATAL("Truth is not supported");
            return StatusCode::FAILURE;
       } else if (muon->type() == xAOD::Type::ObjectType::TrackParticle) {
          track = static_cast<const xAOD::TrackParticle*>(muon);
       } else if (muon->type() == xAOD::Type::ObjectType::Muon) {
            const xAOD::Muon* probeMuon = static_cast<const xAOD::Muon*>(muon);
            track = probeMuon->trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);
            if (!track) { track = probeMuon->primaryTrackParticle(); }
       }
       bool passSelection = muon->pt() > m_ptMin && (selDecors.empty () || 
                                                     std::find_if(selDecors.begin(),selDecors.end(),
                                                     [&muon](const SelDecorator& dec){
                                                        return dec(*muon);
                                                     }) != selDecors.end());
       std::unique_ptr<Trk::TrackParameters> pTag = passSelection && track ? extrapolateToTriggerPivotPlane(ctx, *track) : nullptr;
       int extr_code = ExtStatus::NotPresent;
       float eta{dummy_result}, phi{dummy_result};
       if (!pTag) {
            // complain only if the particle has sufficient pt to actually make it to the MS...
            if (passSelection && muon->pt() > min_warn_pt)
                ATH_MSG_WARNING("Warning - Pivot plane extrapolation failed for a track particle with IP pt "
                                << muon->pt() << ", eta " << muon->eta() << ", phi " << muon->phi());
            extr_code = ExtStatus::Failed;
        } else {
            eta = pTag->position().eta();
            phi = pTag->position().phi();
            extr_code = ExtStatus::Success;
        }
        dec_Eta(*muon) = eta;
        dec_Phi(*muon) = phi;
        dec_Decorated(*muon) = extr_code;
    }
    return StatusCode::SUCCESS;
}

std::unique_ptr<Trk::TrackParameters> MuonTPExtrapolationAlg::extrapolateToTriggerPivotPlane(const EventContext& ctx,
                                                                                             const xAOD::TrackParticle& track) const {
    // BARREL
    const Trk::Perigee& perigee = track.perigeeParameters();

    // create the barrel as a cylinder surface centered at 0,0,0
    Amg::Transform3D matrix = Amg::Transform3D(Amg::RotationMatrix3D::Identity(), Amg::Vector3D::Zero());

    std::unique_ptr<Trk::CylinderSurface> cylinder =
        std::make_unique<Trk::CylinderSurface>(matrix, m_barrelPivotPlaneRadius, m_barrelPivotPlaneHalfLength);

    // and then attempt to extrapolate our track to this surface, checking for the boundaries of the barrel
    bool boundaryCheck = true;

    std::unique_ptr<Trk::TrackParameters> p{
        m_extrapolator->extrapolate(ctx, perigee, *cylinder, Trk::alongMomentum, boundaryCheck, Trk::muon)};

    // if the extrapolation worked out (so we are in the barrel) we are done and can return the
    // track parameters at this surface.
    if (p) return p;

    // if we get here, the muon did not cross the barrel surface
    // so we assume it is going into the endcap.
    // ENDCAP

    // After 2 years of using this code, we realised that ATLAS actually has endcaps on both sides ;-)
    // So better make sure we place our endcap at the correct side of the detector!
    // Hopefully no-one will ever read this comment...
    const int SignOfEta = track.eta() > 0 ? 1. : -1.;
    // much better!
    matrix = Amg::Transform3D(Amg::RotationMatrix3D::Identity(), SignOfEta * m_endcapPivotPlaneZ * Amg::Vector3D::UnitZ());
    std::unique_ptr<Trk::DiscSurface> disc = std::make_unique<Trk::DiscSurface>(matrix, m_endcapPivotPlaneMinimumRadius, 
                                                                                m_endcapPivotPlaneMaximumRadius);

    boundaryCheck = false;
    return m_extrapolator->extrapolate(ctx, perigee, *disc, Trk::alongMomentum, boundaryCheck, Trk::muon);
}

}