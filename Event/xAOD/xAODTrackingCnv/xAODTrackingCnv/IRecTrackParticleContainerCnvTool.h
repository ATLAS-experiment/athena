// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRACKINGCNV_IRECTRACKPARTICLECONTAINERCNVTOOL_H
#define XAODTRACKINGCNV_IRECTRACKPARTICLECONTAINERCNVTOOL_H

// Gaudi/Athena include(s):
#include "GaudiKernel/IAlgTool.h"

// EDM include(s):
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/Vertex.h"
//#include "TrkTrack/TrackCollection.h"
#include "TrkValInterfaces/ITrkObserverTool.h"

// Forward declaration(s):
namespace Rec {
class TrackParticleContainer;
}

namespace Trk {
  class ITrackParticleCreatorTool;
}


namespace xAODMaker {

  class IRecTrackParticleContainerCnvTool : public virtual IAlgTool {
    
  public:
    /// The interface provided by IRecTrackParticleContainerCnvTool
    DeclareInterfaceID( IRecTrackParticleContainerCnvTool, 1, 0);

    /// Function that fills an existing xAOD::TrackParticleContainer
    virtual StatusCode convert( const EventContext& ctx, const Rec::TrackParticleContainer* aod,
				xAOD::TrackParticleContainer* xaod, const xAOD::Vertex* vtx = nullptr) const = 0;

    /// Function that fills an existing xAOD::TrackParticleContainer and augments track particles
    virtual StatusCode convertAndAugment( const EventContext& ctx, const Rec::TrackParticleContainer* aod,
				xAOD::TrackParticleContainer* xaod, const ObservedTrackMap* trk_map, const xAOD::Vertex* vtx = nullptr) const = 0;

    virtual StatusCode setParticleCreatorTool(ToolHandle<Trk::ITrackParticleCreatorTool> *tool) = 0;

  };//class definition
    
} // xAODMaker namespace


#endif // XAODTRACKINGCNV_IRECTRACKPARTICLECONTAINERCNVTOOL_H
