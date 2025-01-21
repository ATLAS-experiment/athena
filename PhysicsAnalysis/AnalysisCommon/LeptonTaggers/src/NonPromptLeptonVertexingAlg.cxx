/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local
#include "LeptonTaggers/NonPromptLeptonVertexingAlg.h"
#include "LeptonTaggers/PromptUtils.h"

// Athena
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/VertexAuxContainer.h"

// C/C++
#include <cmath>
#include <iostream>
#include <sstream>

using namespace std;

//======================================================================================================
Prompt::NonPromptLeptonVertexingAlg::NonPromptLeptonVertexingAlg(const std::string& name, ISvcLocator *pSvcLocator):
  AthAlgorithm     (name, pSvcLocator),
  m_countEvents    (0)
{}

//=============================================================================
StatusCode Prompt::NonPromptLeptonVertexingAlg::initialize()
{
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

  if(m_svContainerName.empty()) {
    ATH_MSG_ERROR("NonPromptLeptonVertexingAlg::initialize - empty SV container name: \"" << m_svContainerName << "\"");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_vertexMerger.retrieve());
  ATH_CHECK(m_vertexFitterTool.retrieve());

  ATH_CHECK(m_inDetTracksKey.initialize());
  ATH_CHECK(m_leptonContainerKey.initialize());
  ATH_CHECK(m_primaryVertexContainerName.initialize());
  ATH_CHECK(m_refittedPriVtxContainerName.initialize());

  ATH_CHECK(m_svContainerName.initialize());

  m_indexVectorDec           = std::make_unique<decoratorVecInt_t>    (m_decoratorNameIndexVector);
  m_indexVectorDecDeepMerge  = std::make_unique<decoratorVecInt_t>    (m_decoratorNameIndexVector+"DeepMerge");

  m_lepSVElementLinksDec           = std::make_unique<decoratorVecElemVtx_t>(m_decoratorNameSecVtxLinks);
  m_lepDeepMergedSVElementLinksDec = std::make_unique<decoratorVecElemVtx_t>(m_decoratorNameDeepMergedSecVtxLinks);

  ATH_MSG_DEBUG("LeptonContainerName      = " << m_leptonContainerKey);
  ATH_MSG_DEBUG("ReFitPriVtxContainerName = " << m_refittedPriVtxContainerName);
  ATH_MSG_DEBUG("SVContainerName          = " << m_svContainerName);
  ATH_MSG_DEBUG("IndexVectorName          = " << m_decoratorNameIndexVector);

  ATH_MSG_DEBUG("mergeMinVtxDist       = " << m_mergeMinVtxDist);
  ATH_MSG_DEBUG("mergeChi2OverDoF      = " << m_mergeChi2OverDoF);

  ATH_MSG_DEBUG("minTrackLeptonDR      = " << m_minTrackLeptonDR);
  ATH_MSG_DEBUG("maxTrackLeptonDR      = " << m_maxTrackLeptonDR);

  ATH_MSG_DEBUG("selectTracks          = " << m_selectTracks);
  ATH_MSG_DEBUG("minTrackpT            = " << m_minTrackpT);
  ATH_MSG_DEBUG("maxTrackEta           = " << m_maxTrackEta);
  ATH_MSG_DEBUG("maxTrackZ0Sin         = " << m_maxTrackZ0Sin);

  ATH_MSG_DEBUG("minTrackSiHits       = " << m_minTrackSiHits);
  ATH_MSG_DEBUG("maxTrackSharedSiHits = " << m_maxTrackSharedSiHits);
  ATH_MSG_DEBUG("maxTrackSiHoles      = " << m_maxTrackSiHoles);
  ATH_MSG_DEBUG("maxTrackPixHoles      = " << m_maxTrackPixHoles);

  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode Prompt::NonPromptLeptonVertexingAlg::finalize()
{
  if(m_printTime) {
    //
    // Print full time stopwatch
    //
    m_timerAll.Stop();

    ATH_MSG_INFO("NonPromptLeptonVertexingAlg - total time:   " << PrintResetStopWatch(m_timerAll));
    ATH_MSG_INFO("NonPromptLeptonVertexingAlg - execute time: " << PrintResetStopWatch(m_timerExec));
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode Prompt::NonPromptLeptonVertexingAlg::execute()
{
  //
  // Start execute timer for new event
  //
  TimerScopeHelper timer(m_timerExec);

  m_countEvents++;

  //
  // Find Inner Detector tracks save them class member variable for convenience.
  //
  SG::ReadHandle<xAOD::TrackParticleContainer> h_inDetTracks(m_inDetTracksKey);
  if (!h_inDetTracks.isValid()){
    ATH_MSG_FATAL("execute - failed to find the InDetTrackParticles");
    return StatusCode::FAILURE;
  }

  const xAOD::TrackParticleContainer inDetTracks = *h_inDetTracks;

  //
  // Create vertex containers and record them in StoreGate
  //
  std::set< xAOD::Vertex* > svSet;

  SG::WriteHandle<xAOD::VertexContainer> h_SVContainer (m_svContainerName);
  ATH_CHECK(h_SVContainer.record(
    std::make_unique< xAOD::VertexContainer>(), std::make_unique< xAOD::VertexAuxContainer>()
  ));
  xAOD::VertexContainer &SVContainerRef = *(h_SVContainer.ptr());

  //
  // Retrieve containers from evtStore
  //
  SG::ReadHandle<xAOD::IParticleContainer> leptonContainer (m_leptonContainerKey);
  SG::ReadHandle<xAOD::VertexContainer> vertices (m_primaryVertexContainerName);
  SG::ReadHandle<xAOD::VertexContainer> refittedVertices(m_refittedPriVtxContainerName);

  ATH_MSG_DEBUG ("NonPromptLeptonVertexingAlg::execute - Read " << vertices->size()          << " primary vertices");
  ATH_MSG_DEBUG ("NonPromptLeptonVertexingAlg::execute - Read " << refittedVertices->size()  << " refitted primary vertices");

  //
  // Find default Primary Vertex
  //
  Prompt::FittingInput fittingInput(&inDetTracks, 0, 0);

  for(const xAOD::Vertex *vertex: *vertices) {
    if(vertex->vertexType() == xAOD::VxType::PriVtx) {
      fittingInput.priVtx = vertex;
      break;
    }
  }

  if(!fittingInput.priVtx) {
    ATH_MSG_INFO("Failed to find primary vertex - skip this event");

    return StatusCode::SUCCESS;
  }

  //
  // Find the refitted Primary Vertex
  //
  for(const xAOD::Vertex *vertex: *refittedVertices) {
    short refittedVertexType = 0;

    if(getVar(vertex, refittedVertexType, m_refittedVertexTypeName) && refittedVertexType == xAOD::VxType::PriVtx) {
      fittingInput.refittedPriVtx = vertex;
    }

    if(fittingInput.refittedPriVtx) {
      break;
    }
  }

  //
  // Dynamic cast IParticle container to electron or muon container
  //
  ATH_MSG_DEBUG("\n\t\t\t  Size of lepton container:  " << leptonContainer ->size());

  SG::AuxElement::ConstAccessor<ElementLink<xAOD::VertexContainer> > priVtxWithoutLepAcc(m_linkNameRefittedPriVtxWithoutLepton);

  for(const xAOD::IParticle *lepton: *leptonContainer) {
    const xAOD::TrackParticle *tracklep = 0;
    const xAOD::Electron      *elec     = dynamic_cast<const xAOD::Electron*>(lepton);
    const xAOD::Muon          *muon     = dynamic_cast<const xAOD::Muon    *>(lepton);

    if(elec) {
      //
      // Get GSF track
      //
      const xAOD::TrackParticle *bestmatchedGSFElTrack = elec->trackParticle(0);

      //
      // Get original ID track for vertex fitting
      //
      if(passElecCand(*elec) && bestmatchedGSFElTrack) {
        tracklep = xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(bestmatchedGSFElTrack);
      }
    }
    else if(muon) {
      if(passMuonCand(*muon) && muon->inDetTrackParticleLink().isValid()) {
        tracklep = *(muon->inDetTrackParticleLink());
      }
    }
    else {
      ATH_MSG_WARNING("NonPromptLeptonVertexingAlg::execute - failed to find electron or muon: should never happen!");
    }

    if(!tracklep) {
      (*m_lepSVElementLinksDec)          (*lepton) = std::vector<ElementLink<xAOD::VertexContainer> >();
      (*m_lepDeepMergedSVElementLinksDec)(*lepton) = std::vector<ElementLink<xAOD::VertexContainer> >();
      (*m_indexVectorDec)                (*lepton) = std::vector<int>();
      (*m_indexVectorDecDeepMerge)       (*lepton) = std::vector<int>();

      ATH_MSG_DEBUG("NonPromptLeptonVertexingAlg::execute - cannot find muon->inDetTrackParticleLink() nor electron->trackParticle()");
      continue;
    }

    ATH_MSG_DEBUG("NonPromptLeptonVertexingAlg::execute - process new lepton track " << tracklep);

    //
    // Find refitted primary vertex with lepton track excluded
    //
    fittingInput.refittedPriVtxWithoutLep = 0;

    if(priVtxWithoutLepAcc.isAvailable(*lepton)) {
      ElementLink<xAOD::VertexContainer> vtxLink = priVtxWithoutLepAcc(*lepton);

      if(vtxLink.isValid()) {
        fittingInput.refittedPriVtxWithoutLep = *vtxLink;

        ATH_MSG_DEBUG("DecorateSecondaryVertex - found refitted primary vertex without lepton: "
          << m_linkNameRefittedPriVtxWithoutLepton << " with Ntrack =" << fittingInput.refittedPriVtxWithoutLep->nTrackParticles());
      }
    }

    //
    // Collect tracks around the lepton track
    //
    std::vector<const xAOD::TrackParticle* > ifitTracks = findNearbyTracks(*tracklep, inDetTracks, *fittingInput.priVtx);

    //
    // Fit 2-track vertices
    //
    std::vector<std::unique_ptr<xAOD::Vertex>> twoTrkVertices = prepLepWithTwoTrkSVVec(
      fittingInput, tracklep, ifitTracks
    );

    // We make a copy so we can store the list of original
    // two-track vertices
    std::vector<std::unique_ptr<xAOD::Vertex>> twoTrkVerticesCopy;
    for (std::unique_ptr<xAOD::Vertex> &vtx : twoTrkVertices) {
      std::unique_ptr<xAOD::Vertex> newVtx = std::make_unique<xAOD::Vertex>(*vtx);
      twoTrkVerticesCopy.push_back(std::move(newVtx));
    }

    //  Deep merge 2-track vertices.
    ATH_MSG_DEBUG("Getting deep merged vertices");
    ATH_MSG_DEBUG("Starting with " << twoTrkVertices.size() << " 2-track vertices");
    Prompt::MergeResultNotOwner deep_merged_result = m_vertexMerger->mergeInitVertices(
        fittingInput, tracklep, twoTrkVertices, ifitTracks);

    //
    // Save secondary vertices
    //
    std::vector<ElementLink<xAOD::VertexContainer> > svLinks;
    std::vector<ElementLink<xAOD::VertexContainer> > deepmergeSVLinks;

    std::vector<int> indexVectorTwoTrk;
    std::vector<int> indexVectorDeepMerged;

    //
    // Record 2-track vertexes and simple merged vertexes
    //
    ATH_MSG_DEBUG("NonPromptLeptonVertexingAlg::execute --- recording " << twoTrkVerticesCopy.size() << " 2-track and simple merged vertices");
    saveSecondaryVertices(twoTrkVerticesCopy, indexVectorTwoTrk, svLinks, SVContainerRef, svSet);

    //
    // Record both merged multi-track vertices and also unmerged 2-track vertices
    //
    ATH_MSG_DEBUG("NonPromptLeptonVertexingAlg::execute --- recording " << deep_merged_result.vtxsNewMerged.size() << " merged multi-track vertices");
    saveSecondaryVertices(deep_merged_result.vtxsNewMerged, indexVectorDeepMerged, deepmergeSVLinks, SVContainerRef, svSet);

    ATH_MSG_DEBUG("NonPromptLeptonVertexingAlg::execute --- recording " << deep_merged_result.vtxsInitPassedNotMerged.size() << " unmerged 2-track vertices");
    saveSecondaryVertices(deep_merged_result.vtxsInitPassedNotMerged, indexVectorDeepMerged, deepmergeSVLinks, SVContainerRef, svSet);

    ATH_MSG_DEBUG ("NonPromptLeptonVertexingAlg::execute -- number of two-track   SV = " << twoTrkVertices.size());
    ATH_MSG_DEBUG ("NonPromptLeptonVertexingAlg::execute -- number of deep merged SV = " << deep_merged_result.vtxsNewMerged.size());

    (*m_lepSVElementLinksDec)          (*lepton) = svLinks;
    (*m_lepDeepMergedSVElementLinksDec)(*lepton) = deepmergeSVLinks;
    (*m_indexVectorDec)                (*lepton) = indexVectorTwoTrk;
    (*m_indexVectorDecDeepMerge)       (*lepton) = indexVectorDeepMerged;

    ATH_MSG_DEBUG("NonPromptLeptonVertexingAlg - done with lepton pT=" << tracklep->pt() << ", " << truthAsStr(*lepton) << endl
      << "___________________________________________________________________________");
  }

  ATH_MSG_DEBUG("SV Vertex container " << m_svContainerName << " recorded in store");

  ATH_MSG_DEBUG(" NonPromptLeptonVertexingAlg::execute - done with this event" << endl
    << "___________________________________________________________________________");

  return StatusCode::SUCCESS;
}

//=============================================================================
bool Prompt::NonPromptLeptonVertexingAlg::passElecCand(const xAOD::Electron &elec) const
{
  //
  // Check whether electron candidate passes loose selection
  //
  char lh_loose  = -1;

  Prompt::GetAuxVar(elec, lh_loose,  "DFCommonElectronsLHLoose");

  ATH_MSG_DEBUG("NonPromptLeptonVertexingAlg::passElecCand - "
    << "pT=" << elec.pt() << ", eta=" << elec.eta() << ", phi=" << elec.phi() << std::endl
    << "   DFCommonElectronsLHLoose  = " << int(lh_loose)  << std::endl
    << "   " << truthAsStr(elec));

  if(!lh_loose) {
    return false;
  }

  return true;
}

//=============================================================================
bool Prompt::NonPromptLeptonVertexingAlg::passMuonCand(const xAOD::Muon &muon) const
{
  //
  // Check whether muon candidate is a combined muon
  //
  const bool combined = (muon.muonType() == xAOD::Muon::Combined);

  ATH_MSG_DEBUG("NonPromptLeptonVertexingAlg::passMuonCand - "
    << "pT=" << muon.pt() << ", eta=" << muon.eta() << ", phi=" << muon.phi() << std::endl
    << "   Type     = " << muon.muonType() << std::endl
    << "   Combined = " << combined        << std::endl
    << "   " << truthAsStr(muon));

  return combined;
}

//=============================================================================
std::vector<const xAOD::TrackParticle*> Prompt::NonPromptLeptonVertexingAlg::findNearbyTracks(
  const xAOD::TrackParticle &tracklep,
  const xAOD::TrackParticleContainer &inDetTracks,
  const xAOD::Vertex &priVtx
) const {
  //
  // Select tracks -- avoid using track selection tool since z0 definition is different
  //
  std::vector<const xAOD::TrackParticle *> mytracks;

  for(const xAOD::TrackParticle *track: inDetTracks) {
    if(!track) {
      ATH_MSG_WARNING("skip null track pointer - should never happen");
      continue;
    }

    //
    // Check minimum track and lepton DR: skip the track that is probably the lepton track
    //
    if(tracklep.p4().DeltaR(track->p4()) < m_minTrackLeptonDR) {
      ATH_MSG_DEBUG("skip the track very close to the lepton ");
      continue;
    }

    //
    // Check track and lepton maximum DR
    //
    if(tracklep.p4().DeltaR(track->p4()) > m_maxTrackLeptonDR) {
      continue;
    }

    const double delta_z0 = track->z0() + track->vz() - priVtx.z();
    const double Z0Sin    = std::abs(delta_z0*std::sin(track->theta()));
    const double abs_eta  = std::abs(track->eta());

    uint8_t numberOfPixelHits       = 0;
    uint8_t numberOfSCTHits         = 0;
    uint8_t numberOfPixelHoles      = 0;
    uint8_t numberOfSCTHoles        = 0;
    uint8_t numberOfPixelSharedHits = 0;
    uint8_t numberOfSCTSharedHits   = 0;

    if(!(track->summaryValue(numberOfPixelHits,       xAOD::numberOfPixelHits)))       continue;
    if(!(track->summaryValue(numberOfSCTHits,         xAOD::numberOfSCTHits)))         continue;
    if(!(track->summaryValue(numberOfPixelHoles,      xAOD::numberOfPixelHoles)))      continue;
    if(!(track->summaryValue(numberOfSCTHoles,        xAOD::numberOfSCTHoles)))        continue;
    if(!(track->summaryValue(numberOfPixelSharedHits, xAOD::numberOfPixelSharedHits))) continue;
    if(!(track->summaryValue(numberOfSCTSharedHits,   xAOD::numberOfSCTSharedHits)))   continue;

    const uint8_t NSiHits   = numberOfPixelHits  + numberOfSCTHits;
    const uint8_t NSiHoles  = numberOfPixelHoles + numberOfSCTHoles;
    const float   NSiShHits = float(numberOfPixelSharedHits) + float(numberOfSCTSharedHits)/2.0;

    if(m_selectTracks) {
      //
      // Kinematic track selection
      //
      if(track->pt()                    < m_minTrackpT)    continue;
      if(abs_eta                        > m_maxTrackEta)   continue;
      if(Z0Sin                          > m_maxTrackZ0Sin) continue;

      //
      // Hit quality track selection
      //
      if(NSiHits                        < m_minTrackSiHits)       continue;
      if(NSiShHits                      > m_maxTrackSharedSiHits) continue;
      if(NSiHoles                       > m_maxTrackSiHoles  )    continue;
      if(numberOfPixelHoles             > m_maxTrackPixHoles  )    continue;
    }

    mytracks.push_back(track);
  }

  return mytracks;
}

//=============================================================================
std::vector<std::unique_ptr<xAOD::Vertex>> Prompt::NonPromptLeptonVertexingAlg::prepLepWithTwoTrkSVVec(
  const FittingInput &input,
  const xAOD::TrackParticle* tracklep,
  const std::vector<const xAOD::TrackParticle*> &tracks
)
{
  //
  // Decorate lepton with vector of two-track vertices.
  // Return vector of finding vertices
  //
  std::vector<std::unique_ptr<xAOD::Vertex>> twoTrkVertices;
  std::vector<const xAOD::TrackParticle*> tracksForFit;

  if(!input.priVtx) {
    ATH_MSG_WARNING("prepLepWithTwoTrkSVVec -- invalid primary vertex: nothing to do");
    return twoTrkVertices;
  }

  for(const xAOD::TrackParticle *selectedtrack: tracks) {
    tracksForFit.clear();
    tracksForFit.push_back(tracklep);
    tracksForFit.push_back(selectedtrack);

    std::unique_ptr<xAOD::Vertex> newSecondaryVertex = m_vertexFitterTool->fitVertexWithPrimarySeed(
      input, tracksForFit, kTwoTrackVtx
    );

    if(!newSecondaryVertex) {
      ATH_MSG_DEBUG("prepLepWithTwoTrkSVVec -- failed to fit 2-track vertex");
      continue;
    }

    twoTrkVertices.push_back(std::move(newSecondaryVertex));
  }

  return twoTrkVertices;
}

//=============================================================================
std::vector<std::unique_ptr<xAOD::Vertex>> Prompt::NonPromptLeptonVertexingAlg::prepLepWithMergedSVVec(
  const FittingInput &input,
  const xAOD::TrackParticle* tracklep,
  std::vector<std::unique_ptr<xAOD::Vertex>> &twoTrkVertices
)
{
  //
  // Merge the two vertices if the distance between them with in 0.5 mm.
  // Re-fit a three-track vertex using the input tracks from the vertices above.
  //
  std::vector<std::unique_ptr<xAOD::Vertex>> twoTrkVerticesPass;
  std::vector<std::unique_ptr<xAOD::Vertex>> resultVertices;

  if(!input.priVtx) {
    ATH_MSG_WARNING("prepLepWithMergedSVVec -- invalid primary vertex: nothing to do");
    return resultVertices;
  }

  for(std::unique_ptr<xAOD::Vertex> &vtx: twoTrkVertices) {
    double chi2OverDoF = -99.;

    if(vtx->numberDoF() > 0 && vtx->chiSquared() > 0) {
      chi2OverDoF = vtx->chiSquared()/double(vtx->numberDoF());
    }

    if(chi2OverDoF >= 0.0 && chi2OverDoF < m_mergeChi2OverDoF) {
      twoTrkVerticesPass.push_back(std::move(vtx));
    }
  }

  std::vector<std::unique_ptr<xAOD::Vertex>>::iterator curr_iter = twoTrkVerticesPass.begin();

  /*
    Cluster the vertices which pass the chi2 selection.

    1. Cluster the vertices.
    2. For each cluster, extract tracks and fit a new secondary vertex.
    3. Push the new secondary vertex into resultVertices.

    Note that every vertex in twoTrkVerticesPass is removed from twoTrkVerticesPass during this function.
  */
  while(curr_iter != twoTrkVerticesPass.end()) {
    std::vector<std::unique_ptr<xAOD::Vertex>> clusterVtxs;
    clusterVtxs.push_back(std::move(*curr_iter));

    twoTrkVerticesPass.erase(curr_iter);

    //
    // Find a new vertex cluster, and remove all vertices in the
    // cluster from twoTrkVerticesPass
    //
    makeVertexCluster(clusterVtxs, twoTrkVerticesPass);

    curr_iter = twoTrkVerticesPass.begin();

    //
    // Fit vertex cluster
    //
    std::vector<const xAOD::TrackParticle*> tracksForFit;

    for(std::unique_ptr<xAOD::Vertex> &vtx: clusterVtxs) {
      for(unsigned k = 0; k < vtx->nTrackParticles(); ++k) {
        const xAOD::TrackParticle *track  = vtx->trackParticle(k);

        if(track) {
          tracksForFit.push_back(track);
        }
      }
    }

    //
    // Push standalone vertices into result
    //
    if(clusterVtxs.size() < 1) {
      continue;
    } else if(clusterVtxs.size() < 2) {
      resultVertices.push_back(std::move(clusterVtxs.at(0)));
      continue;
    }

    //
    // Fit merged vertex
    //
    tracksForFit.push_back(tracklep);

    std::unique_ptr<xAOD::Vertex> newSecondaryVertex = m_vertexFitterTool->fitVertexWithPrimarySeed(
      input, tracksForFit, kSimpleMergedVtx
    );

    if(!newSecondaryVertex) {
      ATH_MSG_DEBUG("DecorateLepWithMergedSVVec -- failed to fit merged vertex");
      continue;
    }

    ATH_MSG_DEBUG("DecorateLepWithMergedSVVec -- NTrack of merged vertex = " << newSecondaryVertex->nTrackParticles());
    resultVertices.push_back(std::move(newSecondaryVertex));
  }

  return resultVertices;
}

//=============================================================================
void Prompt::NonPromptLeptonVertexingAlg::makeVertexCluster(
  std::vector<std::unique_ptr<xAOD::Vertex>> &clusterVtxs,
  std::vector<std::unique_ptr<xAOD::Vertex>> &inputVtxs
)
{
  ATH_MSG_DEBUG("makeVertexCluster - before: clusterVtxs.size()=" << clusterVtxs.size() << ", inputVtxs.size()=" << inputVtxs.size());

  std::vector<std::unique_ptr<xAOD::Vertex>>::iterator vit = inputVtxs.begin();

  while(vit != inputVtxs.end()) {
    bool pass = false;

    for(std::vector<std::unique_ptr<xAOD::Vertex>>::const_iterator cit = clusterVtxs.begin(); cit != clusterVtxs.end(); ++cit) {
      if(vit->get() == cit->get()) {
        ATH_MSG_DEBUG("makeVertexCluster - logic error - found the same vertex twice: " << ((*vit).get()));
        continue;
      }

      const double vdist = getDistance((*vit)->position(), (*cit)->position());

      ATH_MSG_DEBUG("makeVertexCluster - vdist=" << vdist );

      if(vdist < m_mergeMinVtxDist) {
        pass = true;
        break;
      }
    }

    if(pass) {
      clusterVtxs.push_back(std::move(*vit));
      inputVtxs.erase(vit);

      vit = inputVtxs.begin();
    }
    else {
      ++vit;
    }
  }

  ATH_MSG_DEBUG("makeVertexCluster - after:  clusterVtxs.size()=" << clusterVtxs.size() << ", inputVtxs.size()=" << inputVtxs.size());
}

//=============================================================================
void Prompt::NonPromptLeptonVertexingAlg::saveSecondaryVertices(
  std::vector<std::unique_ptr<xAOD::Vertex>> &vtxs,
  std::vector<int> &indexVector,
  std::vector<ElementLink<xAOD::VertexContainer> > &svLinks,
  xAOD::VertexContainer &SVContainer,
  std::set< xAOD::Vertex* >& svSet
)
{
  //
  // Record created xAOD::Vertex in output vertex container
  //
  ATH_MSG_DEBUG("saveSecondaryVertices - will save " << vtxs.size() << " vertexes");

  for(std::unique_ptr<xAOD::Vertex> &vtx: vtxs) {
    int index = -99;
    if(getVar(vtx, index, "SecondaryVertexIndex")) {
      indexVector.push_back(index);
    }
    else {
      ATH_MSG_WARNING("saveSecondaryVertices - missing \"SecondaryVertexIndex\" variable");
    }

    if(svSet.insert(vtx.get()).second) {
      //
      // First time seeing this this vertex - record it in output container
      //
      SVContainer.push_back(std::move(vtx));
      ElementLink<xAOD::VertexContainer> svLink(SVContainer,SVContainer.size()-1);
      svLinks.push_back(svLink);
    } else {
      ATH_MSG_ERROR("saveSecondaryVertices --- the same vertex has been encountered more than once! Is this a logic error?");
    }
  }

  ATH_MSG_DEBUG("saveSecondaryVertices - all done");
}


