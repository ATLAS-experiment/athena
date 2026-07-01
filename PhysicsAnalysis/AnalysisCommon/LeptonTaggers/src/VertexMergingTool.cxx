/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// C/C++
#include <cmath>
#include <iostream>
#include <sstream>

// Local
#include "LeptonTaggers/VertexMergingTool.h"
#include "LeptonTaggers/VarHolder.h"
#include "LeptonTaggers/PromptUtils.h"

using namespace std;

//=============================================================================
Prompt::VertexMergingTool::VertexMergingTool(const std::string &name,
                                             const std::string &type,
                                             const IInterface  *parent):
  AthAlgTool       (name, type, parent)
{
  declareInterface<Prompt::IVertexMergingTool>(this);
}

//=============================================================================
StatusCode Prompt::VertexMergingTool::initialize()
{
  ATH_CHECK(m_vertexFitterTool.retrieve());

  return StatusCode::SUCCESS;
}

//=============================================================================
Prompt::MergeResultNotOwner Prompt::VertexMergingTool::mergeInitVertices(
  const EventContext& ctx,
  const FittingInput &input,
  const xAOD::TrackParticle *tracklep,
  std::vector<std::unique_ptr<xAOD::Vertex>> &init_vtxs,
  const std::vector<const xAOD::TrackParticle *> &selected_tracks
)
{
  //
  // Merge initial (2-track) vertices into new merged vertices with three tracks or more
  //
  MergeResultNotOwner result;

  for(std::unique_ptr<xAOD::Vertex> &vtx: init_vtxs) {
    if(passVertexSelection(vtx.get())) {
      result.vtxsInitPassed.push_back(vtx.get());
    }
  }

  ATH_MSG_DEBUG(name() << "::mergeInitVertices - start processing" << endl
                << "   number of initial  2-track vertices: " << init_vtxs.size()               << endl
                << "   number of selected 2-track vertices: " << result.vtxsInitPassed.size() << endl
                << "   number of selected ID tracks:        " << selected_tracks.size() );

  const unsigned nvtx_init = init_vtxs.size();
  const unsigned nvtx_pass = result.vtxsInitPassed.size();

  //
  // Make vertex clusters
  //
  std::vector<std::unique_ptr<VtxCluster>> clusters;
  std::vector<std::unique_ptr<VtxCluster>> clusters_cand, clusters_1vtx;

  makeClusters(clusters, result.vtxsInitPassed);

  for(std::unique_ptr<VtxCluster> &cluster: clusters) {
    ATH_MSG_DEBUG("   cluster candidate with nvertex=" << cluster->vtxsInit.size());

    if(cluster->vtxsInit.size() < 1) {
      ATH_MSG_DEBUG("VertexMergingTool::mergeInitVertices - logic error: cluster with zero vertexes!!");
    }
    else if(cluster->vtxsInit.size() == 1) {
      clusters_1vtx.push_back(std::move(cluster));
    }
    else {
      clusters_cand.push_back(std::move(cluster));
    }
  }

  ATH_MSG_DEBUG("   # init vertexes:   " << nvtx_init            << endl
            << "   # pass vertexes:   " << nvtx_pass            << endl
            << "   cluster_1vtx size: " << clusters_1vtx.size() << endl
            << "   cluster_cand size: " << clusters_cand.size() );

  //
  // Found zero clusters with two or more vertices - nothing nore to do
  //
  if(clusters_cand.empty()) {
    return result;
  }

  for(std::unique_ptr<VtxCluster> &cluster: clusters_cand) {
    //
    // Initiliase ID tracks
    //
    cluster->trksCurr = cluster->trksInit;

    //
    // Remove lepton tracks from list of tracks for fitting - the lepton track is added by VertexFittingSvc
    //
    cluster->trksCurr.erase(std::remove(cluster->trksCurr.begin(), cluster->trksCurr.end(), tracklep), cluster->trksCurr.end());

    ATH_MSG_DEBUG("Cluster vtxsInit size=" << cluster->vtxsInit.size() << endl
        << "        trksInit size=" << cluster->trksInit.size() << endl
        << "        trksCurr size=" << cluster->trksCurr.size() );

    for(const xAOD::Vertex *vtx: cluster->vtxsInit) {
      ATH_MSG_DEBUG("   init vtx:  " << vtxAsStr(vtx, true));
    }
    for(const xAOD::TrackParticle *trk: cluster->trksInit) {
      ATH_MSG_DEBUG("   init trk:  " << trkAsStr(trk));
    }
    for(const xAOD::TrackParticle *trk: cluster->trksCurr) {
      ATH_MSG_DEBUG("   curr trk:  " << trkAsStr(trk));
    }

    if(cluster->trksCurr.size() != cluster->vtxsInit.size()) {
      ATH_MSG_WARNING("mergeInitVertices - input vertices are not all 2-track: nvtx != ntrk: " << cluster->trksCurr.size() << "!=" << cluster->vtxsInit.size());
    }
  }

  ATH_MSG_DEBUG("Process " << clusters_cand.size() << " candidate clusters");

  for(std::unique_ptr<VtxCluster> &cluster: clusters_cand) {
    //
    // Fit cluster of vertices to obtain one merged vertex
    //
    fitVertexCluster(ctx, input, tracklep, *cluster);

    if(cluster->vtxMerged) {
      result.vtxsNewMerged.push_back(std::move(cluster->vtxMerged));
    }
    else {
      ATH_MSG_INFO("FAILED TO MERGE VERTEX");
    }

    cluster->vtxsFittedBad.clear();
  }

  ATH_MSG_DEBUG(name() << "::mergeInitVertices - result size=" << result.vtxsNewMerged.size());

  return result;
}

