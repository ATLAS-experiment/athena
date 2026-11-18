/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "TruthUtils/TruthClasses.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/TruthClassifiers.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>

#ifndef GENERATIONBASE
#include "xAODCaloEvent/CaloClusterFwd.h"
#include "xAODEgamma/ElectronFwd.h"
#include "xAODEgamma/PhotonFwd.h"
#include "xAODMuon/Muon.h"
#include "xAODTracking/TrackParticleFwd.h"
#endif

class IMCTruthClassifier : virtual public asg::IAsgTool
{
  ASG_TOOL_INTERFACE(IMCTruthClassifier)
protected:
  // Additional information that can be returned by the classifier.
  // Originally, these were all held in member variables in the classifier,
  // but that prevents the classifier methods from being made const.
  class Info
  {
  public:
    Info() : eventContext(Gaudi::Hive::currentContext()) {}
    Info(const EventContext& ctx) : eventContext(ctx) {}

    ~Info() = default;

    const EventContext& eventContext;
    const xAOD::TruthParticle* genPart = nullptr;

    MCTruthPartClassifier::ParticleOutCome particleOutCome = MCTruthPartClassifier::UnknownOutCome;

  };
public:
  /** Virtual destructor */
  virtual ~IMCTruthClassifier() = default;

// Methods for Truth Particle classification
protected:
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::TruthParticle*, IMCTruthClassifier::Info* info) const = 0;

#ifndef GENERATIONBASE
  // Methods for Reco Particle classification
  // Rely on the Reco to Truth association and then the Truth classification
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::TrackParticle*, IMCTruthClassifier::Info* info) const = 0;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Electron*, IMCTruthClassifier::Info* info) const = 0;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Photon*, IMCTruthClassifier::Info* info) const = 0;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Muon*, IMCTruthClassifier::Info* info) const = 0;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::CaloCluster*, IMCTruthClassifier::Info* info) const = 0;

#endif
public:
  /// Returns more info than the particleTruthClassifier 
  template <typename T> std::tuple<MCTruthPartClassifier::ParticleType, 
                                   MCTruthPartClassifier::ParticleOrigin,
                                   const xAOD::TruthParticle*, 
                                   MCTruthPartClassifier::ParticleOutCome> 
  particleTruthClassifier_full(const T* p, std::optional<std::reference_wrapper<const EventContext>> ctx = std::nullopt)  const {
    IMCTruthClassifier::Info info = ctx ? IMCTruthClassifier::Info(ctx->get()) : IMCTruthClassifier::Info();
    auto classification = particleTruthClassifier(p, &info);
    return {classification.first,classification.second,info.genPart,info.particleOutCome};
  }

  template <typename T> std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>  
  particleTruthClassifier(const T* p, std::optional<std::reference_wrapper<const EventContext>> ctx = std::nullopt) const 
  {
	  if (!ctx) return particleTruthClassifier(p, nullptr);
	  IMCTruthClassifier::Info info(ctx->get());
	  return particleTruthClassifier(p, &info);
  }

};

#endif // MCTRUTHCLASSIFIER_IMCTRUTHCLASSIFIER_H
