/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///
/// @author Makayla Vessella <makayla.vessella@cern.ch>
///

// Header include

#include "MuSAVtxFitter/MuSAVtxFitterTool.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "GeoPrimitives/GeoPrimitives.h" //Needed for Amg::Vector3D
#include <vector>
#include <cmath>

namespace Rec {

MuSAVtxFitterTool::MuSAVtxFitterTool(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type, name, parent)
{
  declareInterface<MuSAVtxFitterTool>(this);
}

MuSAVtxFitterTool::~MuSAVtxFitterTool() = default;

StatusCode MuSAVtxFitterTool::initialize()
{
    
    ATH_CHECK(m_vertexFitter.retrieve());
    ATH_MSG_DEBUG("Retrieved tool " << m_vertexFitter);

    ATH_CHECK(m_extrapolator.retrieve());
    ATH_MSG_DEBUG("Retrieved tool " << m_extrapolator);


    ATH_MSG_DEBUG("Initialization successful");

    return StatusCode::SUCCESS;

}

std::unique_ptr<xAOD::TrackParticle> MuSAVtxFitterTool::extrapolateMuSA(const xAOD::TrackParticle& trk, const xAOD::EventInfo& eventInfo, const EventContext& ctx) const {
    
    Amg::Vector3D refPos(eventInfo.beamPosX(), eventInfo.beamPosY(), eventInfo.beamPosZ());
    Trk::PerigeeSurface persurf(refPos);
    Trk::PropDirection propagationDirection = Trk::oppositeMomentum;

    auto newTrack = std::make_unique<xAOD::TrackParticle>();
    newTrack->makePrivateStore();

    auto perigee = trk.perigeeParameters();
    auto entryPars = m_extrapolator->extrapolate(ctx, perigee, persurf, propagationDirection, false, Trk::muon);

    if (!entryPars) {
        ATH_MSG_WARNING("Failed to back extrapolate MSTP, returning dummy!");
        // Return a dummy track that will fail downstream cuts (extrapolations can sometimes fail due to various non-pathological reasons -- SA muons are not always high quality objects) 
        newTrack->setDefiningParameters(1.e19, 1.e19, 1.e19, 1.e19, 0);
        return newTrack;
    }

    newTrack->setDefiningParameters(entryPars->parameters()[Trk::d0], entryPars->parameters()[Trk::z0], entryPars->parameters()[Trk::phi], entryPars->parameters()[Trk::theta], entryPars->parameters()[Trk::qOverP]);
    newTrack->setDefiningParametersCovMatrix(*entryPars->covariance());
    newTrack->setParametersOrigin(eventInfo.beamPosX(), eventInfo.beamPosY(), eventInfo.beamPosZ());

    return newTrack;
}

StatusCode MuSAVtxFitterTool::doMuSAVtxFit(std::vector<MuSAVtxFitterTool::WrkVrt>& workVerticesContainer,
                                          const xAOD::MuonContainer& muonContainer,
                                          const xAOD::EventInfo& eventInfo,
                                          const EventContext& ctx) const
{
    ATH_MSG_DEBUG("MuSAVtxFitterTool::doMuSAVtxFit");

    // first gather all SA muons that pass basic checks
    std::vector<const xAOD::Muon*> candidateSAmuons;
    for (const auto muon : muonContainer) {
        bool isSA = (muon->muonType() == xAOD::Muon::MuonStandAlone);
        bool isCalo = (muon->muonType() == xAOD::Muon::CaloTagged);
        bool isSegment = (muon->muonType() == xAOD::Muon::SegmentTagged);
        bool isSiForward = (muon->muonType() == xAOD::Muon::SiliconAssociatedForwardMuon);

        if (!isSA && !m_doValidation) {
            continue; 
        }

        const xAOD::TrackParticle* MuSAMSTP = muon->trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);
        if (!MuSAMSTP) {
            if (isCalo || isSegment || isSiForward) {
                ATH_MSG_VERBOSE("Skipping non-SA, non-Combined muon type in validation mode!");
            } else {
                ATH_MSG_WARNING("Muon has no MSTP, check your input!");
            }
            continue;
        }

        // pre-fit cuts at "seeding" level go here
        if (std::abs(MuSAMSTP->eta()) > m_etaCutMSTP) {
            ATH_MSG_VERBOSE("Skipping SA muon with |eta| > 2.5!");
            continue;
        }
        // SA muons can get saved with nonphysical pT values, checking prevents extrap crashes
        if (MuSAMSTP->pt() > 13000000) {
            ATH_MSG_DEBUG("Skipping SA muon with pT " << (MuSAMSTP->pt() / 1000.) << " GeV!");
            continue;
        }
        // SA muons can also be saved in regions with 0 magnetic field, which can cause extrapolation crashes
        if (std::abs(muon->spectrometerFieldIntegral) < 0.1) {
            ATH_MSG_DEBUG("Skipping SA muon with spectrometerFieldIntegral " << muon->spectrometerFieldIntegral << " T*m!");
            continue;
        }

        candidateSAmuons.push_back(muon);
    }