//=============================================================================
bool Prompt::VertexMergingTool::passVertexSelection(const xAOD::Vertex *vtx) const
{
  //
  // Check whether vertex passes quality cuts
  //
  if(!vtx) {
    ATH_MSG_WARNING("passVertexSelection - input vertex is null pointer");
    return false;
  }

  if(!(vtx->numberDoF() > 0 && vtx->chiSquared() > 0)) {
    return false;
  }

  const double fit_prob = Prompt::getVertexFitProb(vtx);

  ATH_MSG_DEBUG("passVertexSelection - vertex pointer=" << vtx << " chi2/ndof=" << vtx->chiSquared() << "/" << vtx->numberDoF() << ", prob=" << fit_prob);

  return fit_prob > m_minFitProb;
}

//=============================================================================
bool Prompt::VertexMergingTool::makeClusters(
  std::vector<std::unique_ptr<VtxCluster>> &clusters,
  std::vector<xAOD::Vertex*> &init_vtxs
)
{
  //
  // Make clusters from initial vertexes
  //
  std::vector<xAOD::Vertex*>::iterator icurr_vtx = init_vtxs.begin();

  //
  // Seed initial cluster with the first vertex on the list
  //
  while(icurr_vtx != init_vtxs.end()) {
    bool match_curr = false;

    //
    // First check whether this vertex can be included with existing clusters
    //
    for(std::unique_ptr<VtxCluster> &cluster: clusters) {
      if(matchVtxToCluster(*cluster, *icurr_vtx)) {
        addInitVtxToCluster(*cluster, *icurr_vtx);
        match_curr = true;
        break;
      }
    }


    if(!match_curr) {
      //
      // Start new cluster with current vertex
      //
      clusters.push_back(std::make_unique<VtxCluster>());
      addInitVtxToCluster(*(clusters.back()), std::move(*icurr_vtx));
    }

    //
    // Erase current vertex and start at the beginning
    //
    init_vtxs.erase(icurr_vtx);
    icurr_vtx = init_vtxs.begin();
  }

  return !clusters.empty();
}

