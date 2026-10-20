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
#include "StoreGate/WriteDecorHandleKeyArray.h"

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
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    /** @brief Hit-based truth matching from the Acts::Track -> truth particle  */
     SG::ReadHandleKeyArray<TrackToTruthParticleAssociation>  m_trackToTruth
        {this, "TrackToTruthAssociationMaps",{},
         "Association maps from tracks to generator particles for all Acts tracks linked from the given track particle." };

     SG::ReadHandleKey<xAOD::TrackParticleContainer>  m_trkParticleName
        {this,"TrackParticleContainerName", "InDetTrackParticles",""};

     /** @brief Muon tracks are matched to the truth particle container insert a bypass to bend the
      *         link to the desired truth particle container */
     SG::ReadHandleKey<xAOD::TruthParticleContainer> m_derivateTruthKey{this, "DerivateTruthKey", ""};

     enum FloatDecorations { kMatchingProbability, kHitPurity, kHitEfficiency, kNFloatDecorators};
     SG::WriteDecorHandleKeyArray<xAOD::TrackParticleContainer> m_floatDecor{this, "FloatDecorators", {}};

     enum IntDecorations {kTruthType, kTruthOrigin, kNIntDecorators};
     SG::WriteDecorHandleKeyArray<xAOD::TrackParticleContainer> m_intDecor{this, "IntDecorators", {}};
     SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_truthClassDecor
       {this, "TruthClassification", m_trkParticleName, "truthClassification"};

     ToolHandle<IMCTruthClassifier> m_truthClassifier
       {this, "MCTruthClassifier", "MCTruthClassifier/MCTruthClassifier"};

     SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_linkDecor
       {this, "LinkDecoration", m_trkParticleName, "truthParticleLink"};

  };

} // namespace

#endif
