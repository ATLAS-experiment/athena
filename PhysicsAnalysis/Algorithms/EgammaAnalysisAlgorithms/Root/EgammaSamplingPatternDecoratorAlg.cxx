/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EgammaAnalysisAlgorithms/EgammaSamplingPatternDecoratorAlg.h>

#include <AsgDataHandles/WriteDecorHandle.h>

//
// method implementations
//

namespace CP
{

  StatusCode EgammaSamplingPatternDecoratorAlg ::
  initialize ()
  {
    ANA_CHECK (m_clusterContainer.initialize());
    ANA_CHECK (m_samplingPattern.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode EgammaSamplingPatternDecoratorAlg ::
  execute (const EventContext& ctx) const
  {
    auto clusters = makeHandle (m_clusterContainer, ctx);
    SG::WriteDecorHandle<xAOD::CaloClusterContainer,std::uint32_t> samplingPattern (m_samplingPattern, ctx);
    for (auto cluster : *clusters)
    {
      samplingPattern (*cluster) = cluster->samplingPattern();
    } 
    return StatusCode::SUCCESS;
  }
}