//=============================================================================
bool Prompt::VertexMergingTool::matchVtxToCluster(const VtxCluster &cluster, const xAOD::Vertex *vtx) const
{
  //
  // Add vertex to cluster - always add vertex to empty cluster
  //
  if(!vtx) {
    ATH_MSG_WARNING("matchVtxToCluster - input vertex is null pointer");
    return false;
  }

  //
  // Empty cluster does not match any vertex
  //
  bool match_vtx = false;

  for(const xAOD::Vertex *cluster_vtx: cluster.vtxsInit) {
    double dist = Prompt::getDistance(cluster_vtx, vtx);

    if(m_useMinNormDist) { dist = getMinNormDistVtx  (cluster_vtx, vtx); }
    else                 { dist = Prompt::getDistance(cluster_vtx, vtx); }

    if(dist < m_minDistanceClusterVtx) {
      match_vtx = true;
      break;
    }
  }

  return match_vtx;
}

//=============================================================================
bool Prompt::VertexMergingTool::addInitVtxToCluster(
  VtxCluster &cluster, xAOD::Vertex* vtx
) const
{
  //
  // Add vertex to cluster - always add vertex to empty cluster
  //
  if(!vtx) {
    ATH_MSG_WARNING("AddVtxToCluster - input vertex is null pointer");
    return false;
  }

  if(vtx->nTrackParticles() != 2) {
    ATH_MSG_WARNING("AddVtxToCluster - wrong number of tracks: " << vtx->nTrackParticles());
  }

  for(unsigned k = 0; k < vtx->nTrackParticles(); ++k) {
    const xAOD::TrackParticle *track = vtx->trackParticle(k);

    if(track) {
      cluster.trksInit.push_back(track);
    }
    else {
      ATH_MSG_WARNING("passVertexSelection - vertex contains TrackParticle null pointer");
    }
  }

  cluster.vtxsInit.push_back(vtx);

  return true;
}

//=============================================================================
bool Prompt::VertexMergingTool::fitVertexCluster(
  const EventContext& ctx,
  const FittingInput &input,
  const xAOD::TrackParticle *tracklep,
  VtxCluster &cluster
)
{
  //
  // Fit recursively merged vertex until:
  //  -- good quality vertex is obtained OR;
  //  -- number of tracks is less than 3.
  //
  if(cluster.trksCurr.size() < 2) {
    ATH_MSG_WARNING("fitVertexCluster - number of input tracks is " << cluster.trksCurr.size() << " - nothing to do");
    return false;
  }

  ATH_MSG_DEBUG("fitVertexCluster - trksCurr.size()=" << cluster.trksCurr.size() << endl
            << "   lepton: " << trkAsStr(tracklep));

  for(const xAOD::TrackParticle *trk: cluster.trksCurr) {
    ATH_MSG_DEBUG("   track:  " << trkAsStr(trk));
  }

  std::unique_ptr<xAOD::Vertex> secVtx = m_vertexFitterTool->fitVertexWithPrimarySeed(
    ctx, input, cluster.trksCurr, kDeepMergedVtx
  );

  if(!secVtx) {
    ATH_MSG_WARNING("fitVertexCluster - failed to fit vertex");
    return false;
  }

  if(passVertexSelection(secVtx.get())) {
    //
    // Obtained good vertex fit - stop iterations
    //
    cluster.vtxMerged = std::move(secVtx);

    return true;
  }

  return false;
}

//=============================================================================
double Prompt::VertexMergingTool::getMinNormDistVtx(
  const xAOD::Vertex *vtx1,
  const xAOD::Vertex *vtx2
) const
{
  double mini_normDist = 9999.0;

  if((!vtx1) || (!vtx2)) {
    return mini_normDist;
  }

  const Amg::Vector3D psvtx1 = vtx1->position();
  const Amg::Vector3D psvtx2 = vtx2->position();

  double svtx12_normDist1 = Prompt::getNormDist(psvtx1, psvtx2, vtx1->covariance(), msg(MSG::WARNING));
  double svtx12_normDist2 = Prompt::getNormDist(psvtx1, psvtx2, vtx2->covariance(), msg(MSG::WARNING));

  if (svtx12_normDist2 < svtx12_normDist1) { mini_normDist = svtx12_normDist2; }
  else                                     { mini_normDist = svtx12_normDist1; }

  return mini_normDist;
}