    // bail out if we don't have at least two valid SA muons
    if (candidateSAmuons.size() < 2) {
        ATH_MSG_VERBOSE("Not enough standalone muons to fit a vertex!");
        return StatusCode::SUCCESS;
    }

    // extrapolate SA muons only if we have enough candidates to form a vertex
    std::vector<std::unique_ptr<xAOD::TrackParticle>> extrapolatedMuSATracks;
    std::map<const xAOD::TrackParticle*, const xAOD::Muon*> muonToExtrapolatedTrackMap;
    for (const auto muon : candidateSAmuons) {
        const xAOD::TrackParticle* MuSAMSTP = muon->trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);
        auto extrapolatedMuSATrack = extrapolateMuSA(*MuSAMSTP, eventInfo, ctx);
        if (extrapolatedMuSATrack->definingParameters()[Trk::d0] > 8000) {
            ATH_MSG_DEBUG("Failed to extrapolate MuSA track, skipping!");
            continue;
        }
        extrapolatedMuSATracks.push_back(std::move(extrapolatedMuSATrack));
        muonToExtrapolatedTrackMap[extrapolatedMuSATracks.back().get()] = muon;
        ATH_MSG_VERBOSE("Extrapolated MuSA track! Total extrapolated so far: " << extrapolatedMuSATracks.size());
    }

    if (extrapolatedMuSATracks.size() < 2) {
        ATH_MSG_VERBOSE("Not enough extrapolated tracks to fit a vertex!");
        return StatusCode::SUCCESS;
    }

    std::unique_ptr<Trk::IVKalState> state = m_vertexFitter->makeState(ctx);
    // Loop over all unique pairs.
    for (unsigned int i = 0; i < extrapolatedMuSATracks.size(); i++) {
        for (unsigned int j = i+1; j < extrapolatedMuSATracks.size(); j++) {
            MuSAVtxFitterTool::WrkVrt MuSACandidate;
            std::vector<const xAOD::TrackParticle*> tracksToFit = {
                extrapolatedMuSATracks[i].get(),
                extrapolatedMuSATracks[j].get()
            };
            std::vector<const xAOD::NeutralParticle*> dummyNeutrals;
            StatusCode res = m_vertexFitter->VKalVrtFit(tracksToFit, dummyNeutrals, 
                                                        MuSACandidate.pos, MuSACandidate.mom, 
                                                        MuSACandidate.charge, MuSACandidate.cov, 
                                                        MuSACandidate.chi2PerTrk, MuSACandidate.trkAtVrt, 
                                                        MuSACandidate.chi2, *state);
            if (res.isSuccess()) {
                ATH_MSG_DEBUG("MuSA vertex fit successful!");
                // Transfer persistent ownership by converting the unique_ptr to a shared_ptr
                // for both tracks used in this candidate (lets unused tracks go out of scope peacefully while keeping used ones)
                auto sharedTrack1 = std::make_shared<const xAOD::TrackParticle>(*extrapolatedMuSATracks[i]);
                auto sharedTrack2 = std::make_shared<const xAOD::TrackParticle>(*extrapolatedMuSATracks[j]);
                MuSACandidate.newExtrapolatedTracks.push_back(sharedTrack1);
                MuSACandidate.newExtrapolatedTracks.push_back(sharedTrack2);
                MuSACandidate.muonCandidates = { 
                    muonToExtrapolatedTrackMap[tracksToFit[0]],
                    muonToExtrapolatedTrackMap[tracksToFit[1]]
                };
                MuSACandidate.minOpAng = muonToExtrapolatedTrackMap[tracksToFit[0]]->p4().DeltaR(
                                          muonToExtrapolatedTrackMap[tracksToFit[1]]->p4());
                workVerticesContainer.emplace_back(MuSACandidate);
            } else {
                ATH_MSG_DEBUG("MuSA vertex fit failed!");
            }
        }
    }
    // NOTE: Any remaining extrapolated tracks in the vector will be cleaned up automatically when they go out of scope
    // this means the logic may need to be revisited if we want to "recover" extrapolated tracks that were not used in any fit through another mechanism 
    return StatusCode::SUCCESS;
}

