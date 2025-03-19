/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local
#include "LeptonTaggers/PrimaryVertexReFitter.h"
#include "LeptonTaggers/PromptUtils.h"

// Athena
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODMuon/Muon.h"
#include "xAODEgamma/Electron.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "GaudiKernel/ThreadLocalContext.h"

namespace Prompt {
//======================================================================================================
PrimaryVertexReFitter::PrimaryVertexReFitter(const std::string& name, ISvcLocator *pSvcLocator):
    AthAlgorithm (name, pSvcLocator)
{}

//=============================================================================
StatusCode PrimaryVertexReFitter::initialize()
{
    ANA_MSG_DEBUG("ReFitPriVtxName = " << m_reFitPrimaryVertexKey);
    ANA_MSG_DEBUG("LeptonContainerName = " << m_leptonContainerKey);
    ANA_MSG_DEBUG("PriVertexContainerName = " << m_primaryVertexContainerKey);

    ANA_MSG_DEBUG("PrintTime = " << m_printTime);

    ANA_MSG_DEBUG("DistToRefittedPriVtxName = " << m_distToRefittedPriVtxName);
    ANA_MSG_DEBUG("NormDistToRefittedPriVtxName = " << m_normDistToRefittedPriVtxName);
    ANA_MSG_DEBUG("RefittedVtxWithoutLeptonLinkName = " << m_lepRefittedVtxWithoutLeptonLinkName.key());

    ATH_CHECK(m_inDetTracksKey.initialize());

    ATH_CHECK(m_leptonContainerKey.initialize());
    ATH_CHECK(m_primaryVertexContainerKey.initialize());

    ATH_CHECK(m_reFitPrimaryVertexKey.initialize());

    //
    // Must have non-empty container name for refitted primary vertex with/without lepton
    //
    if(m_reFitPrimaryVertexKey.empty()) {
        ATH_MSG_FATAL("initialize - SecVtx container invalid name: \"" << m_reFitPrimaryVertexKey << "\"");
        return StatusCode::FAILURE;
    }

    m_distToRefittedPriVtx.emplace (m_distToRefittedPriVtxName);
    m_normdistToRefittedPriVtx.emplace (m_normDistToRefittedPriVtxName);
    ATH_CHECK( m_lepRefittedVtxWithoutLeptonLinkName.initialize() );

    ATH_CHECK(m_vertexFitterTool.retrieve());

    if(m_printTime) {
        //
        // Reset timers
        //
        m_timerAll .Reset();
        m_timerExec.Reset();

        //
        // Start full timer
        //
        m_timerAll.Start();
    }

    return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode PrimaryVertexReFitter::finalize()
{
    if(m_printTime) {
        //
        // Print full time stopwatch
        //
        m_timerAll.Stop();

        ATH_MSG_INFO("Real time: " << m_timerAll.RealTime() << "\t CPU time: " << m_timerAll.CpuTime());
        ATH_MSG_INFO("Execute time: " << PrintResetStopWatch(m_timerExec));
    }

    return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode PrimaryVertexReFitter::execute()
{
    const EventContext& ctx = Gaudi::Hive::currentContext();

    //
    // Start execute timer
    //
    TimerScopeHelper timer(m_timerExec);

    //
    // Find Inner Detector tracks
    //
    SG::ReadHandle<xAOD::TrackParticleContainer> h_inDetTracks(m_inDetTracksKey, ctx);
    if (!h_inDetTracks.isValid()){
        ATH_MSG_FATAL("execute - failed to find the InDetTrackParticles");
        return StatusCode::FAILURE;
    }

    const xAOD::TrackParticleContainer inDetTracks = *h_inDetTracks;

    //
    // Create output vertex collections and record them immediately
    // in StoreGate for memory management
    //
    std::unique_ptr<xAOD::VertexContainer> refitVtxContainer = std::make_unique< xAOD::VertexContainer>();
    std::unique_ptr<xAOD::VertexAuxContainer> refitVtxContainerAux = std::make_unique< xAOD::VertexAuxContainer>();

    refitVtxContainer->setStore(refitVtxContainerAux.get());

    // Take reference BEFORE pointers moved to SG
    xAOD::VertexContainer &refitVtxContainerRef = *refitVtxContainer;

    SG::WriteHandle<xAOD::VertexContainer> h_refitVtxContainer (m_reFitPrimaryVertexKey, ctx);
    ATH_CHECK(h_refitVtxContainer.record(
        std::move(refitVtxContainer), std::move(refitVtxContainerAux)
    ));

    //
    // Retrieve containers from evtStore
    //
    SG::ReadHandle<xAOD::IParticleContainer> h_leptonContainer(m_leptonContainerKey, ctx);
    if (!h_leptonContainer.isValid()){
        ATH_MSG_FATAL("execute - failed to find the lepton container");
        return StatusCode::FAILURE;
    }

    SG::ReadHandle<xAOD::VertexContainer> h_vertices(m_primaryVertexContainerKey, ctx);
    if (!h_vertices.isValid()){
        ATH_MSG_FATAL("execute - failed to find the vertices");
        return StatusCode::FAILURE;
    }

    const xAOD::IParticleContainer leptonContainer = *h_leptonContainer;
    const xAOD::VertexContainer    vertices        = *h_vertices;

    Prompt::FittingInput fittingInput(&inDetTracks, 0, 0);

    for(const xAOD::Vertex *vertex: vertices) {
        if(vertex->vertexType() == 1) {
            fittingInput.priVtx = dynamic_cast<const xAOD::Vertex*>(vertex);
            break;
        }
    }

    if(!fittingInput.priVtx) {
        ATH_MSG_INFO("Failed to find primary vertices - save empty containers");
        return StatusCode::SUCCESS;
    }

    //
    // Collect tracks used for primary vertex fit
    //
    std::vector<const xAOD::TrackParticle *> priVtx_tracks;
    priVtx_tracks.reserve(fittingInput.priVtx->nTrackParticles());

    for(unsigned k = 0; k < fittingInput.priVtx->nTrackParticles(); ++k) {
        const xAOD::TrackParticle *track  = fittingInput.priVtx->trackParticle(k);

        if(track) {
            priVtx_tracks.push_back(track);
        }
    }

    // Refit primary vertex
    std::unique_ptr<xAOD::Vertex> refittedPriVtx = m_vertexFitterTool->fitVertexWithSeed(
        fittingInput, priVtx_tracks,
        fittingInput.priVtx->position(),
        Prompt::kRefittedPriVtx
    );

    if(!refittedPriVtx) {
        ATH_MSG_WARNING("Failed to refit primary vertex - save empty containers");
        return StatusCode::SUCCESS;
    }

    //
    // Save refitted primary vertex for fitting service
    //
    fittingInput.refittedPriVtx = refittedPriVtx.get();

    ATH_MSG_DEBUG("execute --          primary vertex NTrack = " << fittingInput.priVtx         ->nTrackParticles());
    ATH_MSG_DEBUG("execute -- refitted primary vertex NTrack = " << fittingInput.refittedPriVtx->nTrackParticles());

    //
    // Dynamic cast IParticle container to electron or muon container
    //
    ATH_MSG_DEBUG("======================================="
            << "\n\t\t\t  Size of lepton container:    " << leptonContainer.size()
            << "\n-----------------------------------------------------------------");

    decoratorHandElemVtx_t lepRefittedRMVtxLinkDec (m_lepRefittedVtxWithoutLeptonLinkName, ctx);

    for(const xAOD::IParticle *lepton: leptonContainer) {
        const xAOD::TrackParticle *tracklep = 0;
        const xAOD::Electron *elec = dynamic_cast<const xAOD::Electron*>(lepton);
        const xAOD::Muon *muon = dynamic_cast<const xAOD::Muon*>(lepton);

        if(elec) {
            //
            // get GSF track
            //
            const xAOD::TrackParticle *bestmatchedGSFElTrack=elec->trackParticle(0);

            //
            // get origin ID track for later study
            //
            tracklep = xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(bestmatchedGSFElTrack);
        }
        else if(muon) {
            if(muon->inDetTrackParticleLink().isValid()) {
                tracklep = *(muon->inDetTrackParticleLink());
            }
            else {
                ATH_MSG_DEBUG("PrimaryVertexReFitter::execute - skip muon without valid inDetTrackParticleLink()");
                continue;
            }
        }

        if(!tracklep) {
            ATH_MSG_WARNING("PrimaryVertexReFitter::execute - cannot find muon->inDetTrackParticleLink() nor electron->trackParticle()");
            continue;
        }

        decorateLepWithReFitPrimaryVertex(fittingInput, tracklep, lepton, priVtx_tracks, refitVtxContainerRef, lepRefittedRMVtxLinkDec);
    }

    h_refitVtxContainer->push_back(std::move(refittedPriVtx));

    ATH_MSG_DEBUG("SV Vertex container " << m_reFitPrimaryVertexKey << " recorded in store");

    ATH_MSG_DEBUG("execute - all done");
    ATH_MSG_DEBUG("=======================================");

    return StatusCode::SUCCESS;
}

//=============================================================================
bool Prompt::PrimaryVertexReFitter::decorateLepWithReFitPrimaryVertex(
    const FittingInput &input,
    const xAOD::TrackParticle* tracklep,
    const xAOD::IParticle *lep,
    const std::vector<const xAOD::TrackParticle*> &tracks,
    xAOD::VertexContainer &refitVtxContainer,
    decoratorHandElemVtx_t& lepRefittedRMVtxLinkDec)
{
    //
    // Check if the lepton track has been used for primary vertex reconstruction.
    // if true, then remove the lepton track from the input track list, re-fit primary vertex again.
    // Save the ElementLink of the re-fit primary vertex to the lepton
    //
    if(!input.priVtx) {
        ATH_MSG_WARNING("decorateLepWithReFitPrimaryVertex - invalid input primary vertex pointer");
        return false;
    }

    //--------------------------------------------------------
    // Remove the lepton track from the track list
    // get re-fitted non-prompt primary vertex
    //
    std::vector<const xAOD::TrackParticle*> priVtx_tracks_pass;
    bool isRefit = false;

    for(const xAOD::TrackParticle *track: tracks) {
        if(track == tracklep) {
            isRefit = true;
            ATH_MSG_DEBUG("decorateLepWithReFitPrimaryVertex -- lepton has been used, lepton pT =" << tracklep->pt() << ", track pT =" << track->pt());
            continue;
        }

        priVtx_tracks_pass.push_back(track);
    }

    ElementLink<xAOD::VertexContainer> refittedRM_pv_link;

    if(!isRefit) {
        ATH_MSG_DEBUG("decorateLepWithReFitPrimaryVertex -- Skip the primary vertex without lepton track");

        lepRefittedRMVtxLinkDec(*lep) = refittedRM_pv_link;
        return false;
    }

    if(priVtx_tracks_pass.size() < 2) {
        ATH_MSG_DEBUG("decorateLepWithReFitPrimaryVertex -- Skip the primary vertex refitting: N tracks =" << priVtx_tracks_pass.size());

        lepRefittedRMVtxLinkDec(*lep) = refittedRM_pv_link;
        return false;
    }

    // TODO: probably need to fix memory management here
    // but I'm not sure what happens with the ElementLink later
    // so didn't want the vertex to be deleted by accident
    xAOD::Vertex* refittedVtxRMLep = m_vertexFitterTool->fitVertexWithSeed(
        input, priVtx_tracks_pass, input.priVtx->position(),
        Prompt::kRefittedPriVtxWithoutLep
    ).release();

    if(refittedVtxRMLep) {
        //
        // Record vertex with output container
        //
        refitVtxContainer.push_back(std::move(refittedVtxRMLep));

        // TODO: I don't know if this is the correct use of an ElementLink
        //
        // Add refitted non-prompt vertex ElementLink to the lepton
        //
        refittedRM_pv_link.toContainedElement(refitVtxContainer, refittedVtxRMLep);

        ATH_MSG_DEBUG("decorateLepWithReFitPrimaryVertex -- save refitted non-prompt primary vertex with NTrack = " << refittedVtxRMLep->nTrackParticles());

        if(input.refittedPriVtx) {
            (*m_distToRefittedPriVtx)    (*refittedVtxRMLep) = Prompt::getDistance(input.refittedPriVtx->position(), refittedVtxRMLep->position());
            (*m_normdistToRefittedPriVtx)(*refittedVtxRMLep) = Prompt::getNormDist(
                input.refittedPriVtx->position(),
                refittedVtxRMLep->position(),
                refittedVtxRMLep->covariance(),
                msg(MSG::WARNING)
            );
        }
    }

    lepRefittedRMVtxLinkDec(*lep) = refittedRM_pv_link;

    return true;
}
}
