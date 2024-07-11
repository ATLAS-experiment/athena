/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATOROBJECTSTPCNV_INITMCEVENTCOLLECTION_H
# define GENERATOROBJECTSTPCNV_INITMCEVENTCOLLECTION_H
/** @file initMcEventCollection.h
 * @brief  minimal gaudi initialization and record an McEventCollection in StoreGate
 *
 **/

#include <string>
#include <vector>
#include "AtlasHepMC/GenParticle.h"

#undef NDEBUG

class ISvcLocator;
#include "AtlasHepMC/GenEvent_fwd.h"

namespace Athena_test {
  /** @fn bool initMcEventCollection(ISvcLocator*& pSvcLoc, HepMC::GenParticle*& pGenParticle)
   *  @brief minimal gaudi initialization and record an McEventCollection in StoreGate
   *  @param pSvcLoc returns a pointer to the Gaudi ServiceLocator
   *  @param genPartVector returns a vector of pointers to GenParticle objects for use in HepMcParticleLink creation
   */
  bool initMcEventCollection(ISvcLocator*& pSvcLoc, std::vector<HepMC::GenParticlePtr>& genPartVector, const bool initGaudi=true);
  /** @fn HepMC::GenParticle* populateGenEvent(HepMC::GenEvent & ge)
   *  @brief fills a HepMC::GenEvent with some dummy GenParticle and GenVertex objects
   *  @param genPartVector returns a vector of pointers to GenParticle objects for use in HepMcParticleLink creation
   */
  void populateGenEvent(HepMC::GenEvent & ge, int pdgid1, int pdgid2, std::vector<HepMC::GenParticlePtr>& genPartVector);
  /** @fn HepMC::GenParticle* populateFilteredGenEvent(HepMC::GenEvent & ge)
   *  @brief fills a HepMC::GenEvent with suppressed pile-up truth information
   *  @param genPartVector returns a vector of pointers to GenParticle objects for use in HepMcParticleLink creation
   */
  void populateFilteredGenEvent(HepMC::GenEvent & ge, std::vector<HepMC::GenParticlePtr>& genPartVector);
}
#endif // GENERATOROBJECTSTPCNV_INITMCEVENTCOLLECTION_H
