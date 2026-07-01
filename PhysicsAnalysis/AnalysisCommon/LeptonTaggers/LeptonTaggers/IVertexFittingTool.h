// This is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LEPTONTAGGERS_IVertexFittingTool_H
#define LEPTONTAGGERS_IVertexFittingTool_H

/**********************************************************************************
 * @Package: LeptonTaggers
 * @Class  : VertexFittingSvc
 * @Author : Fudong He
 * @Author : Rustem Ospanov
 *
 * @Brief  :
 *
 *  Fit the input ID tracks and produced the output vertex.
 *
 **********************************************************************************/

// Athena
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IService.h"
#include "AthenaBaseComps/AthService.h"

// xAOD
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/Vertex.h"

// C++
#include <string>
#include <vector>

namespace Prompt
{
  enum VtxType {
    kTwoTrackVtx                  = 1,
    kSimpleMergedVtx              = 2,
    kDeepMergedVtx                = 3,
    kIterativeFitVtx              = 4,
    kTwoTrackVtxWithoutLepton     = 5,
    kIterativeFitVtxWithoutLepton = 6,
    kRefittedPriVtx               = 7,
    kRefittedPriVtxWithoutLep     = 8
  };

  struct FittingInput
  {
    FittingInput() = default;

    FittingInput(const xAOD::TrackParticleContainer *inDetTracks_p,
     const xAOD::Vertex                 *priVtx_p,
     const xAOD::Vertex                 *refittedPriVtx_p):
      inDetTracks               (inDetTracks_p),
      priVtx                    (priVtx_p),
      refittedPriVtx            (refittedPriVtx_p){}

    const xAOD::TrackParticleContainer *inDetTracks{nullptr};
    const xAOD::Vertex                 *priVtx{nullptr};
    const xAOD::Vertex                 *refittedPriVtx{nullptr};
    const xAOD::Vertex                 *refittedPriVtxWithoutLep{nullptr};

  };

  //
  // Interface of Vertex Merging Tool
  //
  class IVertexFittingTool: public virtual IAlgTool
  {
  public:

    DeclareInterfaceID( Prompt::IVertexFittingTool, 1, 0 );

    virtual std::unique_ptr<xAOD::Vertex> fitVertexWithPrimarySeed(
      const EventContext& ctx,
      const FittingInput &input,
      const std::vector<const xAOD::TrackParticle* > &tracks,
      VtxType vtx
    ) = 0;

    virtual std::unique_ptr<xAOD::Vertex> fitVertexWithSeed(
      const EventContext& ctx,
      const FittingInput &input,
      const std::vector<const xAOD::TrackParticle* > &tracks,
      const Amg::Vector3D& seed,
      VtxType vtxType
    ) = 0;

    virtual bool isValidVertex(const xAOD::Vertex *vtx) const = 0;
  };
}

#endif
