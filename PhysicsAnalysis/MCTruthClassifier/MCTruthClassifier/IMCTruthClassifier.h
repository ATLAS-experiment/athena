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
#include <tuple>

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
  // Full result of a classification: type, origin, the truth particle that was
  // classified (for a reconstructed object, the one it was matched to) and its outcome.
  using Result = std::tuple<MCTruthPartClassifier::ParticleType,
                            MCTruthPartClassifier::ParticleOrigin,
                            const xAOD::TruthParticle*,
                            MCTruthPartClassifier::ParticleOutCome>;
public:
  /** Virtual destructor */
  virtual ~IMCTruthClassifier() = default;

// Methods for Truth Particle classification
protected:
  virtual Result particleTruthClassifier(const EventContext& ctx, const xAOD::TruthParticle*) const = 0;

#ifndef GENERATIONBASE
  // Methods for Reco Particle classification
  // Rely on the Reco to Truth association and then the Truth classification
  virtual Result particleTruthClassifier(const EventContext& ctx, const xAOD::TrackParticle*) const = 0;
  virtual Result particleTruthClassifier(const EventContext& ctx, const xAOD::Electron*) const = 0;
  virtual Result particleTruthClassifier(const EventContext& ctx, const xAOD::Photon*) const = 0;
  virtual Result particleTruthClassifier(const EventContext& ctx, const xAOD::Muon*) const = 0;
  virtual Result particleTruthClassifier(const EventContext& ctx, const xAOD::CaloCluster*) const = 0;
#endif
public:
  /// Returns more info than the particleTruthClassifier
  template <typename T> Result
  particleTruthClassifier_full(const T* p, std::optional<std::reference_wrapper<const EventContext>> ctx = std::nullopt) const {
    return particleTruthClassifier(ctx ? ctx->get() : Gaudi::Hive::currentContext(), p);
  }

  template <typename T> std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const T* p, std::optional<std::reference_wrapper<const EventContext>> ctx = std::nullopt) const
  {
    const Result r = particleTruthClassifier_full(p, ctx);
    return {std::get<0>(r), std::get<1>(r)};
  }
};

#endif // MCTRUTHCLASSIFIER_IMCTRUTHCLASSIFIER_H
