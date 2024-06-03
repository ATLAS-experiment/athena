/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_TRACKFINDINGVALIDATIONALG_H
#define ACTSTRK_TRACKFINDINGVALIDATIONALG_H 1

#undef NDEBUG
// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "Gaudi/Property.h"

// Handle Keys
#include "StoreGate/ReadHandleKey.h"

#include "TrackTruthMatchingBaseAlg.h"
#include "ActsEvent/TrackToTruthParticleAssociation.h"

namespace ActsTrk
{
  class TrackFindingValidationAlg : public TrackTruthMatchingBaseAlg
  {
  public:
    using TrackTruthMatchingBaseAlg::TrackTruthMatchingBaseAlg;

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
     SG::ReadHandleKey<TrackToTruthParticleAssociation>  m_trackToTruth
        {this, "TrackToTruthAssociationMap","", "Association map from tracks to generator particles." };
  };

} // namespace

#endif
