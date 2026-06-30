/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CxxUtils/MurmurHash2.h"
#include "CxxUtils/crc64.h"
#include "CxxUtils/crc_combine.h"

  /// Set the random seed using a string (e.g. algorithm name) and
  /// the current slot, event, and run numbers and an optional
  /// offset. - MC16 Legacy Version attempting to reproduce seeds from
  /// thread-unsafe random number services
size_t ATHRNG::calculateSeedMC16(const std::string& algName, uint64_t ev, uint64_t run, uint32_t offset) {
  uint32_t theHash = static_cast<uint32_t>(ev);
  if (0 != offset) theHash = CxxUtils::crc_combine(theHash, offset);
  uint32_t runNumber = static_cast<uint32_t>(run);
  theHash = CxxUtils::crc_combine(theHash, runNumber);
  theHash = CxxUtils::crc_combine(theHash, algName);
  return theHash;
}

/// Set the random seed using a string (e.g. algorithm name) and the
/// current slot, event, and run numbers. MC20 seeding algorithm
size_t ATHRNG::calculateSeedMC20(const std::string& algName, uint64_t ev, uint64_t run) {
  auto algHash = std::hash<std::string>{}(algName);
  auto evHash = std::hash<uint64_t>{}(ev);
  auto runHash = std::hash<uint64_t>{}(run);
  auto hsh = evHash ^ (runHash + (evHash << 6) + (evHash >> 2));
  hsh = hsh ^ (algHash + (hsh << 6) + (hsh >> 2));
  return hsh;
}

  /// Set the random seed using a string (e.g. algorithm name) and the
  /// current slot, event, and run numbers. MC21 seeding algorithm
void ATHRNG::calculateSeedsMC21(long* seeds, const std::string& algName, uint64_t ev, uint64_t run, uint64_t offset) {
  //RanecuEngine only takes the first seed, so the first seed should be good on it's own
  //using 64bit crc which is already performing quite well on it's own
  uint64_t theHash = CxxUtils::crc64(algName);
  theHash = CxxUtils::crc64addint(theHash,ev);
  theHash = CxxUtils::crc64addint(theHash,offset);
  theHash = CxxUtils::crc64addint(theHash,run);

  size_t iseed=0;
  seeds[iseed]=(long)theHash; //CLHEP takes a zero terminated array for seeding. Avoid 0
  if(seeds[iseed]!=0) ++iseed; //if zero, make the seed vector shorter, which should in itself already cause a different seeding
  seeds[iseed]=(long)ev; //CLHEP takes a zero terminated array for seeding. Avoid ev==0
  if(seeds[iseed]!=0) ++iseed; //Avoid 0
  seeds[iseed]=(long)(ev >> 32); //explicitly split out the upper 32bit as dSFMTEngine only takes 32bits. 
  if(seeds[iseed]!=0) ++iseed; //Avoid 0
  seeds[iseed]=(long)run; 
  if(seeds[iseed]!=0) ++iseed; //Avoid 0
  uint64_t algHash = CxxUtils::MurmurHash64A ( algName.data(), algName.size(), 0 );
  seeds[iseed]=(long)algHash;
  if(seeds[iseed]!=0) ++iseed; //Avoid 0
  seeds[iseed]=(long)offset; 
  if(seeds[iseed]!=0) ++iseed; //Avoid 0
  seeds[iseed]=0; //CLHEP takes a zero terminated array for seeding, so end with 0
  return;
}

long ATHRNG::calculateSeedsPython(const std::string& algName, uint64_t ev, uint64_t run, uint64_t offset) {
  long seeds[7];
  calculateSeedsMC21(seeds, algName, ev, run, offset);
  return seeds[0];
}

ATHRNG::RNGWrapper::RNGWrapper(const factoryFunc& genFact, size_t nSlots)
{
  // Construct the random engines; one per event slot.
  m_engines.reserve(nSlots);
  for(size_t t = 0; t < nSlots; t++){
    m_engines.emplace_back(genFact());
  }
  m_evtSeeded.resize (nSlots, EventContext::INVALID_CONTEXT_EVT);
}

ATHRNG::RNGWrapper::~RNGWrapper()
{
  // Clean up the allocated engines
  for(auto engPtr : m_engines) {
    delete engPtr;
  }
}

void ATHRNG::RNGWrapper::setSeedMC20(const std::string& algName, size_t slot,
                                     uint64_t ev, uint64_t run,
                                     EventContext::ContextEvt_t evt /*= EventContext::INVALID_CONTEXT_EVT*/)
{
  auto hsh = calculateSeedMC20(algName, ev, run);
  setSeed(slot, hsh, evt);
}

void ATHRNG::RNGWrapper::setSeedMC21(const std::string& algName, size_t slot,
                                     uint64_t ev, uint64_t run, uint64_t offset,
                                     EventContext::ContextEvt_t evt /*= EventContext::INVALID_CONTEXT_EVT*/)
{
  long seeds[7]; //CLHEP uses long for seeding
  calculateSeedsMC21(seeds, algName, ev, run, offset);
  setSeeds(slot, seeds, evt);
}

void ATHRNG::RNGWrapper::setSeedLegacy(const std::string& algName, const EventContext& ctx,
                                       uint32_t offset, SeedingOptionType seeding,
                                       EventContext::ContextEvt_t evt /*= EventContext::INVALID_CONTEXT_EVT*/)
{
  if(seeding==MC16Seeding) {
    // Use MC16 legacy seeding
    setSeedMC16(algName, ctx.slot(), ctx.eventID().event_number(), ctx.eventID().run_number(), offset, evt);
    return;
  }
  if(seeding==MC20Seeding) {
    // Use MC20 seeding
    setSeedMC20(algName, ctx.slot(), ctx.eventID().event_number(), ctx.eventID().run_number(), evt);
    return;
  }
  // Use MC21 seeding
  setSeedMC21(algName, ctx.slot(), ctx.eventID().event_number(), ctx.eventID().run_number(), offset, evt);
}

void ATHRNG::RNGWrapper::setSeedMC16(const std::string& algName, size_t slot, uint64_t ev, uint64_t run, uint32_t offset,
                                     EventContext::ContextEvt_t evt /*= EventContext::INVALID_CONTEXT_EVT*/)
{
  uint32_t theHash = calculateSeedMC16(algName, ev, run, offset);
  setSeed(slot, theHash, evt);
}

void ATHRNG::RNGWrapper::setSeed(size_t slot, size_t seed,
                                 EventContext::ContextEvt_t evt /*= EventContext::INVALID_CONTEXT_EVT*/)
{
  m_engines[slot]->setSeed(seed, 0);
  if (evt == EventContext::INVALID_CONTEXT_EVT) {
    evt = Gaudi::Hive::currentContext().evt();
  }
  m_evtSeeded[slot] = evt;
}

void ATHRNG::RNGWrapper::setSeeds(size_t slot, const long * seeds,
                                 EventContext::ContextEvt_t evt /*= EventContext::INVALID_CONTEXT_EVT*/)
{
  m_engines[slot]->setSeeds(seeds, 0);
  if (evt == EventContext::INVALID_CONTEXT_EVT) {
    evt = Gaudi::Hive::currentContext().evt();
  }
  m_evtSeeded[slot] = evt;
}
