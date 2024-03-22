/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_HitsTruthRelink.h"


TRT_HitsTruthRelink::TRT_HitsTruthRelink(const std::string &name, ISvcLocator *pSvcLocator)
  : HitsTruthRelinkBase(name, pSvcLocator)
{
}


StatusCode TRT_HitsTruthRelink::initialize()
{
  ATH_CHECK(HitsTruthRelinkBase::initialize());

  // Check and initialize keys
  ATH_CHECK( m_inputHitsKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_inputHitsKey);
  ATH_CHECK( m_outputHitsKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputHitsKey);
  return StatusCode::SUCCESS;
}


StatusCode TRT_HitsTruthRelink::execute(const EventContext &ctx) const
{
  ATH_MSG_DEBUG("Doing truth relinking");

  SG::ReadHandle<TRTUncompressedHitCollection> inputCollection(m_inputHitsKey, ctx);
  if (!inputCollection.isValid()) {
    ATH_MSG_ERROR("Could not get input hits collection " << inputCollection.name() << " from store " << inputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found input hits collection " << inputCollection.name() << " in store " << inputCollection.store());

  SG::WriteHandle<TRTUncompressedHitCollection> outputCollection(m_outputHitsKey, ctx);
  ATH_CHECK(outputCollection.record(std::make_unique<TRTUncompressedHitCollection>()));
  if (!outputCollection.isValid()) {
    ATH_MSG_ERROR("Could not record output hits collection " << outputCollection.name() << " to store " << outputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Recorded output hits collection " << outputCollection.name() << " in store " << outputCollection.store());

  // Do relinking
  int referenceId{};
  ATH_CHECK(getReferenceId(ctx, &referenceId));

  for (const TRTUncompressedHit &hit : *inputCollection) {
    int pdgID = hit.GetParticleEncoding();
    HepMcParticleLink particleLink = updatedLink(ctx, hit.particleLink(), referenceId, pdgID);
    int   id            = hit.GetHitID();
    float kineticEnergy = hit.GetKineticEnergy();
    float energyDeposit = hit.GetEnergyDeposit();
    float preX          = hit.GetPreStepX();
    float preY          = hit.GetPreStepY();
    float preZ          = hit.GetPreStepZ();
    float postX         = hit.GetPostStepX();
    float postY         = hit.GetPostStepY() ;
    float postZ         = hit.GetPostStepZ();
    float time          = hit.GetGlobalTime();

    outputCollection->Emplace(id, particleLink, pdgID, kineticEnergy, energyDeposit, preX, preY, preZ, postX, postY, postZ, time);
  }

  return StatusCode::SUCCESS;
}


HepMcParticleLink TRT_HitsTruthRelink::updatedLink(const EventContext &ctx, const HepMcParticleLink& oldLink, int referenceId, int pdgId) const {
  ATH_MSG_DEBUG ("oldLink.id() = " << oldLink.id());
  int currentId = oldLink.id();
  // Hits previously linked to truth particles should now be linked to the reference truthParticle
  if (oldLink.id() != 0 || oldLink.barcode() !=0) { // FIXME barcode-based for now to work around reading in HepMcParticleLink_p2 based EDM
    // For the TRT truth electrons may optionally be kept
    if (!(m_keepElectronsLinkedToTRTHits && std::abs(pdgId) == 11)) {
      currentId = referenceId;
    }
  }

  return HepMcParticleLink(currentId, oldLink.eventIndex(), HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_ID, ctx);
}
