// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
#ifndef XAODTRACKINGCNV_TRACKCOLLECTIONCNVTOOL_H
#define XAODTRACKINGCNV_TRACKCOLLECTIONCNVTOOL_H

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// converting tool that will be invoked
#include "TrkToolInterfaces/ITrackParticleCreatorTool.h"

// Local include(s):
#include "xAODTrackingCnv/ITrackCollectionCnvTool.h"
#include "TrkValInterfaces/ITrkObserverTool.h"

// System include(s):
#include <memory>

namespace xAODMaker {

  class TrackCollectionCnvTool : public AthAlgTool,
			       public virtual ITrackCollectionCnvTool {

  public:
    /// Regular AlgTool constructor
    TrackCollectionCnvTool( const std::string& type, const std::string& name,
			  const IInterface* parent );
    
    /// Function initialising the tool
    virtual StatusCode initialize() override;
    
    /// Function that fills an existing xAOD::TrackParticleContainer
    virtual StatusCode convert( const EventContext& ctx, const TrackCollection* aod,
			       xAOD::TrackParticleContainer* xaod, const xAOD::Vertex* vtx = nullptr ) const override;
    
    /// Function that fills an existing xAOD::TrackParticleContainer and augments track particles
    virtual StatusCode convertAndAugment( const EventContext& ctx, const TrackCollection* aod,
				xAOD::TrackParticleContainer* xaod, const ObservedTrackMap* trk_map, const xAOD::Vertex* vtx = nullptr) const override;

    /// allow other algorithms to pass the tool in order to preserve initialisation
    virtual StatusCode setParticleCreatorTool(ToolHandle<Trk::ITrackParticleCreatorTool> *tool) override;

  private:
    ToolHandle<Trk::ITrackParticleCreatorTool> m_particleCreator;
    
    inline xAOD::TrackParticle* createParticle(const EventContext& ctx, xAOD::TrackParticleContainer& xaod, const TrackCollection& container, const Trk::Track& tp, const xAOD::Vertex* vtx) const;


  }; // class TrackCollectionCnvTool definition

} // xAODMaker namespace

#endif // XAODTRACKINGCNV_TRACKCOLLECTIONCNVTOOL_H
