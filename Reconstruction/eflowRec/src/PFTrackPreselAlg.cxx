/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PFTrackPreselAlg.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/ConstDataVector.h"
#include <memory>

PFTrackPreselAlg::PFTrackPreselAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}

PFTrackPreselAlg::~PFTrackPreselAlg() = default;

StatusCode PFTrackPreselAlg::initialize()
{
  ATH_CHECK( m_inputTracksKey.initialize() );
  ATH_CHECK( m_outputTracksKey.initialize() );

  m_outputDecorKey = m_inputTracksKey.key()+"."+m_outputDecorKey.key();
  ATH_CHECK( m_outputDecorKey.initialize() );

  ATH_CHECK( m_trackSelTool.retrieve() );

  return StatusCode::SUCCESS;
}

StatusCode PFTrackPreselAlg::execute(const EventContext &ctx) const
{
  auto input = SG::makeHandle(m_inputTracksKey, ctx);
  if (!input.isValid())
  {
    ATH_MSG_ERROR("Failed to retrieve " << m_inputTracksKey);
    return StatusCode::FAILURE;
  }

  SG::WriteDecorHandle<xAOD::TrackParticleContainer, char> decPass(m_outputDecorKey, ctx);

  auto output = std::make_unique<ConstDataVector<xAOD::TrackParticleContainer>>(SG::VIEW_ELEMENTS);
  for (const xAOD::TrackParticle* itrk : *input)
  {
    if (itrk->pt() > m_upperPtCut || !m_trackSelTool->accept(*itrk))
    {
      decPass(*itrk) = false;
      continue;
    }
    decPass(*itrk) = true;
    output->push_back(itrk);
  }
  auto outputHandle = SG::makeHandle(m_outputTracksKey, ctx);
  ATH_CHECK(outputHandle.put(std::move(output)) != nullptr);

  return StatusCode::SUCCESS;
}
