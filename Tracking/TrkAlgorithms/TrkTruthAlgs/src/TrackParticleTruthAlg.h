/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKPARTICLETRUTHALG_H
#define TRACKPARTICLETRUTHALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrkTruthData/TrackTruthCollection.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

class TrackParticleTruthAlg: public AthAlgorithm {
public:
  TrackParticleTruthAlg(const std::string &name,ISvcLocator *pSvcLocator);
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;
  
private:

  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_particlesLinkKey
    {this, "ParticleLinkKey", ""};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_particlesTypeKey
    {this, "ParticleTypeKey", ""};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_particlesOriginKey
    {this, "ParticleOriginKey", ""};
  StringProperty m_particleName
    {this, "TrackParticleName", "InDetTrackParticles",
     "TrackParticle input name"};
  SG::ReadHandleKey<xAODTruthParticleLinkVector>  m_truthParticleLinkVecKey
    {this, "xAODTruthLinkVector", "xAODTruthLinks",
     "link vector to map HepMC onto xAOD truth"};
  SG::ReadHandleKey<TrackTruthCollection> m_truthTracksKey
    {this, "TrackTruthName", "TrackTruthCollection",
     "Track(Particle)TruthCollection input name"};

  ToolHandle<IMCTruthClassifier> m_truthClassifier{this, "MCTruthClassifier",
    "MCTruthClassifier/MCTruthClassifier"};
};

#endif/*TRACKTRUTHSELECTOR_H*/
