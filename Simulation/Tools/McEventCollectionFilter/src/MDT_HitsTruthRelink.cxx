/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MDT_HitsTruthRelink.h"


MDT_HitsTruthRelink::MDT_HitsTruthRelink(const std::string &name, ISvcLocator *pSvcLocator)
  : HitsTruthRelinkBase(name, pSvcLocator)
{
}


StatusCode MDT_HitsTruthRelink::initialize()
{
  ATH_CHECK(HitsTruthRelinkBase::initialize());

  // Check and initialize keys
  ATH_CHECK( m_inputHitsKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_inputHitsKey);
  ATH_CHECK( m_outputHitsKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputHitsKey);
  return StatusCode::SUCCESS;
}


StatusCode MDT_HitsTruthRelink::execute(const EventContext &ctx) const
{
  ATH_MSG_DEBUG("Doing truth relinking");

  SG::ReadHandle<MDTSimHitCollection> inputCollection(m_inputHitsKey, ctx);
  if (!inputCollection.isValid()) {
    ATH_MSG_ERROR("Could not get input hits collection " << inputCollection.name() << " from store " << inputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found input hits collection " << inputCollection.name() << " in store " << inputCollection.store());

  SG::WriteHandle<MDTSimHitCollection> outputCollection(m_outputHitsKey, ctx);
  ATH_CHECK(outputCollection.record(std::make_unique<MDTSimHitCollection>()));
  if (!outputCollection.isValid()) {
    ATH_MSG_ERROR("Could not record output hits collection " << outputCollection.name() << " to store " << outputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Recorded output hits collection " << outputCollection.name() << " in store " << outputCollection.store());

  // Do relinking
  int referenceId{};
  ATH_CHECK(getReferenceId(ctx, &referenceId));

  for (const MDTSimHit &hit : *inputCollection) {
    HepMcParticleLink particleLink = updatedLink(ctx, hit.particleLink(), referenceId);
    int    id              = hit.MDTid();
    double time            = hit.globalTime();
    double radius          = hit.driftRadius();
    Amg::Vector3D position = hit.localPosition();
    double stepLength      = hit.stepLength();
    double energyDeposit   = hit.energyDeposit();
    int    pdgID           = hit.particleEncoding();
    double kineticEnergy   = hit.kineticEnergy();

    outputCollection->Emplace(id, time, radius, position, particleLink, stepLength, energyDeposit, pdgID, kineticEnergy);
  }

  return StatusCode::SUCCESS;
}
