/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


// Local
#include "LeptonTaggers/VertexFittingTool.h"
#include "LeptonTaggers/PromptUtils.h"

// Athena
#include "xAODBTagging/SecVtxHelper.h"

// C/C++
#include <cmath>
#include <iostream>
#include <sstream>

using namespace std;

//=============================================================================
Prompt::VertexFittingTool::VertexFittingTool(
  const std::string& t, const std::string &name, const IInterface* p
): AthAlgTool(t, name, p)
{
  declareInterface<Prompt::IVertexFittingTool>(this);
}

//=============================================================================
StatusCode Prompt::VertexFittingTool::initialize()
{
  ATH_CHECK(m_vertexFitter.retrieve());

  if(m_doSeedVertexFit) {
    ATH_CHECK(m_seedVertexFitter.retrieve());
  }

  m_distToPriVtx                   = std::make_unique<SG::AuxElement::Decorator<float> > (m_distToPriVtxName);
  m_normDistToPriVtx               = std::make_unique<SG::AuxElement::Decorator<float> > (m_normDistToPriVtxName);
  m_distToRefittedPriVtx           = std::make_unique<SG::AuxElement::Decorator<float> > (m_distToRefittedPriVtxName);
  m_normDistToRefittedPriVtx       = std::make_unique<SG::AuxElement::Decorator<float> > (m_normDistToRefittedPriVtxName);
  m_distToRefittedRmLepPriVtx      = std::make_unique<SG::AuxElement::Decorator<float> > (m_distToRefittedRmLepPriVtxName);
  m_normDistToRefittedRmLepPriVtx  = std::make_unique<SG::AuxElement::Decorator<float> > (m_normDistToRefittedRmLepPriVtxName);

  m_timer.Reset();

  ATH_MSG_DEBUG("VertexFittingSvc::initialize - doSeedVertexFit = " << m_doSeedVertexFit);
  ATH_MSG_DEBUG("VertexFittingSvc::initialize - vertexFitter    = " << m_vertexFitter.name());

  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode Prompt::VertexFittingTool::finalize()
{
  //
  // Delete pointers
  //
  m_timer.Stop();

  ATH_MSG_INFO("VertexFittingSvc::finalize - number of total   fits: " << m_countNumberOfFits);
  ATH_MSG_INFO("VertexFittingSvc::finalize - number of failed  fits: " << m_countNumberOfFitsFailed);
  ATH_MSG_INFO("VertexFittingSvc::finalize - number of invalid vtxs: " << m_countNumberOfFitsInvalid);
  ATH_MSG_INFO("VertexFittingSvc::finalize - fitting timer: " << PrintResetStopWatch(m_timer));

  return StatusCode::SUCCESS;
}

//=============================================================================
std::unique_ptr<xAOD::Vertex> Prompt::VertexFittingTool::fitVertexWithPrimarySeed(
  const FittingInput &input,
  const std::vector<const xAOD::TrackParticle* > &tracks,
  VtxType vtxType
)
{
  //
  // Make new secondary vertex
  //
  TimerScopeHelper timer(m_timer);
  m_countNumberOfFits++;

  if(!input.priVtx) {
    ATH_MSG_WARNING("fitVertexWithPrimarySeed -- missing primary vertex");
    return nullptr;
  }

  // Fit a new secondary vertex
  std::unique_ptr<xAOD::Vertex> secondaryVtx = getSecondaryVertexWithSeed(
    tracks, input.inDetTracks, input.priVtx->position()
  );

  if(!secondaryVtx) {
    m_countNumberOfFitsFailed++;
    ATH_MSG_WARNING("fitVertexWithPrimarySeed -- failed to fit vertex");
    return nullptr;
  }

  if(!isValidVertex(secondaryVtx.get())) {
    m_countNumberOfFitsInvalid++;

    ATH_MSG_WARNING("fitVertexWithPrimarySeed -- failed to get valid vertex");
    return nullptr;
  }

  m_secondaryVertexIndex++;

  // Decorate the newly created vertex
  static const SG::AuxElement::Accessor<int> indexAcc("SecondaryVertexIndex");
  static const SG::AuxElement::Accessor<int> typeAcc("SVType");

  indexAcc(*secondaryVtx) = m_secondaryVertexIndex;
  typeAcc(*secondaryVtx) = static_cast<int>(vtxType);

  decorateNewSecondaryVertex(input, secondaryVtx.get());

  return secondaryVtx;
}

//=============================================================================
std::unique_ptr<xAOD::Vertex> Prompt::VertexFittingTool::fitVertexWithSeed(
  const FittingInput &input,
  const std::vector<const xAOD::TrackParticle* > &tracks,
  const Amg::Vector3D& seed,
  VtxType vtxType
)
{
  //
  // Make new secondary vertex
  //
  TimerScopeHelper timer(m_timer);
  m_countNumberOfFits++;

  std::unique_ptr<xAOD::Vertex> secondaryVtx = getSecondaryVertexWithSeed(
    tracks, input.inDetTracks, seed
  );

  if(!secondaryVtx) {
    m_countNumberOfFitsFailed++;

    ATH_MSG_WARNING("fitVertexWithSeed -- failed to fit vertex");
    return nullptr;
  }

  if(!isValidVertex(secondaryVtx.get())) {
    m_countNumberOfFitsInvalid++;

    ATH_MSG_WARNING("fitVertexWithSeed -- failed to get valid vertex");
    return nullptr;
  }

  m_secondaryVertexIndex++;

  // Decorate the newly created vertex
  static const SG::AuxElement::Accessor<int> indexAcc("SecondaryVertexIndex");
  static const SG::AuxElement::Accessor<int> typeAcc("SVType");

  indexAcc(*secondaryVtx) = m_secondaryVertexIndex;
  typeAcc(*secondaryVtx) = static_cast<int>(vtxType);

  decorateNewSecondaryVertex(input, secondaryVtx.get());

  return secondaryVtx;
}

//=============================================================================
bool Prompt::VertexFittingTool::isValidVertex(const xAOD::Vertex *vtx) const
{
  //
  // Check if vertex is valid
  //
  bool bad_vtx = false;

  if(!vtx){
    ATH_MSG_WARNING("VertexFittingSvc::Validate_Vertex -- invalid vtx pointer!!!");
    return false;
  }

  if(vtx->covariance().empty()) {
    bad_vtx = true;
    ATH_MSG_WARNING("VertexFittingSvc::Validate_Vertex -- empty vtx covariance!!!");
  }

  float chisquared = -9999;

  if(!getVar(vtx, chisquared, "chiSquared")) {
    bad_vtx = true;
    ATH_MSG_WARNING("VertexFittingSvc::Validate_Vertex -- not valid vtx chiSquared!!!");
  }

  float numberdof  = -9999;

  if(!getVar(vtx, numberdof, "numberDoF")) {
    bad_vtx = true;
    ATH_MSG_WARNING("VertexFittingSvc::Validate_Vertex -- not valid vtx numberDoF!!!");
  }

  if(std::isnan(vtx->x()) || std::isnan(vtx->y()) || std::isnan(vtx->z())) {
    bad_vtx = true;
    ATH_MSG_WARNING("VertexFittingSvc::Validate_Vertex -- vertex coordinate is nan");
  }

  if(bad_vtx) {
    ATH_MSG_WARNING("VertexFittingSvc::Validate_Vertex -- bad vertex!!!");
    ATH_MSG_INFO(printPromptVertexAsStr(vtx, msg(MSG::WARNING)));
  }

  return !bad_vtx;
}

//=============================================================================
void Prompt::VertexFittingTool::removeDoubleEntries(std::vector<const xAOD::TrackParticle*>& tracks)
{
  const unsigned nbefore = tracks.size();

  sort(tracks.begin(), tracks.end());

  typename std::vector<const xAOD::TrackParticle*>::iterator TransfEnd = std::unique(tracks.begin(), tracks.end());

  tracks.erase(TransfEnd, tracks.end());

  if(nbefore != tracks.size()) {
    ATH_MSG_DEBUG("removeDoubleEntries nbefore != tracks.size()): " << nbefore << " != " << tracks.size());
  }
}

//=============================================================================
bool Prompt::VertexFittingTool::decorateNewSecondaryVertex(
  const FittingInput &input,
  xAOD::Vertex *secVtx
)
{
  //
  // Decorate secondary vertex with all useful information
  //
  if(!secVtx) {
    ATH_MSG_WARNING("decorateNewSecondaryVertex - invalid pointer");
    return false;
  }

  //
  // Decorate secondary vertex with the distance/norm_distance it from Prmary Vertex, Refitted Primary vertex and Refitted Primary Vertex that removed lepton itself.
  //
  float distToPriVtx                  = -1;
  float normDistToPriVtx              = -1;
  float distToRefittedPriVtx          = -1;
  float normDistToRefittedPriVtx      = -1;
  float distToRefittedRmLepPriVtx     = -1;
  float normDistToRefittedRmLepPriVtx = -1;

  if(input.priVtx) {
    distToPriVtx     = Prompt::getDistance(input.priVtx->position(), secVtx->position());
    normDistToPriVtx = Prompt::getNormDist(input.priVtx->position(), secVtx->position(), secVtx->covariance(), msg(MSG::WARNING));
  }

  if(input.refittedPriVtx) {
    distToRefittedPriVtx     = Prompt::getDistance(input.refittedPriVtx->position(), secVtx->position());
    normDistToRefittedPriVtx = Prompt::getNormDist(input.refittedPriVtx->position(), secVtx->position(), secVtx->covariance(), msg(MSG::WARNING));
  }

  if(input.refittedPriVtxWithoutLep) {
    distToRefittedRmLepPriVtx     = Prompt::getDistance(input.refittedPriVtxWithoutLep->position(), secVtx->position());
    normDistToRefittedRmLepPriVtx = Prompt::getNormDist(input.refittedPriVtxWithoutLep->position(), secVtx->position(), secVtx->covariance(), msg(MSG::WARNING));
  }

  (*m_distToPriVtx)                 (*secVtx) = distToPriVtx;
  (*m_normDistToPriVtx)             (*secVtx) = normDistToPriVtx;
  (*m_distToRefittedPriVtx)         (*secVtx) = distToRefittedPriVtx;
  (*m_normDistToRefittedPriVtx)     (*secVtx) = normDistToRefittedPriVtx;
  (*m_distToRefittedRmLepPriVtx)    (*secVtx) = distToRefittedRmLepPriVtx;
  (*m_normDistToRefittedRmLepPriVtx)(*secVtx) = normDistToRefittedRmLepPriVtx;

  return true;
}

//=============================================================================
std::unique_ptr<xAOD::Vertex> Prompt::VertexFittingTool::getSecondaryVertexWithSeed(
  const std::vector<const xAOD::TrackParticle*> &tracks,
  const xAOD::TrackParticleContainer *inDetTracks,
  const Amg::Vector3D& seed
)
{
  //
  // Fit one vertex with given tracks
  //
  std::vector<const xAOD::TrackParticle*> tracksForFit(tracks);

  ATH_MSG_DEBUG("getSecondaryVertexWithSeed -- before remove " << tracksForFit.size());

  removeDoubleEntries(tracksForFit);

  ATH_MSG_DEBUG("getSecondaryVertexWithSeed -- after remove " << tracksForFit.size());

  if(tracksForFit.size() < 2) {
    ATH_MSG_WARNING("getSecondaryVertexWithSeed -- cannot fit vertex with one or zero input track: ntrack=" << tracksForFit.size());
    return 0;
  }

  //
  // Run fit
  //
  ATH_MSG_DEBUG(name() << "::getSecondaryVertexWithSeed -- N tracks = " << tracksForFit.size());

  for(const xAOD::TrackParticle* track: tracksForFit) {
    ATH_MSG_DEBUG( name() << "::getSecondaryVertexWithSeed -- track pt, eta = " << track->pt() << "," << track->eta());
    ATH_MSG_DEBUG( name() << "::getSecondaryVertexWithSeed -- track chi2    = " << track->chiSquared());
  }

  xAOD::Vertex *newVertex = 0;
  std::unique_ptr<xAOD::Vertex> seedVertex;

  if(m_doSeedVertexFit) {
    seedVertex = std::unique_ptr<xAOD::Vertex>(m_seedVertexFitter->fit(tracksForFit, seed));

    if(seedVertex.get() && !isValidVertex(seedVertex.get())) {
      ATH_MSG_DEBUG("getSecondaryVertexWithSeed -- failed to fit seed vertex");

      seedVertex.reset();
    }
  }

  if(seedVertex.get()) {
    newVertex = m_vertexFitter->fit(tracksForFit, seedVertex->position());
  }
  else {
    newVertex = m_vertexFitter->fit(tracksForFit, seed);
  }

  if(!newVertex) {
    ATH_MSG_INFO("getSecondaryVertexWithSeed -- failed to fit vertex and fitter returned null xAOD::Vertex pointer");
    return 0;
  }

  //
  // Save vertex tracks
  //
  std::vector<ElementLink< xAOD::TrackParticleContainer> > tpLinks;

  if(inDetTracks) {
    //
    // Record links to ID tracks if container pointer is provided
    //
    for(const xAOD::TrackParticle *selectedtrack: tracksForFit) {
      ElementLink<xAOD::TrackParticleContainer> tpLink;

      tpLink.toContainedElement(*inDetTracks, selectedtrack);
      tpLinks.push_back(tpLink);
    }
  }

  newVertex->setTrackParticleLinks(tpLinks);

  TLorentzVector Momentum;

  for(const xAOD::TrackParticle* track: tracksForFit) {
    Momentum += static_cast<TLorentzVector>(track->p4());
  }

  xAOD::SecVtxHelper::setVertexMass(newVertex, Momentum.M()); // "mass"

  // newVertex was returned from the vertex fitter as a raw pointer
  // It looks like Trk::IVertexFitter::fit function expects
  // memory management to be done by the caller.
  // Therefore, we take ownership by casting to a unique_ptr

  std::unique_ptr<xAOD::Vertex> returnPtr(newVertex);

  return returnPtr;
}

