/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSOBJECTDECORATION_GNN_SCORE_DECORATORALG_H
#define ACTSOBJECTDECORATION_GNN_SCORE_DECORATORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"

namespace ActsTrk {

	class GNNScoreDecoratorAlg
    : public AthReentrantAlgorithm {
  public:
    GNNScoreDecoratorAlg(const std::string &name,ISvcLocator *pSvcLocator);
    virtual ~GNNScoreDecoratorAlg() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
	private:
    // Input containers
		SG::ReadHandleKey< xAOD::TrackParticleContainer > m_trackParticlesKey {this, "TrackParticlesKey", "InDetTrackParticles", "Input xAOD::TrackParticles"};

    SG::ReadHandleKey<std::vector<std::vector<float>>> m_edgeScoresKey{this, "EdgeScoresKey", "GNNEdgeScores", "Temporary container for GNN edge scores"};
    
		// GNN edge scores
		SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decorEdgeScoresKey {this, "DecorEdgeScoresKey", m_trackParticlesKey, "edgeScores"};
    
	};
}

#endif