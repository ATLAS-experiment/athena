// This is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PROMPT_IVERTEXMERGINGTOOL_H
#define PROMPT_IVERTEXMERGINGTOOL_H

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

// Athena
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// xAOD
#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticle.h"

namespace Prompt
{
  //======================================================================================================
  // Vertex cluster helper class
  //
  struct VtxCluster
  {
    VtxCluster():vtxMerged(nullptr) {}

    std::vector<xAOD::Vertex*> vtxsInit;
    std::vector<xAOD::Vertex *>                vtxsFittedBad;

    std::vector<const xAOD::TrackParticle *>   trksInit;
    std::vector<const xAOD::TrackParticle *>   trksCurr;

    std::unique_ptr<xAOD::Vertex> vtxMerged;
  };

  //======================================================================================================
  struct MergeResult
  {
    std::vector<std::unique_ptr<xAOD::Vertex>> vtxsNewMerged;

    std::vector<std::unique_ptr<xAOD::Vertex>> vtxsInitPassed;
    std::vector<std::unique_ptr<xAOD::Vertex>> vtxsInitPassedNotMerged;
  };

  struct MergeResultNotOwner
  {
    std::vector<std::unique_ptr<xAOD::Vertex>> vtxsNewMerged;

    std::vector<xAOD::Vertex*> vtxsInitPassed;
    std::vector<std::unique_ptr<xAOD::Vertex>> vtxsInitPassedNotMerged;
  };

  struct FittingInput;

  //======================================================================================================
  // Interface of Vertex Merging Tool
  //
  class IVertexMergingTool: public virtual IAlgTool
  {
  public:
    DeclareInterfaceID(Prompt::IVertexMergingTool, 1, 0);

    virtual MergeResultNotOwner mergeInitVertices(
      const EventContext& ctx,
      const FittingInput &input,
      const xAOD::TrackParticle *tracklep,
      std::vector<std::unique_ptr<xAOD::Vertex>> &initVtxs,
      const std::vector<const xAOD::TrackParticle *> &selectedTracks
    ) = 0;
  };
}

#endif