StatusCode MuSAVtxFitterTool::doPostFitSelections(std::vector<MuSAVtxFitterTool::WrkVrt>& workVerticesContainer) const{
    ATH_MSG_DEBUG("MuSAVtxFitterTool::doPostFitSelections");
    //minimal method for post-fit selections
    for (auto& workVertex : workVerticesContainer) {
        if (workVertex.chi2 > m_baseChi2Cut) {
            ATH_MSG_DEBUG("SA vertex chi2 too high, removing from consideration");
            workVertex.isGood = false; // isGood flag has no specific utility here yet, but may be used in the future for more sophisticated recovery/reassembly 
        }
    }

    //remove any vertices marked as bad
    workVerticesContainer.erase(std::remove_if(workVerticesContainer.begin(), workVerticesContainer.end(), 
        [&](const MuSAVtxFitterTool::WrkVrt& vtx) { return !vtx.isGood; }), workVerticesContainer.end());

    return StatusCode::SUCCESS;
}

StatusCode MuSAVtxFitterTool::selectBestVertices(std::vector<MuSAVtxFitterTool::WrkVrt>& workVerticesContainer) const
{
    ATH_MSG_DEBUG("MuSAVtxFitterTool::selectBestVertices");
    //the default logic will create vertices with all possible combinations of SA muons
    //in rare cases this will cause two MuSA vertices in one event that re-use a muon (more likely in signal models with two or more LLP BSM decays)
    //if this occurs we can select based on the highest chi2 and remove the poorer-quality vertex from consideration

    //first check if a muon is re-used in multiple vertices

    if (workVerticesContainer.size() < 2) {
        ATH_MSG_VERBOSE("Only one MuSA vertex, no need to select best!");
        return StatusCode::SUCCESS;
    }

    //sort all the vertices by chi2 starting with the lowest
    std::sort(workVerticesContainer.begin(), workVerticesContainer.end(), [](const MuSAVtxFitterTool::WrkVrt& v1, const MuSAVtxFitterTool::WrkVrt& v2) { return v1.chi2 < v2.chi2; });
    ATH_MSG_VERBOSE("Sorted vertices by chi2! Lowest chi2: " << workVerticesContainer.front().chi2 << " Highest chi2: " << workVerticesContainer.back().chi2);

    //starting with the lowest chi2 vertex, check its muons and remove any other vertices that use the same muons
    for (unsigned int i = 0; i < workVerticesContainer.size(); i++) {
        auto& vertex = workVerticesContainer.at(i);
        if (!vertex.isGood) {
            continue; // skip vertices we've already marked as bad
        }
        for (unsigned int j = i+1; j < workVerticesContainer.size(); j++) {
            auto& otherVertex = workVerticesContainer.at(j);
            for (const auto& muon : vertex.muonCandidates) {
                if (std::find(otherVertex.muonCandidates.begin(), otherVertex.muonCandidates.end(), muon) != otherVertex.muonCandidates.end()) {
                    //remove the other vertex
                    ATH_MSG_VERBOSE("Found a vertex re-using a muon with chi2: " << otherVertex.chi2 << " removing from consideration!");
                    otherVertex.isGood = false;
                }
            }
        }
    }

    //remove any vertices marked as bad
    workVerticesContainer.erase(std::remove_if(workVerticesContainer.begin(), workVerticesContainer.end(), 
        [&](const MuSAVtxFitterTool::WrkVrt& vtx) { return !vtx.isGood; }), workVerticesContainer.end());

    return StatusCode::SUCCESS;
    
    
}
} // namespace Rec
