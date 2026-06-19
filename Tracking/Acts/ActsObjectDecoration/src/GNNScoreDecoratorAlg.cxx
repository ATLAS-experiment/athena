/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GNNScoreDecoratorAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include <cstddef>

namespace ActsTrk {

	GNNScoreDecoratorAlg::GNNScoreDecoratorAlg(const std::string &name,
										 ISvcLocator *pSvcLocator)
		: AthReentrantAlgorithm(name, pSvcLocator)
	{}


	StatusCode GNNScoreDecoratorAlg::initialize()
	{
		ATH_MSG_DEBUG( "Initializing " << name() << " ..." );

		ATH_CHECK(m_trackParticlesKey.initialize());
		ATH_CHECK(m_edgeScoresKey.initialize());
		ATH_CHECK(m_decorEdgeScoresKey.initialize());

		return StatusCode::SUCCESS;
	}

	StatusCode GNNScoreDecoratorAlg::execute(const EventContext& ctx) const
	{
		ATH_MSG_DEBUG("Executing " << name() << " ...");

		ATH_MSG_DEBUG( "Retrieving TrackParticleContainer with key: " << m_trackParticlesKey.key() );
		SG::ReadHandle<xAOD::TrackParticleContainer> trackParticleHandle = SG::makeHandle( m_trackParticlesKey, ctx );
		ATH_CHECK(trackParticleHandle.isValid());
		const xAOD::TrackParticleContainer* trackParticles = trackParticleHandle.cptr();

		SG::ReadHandle<std::vector<std::vector<float>>> edgeScoresHandle = SG::makeHandle( m_edgeScoresKey, ctx );
		ATH_CHECK(edgeScoresHandle.isValid());
		const std::vector<std::vector<float>>* edgeScores = edgeScoresHandle.cptr();
		
		size_t nTracks = trackParticles->size();
		size_t nEdges = edgeScores->size();
		if (nTracks != nEdges) {
			ATH_MSG_ERROR("trackParticles ("<< nTracks <<") and edgeScores ("<< nEdges <<") containers do not have the same size." );
			return StatusCode::FAILURE;
		}

		// Edge scores decorator
		SG::WriteDecorHandle< xAOD::TrackParticleContainer, std::vector<float> > decor_edgeScores( m_decorEdgeScoresKey, ctx );

		int TrackCounter = -1;

		// Decorate tracks with scores from the GNN
		for (const xAOD::TrackParticle* trackParticle : *trackParticles) {
			TrackCounter++;
			const std::vector<float>& edges = edgeScores->at(TrackCounter);
			decor_edgeScores(*trackParticle) = edges;
		}

		return StatusCode::SUCCESS;
	}

}