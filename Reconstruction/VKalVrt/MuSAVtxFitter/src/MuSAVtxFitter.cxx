/*
Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "MuSAVtxFitter/MuSAVtxFitter.h"
#include "MuSAVtxFitter/MuSAVtxFitterTool.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "StoreGate/WriteDecorHandle.h"
#include "VrtSecInclusive/Constants.h"

namespace {
  // accessors for vertex links
  const SG::AuxElement::Accessor<std::vector<ElementLink<xAOD::TrackParticleContainer>>> acc_MSTPLinks("MuSAVtx_MSTPLinks");
  const SG::AuxElement::Accessor<std::vector<ElementLink<xAOD::MuonContainer>>> acc_MuonLinks("MuSAVtx_MuonLinks");
  const SG::AuxElement::Accessor<ElementLink<xAOD::MuonContainer>> acc_MuonLink("MuSATrk_MuonLink");
  const SG::AuxElement::Accessor<ElementLink<xAOD::TrackParticleContainer>> acc_MSTPLink("MuSATrk_MSTPLink");
  // accessors for vertex properties
  const SG::AuxElement::Accessor<float> vtx_pxAcc("vtx_px");
  const SG::AuxElement::Accessor<float> vtx_pyAcc("vtx_py");
  const SG::AuxElement::Accessor<float> vtx_pzAcc("vtx_pz");
  const SG::AuxElement::Accessor<float> vtx_massAcc("vtx_mass");
  const SG::AuxElement::Accessor<float> vtx_chargeAcc("vtx_charge");
  const SG::AuxElement::Accessor<float> minOpAngAcc("minOpAng");
  const SG::AuxElement::Accessor<float> chi2_coreAcc("chi2_core");
  const SG::AuxElement::Accessor<float> ndof_coreAcc("ndof_core");
  const SG::AuxElement::Accessor<float> chi2_assocAcc("chi2_assoc");
  const SG::AuxElement::Accessor<float> ndof_assocAcc("ndof_assoc");
  const SG::AuxElement::Accessor<float> massAcc("mass");
  const SG::AuxElement::Accessor<float> mass_eAcc("mass_e");
  const SG::AuxElement::Accessor<float> mass_selectedTracksAcc("mass_selectedTracks");
  const SG::AuxElement::Accessor<int>   num_trksAcc("num_trks");
  const SG::AuxElement::Accessor<int>   num_selectedTracksAcc("num_selectedTracks");
  const SG::AuxElement::Accessor<int>   num_associatedTracksAcc("num_associatedTracks");
  const SG::AuxElement::Accessor<float> dCloseVrtAcc("dCloseVrt");
  // accessors for track parameters wrt vertex
  const SG::AuxElement::Accessor<float> qOverP_wrtSVAcc("qOverP_wrtSV");
  const SG::AuxElement::Accessor<float> theta_wrtSVAcc("theta_wrtSV");
  const SG::AuxElement::Accessor<float> p_wrtSVAcc("p_wrtSV");
  const SG::AuxElement::Accessor<float> pt_wrtSVAcc("pt_wrtSV");
  const SG::AuxElement::Accessor<float> eta_wrtSVAcc("eta_wrtSV");
  const SG::AuxElement::Accessor<float> phi_wrtSVAcc("phi_wrtSV");
  const SG::AuxElement::Accessor<float> d0_wrtSVAcc("d0_wrtSV");
  const SG::AuxElement::Accessor<float> z0_wrtSVAcc("z0_wrtSV");
  const SG::AuxElement::Accessor<float> sqrd0Err_wrtSVAcc("sqrd0Err_wrtSV");
  const SG::AuxElement::Accessor<float> sqrz0Err_wrtSVAcc("sqrz0Err_wrtSV");
  const SG::AuxElement::Accessor<float> sqrQoPErr_wrtSVAcc("sqrQoPErr_wrtSV");
}

namespace Rec {

MuSAVtxFitter::~MuSAVtxFitter() = default;

StatusCode MuSAVtxFitter::initialize()
{
    ATH_CHECK( m_muonContainer.initialize() );
    ATH_CHECK( m_MSTPContainer.initialize() );
    ATH_CHECK( m_eventInfo.initialize() );
    ATH_CHECK( m_MuSAVertices.initialize() );
    ATH_CHECK( m_MuSAExtrapolatedTracks.initialize() );

    ATH_CHECK( m_MuSAVtxFitterTool.retrieve() );
    ATH_MSG_DEBUG("Retrieved tool " << m_MuSAVtxFitterTool);

    ATH_CHECK( m_trackToVertexTool.retrieve() );
    ATH_MSG_DEBUG("Retrieved tool " << m_trackToVertexTool);

    ATH_MSG_DEBUG("Initialization successful");

    return StatusCode::SUCCESS;

}

StatusCode MuSAVtxFitter::fillCollections(std::vector<MuSAVtxFitterTool::WrkVrt>& workVerticesContainer,
                                     xAOD::VertexContainer* MuSAVtxContainer,
                                     xAOD::TrackParticleContainer* MuSAExtrapolatedTracksContainer,
                                     const xAOD::MuonContainer& muonContainer,
                                     const xAOD::TrackParticleContainer& MSTPContainer,
                                     const EventContext& ctx) const
{
    ATH_MSG_DEBUG("MuSAVtxFitter::fillCollections");
    for (const auto& workVertex : workVerticesContainer) {

        xAOD::Vertex* MuSAVertex = new xAOD::Vertex();
        MuSAVtxContainer->emplace_back(MuSAVertex);

        // Place the new extrapolated tracks in the new container (we need the index for bookkeeping)
        for (size_t i = 0; i < workVertex.newExtrapolatedTracks.size(); i++) {

            const xAOD::TrackParticle* newTrack = workVertex.newExtrapolatedTracks[i].get();
            // copy transfers ownership of the shared_ptr in wrkvrt to container
            xAOD::TrackParticle*  containerTrack  =  MuSAExtrapolatedTracksContainer->push_back(new xAOD::TrackParticle());
            *containerTrack=*newTrack;

            // Link the new tracks to the vertex
            ElementLink<xAOD::TrackParticleContainer> link_trk(*MuSAExtrapolatedTracksContainer, MuSAExtrapolatedTracksContainer->size() - 1);
            MuSAVertex->addTrackAtVertex(link_trk, 1.0);

            //Also link the extrapolated tracks and vertices to their original muons -- extrapolated tracks and their muons should have the same index in workVertex

            bool muonLinkSuccess = false;
            bool MSTPLinkSuccess = false;

            ElementLink<xAOD::MuonContainer> link_muon;
            ElementLink<xAOD::TrackParticleContainer> link_MSTP;
            muonLinkSuccess = link_muon.toContainedElement(muonContainer, workVertex.muonCandidates[i]);

            const xAOD::Muon* muon = muonContainer[link_muon.index()];
            const xAOD::TrackParticle* MSTP = muon->trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);
            if (MSTP) {
                MSTPLinkSuccess = link_MSTP.toContainedElement(MSTPContainer, MSTP);
            }

            // Check that links are valid
            if (!MSTPLinkSuccess) {
                ATH_MSG_WARNING("Failed to link MSTP!");
            } else {
                acc_MSTPLink(*containerTrack) = link_MSTP;
                auto& mstpLinks = acc_MSTPLinks(*MuSAVertex);
                mstpLinks.push_back(link_MSTP);
            }

            if (!muonLinkSuccess) {
                ATH_MSG_WARNING("Failed to link muon!");
            } else {
                acc_MuonLink(*containerTrack) = link_muon;
                auto& muonLinks = acc_MuonLinks(*MuSAVertex);
                muonLinks.push_back(link_muon);
            }
        }

        MuSAVertex->setPosition(workVertex.pos);
        MuSAVertex->setVertexType(xAOD::VxType::SecVtx);
        MuSAVertex->setFitQuality(workVertex.chi2, 1);
        std::vector<float> fCov(workVertex.cov.cbegin(), workVertex.cov.cend());
        MuSAVertex->setCovariance(fCov);

        vtx_pxAcc(*MuSAVertex) = workVertex.mom.Px();
        vtx_pyAcc(*MuSAVertex) = workVertex.mom.Py();
        vtx_pzAcc(*MuSAVertex) = workVertex.mom.Pz();

        vtx_massAcc(*MuSAVertex) = workVertex.mom.M();
        vtx_chargeAcc(*MuSAVertex) = workVertex.charge;

        minOpAngAcc(*MuSAVertex) = workVertex.minOpAng;

        chi2_coreAcc(*MuSAVertex) = workVertex.chi2Core;
        ndof_coreAcc(*MuSAVertex) = 1;
        chi2_assocAcc(*MuSAVertex) = workVertex.chi2;
        ndof_assocAcc(*MuSAVertex) = workVertex.ndof();

        TLorentzVector sumP4_muon;
        TLorentzVector sumP4_electron;
        TLorentzVector sumP4_selected;

        constexpr double muonMass = 105.658; // Muon mass in MeV

        for (size_t i = 0; i < workVertex.newExtrapolatedTracks.size(); i++) {
            const xAOD::TrackParticle* track = workVertex.newExtrapolatedTracks[i].get();
            double pt_wrtSV = track->pt();
            double eta_wrtSV = track->eta();
            double phi_wrtSV = track->phi();

            TLorentzVector p4wrtSV_muon;
            TLorentzVector p4wrtSV_electron;

            p4wrtSV_muon.SetPtEtaPhiM(pt_wrtSV, eta_wrtSV, phi_wrtSV, muonMass);
            p4wrtSV_electron.SetPtEtaPhiM(pt_wrtSV, eta_wrtSV, phi_wrtSV, VKalVrtAthena::PhysConsts::mass_electron);

            sumP4_muon += p4wrtSV_muon;
            sumP4_electron += p4wrtSV_electron;
            sumP4_selected += p4wrtSV_muon;

            size_t trackIndex = MuSAVertex->trackParticleLinks()[i].index();
            xAOD::TrackParticle* containerTrack = MuSAExtrapolatedTracksContainer->at(trackIndex);

            std::unique_ptr<Trk::Perigee> sv_perigee = m_trackToVertexTool->perigeeAtVertex(ctx, *containerTrack, workVertex.pos);
            if (!sv_perigee) {
                ATH_MSG_WARNING("Failed in obtaining the SV perigee for track!");
                continue;
            }

            double qOverP_wrtSV = sv_perigee->parameters()[Trk::qOverP];
            double theta_wrtSV = sv_perigee->parameters()[Trk::theta];
            double p_wrtSV = 1.0 / std::abs(qOverP_wrtSV);
            pt_wrtSV = p_wrtSV * sin(theta_wrtSV);
            eta_wrtSV = -log(tan(theta_wrtSV / 2.0));
            phi_wrtSV = sv_perigee->parameters()[Trk::phi];
            double d0_wrtSV = sv_perigee->parameters()[Trk::d0];
            double z0_wrtSV = sv_perigee->parameters()[Trk::z0];
            double sqrd0Err_wrtSV = (*sv_perigee->covariance())(Trk::d0, Trk::d0);
            double sqrz0Err_wrtSV = (*sv_perigee->covariance())(Trk::z0, Trk::z0);
            double sqrQoPErr_wrtSV = (*sv_perigee->covariance())(Trk::qOverP, Trk::qOverP);

            qOverP_wrtSVAcc(*containerTrack) = qOverP_wrtSV;
            theta_wrtSVAcc(*containerTrack) = theta_wrtSV;
            p_wrtSVAcc(*containerTrack) = p_wrtSV;
            pt_wrtSVAcc(*containerTrack) = pt_wrtSV;
            eta_wrtSVAcc(*containerTrack) = eta_wrtSV;
            phi_wrtSVAcc(*containerTrack) = phi_wrtSV;
            d0_wrtSVAcc(*containerTrack) = d0_wrtSV;
            z0_wrtSVAcc(*containerTrack) = z0_wrtSV;
            sqrd0Err_wrtSVAcc(*containerTrack) = sqrd0Err_wrtSV;
            sqrz0Err_wrtSVAcc(*containerTrack) = sqrz0Err_wrtSV;
            sqrQoPErr_wrtSVAcc(*containerTrack) = sqrQoPErr_wrtSV;
        }

        massAcc(*MuSAVertex) = sumP4_muon.M();
        mass_eAcc(*MuSAVertex) = sumP4_electron.M();
        mass_selectedTracksAcc(*MuSAVertex) = sumP4_selected.M();
        num_trksAcc(*MuSAVertex) = workVertex.nTracksTotal();
        num_selectedTracksAcc(*MuSAVertex) = workVertex.selectedTrackIndices.size();
        num_associatedTracksAcc(*MuSAVertex) = workVertex.associatedTrackIndices.size();
        dCloseVrtAcc(*MuSAVertex) = workVertex.closestWrkVrtValue;
    }
    return StatusCode::SUCCESS;
}

StatusCode MuSAVtxFitter::execute(const EventContext& ctx) const
{
    SG::ReadHandle muonContainer(m_muonContainer, ctx);
    ATH_CHECK(muonContainer.isValid());

    SG::ReadHandle MSTPContainer(m_MSTPContainer, ctx);
    ATH_CHECK(MSTPContainer.isValid());

    SG::ReadHandle eventInfo(m_eventInfo, ctx);
    ATH_CHECK(eventInfo.isValid());

    SG::WriteHandle MuSAVtxContainer(m_MuSAVertices, ctx);
    ATH_CHECK(MuSAVtxContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));

    SG::WriteHandle MuSAExtrapolatedTracksContainer(m_MuSAExtrapolatedTracks, ctx);
    ATH_CHECK(MuSAExtrapolatedTracksContainer.record(std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>()));

    std::vector<MuSAVtxFitterTool::WrkVrt> workVerticesContainer;

    // Perform primary MuSA vertex fit
    ATH_CHECK(m_MuSAVtxFitterTool->doMuSAVtxFit(workVerticesContainer, *muonContainer, *eventInfo, ctx));

    // If there's no MuSA vertices to begin with, stop here to avoid wasting CPU cycles
    if (workVerticesContainer.empty()) {
        ATH_MSG_DEBUG("No MuSA vertices found");
        return StatusCode::SUCCESS;
    }

    // Perform user-configurable post-fit selections
    ATH_CHECK(m_MuSAVtxFitterTool->doPostFitSelections(workVerticesContainer));

    // If there's no MuSA vertices remaining, stop here to again avoid wasting CPU cycles
    if (workVerticesContainer.empty()) {
        ATH_MSG_DEBUG("No MuSA vertices found after post-fit selection!");
        return StatusCode::SUCCESS;
    }

    // Perform "ambiguity resolution" to prevent re-used SA muons in events with more than one MuSA vtx
    ATH_CHECK(m_MuSAVtxFitterTool->selectBestVertices(workVerticesContainer));

    // Again avoiding wasting CPU cycles if no MuSA vertices remain
    if (workVerticesContainer.empty()) {
        ATH_MSG_DEBUG("No MuSA vertices found after best overlap selection!");
        return StatusCode::SUCCESS;
    } else {
        ATH_MSG_DEBUG("Found " << workVerticesContainer.size() << " MuSA vertices");
    }

    ATH_CHECK(fillCollections(workVerticesContainer, MuSAVtxContainer.ptr(), MuSAExtrapolatedTracksContainer.ptr(), *muonContainer, *MSTPContainer, ctx));
    ATH_MSG_DEBUG("Saved MuSA collections!");

    return StatusCode::SUCCESS;
}
}
