/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HitsTruthRelinkBase.h"
#include "TruthUtils/MagicNumbers.h"

HitsTruthRelinkBase::HitsTruthRelinkBase(const std::string &name, ISvcLocator *pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}


StatusCode HitsTruthRelinkBase::initialize()
{
  // Check and initialize keys
  ATH_CHECK( m_inputTruthCollectionKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_inputTruthCollectionKey);
  return StatusCode::SUCCESS;
}


StatusCode HitsTruthRelinkBase::getReferenceBarcode(const EventContext &ctx, int *barcode) const
{
  SG::ReadHandle<McEventCollection> inputCollection(m_inputTruthCollectionKey, ctx);
  if (!inputCollection.isValid()) {
    ATH_MSG_ERROR("Could not get input truth collection " << inputCollection.name() << " from store " << inputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found input truth collection " << inputCollection.name() << " in store " << inputCollection.store());

  const HepMC::GenEvent *genEvt = *(inputCollection->begin());
#ifdef HEPMC3
  size_t nVertices = genEvt->vertices().size();
  if (nVertices == 0) {
    ATH_MSG_ERROR("Truth collection should have at least one vertex!");
    return StatusCode::FAILURE;
  }
  const HepMC::ConstGenVertexPtr& genVtx = genEvt->vertices().back();
  size_t nParticles = genVtx->particles_out().size();
  if (nParticles == 0) {
    ATH_MSG_ERROR("Truth vertex should have at least one particle!");
    return StatusCode::FAILURE;
  }
  *barcode = HepMC::barcode(genVtx->particles_out().front());
#else
  size_t nVertices = genEvt->vertices_size();
  if (nVertices == 0) {
    ATH_MSG_ERROR("Truth collection should have at least one vertex!");
    return StatusCode::FAILURE;
  }
  auto genVtx = *(genEvt->vertices_end());
  size_t nParticles = genVtx->particles_out_size();
  if (nParticles == 0) {
    ATH_MSG_ERROR("Truth vertex should have at least one particle!");
    return StatusCode::FAILURE;
  }
  *barcode = HepMC::barcode(*(genVtx->particles_out_const_begin()));
#endif

  ATH_MSG_DEBUG("Reference barcode: " << *barcode);

  return StatusCode::SUCCESS;

}


StatusCode HitsTruthRelinkBase::getReferenceId(const EventContext &ctx, int *id) const
{
  SG::ReadHandle<McEventCollection> inputCollection(m_inputTruthCollectionKey, ctx);
  if (!inputCollection.isValid()) {
    ATH_MSG_ERROR("Could not get input truth collection " << inputCollection.name() << " from store " << inputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found input truth collection " << inputCollection.name() << " in store " << inputCollection.store());
  int barcode{0};
  const HepMC::GenEvent *genEvt = *(inputCollection->begin());
#ifdef HEPMC3
  size_t nVertices = genEvt->vertices().size();
  if (nVertices == 0) {
    ATH_MSG_ERROR("Truth collection should have at least one vertex!");
    return StatusCode::FAILURE;
  }
  const HepMC::ConstGenVertexPtr& genVtx = genEvt->vertices().back();
  size_t nParticles = genVtx->particles_out().size();
  if (nParticles == 0) {
    ATH_MSG_ERROR("Truth vertex should have at least one particle!");
    return StatusCode::FAILURE;
  }
  *id = HepMC::uniqueID(genVtx->particles_out().front());
  barcode = HepMC::barcode(genVtx->particles_out().front());
#else
  size_t nVertices = genEvt->vertices_size();
  if (nVertices == 0) {
    ATH_MSG_ERROR("Truth collection should have at least one vertex!");
    return StatusCode::FAILURE;
  }
  auto genVtx = *(genEvt->vertices_end());
  size_t nParticles = genVtx->particles_out_size();
  if (nParticles == 0) {
    ATH_MSG_ERROR("Truth vertex should have at least one particle!");
    return StatusCode::FAILURE;
  }
  *id = HepMC::uniqueID(*(genVtx->particles_out_const_begin()));
  barcode = HepMC::barcode(*(genVtx->particles_out_const_begin()));
#endif

  ATH_MSG_DEBUG("Reference id: " << *id);
  ATH_MSG_DEBUG("Reference barcode: " << barcode);

  return StatusCode::SUCCESS;

}

HepMcParticleLink HitsTruthRelinkBase::updatedLink(const EventContext &ctx, const HepMcParticleLink& oldLink, int referenceId, int /*pdgId*/) const {
  ATH_MSG_DEBUG ("oldLink.id() = " << oldLink.id());
  int currentId{};
  // Hits previously linked to truth particles should now be linked to the reference truthParticle
  if (oldLink.id() != 0 || oldLink.barcode() !=0) { // FIXME barcode-based for now to work around reading in HepMcParticleLink_p2 based EDM
    currentId = referenceId;
  }
  // TODO test using the ConstGenParticlePtr directly in the HepMcParticleLink constructor
  return HepMcParticleLink(currentId, oldLink.eventIndex(), HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_ID, ctx);
}
