// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRACKINGCNV_ITRACKCOLLECTIONCNVTOOL_H
#define XAODTRACKINGCNV_ITRACKCOLLECTIONCNVTOOL_H

// Gaudi/Athena include(s):
#include "GaudiKernel/IAlgTool.h"

// EDM include(s):
#include "xAODTracking/TrackParticleContainer.h"
#include "TrkTrack/TrackCollection.h"
#include "xAODTracking/Vertex.h"
// Forward declaration(s):
//class TrackCollection; - no - typedef
#include "TrkValInterfaces/ITrkObserverTool.h"

namespace Trk {
  class ITrackParticleCreatorTool;
}

namespace xAODMaker {

  class ITrackCollectionCnvTool : public virtual IAlgTool {
    
  public:
    /// The interface provided by ITrackCollectionCnvTool
    DeclareInterfaceID(ITrackCollectionCnvTool, 1, 0);

    /// Function that fills an existing xAOD::TrackParticleContainer
    virtual StatusCode convert( const EventContext& ctx, const TrackCollection* aod,
				xAOD::TrackParticleContainer* xaod, const xAOD::Vertex* vtx = nullptr ) const = 0;

    /// Function that fills an existing xAOD::TrackParticleContainer and augments track particles
    virtual StatusCode convertAndAugment( const EventContext& ctx, const TrackCollection* aod,
				xAOD::TrackParticleContainer* xaod, const ObservedTrackMap* trk_map, const xAOD::Vertex* vtx = nullptr ) const = 0;

    virtual StatusCode setParticleCreatorTool(ToolHandle<Trk::ITrackParticleCreatorTool> *tool) = 0;

  };//class definition
    
} // xAODMaker namespace


#endif // XAODTRACKINGCNV_ITRACKCOLLECTIONCNVTOOL_H
