/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////
// Oleg.Fedin@cern.ch, August 2010
//////////////////////////////////////////////////////////////////////////
#include "McEventCollectionFilter.h"
//
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/Flow.h"
#include "AtlasHepMC/Polarization.h"
//
#include "InDetSimEvent/SiHit.h"
#include "MuonSimEvent/TGCSimHit.h"
#include "MuonSimEvent/CSCSimHit.h"
#include "MuonSimEvent/sTGCSimHit.h"
#include "MuonSimEvent/MMSimHit.h"
// CLHEP
#include "GeoPrimitives/GeoPrimitives.h"

#include "TruthUtils/MagicNumbers.h" // for SUPPRESSED_PILEUP_BARCODE

McEventCollectionFilter::McEventCollectionFilter(const std::string &name, ISvcLocator *pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}


StatusCode McEventCollectionFilter::initialize()
{
  // Check and initialize keys
  ATH_CHECK( m_inputTruthCollectionKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_inputTruthCollectionKey);
  ATH_CHECK( m_outputTruthCollectionKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputTruthCollectionKey);

  ATH_CHECK( m_inputTRTHitsKey.initialize(!m_inputTRTHitsKey.empty() && m_keepElectronsLinkedToTRTHits) );
  if (m_keepElectronsLinkedToTRTHits) {
    ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_inputTRTHitsKey);
  } else {
    ATH_MSG_DEBUG("Not keeping electrons from TRT hits");
  }

  return StatusCode::SUCCESS;
}


