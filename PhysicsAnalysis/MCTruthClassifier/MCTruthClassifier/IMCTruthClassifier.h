/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MCTRUTHCLASSIFIER_IMCTRUTHCLASSIFIER_H
#define MCTRUTHCLASSIFIER_IMCTRUTHCLASSIFIER_H

/** @class IMCTruthclassifier
    Interface for the PhysicsAnalysis/MCTruthClassifier/MCTruthclassifier
    @author Frederic Derue derue@lpnhe.in2p3.fr
    CREATED : 01/09/2008
*/

#include "AsgTools/CurrentContext.h"
#include "AsgTools/IAsgTool.h"
#include "MCTruthClassifier/MCTruthClassifierDefs.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/TruthClassifiers.h"

#include <memory>
#include <unordered_map>
#include <vector>

#ifndef GENERATIONBASE
#include "xAODCaloEvent/CaloClusterFwd.h"
#include "xAODEgamma/ElectronFwd.h"
#include "xAODEgamma/PhotonFwd.h"
#include "xAODJet/Jet.h"
#include "xAODMuon/Muon.h"
#include "xAODTracking/TrackParticleFwd.h"
#endif

#ifndef XAOD_ANALYSIS
#include <AtlasHepMC/GenParticle.h>
class HepMcParticleLink;
#endif

#if !defined(XAOD_ANALYSIS) &&  !defined(GENERATIONBASE)
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#endif

namespace MCTruthPartClassifier {
  // Additional information that can be returned by the classifier.
  // Originally, these were all held in member variables in the classifier,
  // but that prevents the classifier methods from being made const.
  // Not all of these are filled by all classifier calls; it would probably
  // be better to split this up into a hierarchy of structures.
  class Info
  {
  public:
    Info() : eventContext(Gaudi::Hive::currentContext()) {}
    Info(const EventContext& ctx) : eventContext(ctx) {}

    ~Info() = default;

    const EventContext& eventContext;
    const xAOD::TruthParticle* genPart = nullptr;

    MCTruthPartClassifier::ParticleOutCome particleOutCome = MCTruthPartClassifier::UnknownOutCome;

    const xAOD::TruthParticle* mother = nullptr;
    const xAOD::TruthParticle* Mother() const { return mother;}
    inline void setMotherProperties(const xAOD::TruthParticle* from) {
      mother = from;
      if (!from) return;
    }
    inline void resetMotherProperties() { mother = nullptr;  }

    const xAOD::TruthParticle* PhotonMother() const { return photonMother;}
    const xAOD::TruthParticle* photonMother = nullptr;

#ifndef GENERATIONBASE /*Disable when no reconstruction packages are expected*/
    float deltaRMatch = -999;
    float deltaPhi = -999;
    float probTrkToTruth = 0;
    uint8_t numOfSiHits = 0;

    std::vector<const xAOD::TruthParticle*> egPartPtr;
    std::vector<float> egPartdR;
    std::vector<std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>> egPartClas;

    std::vector<const xAOD::TrackParticle*> cnvPhotTrkPtr;
    std::vector<const xAOD::TruthParticle*> cnvPhotTrkToTruthPart;
    std::vector<MCTruthPartClassifier::ParticleType> cnvPhotPartType;
    std::vector<MCTruthPartClassifier::ParticleOrigin> cnvPhotPartOrig;
#endif
  };

}

class IMCTruthClassifier : virtual public asg::IAsgTool
{
  ASG_TOOL_INTERFACE(IMCTruthClassifier)
public:
  /** Virtual destructor */
  virtual ~IMCTruthClassifier() = default;

  // Methods for Reco object to Truth association
#ifndef GENERATIONBASE
  // Method for Track to Truth association
  virtual const xAOD::TruthParticle* getGenPart(
      const xAOD::TrackParticle*,
      MCTruthPartClassifier::Info* info = nullptr) const = 0;
#ifndef XAOD_ANALYSIS
  // Method for egamma clusters to Truth Particle association
  virtual const xAOD::TruthParticle* egammaClusMatch(
      const xAOD::CaloCluster*, bool,
      MCTruthPartClassifier::Info* info = nullptr) const = 0;
#endif
#endif

// Methods for Truth Particle classification
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::TruthParticle*, MCTruthPartClassifier::Info* info = nullptr) const = 0;

#ifndef XAOD_ANALYSIS /*These can not run in Analysis Base*/
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleHepMCTruthClassifier(const HepMcParticleLink& theLink,MCTruthPartClassifier::Info* info = nullptr) const = 0;
#endif

#ifndef GENERATIONBASE
  // Methods for Reco Particle classification
  // Rely on the Reco to Truth association and then the Truth classification
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::TrackParticle*, MCTruthPartClassifier::Info* info = nullptr) const = 0;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Electron*, MCTruthPartClassifier::Info* info = nullptr) const = 0;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Photon*, MCTruthPartClassifier::Info* info = nullptr) const = 0;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Muon*, MCTruthPartClassifier::Info* info = nullptr) const = 0;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::CaloCluster*, MCTruthPartClassifier::Info* info = nullptr) const = 0;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Jet*, bool DR, MCTruthPartClassifier::Info* info = nullptr) const = 0;

#endif
};

#endif // MCTRUTHCLASSIFIER_IMCTRUTHCLASSIFIER_H
