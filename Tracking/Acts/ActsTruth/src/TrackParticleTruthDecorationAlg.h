/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "MCTruthClassifier/IMCTruthClassifier.h"

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
     std::vector<SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> >   m_floatDecor;

     enum IntDecorations {kTruthType, kTruthOrigin, kNIntDecorators};
     std::vector<SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> > m_intDecor;
     SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_truthClassDecor
       {this, "TruthClassification", m_trkParticleName, "truthClassification"};

     ToolHandle<IMCTruthClassifier> m_truthClassifier
       {this, "MCTruthClassifier", "MCTruthClassifier/MCTruthClassifier"};

     SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_linkDecor
       {this, "LinkDecoration", m_trkParticleName, "truthParticleLink"};

  };

} // namespace

#endif
