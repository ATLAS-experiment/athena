/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CSC_HitsTruthRelink.h"


CSC_HitsTruthRelink::CSC_HitsTruthRelink(const std::string &name, ISvcLocator *pSvcLocator)
  : HitsTruthRelinkBase(name, pSvcLocator)
{
}


StatusCode CSC_HitsTruthRelink::initialize()
{
  ATH_CHECK(HitsTruthRelinkBase::initialize());

  // Check and initialize keys
  ATH_CHECK( m_inputHitsKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_inputHitsKey);
  ATH_CHECK( m_outputHitsKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputHitsKey);
  return StatusCode::SUCCESS;
}


StatusCode CSC_HitsTruthRelink::execute(const EventContext &ctx) const
{
  ATH_MSG_DEBUG("Doing truth relinking");

  SG::ReadHandle<CSCSimHitCollection> inputCollection(m_inputHitsKey, ctx);
  if (!inputCollection.isValid()) {
    ATH_MSG_ERROR("Could not get input hits collection " << inputCollection.name() << " from store " << inputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found input hits collection " << inputCollection.name() << " in store " << inputCollection.store());

  SG::WriteHandle<CSCSimHitCollection> outputCollection(m_outputHitsKey, ctx);
  ATH_CHECK(outputCollection.record(std::make_unique<CSCSimHitCollection>()));
  if (!outputCollection.isValid()) {
    ATH_MSG_ERROR("Could not record output hits collection " << outputCollection.name() << " to store " << outputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Recorded output hits collection " << outputCollection.name() << " in store " << outputCollection.store());

  // Do relinking
  int referenceId{};
  ATH_CHECK(getReferenceId(ctx, &referenceId));

  for (const CSCSimHit &hit : *inputCollection) {
    HepMcParticleLink particleLink = updatedLink(ctx, hit.particleLink(), referenceId);
    int    id            = hit.CSCid();
    double time          = hit.globalTime();
    double energyDeposit = hit.energyDeposit();
    Amg::Vector3D start  = hit.getHitStart();
    Amg::Vector3D end    = hit.getHitEnd();
    int    pdgID         = hit.particleID();
    double kineticEnergy = hit.kineticEnergy();

    outputCollection->Emplace(id, time, energyDeposit, start, end, pdgID, particleLink, kineticEnergy);
  }

  return StatusCode::SUCCESS;
}
