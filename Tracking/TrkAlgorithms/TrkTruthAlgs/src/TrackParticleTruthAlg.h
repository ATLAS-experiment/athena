/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKTRUTHALGS_TRACKPARTICLETRUTHALG_H
#define TRKTRUTHALGS_TRACKPARTICLETRUTHALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrkTruthData/TrackTruthCollection.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

class TrackParticleTruthAlg: public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trkKey{this, "TrackParticleName", "InDetTrackParticles",
      "TrackParticle input name"};
  /// Decorations for input TrackParticleContainer
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_particlesLinkKey
    {this, "ParticleLinkKey", m_trkKey,"truthParticleLink"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_particlesTypeKey
    {this, "ParticleTypeKey" ,m_trkKey, "truthType"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_particlesOriginKey
    {this, "ParticleOriginKey", m_trkKey, "truthOrigin"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_particlesClassificationKey
    {this, "ParticleClassificationKey", m_trkKey, "truthClassification"};

  SG::ReadHandleKey<xAODTruthParticleLinkVector>  m_truthParticleLinkVecKey
    {this, "xAODTruthLinkVector", "xAODTruthLinks",
     "link vector to map HepMC onto xAOD truth"};
  SG::ReadHandleKey<TrackTruthCollection> m_truthTracksKey
    {this, "TrackTruthName", "TrackTruthCollection",
     "Track(Particle)TruthCollection input name"};

  ToolHandle<IMCTruthClassifier> m_truthClassifier{this, "MCTruthClassifier",
    "MCTruthClassifier/MCTruthClassifier"};
};

#endif/*TRKTRUTHALGS_TRACKPARTICLETRUTHALG_H*/
