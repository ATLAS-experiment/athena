// This is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VERTEXMERGINGTOOL_H
#define VERTEXMERGINGTOOL_H

/**********************************************************************************
 * @Package: LeptonTaggers
 * @Class  : VertexMergingTool
 * @Author : Fudong He
 * @Author : Rustem Ospanov
 *
 * @Brief  :
 *
 *  Merge the input vertices and output merged vertices.
 *
 **********************************************************************************/

// Local
#include "IVertexFittingTool.h"
#include "IVertexMergingTool.h"

// Athena
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// xAOD
#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticle.h"

namespace Prompt
{
  //
  // Vertex Merging Tool
  //
  class VertexMergingTool: public AthAlgTool, virtual public IVertexMergingTool
  {
  public:

    VertexMergingTool(const std::string &name,
                      const std::string &type,
                      const IInterface  *parent);

    virtual StatusCode initialize() override;

    virtual MergeResultNotOwner mergeInitVertices(
      const EventContext& ctx,
      const FittingInput &input,
      const xAOD::TrackParticle *tracklep,
      std::vector<std::unique_ptr<xAOD::Vertex>> &init_vtxs,
      const std::vector<const xAOD::TrackParticle *> &selected_tracks
    ) override;

  private:

    bool passVertexSelection(const xAOD::Vertex *vtx) const;

    bool makeClusters(
      std::vector<std::unique_ptr<VtxCluster>> &clusters,
      std::vector<xAOD::Vertex*> &init_vtxs
    );

    bool matchVtxToCluster(const VtxCluster &cluster, const xAOD::Vertex *vtx) const;

    bool addInitVtxToCluster(
      VtxCluster &cluster, xAOD::Vertex* vtx
    ) const;

    bool fitVertexCluster(
      const EventContext& ctx,
      const FittingInput &input,
      const xAOD::TrackParticle *tracklep,
      VtxCluster &cluster
    );

    double getMinNormDistVtx(const xAOD::Vertex *vtx1, const xAOD::Vertex *vtx2) const;

    //
    // Properties:
    //
    ServiceHandle<Prompt::IVertexFittingTool> m_vertexFitterTool {
      this, "VertexFittingTool", "Prompt::VertexFittingSvc/PromptVertexFittingSvc"
    };

    Gaudi::Property<bool> m_useMinNormDist {this, "useMinNormDist", false};

    Gaudi::Property<double> m_minFitProb {this, "minFitProb", 0.01};
    Gaudi::Property<double> m_minDistanceClusterVtx {this, "minDistanceClusterVtx", 1.00};
  };
}

#endif
