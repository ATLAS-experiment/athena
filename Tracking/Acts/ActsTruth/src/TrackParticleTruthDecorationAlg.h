/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_TRACKPARTICLETRUTHDECORATIONALG_H
#define ACTSTRK_TRACKPARTICLETRUTHDECORATIONALG_H 1

#undef NDEBUG
// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "Gaudi/Property.h"

// Handle Keys
#include "StoreGate/ReadHandleKey.h"

#include "TrackTruthMatchingBaseAlg.h"
#include "ActsEvent/TrackToTruthParticleAssociation.h"
#include "xAODTracking/TrackParticleContainer.h"

namespace ActsTrk
{
  class TrackParticleTruthDecorationAlg : public TrackTruthMatchingBaseAlg
  {
  public:
    using TrackTruthMatchingBaseAlg::TrackTruthMatchingBaseAlg;

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
     SG::ReadHandleKeyArray<TrackToTruthParticleAssociation>  m_trackToTruth
        {this, "TrackToTruthAssociationMaps",{},
         "Association maps from tracks to generator particles for all Acts tracks linked from the given track particle." };

     SG::ReadHandleKey<xAOD::TrackParticleContainer>  m_trkParticleName
        {this,"TrackParticleContainerName", "InDetTrackParticles",""};

     enum FloatDecorations { kMatchingProbability, kHitPurity, kHitEfficiency, kNFloatDecorators};
     std::vector<SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> >   m_linkDecor;
     std::vector<SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> >   m_floatDecor;
  };

} // namespace

#endif
