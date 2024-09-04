/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetTrackSelectorTool_InDetCosmicTrackSelectorTool_H
#define InDetTrackSelectorTool_InDetCosmicTrackSelectorTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkParameters/TrackParameters.h"
// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"

namespace Trk
{
  class Vertex;
  class TrackParticleBase;
  class Track;
}


namespace InDet
{

  class InDetCosmicTrackSelectorTool : virtual public Trk::ITrackSelectorTool, public AthAlgTool
  {

  public:

    virtual StatusCode initialize() override;

    InDetCosmicTrackSelectorTool(const std::string& t, const std::string& n, const IInterface*  p);

    ~InDetCosmicTrackSelectorTool();

    virtual bool decision(const Trk::Track& track,const Trk::Vertex* vertex) const override;

    virtual bool decision(const Trk::TrackParticleBase& track,const Trk::Vertex* vertex) const override;

    virtual bool decision(const xAOD::TrackParticle&,const xAOD::Vertex*) const  override {
      ATH_MSG_WARNING("xAOD::TrackParticle selection not implemented yet");
      return false;
    }

  private:

    static int getNSiHits(const Trk::Track* track, bool top) ;
    bool decision(const Trk::TrackParameters* track, const Trk::Vertex* vertex, const Trk::ParticleHypothesis) const;
    DoubleProperty m_maxZ0{this, "maxZ0", 150., "Maximum z0 of tracks"};
    DoubleProperty m_maxD0{this, "maxD0", 2.5, "Maximum d0 of tracks"};
    DoubleProperty m_minPt{this, "minPt", 0., "Minimum pT of tracks"};
    IntegerProperty m_numberOfPixelHits
      {this, "numberOfPixelHits", 0, "Minimum number of Pixel hits"};
    IntegerProperty m_numberOfSCTHits
      {this, "numberOfSCTHits", 0, "Minimum number of SCT hits"};
    IntegerProperty m_numberOfTRTHits
      {this, "numberOfTRTHits", 15, "Minimum number of TRT hits"};
    IntegerProperty m_numberOfSiHits
      {this, "numberOfSiliconHits", 8, "Minimum number of Silicon hits"};
    IntegerProperty m_numberOfSiHitsTop{this, "numberOfSiliconHitsTop", -1};
    IntegerProperty m_numberOfSiHitsBottom{this, "numberOfSiliconHitsBottom", -1};

    ToolHandle<Trk::ITrackSummaryTool> m_trackSumTool{this, "TrackSummaryTool", ""};
    bool m_trackSumToolAvailable = false;

    // Read handle for conditions object to get the field cache
    SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};
    

  }; //end of class definitions
} //end of namespace definitions

#endif //TrkMultipleVertexSeedFinders_PVFindingTrackSelectoTool_H