StatusCode McEventCollectionFilter::execute(const EventContext &ctx) const
{
  ATH_MSG_DEBUG("Filtering McEventCollection...");

  SG::ReadHandle<McEventCollection> inputCollection(m_inputTruthCollectionKey, ctx);
  if (!inputCollection.isValid()) {
    ATH_MSG_ERROR("Could not get input truth collection " << inputCollection.name() << " from store " << inputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found input truth collection " << inputCollection.name() << " in store " << inputCollection.store());

  SG::WriteHandle<McEventCollection> outputCollection(m_outputTruthCollectionKey, ctx);
  ATH_CHECK(outputCollection.record(std::make_unique<McEventCollection>()));
  if (!outputCollection.isValid()) {
    ATH_MSG_ERROR("Could not record output truth collection " << outputCollection.name() << " to store " << outputCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Recorded output truth collection " << outputCollection.name() << " in store " << outputCollection.store());

  //.......Create new particle (geantino) to link  hits from pileup
  HepMC::GenParticlePtr genPart=HepMC::newGenParticlePtr();
  genPart->set_pdg_id(m_pileUpParticlePDGID); //Geantino
  genPart->set_status(1); //!< set decay status
#ifndef HEPMC3
  HepMC::suggest_barcode(genPart, HepMC::SUPPRESSED_PILEUP_BARCODE );
#endif
  HepMC::GenVertexPtr genVertex = HepMC::newGenVertexPtr();
  genVertex->add_particle_out(genPart);

  const HepMC::GenEvent* genEvt = *(inputCollection->begin());
  //......copy GenEvent to the new one and remove all vertex
  HepMC::GenEvent* evt = HepMC::copyemptyGenEvent(genEvt);
#ifdef HEPMC3
  for (const auto &oldbp : genEvt->beams()) {
    // Would be good to have a helper function here like:
    // GenParticlePtr copyGenParticleToNewGenEvent(ConstGenParticlePtr particleToBeCopied, GenEvent* newGenEvent);
    // This would copy GenParticle Attributes as well
    HepMC::GenParticlePtr bp=std::make_shared<HepMC::GenParticle>(oldbp->data());
    evt->add_beam_particle(bp);
    HepMC::suggest_barcode(bp, HepMC::barcode(oldbp) );
    // Possibly also add a call to genVertex->add_particle_in(bp); ?
  }
  if (genEvt->cross_section()) {
    auto cs = std::make_shared<HepMC3::GenCrossSection>(*genEvt->cross_section().get());
    evt->set_cross_section(std::move(cs));
  }
  // to set geantino vertex as a truth primary vertex
  HepMC::ConstGenVertexPtr hScatVx = genEvt->vertices().at(3-1);
  if (hScatVx != nullptr) {
    HepMC::FourVector pmvxpos=hScatVx->position();
    genVertex->set_position(pmvxpos);
    // to set geantino kinematic phi=eta=0, E=p=E_hard_scat

    if (hScatVx->particles_in().size()==2) {
      double sum = hScatVx->particles_in().at(0)->momentum().e()+hScatVx->particles_in().at(1)->momentum().e();
      genPart->set_momentum(HepMC::FourVector(sum,0,0,sum));
    }
  }
#else
  evt->set_beam_particles(genEvt->beam_particles());
  if (genEvt->cross_section()) {
    evt->set_cross_section(*genEvt->cross_section());
  }
  // to set geantino vertex as a truth primary vertex
  HepMC::ConstGenVertexPtr hScatVx = HepMC::barcode_to_vertex(genEvt,-3);
  if (hScatVx != nullptr) {
    HepMC::FourVector pmvxpos=hScatVx->position();
    genVertex->set_position(pmvxpos);
    // to set geantino kinematic phi=eta=0, E=p=E_hard_scat
    HepMC::GenVertex::particles_in_const_iterator itrp =hScatVx->particles_in_const_begin();
    if (hScatVx->particles_in_size()==2) {
      HepMC::FourVector mom1=(*itrp)->momentum();
      HepMC::FourVector mom2=(*(++itrp))->momentum();
      double sum = mom1.e()+mom2.e();
      genPart->set_momentum(HepMC::FourVector(sum,0,0,sum));
    }
  }
#endif


  // electrons from TRT hits
  if (m_keepElectronsLinkedToTRTHits) {
    SG::ReadHandle<TRTUncompressedHitCollection> inputCollectionH(m_inputTRTHitsKey, ctx);
    if (!inputCollectionH.isValid()) {
      ATH_MSG_ERROR("Could not get input hits collection " << inputCollectionH.name() << " from store " << inputCollectionH.store());
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Found input hits collection " << inputCollectionH.name() << " in store " << inputCollectionH.store());

    for (const TRTUncompressedHit &hit : *inputCollectionH) {
      const HepMcParticleLink& link = hit.particleLink();
      int pdgID = hit.GetParticleEncoding();
      if (std::abs(pdgID) != 11 || HepMC::barcode(link) == HepMC::UNDEFINED_ID) continue; // FIXME barcode-based
      HepMC::ConstGenParticlePtr particle = link.cptr();
      HepMC::ConstGenVertexPtr vx = particle->production_vertex();
      HepMC::GenParticlePtr newParticle = HepMC::newGenParticlePtr(particle->momentum(), particle->pdg_id(), particle->status());
#ifndef HEPMC3
      HepMC::suggest_barcode(newParticle, HepMC::barcode(link)); // HepMC2 still barcode-based
#endif
      const HepMC::FourVector &position = vx->position();
      HepMC::GenVertexPtr newVertex = HepMC::newGenVertexPtr(position);
      newVertex->add_particle_out(newParticle);
      evt->add_vertex(std::move(newVertex));
#ifdef HEPMC3
      HepMC::suggest_barcode(newParticle, HepMC::barcode(link)); // FIXME
#endif
    }
  }

  //.....add new vertex with geantino
  evt->add_vertex(std::move(genVertex));
#ifdef HEPMC3
  HepMC::suggest_barcode(genPart, HepMC::SUPPRESSED_PILEUP_BARCODE ); // FIXME
#endif
  int referenceBarcode = HepMC::barcode(genPart);
  ATH_MSG_DEBUG("Reference barcode: " << referenceBarcode);

  outputCollection->push_back(evt);

  return StatusCode::SUCCESS;
}
